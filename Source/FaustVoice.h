/*
 ==============================================================================
 
 FaustVoice.h
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "MonoSource.h"
class MonoSource;
class MapUI;


//==============================================================================
// This "Sound" Class acts as an ID meaning "any FaustVoice".
// Originally only sampling synthesis was implemented in JUCE,
// so a "Sound" represented some wavetable sounding on some MIDI channel(s)
// over some range of MIDI note numbers.
class FaustSound : public juce::SynthesiserSound
{
public:
  bool appliesToNote(int /*midiNoteNumber*/) {return true;}
  bool appliesToChannel(int /*midiNoteNumber*/) {return true;}
};
//==============================================================================

class FaustVoice : public juce::SynthesiserVoice
{
public:
  FaustVoice();
  ~FaustVoice();
  //==============================================================================
  bool canPlaySound(juce::SynthesiserSound* sound) {return dynamic_cast<FaustSound*>(sound) != nullptr;}
  void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition);
  void stopNote(float /*velocity*/, bool allowTailOff);
  
  //==============================================================================
  void pitchWheelMoved(int newPitchWheelValue) {}
  void controllerMoved(int controllerNumber, int newControllerValue) {}
  
  void renderNextBlock( juce::AudioBuffer<float> & buffer, int startSample, int numSample);
  //==============================================================================
  // Parameter Control functions
  void prepareToPlay(float sampleRate, int maxBlockSize);
  void setParam(std::string paramID, float newValue);
  float getParam(std::string paramID);
  
private:
    std::unique_ptr<::MapUI> UI;
    std::unique_ptr<::MonoSource> MonoSource;

    // Issue with having two ADSR's: how should we set tailOff here?
    int tailOff = 0;
};
