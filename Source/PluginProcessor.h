/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>


//==============================================================================
/**
*/
class WaveFuckerAudioProcessor  : public juce::AudioProcessor
{
public:

    //wave user choice
    juce::AudioProcessorValueTreeState& getWaveAPVTS() { return apvtsWave; };
    juce::AudioProcessorValueTreeState& getMethocAVPTS() { return apvtsMethod; };
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
    float SampleRate = 48000.0f;
    float Frequency = 440.0f;
    float Phase = 0.0f;
    int activeNumNote = 0;
    juce::AudioProcessorValueTreeState apvtsWave;
    juce::AudioProcessorValueTreeState apvtsMethod;
    
    bool isPlaying = false;
    bool notesActive[128] = { false };
    // FFT
    juce::dsp::FFT fft{ 10 }; //window size 2^10 = 1024
    juce::dsp::WindowingFunction<float> window{ 1024, juce::dsp::WindowingFunction<float>::blackmanHarris, true };
    float fftData[2048] = { 0.0f }; //time buffor
    
    int fftIndex = 0;
    bool nextFFTReady = false;



JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFuckerAudioProcessor)
};
