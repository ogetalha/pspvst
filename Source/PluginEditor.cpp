#include "PluginProcessor.h"
#include "PluginEditor.h"


ResponseCurveComponent::ResponseCurveComponent(AudioPluginAudioProcessor& p) : processorRef(p),
leftPathProducer(processorRef.leftChannelFifo),
rightPathProducer(processorRef.rightChannelFifo)
{
    for (auto* param : processorRef.getParameters())
        param->addListener(this);

    startTimerHz(60);
}

ResponseCurveComponent::~ResponseCurveComponent()
{
    for (auto* param : processorRef.getParameters())
        param->removeListener(this);
}

void ResponseCurveComponent::updateResponseCurve()
{
    using namespace juce;

    auto responseArea = getAnalysisArea();
    auto w = responseArea.getWidth();

    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();

    auto sampleRate = processorRef.getSampleRate();

    std::vector<double> mags(w);

    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (!lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        if (!monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (!highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
            if (!highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }

        mags[i] = Decibels::gainToDecibels(mag);
    }

    responseCurve.clear();

    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();

    auto mapY = [outputMin, outputMax](double input)
        {
            return jmap(input, -24.0, 24.0, outputMin, outputMax);
        };

    responseCurve.startNewSubPath(responseArea.getX(), mapY(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
        responseCurve.lineTo(responseArea.getX() + i, mapY(mags[i]));
}


void ResponseCurveComponent::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    drawBackgroundGrid(g);

    auto responseArea = getAnalysisArea();

    if (shouldShowFFTAnalysis)
    {
        auto leftChannelFFTPath = leftPathProducer.getPath();
        leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colour(97u, 18u, 167u)); //purple-
        g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

        auto rightChannelFFTPath = rightPathProducer.getPath();
        rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), responseArea.getY()));

        g.setColour(Colour(215u, 201u, 134u));
        g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
    }

    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));

    Path border;

    border.setUsingNonZeroWinding(false);

    border.addRoundedRectangle(getRenderArea(), 4);
    border.addRectangle(getLocalBounds());

    g.setColour(Colours::black);

    g.fillPath(border);

    drawTextLabels(g);

    g.setColour(Colours::orange);
    g.drawRoundedRectangle(getRenderArea().toFloat(), 4.f, 1.f);
}

std::vector<float> ResponseCurveComponent::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
            200, /*300, 400,*/ 500, 1000,
            2000, /*3000, 4000,*/ 5000, 10000,
            20000
    };
}

std::vector<float> ResponseCurveComponent::getGains()
{
    return std::vector<float>
    {
        -24, -12, 0, 12, 24
    };
}

std::vector<float> ResponseCurveComponent::getXs(const std::vector<float>& freqs, float left, float width)
{
    std::vector<float> xs;
    for (auto f : freqs)
    {
        auto normX = juce::mapFromLog10(f, 20.f, 20000.f);
        xs.push_back(left + width * normX);
    }

    return xs;
}

void ResponseCurveComponent::drawBackgroundGrid(juce::Graphics& g)
{
    using namespace juce;
    auto freqs = getFrequencies();

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    auto xs = getXs(freqs, left, width);

    g.setColour(Colours::dimgrey);
    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::darkgrey);
        g.drawHorizontalLine(y, left, right);
    }
}

void ResponseCurveComponent::drawTextLabels(juce::Graphics& g)
{
    using namespace juce;
    g.setColour(Colours::lightgrey);
    const int fontHeight = 10;
    g.setFont(fontHeight);

    auto renderArea = getAnalysisArea();
    auto left = renderArea.getX();

    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(1);

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));

        String str;
        if (gDb > 0)
            str << "+";
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(getWidth() - textWidth);
        r.setCentre(r.getCentreX(), y);

        g.setColour(gDb == 0.f ? Colour(0u, 172u, 1u) : Colours::lightgrey);

        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);

        str.clear();
        str << (gDb - 24.f);

        r.setX(1);
        textWidth = g.getCurrentFont().getStringWidth(str);
        r.setSize(textWidth, fontHeight);
        g.setColour(Colours::lightgrey);
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
    }
}


