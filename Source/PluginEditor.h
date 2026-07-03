/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class WaveFuckerAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    WaveFuckerAudioProcessorEditor (WaveFuckerAudioProcessor&);
    ~WaveFuckerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override { repaint(); }
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    WaveFuckerAudioProcessor& audioProcessor;

    juce::ToggleButton sawButton{ "Sawtooth" };
    juce::ToggleButton triButton{ "Triangle" };
    juce::ToggleButton sqrButton{ "Square" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sawAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> triAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sqrAttachment;

    juce::ToggleButton naiveButton{ "Naive" };
    juce::ToggleButton blitButton{ "Blit" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> naiveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> blitAttachment;
    
    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;

    juce::Slider resSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;


    juce::Slider lfoFreqSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoFreqAttachment;

    juce::Slider lfoDepthSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attAttach, decAttach, susAttach, relAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveFuckerAudioProcessorEditor)
};
