/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

static juce::String gainID {"gain"};

static juce::String attackID {"attack"};
static juce::String decayID {"decay"};
static juce::String sustainID {"sustain"};
static juce::String releaseID {"release"};

static juce::String osc1TypeID {"osc1Type"};
static juce::Identifier osc1ID {"osc1"};

static juce::String osc2TypeID {"osc2Type"};
static juce::Identifier osc2ID {"osc2"};

static juce::String osc2MixID {"osc2Mix"};

static juce::String detuneAmountID {"detuneAmount"};

static juce::String cutoffID {"cutoff"};
static juce::String filterTypeID {"filterType"};

static juce::String lfoDepthID {"lfoDepth"};
static juce::String lfoFreqID {"lfoFreq"};

juce::AudioProcessorValueTreeState::ParameterLayout FaustSynth::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto adsrGroup = std::make_unique<juce::AudioProcessorParameterGroup>("adsr", "ADSR", "|");
    adsrGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{attackID, 1},
                                                                        "attack",
                                                                        juce::NormalisableRange<float>(0, 1.0, 0.01), 0.1));
    adsrGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{decayID, 1},
                                                                        "decay",
                                                                        juce::NormalisableRange<float>(0, 1.0, 0.01), 0.25));
    adsrGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{sustainID, 1},
                                                                        "sustain",
                                                                        juce::NormalisableRange<float>(0, 1.0, 0.01), 0.5));
    adsrGroup->addChild(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{releaseID, 1},
                                                                        "release",
                                                                        juce::NormalisableRange<float>(0, 5.0, 0.01), 1.0));
    layout.add(std::move(adsrGroup));
    
    auto osc1TypeParam = std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{osc1TypeID, 1},
                                                                      "osc1Type",
                                                                      juce::StringArray("Sine", "Square", "Triangle", "Saw"), 0);
    layout.add(std::move(osc1TypeParam));
    
    auto osc2TypeParam = std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{osc2TypeID, 1},
                                                                      "osc2Type",
                                                                      juce::StringArray("Sine", "Square", "Triangle", "Saw"), 0);
    layout.add(std::move(osc2TypeParam));
    
    auto osc2MixParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{osc2MixID, 1},
                                                                    "osc2Mix",
                                                                    juce::NormalisableRange<float>(0, 1.0, 0.01), 0);
    layout.add(std::move(osc2MixParam));
    
    auto gainParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{gainID, 1},
                                                                 "gain",
                                                                 juce::NormalisableRange<float>(0, 1.0, 0.01), 0.1);
    layout.add(std::move(gainParam));
    
    auto detuneAmountParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{detuneAmountID, 1},
                                                                         "detuneAmount",
                                                                         juce::NormalisableRange<float>(-100.0, 100.0, 1.0), 0.0);
    layout.add(std::move(detuneAmountParam));
    
    auto cutoffParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{cutoffID, 1},
                                                                   "cutoff",
                                                                   juce::NormalisableRange<float>(20.0, 20000.0, 1.0), 1000.0);
    layout.add(std::move(cutoffParam));
    
    auto filterTypeParam = std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{filterTypeID, 1},
                                                                        "filterType",
                                                                        juce::StringArray("No Filter", "Low Pass", "High Pass"), 0);
    layout.add(std::move(filterTypeParam));
    
    auto lfoDepthParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{lfoDepthID, 1},
                                                                     "lfoDepth",
                                                                     juce::NormalisableRange<float>(0, 1000.0, 1.0), 0);
    layout.add(std::move(lfoDepthParam));
    
    auto lfoFreqParam = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{lfoFreqID, 1},
                                                                    "lfoFreq",
                                                                    juce::NormalisableRange<float>(0.1, 10.0, 0.1), 1.0);
    layout.add(std::move(lfoFreqParam));

    return layout;
}
//==============================================================================
FaustSynth::FaustSynth()
     :
