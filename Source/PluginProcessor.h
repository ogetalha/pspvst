#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

enum class Slope
{
    slope12dBPerOctave,
    slope24dBPerOctave, 
    slope36dBPerOctave,
    slope48dBPerOctave
};

struct ChainSettings {
    float lowCutFreq{ 20.f };
    float highCutFreq{ 20000.f };
    float peakFreq{ 750.f };
    float peakGainInDecibels{ 0.0f };
    float peakQuality{ 1.0f };
    
    Slope lowCutSlope{ Slope::slope12dBPerOctave };
    Slope highCutSlope{ Slope::slope12dBPerOctave };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout
		createParameterLayout();


	juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "PARAMETERS", createParameterLayout() };

private:
    //==============================================================================
	using Filter = juce::dsp::IIR::Filter<float>;

	using CutFilter = juce::dsp::ProcessorChain<
                                                Filter, 
                                                Filter, 
                                                Filter, 
                                                Filter
    >;

    using MonoChain = juce::dsp::ProcessorChain<
                                                CutFilter, // LowCut
                                                Filter,    // Peak
		                                        CutFilter  // HighCut
	>;

	MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
	};

	void updatePeakFilter(const ChainSettings& chainSettings);
	using Coefficients = Filter::CoefficientsPtr;
    static void updateCoefficients(Coefficients& old, const Coefficients& replacements);

	template<typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut, const CoefficientType& cutCoefficients, const Slope& lowCutSlope)
    {

        leftLowCut.template setBypassed<0>(true);
        leftLowCut.template setBypassed<1>(true);
        leftLowCut.template setBypassed<2>(true);
        leftLowCut.template setBypassed<3>(true);

        switch (lowCutSlope)
        {
            case Slope::slope12dBPerOctave:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                leftLowCut.template setBypassed<0>(false);
                break;
            }

            case Slope::slope24dBPerOctave:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                leftLowCut.template setBypassed<0>(false);
                leftLowCut.template setBypassed<1>(false);
                break;
            }
            case Slope::slope36dBPerOctave:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                *leftLowCut.template get<2>().coefficients = *cutCoefficients[2];
                leftLowCut.template setBypassed<0>(false);
                leftLowCut.template setBypassed<1>(false);
                leftLowCut.template setBypassed<2>(false);
                break;
            }

            case Slope::slope48dBPerOctave:
            {
                *leftLowCut.template get<0>().coefficients = *cutCoefficients[0];
                *leftLowCut.template get<1>().coefficients = *cutCoefficients[1];
                *leftLowCut.template get<2>().coefficients = *cutCoefficients[2];
                *leftLowCut.template get<3>().coefficients = *cutCoefficients[3];
                leftLowCut.template setBypassed<0>(false);
                leftLowCut.template setBypassed<1>(false);
                leftLowCut.template setBypassed<2>(false);
                leftLowCut.template setBypassed<3>(false);
                break;
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
