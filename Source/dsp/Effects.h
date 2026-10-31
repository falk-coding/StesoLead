#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace steso::dsp
{
// Stereo-delaylinje med linjär interpolation.
class DelayLine
{
public:
    void prepare (int maxSamples)
    {
        buffer.assign ((size_t) maxSamples + 4, 0.0f);
        writePos = 0;
    }

    void reset() { std::fill (buffer.begin(), buffer.end(), 0.0f); writePos = 0; }

    void push (float x) noexcept
    {
        buffer[(size_t) writePos] = x;
        if (++writePos >= (int) buffer.size()) writePos = 0;
    }

    // delay i sampel (>= 1). Läser relativt senast skrivna sampel.
    float read (float delay) const noexcept
    {
        const int size = (int) buffer.size();
        float rp = (float) writePos - delay;
        while (rp < 0.0f) rp += (float) size;
        const int i0 = (int) rp;
        const int i1 = (i0 + 1) % size;
        const float frac = rp - (float) i0;
        return buffer[(size_t) (i0 % size)] + frac * (buffer[(size_t) i1] - buffer[(size_t) (i0 % size)]);
    }

private:
    std::vector<float> buffer;
    int writePos = 0;
};

// Chorus i Juno-stil: två lägen, motfasade LFO:er för vänster/höger.
class Chorus
{
public:
    void prepare (double sampleRate);
    void reset();
    void process (float* left, float* right, int numSamples, int mode);

private:
    float sr = 44100.0f;
    DelayLine dl, dr;
    float phase = 0.0f;
    float mixSmoothed = 0.0f;
    int lastMode = 0;
};

// Stereo-delay med mjuk återkoppling och lågpass i loopen (analog känsla).
class StereoDelay
{
public:
    void prepare (double sampleRate);
    void reset();
    void process (float* left, float* right, int numSamples, float timeSeconds, float feedback, float mix);

private:
    float sr = 44100.0f;
    DelayLine dl, dr;
    juce::SmoothedValue<float> time, fb, wet;
    float lpL = 0.0f, lpR = 0.0f, lpCoef = 0.5f;
};

class Reverb
{
public:
    void prepare (double sampleRate);
    void reset();
    void process (float* left, float* right, int numSamples, float mix);

private:
    juce::Reverb reverb;
    juce::SmoothedValue<float> wet;
    std::vector<float> wetL, wetR;
};
} // namespace steso::dsp
