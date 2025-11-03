#pragma once
#include "PluginProcessor.h"

struct ResponseCurveComponent: juce::Component,
	juce::AudioProcessorParameter::Listener,
	juce::Timer
{
    ResponseCurveComponent(AudioPluginAudioProcessor&);
    ~ResponseCurveComponent() override;
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    void paint(juce::Graphics&) override;


    void timerCallback() override;

private:
    AudioPluginAudioProcessor& processorRef;
    juce::Atomic<bool> parametersChanged{ false };
	MonoChain monoChain;
};

class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
                                                    
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;

	ResponseCurveComponent responseCurveComponent;

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
