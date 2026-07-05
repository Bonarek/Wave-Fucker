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
class CartoonSynthAudioProcessor : public juce::AudioProcessor
{
public:

    float leftChannelLevel = 0.0f;
    float rightChannelLevel = 0.0f;

    // Filtry i Saturacja
    juce::dsp::ProcessorChain<juce::dsp::LadderFilter<float>> dspChain;
    juce::dsp::WaveShaper<float, std::function<float(float)>> softClipper;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    CartoonSynthAudioProcessor();
    ~CartoonSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    float visualBuffer[256] = { 0.0f }; // last 256 samples to draw a signal
    float fftMaginitude[512] = { 0.0f }; // wyniki FFT

private:
    //==============================================================================
    juce::MidiKeyboardState midiState;


    float Frequency = 440.0f;
    float currentFrequency = 440.0f;
    float Phase = 0.5f; 
    float lfoPhase = 0.0f;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParameters;

    juce::ADSR filterAdsr;
    juce::ADSR::Parameters filterAdrsParams;
    std::vector<int> activeNotes;


    juce::dsp::FFT fft{ 10 }; 
    juce::dsp::WindowingFunction<float> window{ 1024, juce::dsp::WindowingFunction<float>::blackmanHarris, true };
    float fftData[2048] = { 0.0f };
    int fftIndex = 0;

    float integrator = 0.0f;
    float integrator2 = 0.0f;
    const float R = 0.995f;

    float currentM = 0.0f;
    float currentN = 0.0f;

    float getBlit(float phase, float P, float M)
    {
        float denom = std::sin(juce::MathConstants<float>::pi * phase);
        if (std::abs(denom) < 0.001f) return (M / P);
        return std::sin(M * juce::MathConstants<float>::pi * phase) / (P * denom);
    }

    float getDsf(float phase, float N)
    {
        float theta = phase * juce::MathConstants<float>::twoPi;
        float den = 2.0f * std::sin(0.5f * theta);
        if (std::abs(den) < 0.001f) return N;
        return std::sin((N + 0.5f) * theta) / den - 0.5f;
    }

    float getBlep(float p, float dt)
    {
        if (p < dt)
        {
            p /= dt;
            return -p * p + 2.0f * p - 1.0f;
        }
        else if (p > 1.0f - dt)
        {
            p = (p - 1.0f) / dt;
            return p * p + 2.0f * p + 1.0f;
        }
        return 0.0f;
    }

    float getBlamp(float p, float dt)
    {
        if (p < dt) {
            p /= dt;
            return -p * p * p / 6.0f + p * p / 2.0f - p / 2.0f + 1.0f / 6.0f;
        }
        else if (p > 1.0f - dt) {
            p = (p - 1.0f) / dt;
            return p * p * p / 6.0f + 1.0f / 6.0f;
        }
        return 0.0f;
    }

    float dpwState = 0.0f;

    float dcBlockerState = 0.0f;
    float dcBlocker(float input)
    {
        float output = input - dcBlockerState;
        dcBlockerState = input - 0.99f * output;
        return output;
    }

    float dcBlockerState2 = 0.0f;
    float dcBlocker2(float input)
    {
        float output = input - dcBlockerState2;
        dcBlockerState2 = input - 0.99f * output;
        return output;
    }

    int scopeIndex = 0;
    int scopeStep = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CartoonSynthAudioProcessor)
};