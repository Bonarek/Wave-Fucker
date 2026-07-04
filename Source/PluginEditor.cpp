/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveFuckerAudioProcessorEditor::WaveFuckerAudioProcessorEditor (WaveFuckerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), peakMeter(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 600);
    addAndMakeVisible(peakMeter);

    sawButton.setRadioGroupId(1);
    triButton.setRadioGroupId(1);
    sqrButton.setRadioGroupId(1);
    addAndMakeVisible(sqrButton);
    addAndMakeVisible(sawButton);
    addAndMakeVisible(triButton);


    naiveButton.setRadioGroupId(2);
    blitButton.setRadioGroupId(2);
    dsfButton.setRadioGroupId(2);
    polyBlepButton.setRadioGroupId(2);
    dpwButton.setRadioGroupId(2);
    addAndMakeVisible(naiveButton);
    addAndMakeVisible(blitButton);
    addAndMakeVisible(dsfButton);
    addAndMakeVisible(polyBlepButton);
    addAndMakeVisible(dpwButton);

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
    dsfButton.onClick = [updateMethod] { updateMethod(2); };
    polyBlepButton.onClick = [updateMethod] { updateMethod(3); };
    dpwButton.onClick = [updateMethod] {updateMethod(4); };

    auto setupSlider = [this](juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attach, const juce::String& paramID) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    setupSlider(cutoffSlider, cutoffAttachment, "CUTOFF");
    setupSlider(resSlider, resonanceAttachment, "RESONANCE");
    setupSlider(lfoFreqSlider, lfoFreqAttachment, "LFO_FREQ");
    setupSlider(lfoDepthSlider, lfoDepthAttachment, "LFO_DEPTH");
    setupSlider(glideSlider, glideAttachment, "GLIDE");

    setupSlider(attackSlider, attAttach, "ATTACK");
    setupSlider(decaySlider, decAttach, "DECAY");
    setupSlider(sustainSlider, susAttach, "SUSTAIN");
    setupSlider(releaseSlider, relAttach, "RELEASE");

    setupSlider(fAttackSlider, fAttAttach, "F_ATTACK");
    setupSlider(fDecaySlider, fDecAttach, "F_DECAY");
    setupSlider(fSustainSlider, fSusAttach, "F_SUSTAIN");
    setupSlider(fReleaseSlider, fRelAttach, "F_RELEASE");
    setupSlider(fDepthSlider, fDepAttach, "ENVELOPE");
    setupSlider(noiseSlider, noiseAttachment, "NOISE");

    volSlider.setSliderStyle(juce::Slider::LinearVertical);

    volSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); 
    addAndMakeVisible(volSlider);
    volAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "VOLUME", volSlider);

    int currentWave = (int)*audioProcessor.apvts.getRawParameterValue("WAVE_TYPE");
    int currentMethod = (int)*audioProcessor.apvts.getRawParameterValue("METHOD_TYPE");

    if (currentWave == 0) sawButton.setToggleState(true, juce::dontSendNotification);
    else if (currentWave == 1) triButton.setToggleState(true, juce::dontSendNotification);
    else if (currentWave == 2) sqrButton.setToggleState(true, juce::dontSendNotification);

    if (currentMethod == 0) naiveButton.setToggleState(true, juce::dontSendNotification);
    else if (currentMethod == 1) blitButton.setToggleState(true, juce::dontSendNotification);
    else if (currentMethod == 2) dsfButton.setToggleState(true, juce::dontSendNotification);
    else if (currentMethod == 3) polyBlepButton.setToggleState(true, juce::dontSendNotification);
    else if (currentMethod == 4) dpwButton.setToggleState(true, juce::dontSendNotification);

    startTimer(30);
}

WaveFuckerAudioProcessorEditor::~WaveFuckerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void WaveFuckerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);


    g.drawText("WaveFucker Synth", 0, 10, 800, 30, juce::Justification::centred);


    g.drawText("Cutoff", 260, 80, 70, 30, juce::Justification::centred);
    g.drawText("Reson", 340, 80, 70, 30, juce::Justification::centred);
    g.drawText("LFO F", 460, 80, 70, 30, juce::Justification::centred);
    g.drawText("LFO D", 540, 80, 70, 30, juce::Justification::centred);
    g.drawText("Glide", 680, 80, 70, 30, juce::Justification::centred);


    g.drawText("Amp A", 40, 220, 70, 30, juce::Justification::centred);
    g.drawText("Amp D", 120, 220, 70, 30, juce::Justification::centred);
    g.drawText("Amp S", 200, 220, 70, 30, juce::Justification::centred);
    g.drawText("Amp R", 280, 220, 70, 30, juce::Justification::centred);


    g.drawText("Filt A", 400, 220, 70, 30, juce::Justification::centred);
    g.drawText("Filt D", 480, 220, 70, 30, juce::Justification::centred);
    g.drawText("Filt S", 560, 220, 70, 30, juce::Justification::centred);
    g.drawText("Filt R", 640, 220, 70, 30, juce::Justification::centred);
    g.drawText("Env Dep", 720, 220, 70, 30, juce::Justification::centred);
    g.drawText("Volume", 730, 120, 60, 30, juce::Justification::centred);   

    g.drawText("Noise", 680, 220, 70, 30, juce::Justification::centred);

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
    sawButton.setBounds(40, 100, 100, 25);
    triButton.setBounds(40, 130, 100, 25);
    sqrButton.setBounds(40, 160, 100, 25);

    naiveButton.setBounds(150, 100, 100, 25);
    blitButton.setBounds(150, 130, 100, 25);
    dsfButton.setBounds(150, 160, 100, 25);
    polyBlepButton.setBounds(150, 190, 100, 25);
    dpwButton.setBounds(150, 220, 100, 25);

    cutoffSlider.setBounds(260, 110, 70, 70);
    resSlider.setBounds(340, 110, 70, 70);
    lfoFreqSlider.setBounds(460, 110, 70, 70);
    lfoDepthSlider.setBounds(540, 110, 70, 70);
    glideSlider.setBounds(680, 110, 70, 70);

    attackSlider.setBounds(40, 250, 70, 70);
    decaySlider.setBounds(120, 250, 70, 70);
    sustainSlider.setBounds(200, 250, 70, 70);
    releaseSlider.setBounds(280, 250, 70, 70);

    fAttackSlider.setBounds(400, 250, 70, 70);
    fDecaySlider.setBounds(480, 250, 70, 70);
    fSustainSlider.setBounds(560, 250, 70, 70);
    fReleaseSlider.setBounds(640, 250, 70, 70);
    fDepthSlider.setBounds(720, 250, 70, 70);

    volSlider.setBounds(730, 150, 60, 60);
    peakMeter.setBounds(745, 50, 30, 60);
    noiseSlider.setBounds(680, 250, 70, 70);
}
    