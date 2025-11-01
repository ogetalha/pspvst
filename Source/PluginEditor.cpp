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

    // Populate slope boxes manually (JUCE attachments don't add item text)
    juce::StringArray slopeChoices{ "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" };
    for (int i = 0; i < slopeChoices.size(); ++i)
    {
        lowCutSlopeBox.addItem(slopeChoices[i], i + 1);
        highCutSlopeBox.addItem(slopeChoices[i], i + 1);
    }

    // Attach parameters
    lowCutAttach = std::make_unique<SliderAttachment>(apvts, "LowCut Freq", lowCutSlider);
    highCutAttach = std::make_unique<SliderAttachment>(apvts, "HighCut Freq", highCutSlider);
    peakFreqAttach = std::make_unique<SliderAttachment>(apvts, "Peak Freq", peakFreqSlider);
    peakGainAttach = std::make_unique<SliderAttachment>(apvts, "Peak Gain", peakGainSlider);
    peakQualityAttach = std::make_unique<SliderAttachment>(apvts, "Peak Quality", peakQualitySlider);
    lowSlopeAttach = std::make_unique<ComboAttachment>(apvts, "LowCut Slope", lowCutSlopeBox);
    highSlopeAttach = std::make_unique<ComboAttachment>(apvts, "HighCut Slope", highCutSlopeBox);

    setLabel(lowCutLabel, "LowCut\nFreq");
    setLabel(highCutLabel, "HighCut\nFreq");
    setLabel(peakFreqLabel, "Peak\nFreq");
    setLabel(peakGainLabel, "Peak\nGain");
    setLabel(peakQualityLabel, "Peak\nQ");


    addAndMakeVisible(lowCutLabel);
    addAndMakeVisible(highCutLabel);
    addAndMakeVisible(peakFreqLabel);
    addAndMakeVisible(peakGainLabel);
    addAndMakeVisible(peakQualityLabel);

    for (auto* param : processorRef.getParameters())
        param->addListener(this);

    setSize(800, 600);
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
    s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::white.withAlpha(0.8f));
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

    auto bounds = getLocalBounds();
	auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

	auto w = responseArea.getWidth();

	auto& lowcut = monoChain.get<ChainPositions::LowCut>();
	auto& peak = monoChain.get<ChainPositions::Peak>();
	auto& highcut = monoChain.get<ChainPositions::HighCut>();
    
	auto sampleRate = processorRef.getSampleRate();

	std::vector<double> mags;

    mags.resize(w);
    
    for (int i = 0; i < w; ++i)
        {
        double mag = 1.f;
        auto freq = juce::mapToLog10((double)i / (double)w, 20.0, 20000.0);

        if (!monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowcut.isBypassed<0>())
            mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<1>())
            mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<2>())
            mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowcut.isBypassed<3>())
            mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highcut.isBypassed<0>())
            mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<1>())
            mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<2>())
            mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highcut.isBypassed<3>())
            mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

	juce::Path responseCurve;

	const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();\
    auto map = [outputMax, outputMin](double input)
        {
            return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
	};

	responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for(size_t i = 1; i < mags.size(); ++i)
        {
            responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
	}



	g.setColour(juce::Colours::orange);
	g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.f, 1.f);

	g.setColour(juce::Colours::white);
	g.strokePath(responseCurve, juce::PathStrokeType(2.f));

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10), 20.f);

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.setFont(juce::Font("PerSPire", 60.f, juce::Font::bold));
    g.drawFittedText("PSPVST", 0, 40, getWidth(), 70, juce::Justification::centredTop, 1);

    g.setFont(juce::Font("FOT-NewRodin Pro M", 18.f, juce::Font::plain));
    g.drawFittedText("Inspired by the PlayStation®Portable",
                     0, 110, getWidth(), 30, juce::Justification::centredTop, 1);

}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(40);
    area.removeFromTop(150);

    auto rowHeight = 200;
    auto knobRow = area.removeFromTop(rowHeight);
    auto w = knobRow.getWidth() / 5;

    lowCutSlider.setBounds(knobRow.removeFromLeft(w).reduced(10));
    peakFreqSlider.setBounds(knobRow.removeFromLeft(w).reduced(10));
    peakGainSlider.setBounds(knobRow.removeFromLeft(w).reduced(10));
    peakQualitySlider.setBounds(knobRow.removeFromLeft(w).reduced(10));
    highCutSlider.setBounds(knobRow.removeFromLeft(w).reduced(10));

    auto labelRow = area.removeFromTop(50);
    auto lw = labelRow.getWidth() / 5;
    lowCutLabel.setBounds(labelRow.removeFromLeft(lw));
    peakFreqLabel.setBounds(labelRow.removeFromLeft(lw));
    peakGainLabel.setBounds(labelRow.removeFromLeft(lw));
    peakQualityLabel.setBounds(labelRow.removeFromLeft(lw));
    highCutLabel.setBounds(labelRow.removeFromLeft(lw));

    auto comboRow = area.removeFromTop(50);
    auto cw = comboRow.getWidth() / 5;
    lowCutSlopeBox.setBounds(comboRow.removeFromLeft(cw).reduced(10, 0));
    comboRow.removeFromLeft(cw * 3);
    highCutSlopeBox.setBounds(comboRow.removeFromLeft(cw).reduced(10, 0));
}

void AudioPluginAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
	
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
        repaint();
    }
}
