#pragma once
#include "PluginProcessor.h"

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    juce::Slider lowCutSlider, highCutSlider, peakFreqSlider,
        peakGainSlider, peakQualitySlider;
    juce::ComboBox lowCutSlopeBox, highCutSlopeBox;

    std::unique_ptr<SliderAttachment> lowCutAttach, highCutAttach,
        peakFreqAttach, peakGainAttach, peakQualityAttach;
    std::unique_ptr<ComboAttachment> lowSlopeAttach, highSlopeAttach;

    juce::Label lowCutLabel, highCutLabel,
        peakFreqLabel, peakGainLabel, peakQualityLabel;

    void setKnob(juce::Slider&);
    void setLabel(juce::Label&, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