void ResponseCurveComponent::resized()
{
    using namespace juce;

    responseCurve.preallocateSpace(getWidth() * 3);
    updateResponseCurve();
}


void ResponseCurveComponent::parameterValueChanged(int, float)
{
    parametersChanged.set(true);
}

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, 0),
                                              monoBuffer.getReadPointer(0, size),
                                              monoBuffer.getNumSamples() - size);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                                              tempIncomingBuffer.getReadPointer(0, 0),
                                              size);

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, -48.f);
        }
    }

    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, -48.f);
        }
    }

    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}

void ResponseCurveComponent::timerCallback()
{
        if (shouldShowFFTAnalysis)
        {
            auto fftBounds = getAnalysisArea().toFloat();
            auto sampleRate = processorRef.getSampleRate();

            leftPathProducer.process(fftBounds, sampleRate);
            rightPathProducer.process(fftBounds, sampleRate);
        }

        if (parametersChanged.compareAndSetBool(false, true))
        {
            updateChain();
            updateResponseCurve();
        }


        repaint();
    }

void ResponseCurveComponent::updateChain()
{
    auto chainSettings = getChainSettings(processorRef.apvts);

    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);

    auto peakCoefficients = makePeakFilter(chainSettings, processorRef.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);

    auto lowCutCoefficients = makeLowCutFilter(chainSettings, processorRef.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, processorRef.getSampleRate());

    updateCutFilter(monoChain.get<ChainPositions::LowCut>(),
                    lowCutCoefficients,
                    chainSettings.lowCutSlope);

    updateCutFilter(monoChain.get<ChainPositions::HighCut>(),
                    highCutCoefficients,
                    chainSettings.highCutSlope);
}

juce::Rectangle<int> ResponseCurveComponent::getRenderArea()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}


juce::Rectangle<int> ResponseCurveComponent::getAnalysisArea()
{
    auto bounds = getRenderArea();
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p), responseCurveComponent(p)
{
    auto& apvts = processorRef.apvts;

    setKnob(lowCutSlider);
    setKnob(highCutSlider);
    setKnob(peakFreqSlider);
    setKnob(peakGainSlider);
    setKnob(peakQualitySlider);
    

    addAndMakeVisible(lowCutSlopeBox);
    addAndMakeVisible(highCutSlopeBox);
	addAndMakeVisible(responseCurveComponent);
    addAndMakeVisible(splashScreen);
    


    juce::StringArray slopeChoices{ "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" };
    for (int i = 0; i < slopeChoices.size(); ++i)
    {
        lowCutSlopeBox.addItem(slopeChoices[i], i + 1);
        highCutSlopeBox.addItem(slopeChoices[i], i + 1);
    }

    lowCutAttach = std::make_unique<SliderAttachment>(apvts, "LowCut Freq", lowCutSlider);
    highCutAttach = std::make_unique<SliderAttachment>(apvts, "HighCut Freq", highCutSlider);
    peakFreqAttach = std::make_unique<SliderAttachment>(apvts, "Peak Freq", peakFreqSlider);
    peakGainAttach = std::make_unique<SliderAttachment>(apvts, "Peak Gain", peakGainSlider);
    peakQualityAttach = std::make_unique<SliderAttachment>(apvts, "Peak Quality", peakQualitySlider);
    lowSlopeAttach = std::make_unique<ComboAttachment>(apvts, "LowCut Slope", lowCutSlopeBox);
    highSlopeAttach = std::make_unique<ComboAttachment>(apvts, "HighCut Slope", highCutSlopeBox);

    setLabel(lowCutLabel, "LowCut");
    setLabel(highCutLabel, "HighCut");
    setLabel(peakFreqLabel, "Peak Freq");
    setLabel(peakGainLabel, "Peak Gain");
    setLabel(peakQualityLabel, "Peak Q");

    addAndMakeVisible(lowCutLabel);
    addAndMakeVisible(highCutLabel);
    addAndMakeVisible(peakFreqLabel);
    addAndMakeVisible(peakGainLabel);   
    addAndMakeVisible(peakQualityLabel);

    splashScreen.toFront(true);

    setSize(1000, 600);

}



AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() = default;


void AudioPluginAudioProcessorEditor::setKnob(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white.withAlpha(0.85f));
    s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    addAndMakeVisible(s);
}

void AudioPluginAudioProcessorEditor::setLabel(juce::Label& l, const juce::String& text)
{
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::centred);
    l.setFont(juce::Font("FOT-NewRodin Pro M", 15.0f, juce::Font::plain));
    l.setColour(juce::Label::textColourId, juce::Colours::black);
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{

    juce::Colour top = juce::Colour::fromRGB(180, 240, 255);
    juce::Colour bottom = juce::Colour::fromRGB(210, 255, 170);
    g.setGradientFill(juce::ColourGradient(top, 0, 0, bottom, 0, (float)getHeight(), false));
    g.fillAll();

    juce::Rectangle<float> frameArea = getLocalBounds().toFloat().reduced(20, 10);
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(frameArea, 15.f);

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.setFont(juce::Font("PerSPire", 60.f, juce::Font::bold));
    g.drawFittedText("PSPVST", 0, 30, getWidth(), 70, juce::Justification::centredTop, 1);

    g.setFont(juce::Font("FOT-NewRodin Pro M", 18.f, juce::Font::plain));
    g.drawFittedText("Inspired by the PlayStation®Portable",
                     0, 95, getWidth(), 30, juce::Justification::centredTop, 1);

    g.setColour(juce::Colours::orange.withAlpha(0.8f));
    g.drawRoundedRectangle(frameArea, 20.f, 3.f);

    
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(50, 30);
    auto headerHeight = 110;
    area.removeFromTop(headerHeight);

    auto screenHeight = (int)(getHeight() * 0.33f);
    auto totalWidth = getWidth();
    auto screenWidth = (int)(totalWidth * 0.55f);
    auto screenX = (totalWidth - screenWidth) / 2;
    juce::Rectangle<int> screenRect(screenX, headerHeight + 20, screenWidth, screenHeight);

    responseCurveComponent.setBounds(screenRect);
    splashScreen.setBounds(getLocalBounds());


    int knobSize = 130;
    lowCutSlider.setBounds(screenRect.getX() - knobSize - 20,
                           screenRect.getCentreY() - knobSize / 2,
                           knobSize, knobSize);

    highCutSlider.setBounds(screenRect.getRight() + 20,
                            screenRect.getCentreY() - knobSize / 2,
                            knobSize, knobSize);

    lowCutLabel.setBounds(lowCutSlider.getX(), lowCutSlider.getBottom() + 5, knobSize, 25);
    highCutLabel.setBounds(highCutSlider.getX(), highCutSlider.getBottom() + 5, knobSize, 25);

    lowCutSlopeBox.setBounds(lowCutSlider.getX(), lowCutLabel.getBottom() + 5, knobSize, 25);
    highCutSlopeBox.setBounds(highCutSlider.getX(), highCutLabel.getBottom() + 5, knobSize, 25);

    auto peakArea = area.withY(screenRect.getBottom() + 60).withHeight(160);
    auto w = peakArea.getWidth() / 3;

    peakFreqSlider.setBounds(peakArea.removeFromLeft(w).reduced(9));
    peakGainSlider.setBounds(peakArea.removeFromLeft(w).reduced(9));
    peakQualitySlider.setBounds(peakArea.removeFromLeft(w).reduced(9));

    auto labelY = peakFreqSlider.getBottom() + 8;
    peakFreqLabel.setBounds(peakFreqSlider.getX(), labelY, peakFreqSlider.getWidth(), 25);
    peakGainLabel.setBounds(peakGainSlider.getX(), labelY, peakGainSlider.getWidth(), 25);
    peakQualityLabel.setBounds(peakQualitySlider.getX(), labelY, peakQualitySlider.getWidth(), 25);
}


