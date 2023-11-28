/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FaustVoice.h"

#define NUM_VOICES 8

//==============================================================================
/**
*/
class FaustSynth : public foleys::MagicProcessor,
                                private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    FaustSynth();
    ~FaustSynth() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void parameterChanged (const juce::String& param, float value) override;
private:
    juce::Synthesiser synth;

    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::string osc1Types[4] = {"Sine", "Square", "Triangle", "Saw"};
    std::string osc2Types[4] = {"Sine", "Square", "Triangle", "Saw"};
    
    float gain = 0.0;

    foleys::MagicPlotSource*    osc1 = nullptr;
    foleys::MagicPlotSource*    osc2 = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaustSynth)
};
