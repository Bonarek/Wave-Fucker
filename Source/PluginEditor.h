/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PeakMeter.h" 


class CartoonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CartoonLookAndFeel()
    {
        knobImage = juce::ImageCache::getFromMemory(BinaryData::WaveFucker_knob_png, BinaryData::WaveFucker_knob_pngSize);
        checkImage = juce::ImageCache::getFromMemory(BinaryData::WaveFucker_check_png, BinaryData::WaveFucker_check_pngSize);
        volTrackImage = juce::ImageCache::getFromMemory(BinaryData::WaveFucker_volume1_png, BinaryData::WaveFucker_volume1_pngSize);
        volThumbImage = juce::ImageCache::getFromMemory(BinaryData::WaveFucker_volume2_png, BinaryData::WaveFucker_volume2_pngSize);
    
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::black); 
        setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);       
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::black);   
        setColour(juce::Slider::textBoxHighlightColourId, juce::Colours::darkgrey); 
    }


    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
        juce::Slider& slider) override
    {
        if (knobImage.isValid())
        {
            int imgW = knobImage.getWidth();
            int imgH = knobImage.getHeight();

 
            bool isHorizontal = imgW > imgH;

            int numFrames = isHorizontal ? (imgW / imgH) : (imgH / imgW);
            if (numFrames < 1) numFrames = 1;

            int frameIdx = (int)std::round(sliderPos * (numFrames - 1));

            int frameW = isHorizontal ? (imgW / numFrames) : imgW;
            int frameH = isHorizontal ? imgH : (imgH / numFrames);

            int srcX = isHorizontal ? (frameIdx * frameW) : 0;
            int srcY = isHorizontal ? 0 : (frameIdx * frameH);

            float scale = 0.65f;
            int destW = (int)(frameW * scale);
            int destH = (int)(frameH * scale);

            int drawX = x + (width - destW) / 2;
            int drawY = y + (height - destH) / 2;

            g.drawImage(knobImage, drawX, drawY, destW, destH,
                srcX, srcY, frameW, frameH);
        }
    }


    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (slider.isHorizontal()) return;

        if (volTrackImage.isValid())
        {
            int drawX = x + (width - volTrackImage.getWidth()) / 2;
            g.drawImageAt(volTrackImage, drawX, y);
        }

        if (volThumbImage.isValid())
        {
            int thumbW = volThumbImage.getWidth();
            int thumbH = volThumbImage.getHeight();
            int drawX = x + (width - thumbW) / 2;
            int drawY = (int)sliderPos - thumbH / 2;

            drawY = juce::jlimit(y, y + height - thumbH, drawY);
            g.drawImageAt(volThumbImage, drawX, drawY);
        }
    }


    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (checkImage.isValid())
        {
            int imgW = checkImage.getWidth();
            int imgH = checkImage.getHeight();

            bool isHorizontal = imgW > imgH;

            int numFrames = isHorizontal ? (imgW / imgH) : (imgH / imgW);
            if (numFrames < 2) numFrames = 2;

            int frameW = isHorizontal ? (imgW / numFrames) : imgW;
            int frameH = isHorizontal ? imgH : (imgH / numFrames);

            int frameIdx = button.getToggleState() ? 1 : 0;

            int srcX = isHorizontal ? (frameIdx * frameW) : 0;
            int srcY = isHorizontal ? 0 : (frameIdx * frameH);


            int boxSize = 20; 
            int drawY = (button.getHeight() - boxSize) / 2;

            g.drawImage(checkImage, 0, drawY, boxSize, boxSize,
                srcX, srcY, frameW, frameH);
        }
    }

private:
    juce::Image knobImage, checkImage, volTrackImage, volThumbImage;
};



class WaveFuckerAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer
{
public:
    WaveFuckerAudioProcessorEditor(WaveFuckerAudioProcessor&);
    ~WaveFuckerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    WaveFuckerAudioProcessor& audioProcessor;

    CartoonLookAndFeel cartoonStyle;

    PeakMeter peakMeter;

    juce::ToggleButton sawButton, triButton, sqrButton;
    juce::ToggleButton naiveButton, blitButton, dsfButton, polyBlepButton, dpwButton;

    juce::Slider cutoffSlider, resSlider, lfoFreqSlider, lfoDepthSlider, glideSlider;
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider fAttackSlider, fDecaySlider, fSustainSlider, fReleaseSlider, fDepthSlider, noiseSlider;
    juce::Slider volSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        cutoffAttachment, resonanceAttachment, lfoFreqAttachment, lfoDepthAttachment, glideAttachment,
        attAttach, decAttach, susAttach, relAttach,
        fAttAttach, fDecAttach, fSusAttach, fRelAttach, fDepAttach, noiseAttachment, volAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveFuckerAudioProcessorEditor)
};