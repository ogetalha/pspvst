#include "PluginProcessor.h"
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto& apvts = processorRef.apvts;

    setKnob(lowCutSlider);
    setKnob(highCutSlider);
    setKnob(peakFreqSlider);
    setKnob(peakGainSlider);
    setKnob(peakQualitySlider);

    addAndMakeVisible(lowCutSlopeBox);
    addAndMakeVisible(highCutSlopeBox);

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

    for (auto* param : processorRef.getParameters())
        param->addListener(this);

    startTimerHz(60);
    setSize(900, 620);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    for (auto* param : processorRef.getParameters())
        param->removeListener(this);
}

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

    juce::Rectangle<float> frameArea = getLocalBounds().toFloat().reduced(30, 20);
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(frameArea, 15.f);

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.setFont(juce::Font("PerSPire", 60.f, juce::Font::bold));
    g.drawFittedText("PSPVST", 0, 30, getWidth(), 70, juce::Justification::centredTop, 1);

    g.setFont(juce::Font("FOT-NewRodin Pro M", 18.f, juce::Font::plain));
    g.drawFittedText("Inspired by the PlayStation®Portable",
                     0, 95, getWidth(), 30, juce::Justification::centredTop, 1);

    auto responseArea = juce::Rectangle<float>(
        (float)getWidth() * 0.225f,
        (float)getHeight() * 0.23f,
        (float)getWidth() * 0.55f,
        (float)getHeight() * 0.33f);

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(responseArea, 10.0f);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(responseArea, 10.0f, 1.5f);

    auto w = (int)responseArea.getWidth();
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    auto sampleRate = processorRef.getSampleRate();

    std::vector<double> mags(w, 1.0);
    for (int i = 0; i < w; ++i)
    {
        double mag = 1.f;
        auto freq = juce::mapToLog10((double)i / (double)w, 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowcut.isBypassed<0>()) mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>()) mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>()) mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>()) mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>()) mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>()) mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>()) mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>()) mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

    juce::Path responseCurve;
    const double outMin = responseArea.getBottom();
    const double outMax = responseArea.getY();
    auto map = [outMax, outMin](double in) { return juce::jmap(in, -24.0, 24.0, outMin, outMax); };

    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    for (size_t i = 1; i < mags.size(); ++i)
        responseCurve.lineTo(responseArea.getX() + (float)i, (float)map(mags[i]));

    g.setColour(juce::Colours::white);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));

    g.setColour(juce::Colours::orange.withAlpha(0.8f));
    g.drawRoundedRectangle(frameArea, 15.f, 2.f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(40);
    auto headerHeight = 130;
    area.removeFromTop(headerHeight);

    auto screenHeight = (int)(getHeight() * 0.33f);
    auto totalWidth = getWidth();
    auto screenWidth = (int)(totalWidth * 0.55f);
    auto screenX = (totalWidth - screenWidth) / 2;
    juce::Rectangle<int> screenRect(screenX, headerHeight + 20, screenWidth, screenHeight);

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

    peakFreqSlider.setBounds(peakArea.removeFromLeft(w).reduced(10));
    peakGainSlider.setBounds(peakArea.removeFromLeft(w).reduced(10));
    peakQualitySlider.setBounds(peakArea.removeFromLeft(w).reduced(10));

    auto labelY = peakFreqSlider.getBottom() + 8;
    peakFreqLabel.setBounds(peakFreqSlider.getX(), labelY, peakFreqSlider.getWidth(), 25);
    peakGainLabel.setBounds(peakGainSlider.getX(), labelY, peakGainSlider.getWidth(), 25);
    peakQualityLabel.setBounds(peakQualitySlider.getX(), labelY, peakQualitySlider.getWidth(), 25);
}

void AudioPluginAudioProcessorEditor::parameterValueChanged(int, float)
{
    parametersChanged.set(true);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        auto chainSettings = getChainSettings(processorRef.apvts);
        auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
            processorRef.getSampleRate(),
            chainSettings.peakFreq,
            chainSettings.peakQuality,
            juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels));

        updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
        repaint();
    }
}
