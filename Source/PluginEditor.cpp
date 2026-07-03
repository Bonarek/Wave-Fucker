/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveFuckerAudioProcessorEditor::WaveFuckerAudioProcessorEditor (WaveFuckerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    sawButton.setRadioGroupId(1);
    triButton.setRadioGroupId(1);
    sqrButton.setRadioGroupId(1);

    addAndMakeVisible(sqrButton);
    addAndMakeVisible(sawButton);
    addAndMakeVisible(triButton);


    naiveButton.setRadioGroupId(2);
    blitButton.setRadioGroupId(2);

    addAndMakeVisible(naiveButton);
    addAndMakeVisible(blitButton);

    auto updateWave = [this](int value) {
        if(auto * param = audioProcessor.apvts.getParameter("WAVE_TYPE"))
            param->setValueNotifyingHost(param->convertTo0to1(value)); 
    };
    sawButton.onClick = [updateWave] { updateWave(0); }; // 0
    triButton.onClick = [updateWave] { updateWave(1); }; // 1
    sqrButton.onClick = [updateWave] { updateWave(2); }; // 2

    auto updateMethod = [this](int value) {
        if (auto* param = audioProcessor.apvts.getParameter("METHOD_TYPE"))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    };

    naiveButton.onClick = [updateMethod] { updateMethod(0); };
    blitButton.onClick = [updateMethod] { updateMethod(1); };

    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(cutoffSlider);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "CUTOFF", cutoffSlider
        );
    cutoffLabel.setText("Filter Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);


    resSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(resSlider);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "RESONANCE", resSlider
        );

    lfoFreqSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lfoFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(lfoFreqSlider);
    lfoFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "LFO_FREQ", lfoFreqSlider
        );


    lfoDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(lfoDepthSlider);

    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "LFO_DEPTH", lfoDepthSlider
        );
    auto setupSlider = [this](juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attach, const juce::String& paramID) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    setupSlider(attackSlider, attAttach, "ATTACK");
    setupSlider(decaySlider, decAttach, "DECAY");
    setupSlider(sustainSlider, susAttach, "SUSTAIN");
    setupSlider(releaseSlider, relAttach, "RELEASE");

    int currentWave = (int)*audioProcessor.apvts.getRawParameterValue("WAVE_TYPE");
    int currentMethod = (int)*audioProcessor.apvts.getRawParameterValue("METHOD_TYPE");

    if (currentWave == 0) sawButton.setToggleState(true, juce::dontSendNotification);
    else if (currentWave == 1) triButton.setToggleState(true, juce::dontSendNotification);
    else if (currentWave == 2) sqrButton.setToggleState(true, juce::dontSendNotification);

    if (currentMethod == 0) naiveButton.setToggleState(true, juce::dontSendNotification);
    else if (currentMethod == 1) blitButton.setToggleState(true, juce::dontSendNotification);

    startTimer(30);
}

