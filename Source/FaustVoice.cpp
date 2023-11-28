#include "FaustVoice.h"

FaustVoice::FaustVoice()
{
    MonoSource = std::make_unique<::MonoSource>();
    UI = std::make_unique<::MapUI>();
    MonoSource->buildUserInterface(UI.get());
    MonoSource->init(getSampleRate());
}

FaustVoice::~FaustVoice()
{
}

void FaustVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    setParam("midiNote", midiNoteNumber);
    setParam("velocity", velocity);
    setParam("gate", 1);
    tailOff = 0;
}

void FaustVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    setParam("gate", 0);
    if (allowTailOff)
    {
        float releaseTimeSec = UI->getParamValue("release");
        tailOff = int(std::ceil(releaseTimeSec * getSampleRate()));
    }
    else
    {
      clearCurrentNote();
    }
}

void FaustVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples)
{
    int nChannelsOut = outputBuffer.getNumChannels();
    int nChannelsFaust = MonoSource->getNumOutputs();

    juce::AudioBuffer<float> tmpBuffer = juce::AudioBuffer<float>(nChannelsFaust, numSamples);
#if 1
    float*const* writePointers = tmpBuffer.getArrayOfWritePointers();
    float* tmpBuffersFaust[nChannelsFaust];
    for (int i=0; i<nChannelsFaust; i++)
    {
        tmpBuffersFaust[i] = writePointers[i];
    }
    MonoSource->compute(numSamples, NULL, tmpBuffersFaust);
#else
    MonoSource->compute(numSamples, NULL, const_cast<float**>(tmpBuffer.getArrayOfWritePointers()));
#endif

    for (int dstChan = 0; dstChan < nChannelsOut; ++dstChan)
    {
        int srcChan = std::min(nChannelsFaust-1,dstChan);
        outputBuffer.addFrom(dstChan, startSample, tmpBuffer, srcChan, 0, numSamples, 1.0);
    }

    if (tailOff > 0)
    {
        tailOff -= numSamples;
        if (tailOff <= 0)
        {
            clearCurrentNote();
            tailOff = 0;
        }
    }
}

void FaustVoice::setParam(std::string paramID, float newValue)
{
    UI->setParamValue(paramID, newValue);
}

float FaustVoice::getParam(std::string paramID)
{
    float value = UI->getParamValue(paramID);
    return value;
}
