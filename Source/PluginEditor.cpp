/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WaveFuckerAudioProcessorEditor::WaveFuckerAudioProcessorEditor(WaveFuckerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), peakMeter(p)
{
    setSize(800, 600);
    addAndMakeVisible(peakMeter);


    juce::LookAndFeel::setDefaultLookAndFeel(&cartoonStyle);


    cartoonStyle.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

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
        if (auto* param = audioProcessor.apvts.getParameter("WAVE_TYPE"))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    };
    sawButton.onClick = [updateWave] { updateWave(0); };
    triButton.onClick = [updateWave] { updateWave(1); };
    sqrButton.onClick = [updateWave] { updateWave(2); };

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

    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    stopTimer();
}

void WaveFuckerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.brighter(0.1f));

    juce::Image bg = juce::ImageCache::getFromMemory(BinaryData::WaveFucker_background_png, BinaryData::WaveFucker_background_pngSize);
    if (bg.isValid())
    {
        g.drawImageAt(bg, 0, 0);
    }

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
        float x = rightArea.getX() + juce::jmap(std::log10(freq), std::log10(minFreqGrid), std::log10(maxFreqGrid), 0.0f, rightArea.getWidth());
        g.setColour(juce::Colours::grey.withAlpha(0.3f));
        g.drawLine(x, rightArea.getY(), x, rightArea.getBottom());
        g.setColour(juce::Colours::lime);
        g.drawText(gridLabels[i], x - 15, rightArea.getBottom() - 15, 30, 10, juce::Justification::centred);
    }

    juce::Path p;
    for (int i = 0; i < 256; ++i)
    {
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

        float xPos = rightArea.getX() + juce::jmap(std::log10(currentFreq), std::log10(minFreq), std::log10(maxFreq), 0.0f, rightArea.getWidth());
        float nextFreq = ((float)(i + 1) / (float)(fftSize / 2)) * maxFreq;
        float nextXPos = rightArea.getX() + juce::jmap(std::log10(nextFreq), std::log10(minFreq), std::log10(maxFreq), 0.0f, rightArea.getWidth());

        float width = nextXPos - xPos;
        float barHeight = std::max(1.0f, audioProcessor.fftMaginitude[i] * rightArea.getHeight());
        g.fillRect(xPos, rightArea.getBottom() - barHeight, width + 0.5f, barHeight);
    }
}


void WaveFuckerAudioProcessorEditor::resized()
{

    sawButton.setBounds(30, 90, 90, 25);
    triButton.setBounds(30, 120, 90, 25);
    sqrButton.setBounds(30, 150, 90, 25);

    naiveButton.setBounds(130, 90, 100, 25);
    blitButton.setBounds(130, 120, 100, 25);
    dsfButton.setBounds(130, 150, 100, 25);
    polyBlepButton.setBounds(130, 180, 100, 25);
    dpwButton.setBounds(130, 210, 100, 25);

    cutoffSlider.setBounds(260, 110, 60, 60);
    resSlider.setBounds(330, 110, 60, 60);
    lfoFreqSlider.setBounds(410, 110, 60, 60);
    lfoDepthSlider.setBounds(480, 110, 60, 60);
    glideSlider.setBounds(560, 110, 60, 60);

    volSlider.setBounds(650, 110, 60, 60);
    peakMeter.setBounds(730, 110, 30, 60);

    attackSlider.setBounds(30, 280, 60, 60);
    decaySlider.setBounds(100, 280, 60, 60);
    sustainSlider.setBounds(170, 280, 60, 60);
    releaseSlider.setBounds(240, 280, 60, 60);

    fAttackSlider.setBounds(330, 280, 60, 60);
    fDecaySlider.setBounds(400, 280, 60, 60);
    fSustainSlider.setBounds(470, 280, 60, 60);
    fReleaseSlider.setBounds(540, 280, 60, 60);
    fDepthSlider.setBounds(610, 280, 60, 60);

    noiseSlider.setBounds(705, 280, 60, 60);
}
void WaveFuckerAudioProcessorEditor::timerCallback()
{
    repaint();
}