#pragma once
#include "PluginProcessor.h"

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, juce::AudioProcessorParameter::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }


	void timerCallback() override;



private:
    AudioPluginAudioProcessor& processorRef;

    juce::Atomic<bool> parametersChanged{ false };

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

    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
