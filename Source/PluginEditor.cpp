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

    setLabel(lowCutLabel, "LowCut Hz");
    setLabel(highCutLabel, "HighCut Hz");
    setLabel(peakFreqLabel, "Peak Freq");
    setLabel(peakGainLabel, "Peak Gain");
    setLabel(peakQualityLabel, "Peak Q");

    addAndMakeVisible(lowCutLabel);
    addAndMakeVisible(highCutLabel);
    addAndMakeVisible(peakFreqLabel);
    addAndMakeVisible(peakGainLabel);
    addAndMakeVisible(peakQualityLabel);

    setSize(800, 600);
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
    l.setFont(juce::Font("Eurostile", 15.0f, juce::Font::plain));
    l.setColour(juce::Label::textColourId, juce::Colours::black);
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::Colour top = juce::Colour::fromRGB(180, 240, 255);
    juce::Colour bottom = juce::Colour::fromRGB(210, 255, 170);
    g.setGradientFill(juce::ColourGradient(top, 0, 0, bottom, 0, (float)getHeight(), false));
    g.fillAll();

    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10), 20.f);

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.setFont(juce::Font("Eurostile", 60.f, juce::Font::bold));
    g.drawFittedText("PSPVST", 0, 40, getWidth(), 70, juce::Justification::centredTop, 1);

    g.setFont(juce::Font("Helvetica", 18.f, juce::Font::plain));
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

    auto labelRow = area.removeFromTop(40);
    auto lw = labelRow.getWidth() / 5;
    lowCutLabel.setBounds(labelRow.removeFromLeft(lw));
    peakFreqLabel.setBounds(labelRow.removeFromLeft(lw));
    peakGainLabel.setBounds(labelRow.removeFromLeft(lw));
    highCutLabel.setBounds(labelRow.removeFromLeft(lw));

    auto comboRow = area.removeFromTop(40);
    auto cw = comboRow.getWidth() / 5;
    lowCutSlopeBox.setBounds(comboRow.removeFromLeft(cw).reduced(10, 0));
    comboRow.removeFromLeft(cw * 3);
    highCutSlopeBox.setBounds(comboRow.removeFromLeft(cw).reduced(10, 0));
}