#ifndef JucePlugin_PreferredChannelConfigurations
       MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
      treeState (*this, nullptr, JucePlugin_Name, createParameterLayout())
{
    synth.clearSounds();
    synth.addSound(new FaustSound());
    for (int nVoices = 0; nVoices < NUM_VOICES; ++nVoices)
    {
        synth.addVoice(new FaustVoice());
    }

    treeState.addParameterListener(gainID, this);

    treeState.addParameterListener(attackID, this);
    treeState.addParameterListener(decayID, this);
    treeState.addParameterListener(sustainID, this);
    treeState.addParameterListener(releaseID, this);

    treeState.addParameterListener(osc1TypeID, this);
    treeState.addParameterListener(osc2TypeID, this);
    
    treeState.addParameterListener(osc2MixID, this);
    
    treeState.addParameterListener(detuneAmountID, this);
    
    treeState.addParameterListener(cutoffID, this);
    treeState.addParameterListener(filterTypeID, this);
    
    treeState.addParameterListener(lfoDepthID, this);
    treeState.addParameterListener(lfoFreqID, this);
    
    gain = treeState.getParameter(gainID)->getValue();

    osc1 = magicState.createAndAddObject<foleys::MagicOscilloscope>(osc1ID, 0);
    osc2 = magicState.createAndAddObject<foleys::MagicOscilloscope>(osc2ID, 0);

    magicState.setGuiValueTree(BinaryData::Layout_xml, BinaryData::Layout_xmlSize);
}

FaustSynth::~FaustSynth()
{
}

//==============================================================================
const juce::String FaustSynth::getName() const
{
    return JucePlugin_Name;
}

bool FaustSynth::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FaustSynth::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FaustSynth::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FaustSynth::getTailLengthSeconds() const
{
    return treeState.getParameter(releaseID)->getValue();
}

int FaustSynth::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FaustSynth::getCurrentProgram()
{
    return 0;
}

void FaustSynth::setCurrentProgram (int index)
{
}

const juce::String FaustSynth::getProgramName (int index)
{
    return {};
}

void FaustSynth::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FaustSynth::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    synth.setCurrentPlaybackSampleRate(sampleRate);
    magicState.prepareToPlay (sampleRate, samplesPerBlock);
}

void FaustSynth::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FaustSynth::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FaustSynth::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    int nSamples = buffer.getNumSamples();
    buffer.clear();
    
    magicState.processMidiBuffer(midiMessages, nSamples);
    
    synth.renderNextBlock(buffer, midiMessages, 0, nSamples);
    
    buffer.applyGain(gain);
    
    osc1->pushSamples (buffer);
    osc2->pushSamples (buffer);
}

void FaustSynth::parameterChanged (const juce::String& param, float value)
{
    if (param == gainID)
    {
      gain = value;
    } else
    if (param == attackID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("attack", value);
        }
    } else
    if (param == decayID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("decay", value);
        }
    } else
    if (param == sustainID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("sustain", value);
        }
    } else
    if (param == releaseID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("release", value);
        }
    } else
    if (param == osc1TypeID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("osc1Type", value);
        }
    } else
    if (param == osc2TypeID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("osc2Type", value);
        }
    } else
    if (param == osc2MixID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("osc2Mix", value);
        }
    } else
    if (param == detuneAmountID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("detuneAmount", value);
        }
    } else
    if (param == cutoffID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("cutoff", value);
        }
    } else
    if (param == filterTypeID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("filterType", value);
        }
    } else
    if (param == lfoDepthID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("lfoDepth", value);
        }
    } else
    if (param == lfoFreqID)
    {
        for(int nVoice=0; nVoice < NUM_VOICES; ++nVoice)
        {
            FaustVoice* synthVoice = dynamic_cast<FaustVoice *>(synth.getVoice(nVoice));
            synthVoice->setParam("lfoFreq", value);
        }
    }


}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaustSynth();
}
