#include "Effects.h"
#include "DspUtils.h"

namespace steso::dsp
{
//==============================================================================
void Chorus::prepare (double sampleRate)
{
    sr = (float) sampleRate;
    dl.prepare ((int) (sr * 0.03f));
    dr.prepare ((int) (sr * 0.03f));
    reset();
}

void Chorus::reset()
{
    dl.reset();
    dr.reset();
    phase = 0.0f;
    mixSmoothed = 0.0f;
}

void Chorus::process (float* left, float* right, int numSamples, int mode)
{
    if (mode != 0)
        lastMode = mode;

    const float target = mode == 0 ? 0.0f : 1.0f;
    if (mode == 0 && mixSmoothed < 1.0e-4f)
    {
        mixSmoothed = 0.0f;
        return;
    }

    // Läge I: långsam och grund. Läge II: snabbare och djupare.
    const float rate   = lastMode == 1 ? 0.5f : 0.83f;
    const float depth  = (lastMode == 1 ? 1.8f : 2.6f) * 0.001f * sr;
    const float centre = 3.5f * 0.001f * sr;
    const float inc = rate / sr;
    const float smooth = 1.0f - std::exp (-1.0f / (0.02f * sr));

    for (int i = 0; i < numSamples; ++i)
    {
        mixSmoothed += (target - mixSmoothed) * smooth;

        // Triangel-LFO som på originalet.
        const float tri = 1.0f - 4.0f * std::abs (phase - 0.5f);
        phase += inc;
        if (phase >= 1.0f) phase -= 1.0f;

        const float inL = left[i], inR = right[i];
        const float mono = 0.5f * (inL + inR);
        dl.push (mono);
        dr.push (mono);

        const float wetL = dl.read (centre + depth * tri);
        const float wetR = dr.read (centre - depth * tri);

        left[i]  = inL + mixSmoothed * (wetL - 0.25f * inL) * 0.8f;
        right[i] = inR + mixSmoothed * (wetR - 0.25f * inR) * 0.8f;
    }
}

//==============================================================================
void StereoDelay::prepare (double sampleRate)
{
    sr = (float) sampleRate;
    dl.prepare ((int) (sr * 1.3f));
    dr.prepare ((int) (sr * 1.3f));
    time.reset (sampleRate, 0.25);
    fb.reset (sampleRate, 0.05);
    wet.reset (sampleRate, 0.05);
    lpCoef = 1.0f - std::exp (-twoPi * 4500.0f / sr);
    reset();
}

void StereoDelay::reset()
{
    dl.reset();
    dr.reset();
    lpL = lpR = 0.0f;
    wet.setCurrentAndTargetValue (0.0f);
}

void StereoDelay::process (float* left, float* right, int numSamples, float timeSeconds, float feedback, float mix)
{
    time.setTargetValue (timeSeconds * sr);
    fb.setTargetValue (feedback);
    wet.setTargetValue (mix);

    if (mix <= 0.0f && ! wet.isSmoothing() && wet.getCurrentValue() <= 0.0f)
    {
        // Håll linjerna tomma så att inget gammalt eko dyker upp när Mix vrids upp.
        for (int i = 0; i < numSamples; ++i)
        {
            dl.push (0.0f);
            dr.push (0.0f);
        }
        time.skip (numSamples);
        fb.skip (numSamples);
        return;
    }

    for (int i = 0; i < numSamples; ++i)
    {
        const float d = std::max (1.0f, time.getNextValue());
        const float f = fb.getNextValue();
        const float w = wet.getNextValue();

        // Lätt ping-pong: höger kanal är något längre.
        const float yl = dl.read (d);
        const float yr = dr.read (d * 1.02f);

        lpL += (yl - lpL) * lpCoef;
        lpR += (yr - lpR) * lpCoef;

        const float inL = left[i], inR = right[i];
        dl.push (softClip (inL + lpR * f));
        dr.push (softClip (inR + lpL * f));

        left[i]  = inL + w * yl;
        right[i] = inR + w * yr;
    }

    if (std::abs (lpL) < 1.0e-15f) lpL = 0.0f;
    if (std::abs (lpR) < 1.0e-15f) lpR = 0.0f;
}

//==============================================================================
void Reverb::prepare (double sampleRate)
{
    reverb.setSampleRate (sampleRate);
    juce::Reverb::Parameters p;
    p.roomSize = 0.72f;
    p.damping = 0.45f;
    p.wetLevel = 1.0f;
    p.dryLevel = 0.0f;
    p.width = 1.0f;
    p.freezeMode = 0.0f;
    reverb.setParameters (p);
    wet.reset (sampleRate, 0.05);
    wetL.assign (2048, 0.0f);
    wetR.assign (2048, 0.0f);
    reset();
}

void Reverb::reset()
{
    reverb.reset();
    wet.setCurrentAndTargetValue (0.0f);
}

void Reverb::process (float* left, float* right, int numSamples, float mix)
{
    wet.setTargetValue (mix);
    if (mix <= 0.0f && ! wet.isSmoothing() && wet.getCurrentValue() <= 0.0f)
    {
        reverb.reset();
        return;
    }

    int done = 0;
    while (done < numSamples)
    {
        const int n = std::min (numSamples - done, (int) wetL.size());
        std::copy (left + done, left + done + n, wetL.begin());
        std::copy (right + done, right + done + n, wetR.begin());
        reverb.processStereo (wetL.data(), wetR.data(), n);

        for (int i = 0; i < n; ++i)
        {
            const float w = wet.getNextValue();
            left[done + i]  = left[done + i]  * (1.0f - 0.35f * w) + wetL[(size_t) i] * w * 0.6f;
            right[done + i] = right[done + i] * (1.0f - 0.35f * w) + wetR[(size_t) i] * w * 0.6f;
        }
        done += n;
    }
}
} // namespace steso::dsp
