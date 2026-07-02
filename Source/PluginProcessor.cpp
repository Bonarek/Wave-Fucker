/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


juce::AudioProcessorValueTreeState::ParameterLayout WaveFuckerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Twoje STARE parametry - identyfikatory ("WAVE_TYPE", "METHOD_TYPE") zostaj¹ takie same!
    layout.add(std::make_unique<juce::AudioParameterInt>("WAVE_TYPE", "Wave Type", 0, 2, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("METHOD_TYPE", "Method Type", 0, 1, 0));

    // Twój NOWY parametr - Cutoff do filtra
    juce::NormalisableRange<float> cutoffRange(20.0f, 20000.0f, 1.0f);

    cutoffRange.setSkewForCentre(1000.0f);


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "CUTOFF",       
        "Cutoff",      
        cutoffRange,    
        20000.0f        
        ));

    return layout;
}
//==============================================================================
WaveFuckerAudioProcessor::WaveFuckerAudioProcessor()
    :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#else
    AudioProcessor(),
#endif
    apvts(*this, nullptr, "Parameters", createParameterLayout()),
    fft(10), // FFT  2^10 = 1024
    window(1024, juce::dsp::WindowingFunction<float>::blackmanHarris, true)
{
    std::fill(std::begin(visualBuffer), std::end(visualBuffer), 0.0f);
    std::fill(std::begin(fftData), std::end(fftData), 0.0f);
    std::fill(std::begin(fftMaginitude), std::end(fftMaginitude), 0.0f);
}

WaveFuckerAudioProcessor::~WaveFuckerAudioProcessor()
{
}

//==============================================================================
const juce::String WaveFuckerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool WaveFuckerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool WaveFuckerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool WaveFuckerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double WaveFuckerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int WaveFuckerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int WaveFuckerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void WaveFuckerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String WaveFuckerAudioProcessor::getProgramName (int index)
{
    return {};
}

void WaveFuckerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void WaveFuckerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{ sampleRate, (juce::uint32)samplesPerBlock, 2 }; 
    dspChain.get<0>().setMode(juce::dsp::LadderFilterMode::LPF12);
    dspChain.prepare(spec);

    std::fill(std::begin(notesActive), std::end(notesActive), false);
    isPlaying = false;
    Phase = 0.0f;
    integrator = 0.0f;
    integrator2 = 0.0f;
}

void WaveFuckerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool WaveFuckerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void WaveFuckerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    buffer.clear();
    //Analisis what DAW send
    midiState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);
    //security
    int waveType = (int)*apvts.getRawParameterValue("WAVE_TYPE");
    int methodType = (int)*apvts.getRawParameterValue("METHOD_TYPE");
    float cutoffFreq = *apvts.getRawParameterValue("CUTOFF");
    
    DBG("Wave: " << waveType << " | Method: " << methodType );
  
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            notesActive[note] = true;
            Frequency = juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber());
            isPlaying = true;
            Phase = 0.0f;
            integrator = 0.0f;
            integrator2 = 0.0f;
            dcBlockerState = 0.0f;
            
        }
        else if (msg.isNoteOff())
        {
            notesActive[msg.getNoteNumber()] = false;
        }
    }

    bool anyNoteActive = false;
    for (bool active : notesActive)
    {
        if (active)
        {
            anyNoteActive = true;
            break;
        }
    }
    isPlaying = anyNoteActive;

    float phaseDelta = (Frequency > 0.0f) ? (Frequency / getSampleRate()) : 0.0f;
    float P = getSampleRate() / Frequency;
    float M = 2.0f * floorf(P / 2.0f) + 1.0f;
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {

        float sampleValue = 0.0f;
        if (isPlaying)
        {
            float b1 = getBlit(Phase, P, M);
            float b2 = getBlit(fmodf(Phase + 0.5f, 1.0f), P, M);
            //user choice
            if (waveType == 0) // saw
            {
                if (methodType == 0) //naive saw
                {
                    sampleValue = (Phase * 2.0f) - 1.0f;
                }
                else if (methodType == 1) // blit saw
                {
                    float blit_centered = b1 - (1.0f / P);
                    integrator = blit_centered + (R * integrator);
                    sampleValue = dcBlocker(integrator);
                }

            }
            else if (waveType == 1) //tri
            {
                if (methodType == 0) //naive tri
                {
                    sampleValue = std::abs((Phase * 2.0f) - 1.0f) * 2.0f - 1.0f;
                }

                else if (methodType == 1) // blit tri
                {
                    float bp_blit = b1 - b2;


                    integrator = bp_blit + (R * integrator);
                    float cleanSquare = dcBlocker(integrator);
                    integrator2 = cleanSquare + (R * integrator2);
                    sampleValue = integrator2 * (4.0f / P);
                }
            }
            else if (waveType == 2) //sqr
            {
                if (methodType == 0) //naive sqr
                {
                    if (Phase < 0.5f)
                    {
                        sampleValue = 1.0f;
                    }
                    else
                    {
                        sampleValue = -1.0f;
                    }
                }
                else if (methodType == 1) // blit sqr
                {
                    float bp_blit = b1 - b2;
                    integrator = bp_blit + (R * integrator);
                    sampleValue = dcBlocker(integrator);
                }

            }
            sampleValue *= 0.2f;

            Phase += phaseDelta;

            if (Phase >= 1.0f)
            {
                Phase -= 1.0f;
            }
        }
        else
        {
            sampleValue = 0.0f;
            Phase = 0.0f;
        }
        sampleValue = juce::jlimit(-1.0f, 1.0f, sampleValue);
        // convert to all chanels (1 - mono, 2 - stereo itp.)
        for (int chanel = 0; chanel < getTotalNumOutputChannels(); ++chanel)
        {
            buffer.setSample(chanel, sample, sampleValue);
        }
    }
    float safeCutoff = juce::jmax(20.0f, cutoffFreq);
    dspChain.get<0>().setCutoffFrequencyHz(safeCutoff);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    dspChain.process(context);


    auto* channelData = buffer.getReadPointer(0);
    for (int sample = 0; sample<buffer.getNumSamples(); ++sample)
    {
        float filteredSample = channelData[sample];


        if (sample < 256)
        {
            visualBuffer[sample] = filteredSample;
        }

        
        fftData[fftIndex] = filteredSample;
        fftIndex++;
        if (fftIndex >= 1024)
        {

            std::fill(fftData + 1024, fftData + 2048, 0.0f);
            window.multiplyWithWindowingTable(fftData, 1024);
            fft.performFrequencyOnlyForwardTransform(fftData);

            for (int i = 0; i < 512; ++i)
            {
                float normalizedVal = std::abs(fftData[i]) / 512.0f;
                float dB = juce::Decibels::gainToDecibels(normalizedVal, -80.0f);
                float mapped = juce::jmap(dB, -80.0f, 0.0f, 0.0f, 1.0f);
                fftMaginitude[i] = juce::jlimit(0.0f, 1.0f, mapped);
            }
            fftIndex = 0;
        }
        
    }

}


//==============================================================================
bool WaveFuckerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* WaveFuckerAudioProcessor::createEditor()
{
    return new WaveFuckerAudioProcessorEditor (*this);
}

//==============================================================================
void WaveFuckerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void WaveFuckerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveFuckerAudioProcessor();
}