WaveFuckerAudioProcessorEditor::~WaveFuckerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void WaveFuckerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawText("WaveFucker Synth ", getLocalBounds().removeFromTop(50), juce::Justification::centred);
    g.drawText("Wave ", 100, 120, 150, 30, juce::Justification::centredLeft);
    g.drawText("Method", 300, 120, 150, 30, juce::Justification::centredLeft);

    g.drawText("Cutoff", 500, 120, 100, 30, juce::Justification::centred);
    g.drawText("Resonance", 620, 120, 100, 30, juce::Justification::centred);
    g.drawText("LFO Rate", 30, 280, 100, 30, juce::Justification::centred);
    g.drawText("LFO Depth", 150, 280, 100, 30, juce::Justification::centred);

    g.drawText("Attack", 300, 280, 80, 30, juce::Justification::centred);
    g.drawText("Decay", 390, 280, 80, 30, juce::Justification::centred);
    g.drawText("Sustain", 480, 280, 80, 30, juce::Justification::centred);
    g.drawText("Release", 570, 280, 80, 30, juce::Justification::centred);

    juce::Rectangle<float> area(0.0f, 400.0f, 800.0f, 200.0f);
    auto leftArea = area.removeFromLeft(400.0f);
    auto rightArea = area;

    g.setColour(juce::Colours::black);
    g.fillRect(leftArea);
    g.fillRect(rightArea);
    g.setColour(juce::Colours::darkgreen);
    g.drawRect(leftArea);
    g.drawRect(rightArea);

    g.setFont(10.0f);

    float midY = leftArea.getY() + leftArea.getHeight() * 0.5f;
    float ampScale = leftArea.getHeight() * 0.5f; 

    std::vector<float> amplitudes = { 1.0f, 0.5f, 0.0f, -0.5f, -1.0f };
    for (float amp : amplitudes)
    {
        float y = midY - (amp * ampScale);

        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawLine(leftArea.getX(), y, leftArea.getRight(), y);

        g.setColour(juce::Colours::lime);

        g.drawText(juce::String(amp, 1), leftArea.getX() + 2, y + 2, 30, 10, juce::Justification::left);
    }

    for (int i = 1; i < 4; ++i)
    {
        float x = leftArea.getX() + i * (leftArea.getWidth() / 4.0f);

        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawLine(x, leftArea.getY(), x, leftArea.getBottom());
    }

    for (int i = 0; i <= 4; ++i)
    {
        float y = rightArea.getY() + i * (rightArea.getHeight() / 4.0f);
        g.setColour(juce::Colours::grey.withAlpha(0.3f)); 
        g.drawLine(rightArea.getX(), y, rightArea.getRight(), y);

        g.setColour(juce::Colours::lime); 
        int dbVal = 0 - (i * 20);
        g.drawText(juce::String(dbVal) + " dB", rightArea.getX() + 2, y + 2, 40, 10, juce::Justification::left);
    }


    float sampleRateGrid = 48000.0f;
    float minFreqGrid = 20.0f;
    float maxFreqGrid = sampleRateGrid / 2.0f;

    std::vector<float> gridFrequencies = { 100.0f, 1000.0f, 10000.0f };
    std::vector<juce::String> gridLabels = { "100", "1k", "10k" };

    for (size_t i = 0; i < gridFrequencies.size(); ++i)
    {
        float freq = gridFrequencies[i];
        float x = rightArea.getX() + juce::jmap(std::log10(freq),
            std::log10(minFreqGrid),
            std::log10(maxFreqGrid),
            0.0f,
            rightArea.getWidth());

        g.setColour(juce::Colours::grey.withAlpha(0.3f)); 
        g.drawLine(x, rightArea.getY(), x, rightArea.getBottom());

        g.setColour(juce::Colours::lime); 
        g.drawText(gridLabels[i], x - 15, rightArea.getBottom() - 15, 30, 10, juce::Justification::centred);
    }


    juce::Path p;

    for (int i = 0; i < 256; ++i)
    {
        //Mapping
        float x = leftArea.getX() + (i / 255.0f) * leftArea.getWidth();
        float midY_osc = leftArea.getY() + leftArea.getHeight() * 0.5f;
        float y = midY_osc - (audioProcessor.visualBuffer[i] * (leftArea.getHeight() * 0.5f));

        if (i == 0) p.startNewSubPath(x, y);
        else p.lineTo(x, y);
    }
    g.setColour(juce::Colours::green.withAlpha(0.3f));
    g.strokePath(p, juce::PathStrokeType(4.0f));
    g.setColour(juce::Colours::limegreen);
    g.strokePath(p, juce::PathStrokeType(1.5f));

    g.setColour(juce::Colours::limegreen); 
    float sampleRate = 48000.0f;
    int fftSize = 1024;
    float minFreq = 20.0f;
    float maxFreq = sampleRate / 2.0f;

    for (int i = 0; i < 512; ++i)
    {
        float currentFreq = ((float)i / (float)(fftSize / 2)) * maxFreq;
        if (currentFreq < minFreq) continue;

        float xPos = rightArea.getX() + juce::jmap(std::log10(currentFreq),
            std::log10(minFreq),
            std::log10(maxFreq),
            0.0f,
            rightArea.getWidth());

        float nextFreq = ((float)(i + 1) / (float)(fftSize / 2)) * maxFreq;
        float nextXPos = rightArea.getX() + juce::jmap(std::log10(nextFreq),
            std::log10(minFreq),
            std::log10(maxFreq),
            0.0f,
            rightArea.getWidth());

        float width = nextXPos - xPos;
        float barHeight = std::max(1.0f, audioProcessor.fftMaginitude[i] * rightArea.getHeight());
        g.fillRect(xPos, rightArea.getBottom() - barHeight, width + 0.5f, barHeight);
    }

}


void WaveFuckerAudioProcessorEditor::resized()
{
    sawButton.setBounds(100, 150, 150, 30);
    triButton.setBounds(100, 180, 150, 30);
    sqrButton.setBounds(100, 210, 150, 30);

    naiveButton.setBounds(300, 150, 150, 30);
    blitButton.setBounds(300, 180, 150, 30); 

    cutoffSlider.setBounds(500, 150, 100, 100);
    cutoffLabel.setBounds(0, 0, 0, 0);

    resSlider.setBounds(620, 150, 100, 100);
    lfoFreqSlider.setBounds(30, 310, 100, 100);
    lfoDepthSlider.setBounds(150, 310, 100, 100);

    attackSlider.setBounds(300, 310, 80, 80);
    decaySlider.setBounds(390, 310, 80, 80);
    sustainSlider.setBounds(480, 310, 80, 80);
    releaseSlider.setBounds(570, 310, 80, 80);
}
    