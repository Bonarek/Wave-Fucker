/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>


//==============================================================================
/**
*/
class WaveFuckerAudioProcessor  : public juce::AudioProcessor
{
public:
    //Cutoff
    juce::dsp::ProcessorChain<juce::dsp::LadderFilter<float>> dspChain;

    
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    WaveFuckerAudioProcessor();
    ~WaveFuckerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

        float visualBuffer[256] = { 0.0f }; // last 256 samples to draw a signal
        float fftMaginitude[512] = { 0.0f }; //wyniki
private:
    //==============================================================================
        //check for midi notes 
    juce::MidiKeyboardState midiState;

    //param for synth   

    float Frequency = 440.0f;
    float Phase = 0.0f;

    float lfoPhase = 0.0f;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;
    std::vector<int> activeNotes;
    // FFT
    juce::dsp::FFT fft{ 10 }; //window size 2^10 = 1024
    juce::dsp::WindowingFunction<float> window{ 1024, juce::dsp::WindowingFunction<float>::blackmanHarris, true };
    float fftData[2048] = { 0.0f }; //time buffor
    
    int fftIndex = 0;



    float integrator = 0.0f;
    float integrator2 = 0.0f;
    const float R = 0.995f;

    float getBlit(float phase, float P, float M)
    {
        float denom = sinf(juce::MathConstants<float>::pi * phase);
        if (fabsf(denom) < 1e-7f) return (M / P);
        return (sinf(M * juce::MathConstants<float>::pi * phase)) / (P * denom);
    }
    //DC block
    float dcBlockerState = 0.0f;
    float dcBlocker(float input)
    {
        float output = input - dcBlockerState;
        dcBlockerState = input - 0.9995f * output; 
        return output;
    }

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFuckerAudioProcessor)
};
