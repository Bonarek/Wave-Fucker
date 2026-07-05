#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h" 

class PeakMeter : public juce::Component, private juce::Timer
{
public:
    PeakMeter(CartoonSynthAudioProcessor& p) : processor(p) { startTimerHz(30); }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float level = (processor.leftChannelLevel + processor.rightChannelLevel) * 0.5f;

        // Wymiary kółek
        float circleSize = 14.0f;
        float padding = 6.0f;
        float startX = (bounds.getWidth() - circleSize) / 2.0f;

        g.setColour(level > 0.95f ? juce::Colours::red : juce::Colours::darkred.withAlpha(0.2f));
        g.fillEllipse(startX, padding, circleSize, circleSize);

        g.setColour(level > 0.7f ? juce::Colours::yellow : juce::Colours::darkgoldenrod.withAlpha(0.2f));
        g.fillEllipse(startX, padding * 2 + circleSize, circleSize, circleSize);

        g.setColour(level > 0.05f ? juce::Colours::limegreen : juce::Colours::darkgreen.withAlpha(0.2f));
        g.fillEllipse(startX, padding * 3 + circleSize * 2, circleSize, circleSize);
    }

    void timerCallback() override { repaint(); }

private:
    CartoonSynthAudioProcessor& processor;
};