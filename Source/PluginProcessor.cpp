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

    layout.add(std::make_unique<juce::AudioParameterInt>("WAVE_TYPE", "Wave Type", 0, 2, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("METHOD_TYPE", "Method Type", 0, 4, 0));
    juce::NormalisableRange<float> cutoffRange(20.0f, 20000.0f, 1.0f);

    cutoffRange.setSkewForCentre(1000.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>("CUTOFF", "Cutoff", cutoffRange, 20000.0f));
   
    layout.add(std::make_unique<juce::AudioParameterFloat>("RESONANCE", "Resonance", 0.0f, 1.0f, 0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("LFO_FREQ", "LFO Freq", 0.1f, 20.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("LFO_DEPTH", "LFO Depth", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("ATTACK", "Attack", 0.01f, 5.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DECAY", "Decay", 0.1f, 5.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release", 0.01f, 5.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("GLIDE", "Glide", 0.0f, 1.0f, 0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("F_ATTACK", "F. Attack", 0.01f, 5.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("F_DECAY", "F. Decay", 0.1f, 5.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("F_SUSTAIN", "F. Sustain", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("F_RELEASE", "F. Release", 0.01f, 5.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ENVELOPE", "Envelope", -1.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("VOLUME", "Master Vol", 0.0f, 1.0f, 0.8f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("NOISE", "Noise", 0.0f, 1.0f, 0.0f));

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

    adsr.setSampleRate(sampleRate);
    filterAdsr.setSampleRate(sampleRate);
    activeNotes.clear();



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
    float ressonanceValue = *apvts.getRawParameterValue("RESONANCE");

    
    adsrParameters.attack = *apvts.getRawParameterValue("ATTACK");
    adsrParameters.decay = *apvts.getRawParameterValue("DECAY");
    adsrParameters.sustain = *apvts.getRawParameterValue("SUSTAIN");
    adsrParameters.release = *apvts.getRawParameterValue("RELEASE");
    adsr.setParameters(adsrParameters);

    filterAdrsParams.attack = *apvts.getRawParameterValue("F_ATTACK");
    filterAdrsParams.decay = *apvts.getRawParameterValue("F_DECAY");
    filterAdrsParams.sustain = *apvts.getRawParameterValue("F_SUSTAIN");
    filterAdrsParams.release = *apvts.getRawParameterValue("F_RELEASE");
    filterAdsr.setParameters(filterAdrsParams);

    float noise = *apvts.getRawParameterValue("NOISE");
    
  
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            int noteNumber = msg.getNoteNumber();
            for (auto it = activeNotes.begin(); it != activeNotes.end(); ++it)
            {
                if (*it == noteNumber) { activeNotes.erase(it); break; }
            }
            activeNotes.push_back(noteNumber);

            Frequency = juce::MidiMessage::getMidiNoteInHertz(noteNumber);
            if (activeNotes.size() == 1)
            {

                Phase = 0.0f;
                integrator = 0.0f;
                integrator2 = 0.0f;
                dcBlockerState = 0.0f;
                dpwState = 0.0f;
            }

            adsr.noteOn();
            filterAdsr.noteOn();
            
        }
        else if (msg.isNoteOff())
        {
            int noteNumber = msg.getNoteNumber();
            for (auto it = activeNotes.begin(); it != activeNotes.end(); ++it)
            {
                if (*it == noteNumber) { activeNotes.erase(it); break; }
            }
            if (activeNotes.empty())
            {
                adsr.noteOff();
                filterAdsr.noteOff();
            }
            else
            {
                int remainingNote = activeNotes.back();
                Frequency = juce::MidiMessage::getMidiNoteInHertz(remainingNote);
            }
        }
    }


    float currentFilterEnv = 0.0f;
    float glideTime = *apvts.getRawParameterValue("GLIDE");
    float glideCoeff = (glideTime > 0.001f) ? std::exp(-1.0f / (glideTime * getSampleRate())) : 0.0f;


    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        currentFrequency = glideCoeff * currentFrequency + (1.0f - glideCoeff) * Frequency;
        float phaseDelta = (currentFrequency > 0.0f) ? (currentFrequency / getSampleRate()) : 0.0f;
        float P = getSampleRate() / currentFrequency;
        float M = 2.0f * floorf(P / 2.0f) + 1.0f;

        float sampleValue = 0.0f;
        if (adsr.isActive())
        {


            float N_harmonics = std::floor((getSampleRate() / 2.0f) / currentFrequency);
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
                else if (methodType == 2) // dsf saw
                {
                    float summation_saw = getDsf(Phase, N_harmonics);
                    integrator = summation_saw + (R * integrator);
                    sampleValue = dcBlocker(integrator) * (2.0f / P);
                }
                else if (methodType == 3) // polyBLEP saw
                {

                    float naiveSaw = (Phase * 2.0f) - 1.0f;
                    sampleValue = naiveSaw - getBlep(Phase, phaseDelta);
                }
                else if (methodType == 4) // dpw saw
                {
                    float trivialSaw = (Phase * 2.0f) - 1.0f;
                    float parabola = trivialSaw * trivialSaw;
                    float diff = parabola - dpwState;
                    dpwState = parabola;
                    sampleValue = diff * (getSampleRate() / (4.0f * currentFrequency));
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
                else if (methodType == 2) // dsf tri
                {
                    float dsf_1 = getDsf(Phase, N_harmonics);
                    float dsf_2 = getDsf(fmodf(Phase + 0.5f, 1.0f), N_harmonics);

                    float summation_sqr = dsf_1 - dsf_2;
                    integrator = summation_sqr + (R * integrator);
                    float square_val = dcBlocker(integrator) * (2.0f / P);
                    integrator2 = square_val + (R * integrator2);
                    sampleValue = dcBlocker(integrator2) * (4.0f / P);
                }
                else if (methodType == 3) // polyBLEP tri
                {
                    float naiveTri = 2.0f * std::abs(2.0f * Phase - 1.0f) - 1.0f;
                    float blamp1 = getBlamp(Phase, phaseDelta);
                    float blamp2 = getBlamp(fmodf(Phase + 0.5f, 1.0f), phaseDelta);
                    sampleValue = naiveTri - 8.0f * phaseDelta * blamp1 + 8.0f * phaseDelta * blamp2;
                }
                else if (methodType == 4) // dpw tri
                {
                    float intTri = (Phase < 0.5f) ?
                        (2.0f * Phase * Phase - Phase + 0.5f) :
                        (-2.0f * Phase * Phase + 3.0f * Phase - 0.5f);
                    float diff = intTri - dpwState;
                    dpwState = intTri;
                    sampleValue = diff * (getSampleRate() / currentFrequency);
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
                else if (methodType == 2) // dsf tri
                {
                    float dsf_1 = getDsf(Phase, N_harmonics);
                    float dsf_2 = getDsf(fmodf(Phase + 0.5f, 1.0f), N_harmonics);

                    float summation_sqr = dsf_1 - dsf_2;
                    integrator = summation_sqr + (R * integrator);
                    sampleValue = dcBlocker(integrator) * (2.0f / P);
                }
                else if (methodType == 3) // polyBLEP tri
                {
                    float naiveSqr = (Phase < 0.5f) ? 1.0f : -1.0f;
                    float blep1 = getBlep(Phase, phaseDelta);
                    float blep2 = getBlep(fmodf(Phase + 0.5f, 1.0f), phaseDelta);
                    sampleValue = naiveSqr + blep1 - blep2;
                }
                else if (methodType == 4) // dpw tri
                {
                    float saw1 = (Phase * 2.0f) - 1.0f;
                    float saw2 = (fmodf(Phase + 0.5f, 1.0f) * 2.0f) - 1.0f;
                    float parabolaSq = (saw1 * saw1) - (saw2 * saw2);
                    float diff = parabolaSq - dpwState;
                    dpwState = parabolaSq;
                    sampleValue = diff * (getSampleRate() / (4.0f * currentFrequency));
                }

            }
            float noiseSample = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * noise;
            sampleValue += noiseSample;
            sampleValue *= 0.2f;
            sampleValue *= adsr.getNextSample();
            currentFilterEnv = filterAdsr.getNextSample();

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
    //LFO
    float lfoRate = *apvts.getRawParameterValue("LFO_FREQ");
    float lfoDepth = *apvts.getRawParameterValue("LFO_DEPTH");

    float fDepth = *apvts.getRawParameterValue("ENVELOPE");
    lfoPhase += lfoRate * (buffer.getNumSamples() / getSampleRate());
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
    float lfoValue = std::sin(lfoPhase * juce::MathConstants<float>::twoPi);    
    float modulatedCutoff = cutoffFreq * (1.0f + (lfoValue * lfoDepth * 0.9f));
    modulatedCutoff += (currentFilterEnv * fDepth * 10000.0f);
    float safeCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);

    dspChain.get<0>().setCutoffFrequencyHz(safeCutoff);
    dspChain.get<0>().setResonance(juce::jlimit(0.0f, 1.0f, ressonanceValue));

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
    float masterVol = *apvts.getRawParameterValue("VOLUME");
    buffer.applyGain(masterVol);

    leftChannelLevel = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    rightChannelLevel = buffer.getMagnitude(1, 0, buffer.getNumSamples());

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
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData); 
}

void WaveFuckerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WaveFuckerAudioProcessor();
}
