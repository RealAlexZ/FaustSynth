declare name "monoSource";
declare description "Monophonic midi waveform generator";

import("stdfaust.lib");

process = monoSource;

monoSource = env * ((1 - osc2Mix) * osc1Wave + osc2Mix * osc2Wave) <: filter
with
{
    osc2Mix = nentry("osc2Mix", 0, 0, 1, 0.01);
    vel = nentry("velocity", 1, 0, 1, 0.01);
    gate = button("gate");
    freqHz = ba.midikey2hz(int(nentry("midiNote", 60, 36, 96, 1)));

    osc1Select = int(nentry("osc1Type", 0, 0, 3, 1));
    osc2Select = int(nentry("osc2Type", 0, 0, 3, 1));

    attack = nentry("attack", 0.1, 0, 1, 0.01);
    decay = nentry("decay", 0.25, 0, 1, 0.01);
    sustain = nentry("sustain", 0.5, 0, 1, 0.01);
    release = nentry("release", 1, 0, 5, 0.01);
    env = vel * en.adsre(attack, decay, sustain, release, gate);

    sqr = os.square(freqHz);
    sine = os.osc(freqHz);
    tri = os.triangle(freqHz);
    saw = os.sawtooth(freqHz);

    osc1Wave = (sine * (osc1Select == 0) +
                sqr * (osc1Select == 1) +
                tri * (osc1Select == 2) +
                saw * (osc1Select == 3));

    detuneAmount = nentry("detuneAmount", 0, -100, 100, 1);
    detuneRatio = pow(2.0, detuneAmount / 1200);
    detunedFreqHz = freqHz * detuneRatio;

    detunedSqr = os.square(detunedFreqHz);
    detunedSine = os.osc(detunedFreqHz);
    detunedTri = os.triangle(detunedFreqHz);
    detunedSaw = os.sawtooth(detunedFreqHz);

    osc2Wave = (detunedSine * (osc2Select == 0) +
                detunedSqr * (osc2Select == 1) +
                detunedTri * (osc2Select == 2) +
                detunedSaw * (osc2Select == 3));

    filterTypeSelect = int(nentry("filterType", 0, 0, 2, 1));
    cutoff = nentry("cutoff", 1000, 20, 20000, 1);

    lfoFreq = nentry("lfoFreq", 1, 0.1, 10, 0.1);
    lfoDepth = nentry("lfoDepth", 0, 0, 1000, 1);
    lfoWave = os.osc(lfoFreq);

    lowpass = fi.lowpass(3, cutoff + lfoDepth * lfoWave);
    highpass = fi.highpass(3, cutoff + lfoDepth * lfoWave);

    filter = select3(filterTypeSelect, _, lowpass, highpass);
};
