#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <array>
#include <iterator>

namespace steso::dsp
{
inline constexpr float twoPi = 6.283185307179586f;

// Snabb tanh-approximation (Padé), mjuk mättning. Exakt ±1 vid |x| >= 3.
inline float softClip (float x) noexcept
{
    x = std::clamp (x, -3.0f, 3.0f);
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

// PolyBLEP-korrektion för bandbegränsade språng (saw/square).
inline float polyBlep (float t, float dt) noexcept
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

inline float midiToHz (float note) noexcept
{
    return 440.0f * std::exp2 ((note - 69.0f) / 12.0f);
}

// Liten, snabb slumpgenerator (xorshift32) – realtidssäker.
struct FastRandom
{
    uint32_t state = 0x9E3779B9u;

    void seed (uint32_t s) noexcept { state = s == 0 ? 0x9E3779B9u : s; }

    uint32_t next() noexcept
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state;
    }

    // -1 .. 1
    float bipolar() noexcept { return (float) (next() >> 8) * (2.0f / 16777216.0f) - 1.0f; }
    // 0 .. 1
    float unipolar() noexcept { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
};

// Bandbegränsad oscillator: saw, square (PWM), triangle, sine.
struct Oscillator
{
    enum Wave { saw = 0, square, triangle, sine };

    float phase = 0.0f;

    float process (int wave, float dt, float pw) noexcept
    {
        float out = 0.0f;
        const float t = phase;

        switch (wave)
        {
            case saw:
                out = 2.0f * t - 1.0f - polyBlep (t, dt);
                break;
            case square:
            {
                out = t < pw ? 1.0f : -1.0f;
                out += polyBlep (t, dt);
                float t2 = t + 1.0f - pw;
                if (t2 >= 1.0f) t2 -= 1.0f;
                out -= polyBlep (t2, dt);
                // Ta bort DC som uppstår vid smal pulsbredd.
                out -= (2.0f * pw - 1.0f);
                break;
            }
            case triangle:
                out = 4.0f * std::abs (t - 0.5f) - 1.0f;
                break;
            case sine:
            default:
                out = std::sin (twoPi * t);
                break;
        }

        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }
};

// Exponentiell ADSR i analog stil (RC-kurvor). Retriggas från aktuell nivå (inga klick).
struct Envelope
{
    enum class Stage { idle, attack, decay, release };

    Stage stage = Stage::idle;
    float level = 0.0f;
    float attackCoef = 0.0f, decayCoef = 0.0f, releaseCoef = 0.0f, sustain = 1.0f;

    static constexpr float attackTarget = 1.3f;

    void setParameters (float sampleRate, float a, float d, float s, float r) noexcept
    {
        // Attack: når 1.0 på a sekunder när målet är 1.3 (överskjutning ger snabbare RC-kurva).
        attackCoef  = std::exp (-1.466f / (std::max (a, 0.0005f) * sampleRate));
        // Decay/release: ~99 % framme på angiven tid.
        decayCoef   = std::exp (-4.6f / (std::max (d, 0.001f) * sampleRate));
        releaseCoef = std::exp (-4.6f / (std::max (r, 0.001f) * sampleRate));
        sustain = s;
    }

    void noteOn() noexcept { stage = Stage::attack; }
    void noteOff() noexcept { if (stage != Stage::idle) stage = Stage::release; }
    void reset() noexcept { stage = Stage::idle; level = 0.0f; }
    bool isActive() const noexcept { return stage != Stage::idle; }

    float process() noexcept
    {
        switch (stage)
        {
            case Stage::attack:
                level = attackTarget + (level - attackTarget) * attackCoef;
                if (level >= 1.0f)
                {
                    level = 1.0f;
                    stage = Stage::decay;
                }
                break;
            case Stage::decay:
                level = sustain + (level - sustain) * decayCoef;
                break;
            case Stage::release:
                level *= releaseCoef;
                if (level < 1.0e-4f)
                {
                    level = 0.0f;
                    stage = Stage::idle;
                }
                break;
            case Stage::idle:
            default:
                break;
        }
        return level;
    }
};

// 4-poligt lågpassfilter av ladder-typ (Moog-stil), ZDF/TPT med mättning i återkopplingen.
struct LadderFilter
{
    float s[4] {};

    void reset() noexcept { s[0] = s[1] = s[2] = s[3] = 0.0f; }

    // g = tan(pi * fc / fs), k = 0..4 (självsvängning nära 4)
    float process (float x, float g, float k) noexcept
    {
        const float G = g / (1.0f + g);
        const float oneMinusG = 1.0f - G;
        const float G2 = G * G, G3 = G2 * G, G4 = G3 * G;

        const float sigma = G3 * oneMinusG * s[0] + G2 * oneMinusG * s[1]
                          + G  * oneMinusG * s[2] +      oneMinusG * s[3];

        const float y4Estimate = (G4 * x + sigma) / (1.0f + k * G4);
        float u = softClip (x - k * y4Estimate);

        for (int i = 0; i < 4; ++i)
        {
            const float v = (u - s[i]) * G;
            const float y = v + s[i];
            s[i] = y + v;
            u = y;
        }

        // Undvik denormaler i svansen.
        for (auto& st : s)
            if (std::abs (st) < 1.0e-15f) st = 0.0f;

        // Kompensera basförlust vid hög resonans (som på originalen, fast mildare).
        return u * (1.0f + 0.45f * k);
    }
};

// Polivoks-inspirerat 12 dB-filter: tillståndsvariabelt (SVF/TPT) där integratorerna mättar
// (op-förstärkare som slår i matningen) -> rått, "skrikigt" resonansläge. LP eller BP.
struct PolivoksFilter
{
    float s1 = 0.0f, s2 = 0.0f;

    void reset() noexcept { s1 = s2 = 0.0f; }

    // g = tan(pi * fc / fs), res = 0..1
    float process (float x, float g, float res, bool bandpass) noexcept
    {
        const float k = 2.0f - 1.96f * std::clamp (res, 0.0f, 1.0f);
        x = softClip (x * 0.8f) * 1.25f;

        const float hp = (x - (k + g) * s1 - s2) / (1.0f + g * (k + g));
        const float v1 = g * hp;
        float bp = v1 + s1;
        // Mättande integrator: begränsar och färgar resonansen.
        bp = softClip (bp * 0.5f) * 2.0f;
        s1 = bp + v1;
        const float v2 = g * bp;
        const float lp = v2 + s2;
        s2 = softClip ((lp + v2) * 0.4f) * 2.5f;

        if (std::abs (s1) < 1.0e-15f) s1 = 0.0f;
        if (std::abs (s2) < 1.0e-15f) s2 = 0.0f;

        return bandpass ? bp * (0.45f + 0.35f * k) : lp;
    }
};

// Halvbands-FIR för 2x -> 1x nedsampling (Kaiser-fönster, ~80 dB spärrband).
class HalfbandDecimator
{
public:
    static constexpr int numTaps = 47;

    HalfbandDecimator()
    {
        constexpr int centre = numTaps / 2;
        constexpr double beta = 8.0;
        auto besselI0 = [] (double x)
        {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 30; ++k)
            {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
            }
            return sum;
        };

        double total = 0.0;
        for (int n = 0; n < numTaps; ++n)
        {
            const int m = n - centre;
            const double sinc = m == 0 ? 0.5 : std::sin (3.14159265358979 * 0.5 * m) / (3.14159265358979 * m);
            const double r = (double) m / centre;
            const double w = besselI0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / besselI0 (beta);
            taps[(size_t) n] = (float) (sinc * w);
            total += sinc * w;
        }
        for (auto& t : taps)
            t = (float) (t / total);
    }

    void reset() noexcept { std::fill (std::begin (history), std::end (history), 0.0f); pos = 0; }

    // in: 2*n sampel i 2x-takt, out: n sampel.
    void process (const float* in, float* out, int n) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            push (in[2 * i]);
            push (in[2 * i + 1]);

            // history[pos .. pos+numTaps) är de senaste numTaps sampelna (dubbelbuffert).
            const float* h = history + pos;
            float acc = 0.0f;
            for (int t = 0; t < numTaps; ++t)
                acc += taps[(size_t) t] * h[t];
            out[i] = acc;
        }
    }

private:
    void push (float x) noexcept
    {
        history[pos] = x;
        history[pos + numTaps] = x;
        if (++pos >= numTaps) pos = 0;
    }

    std::array<float, numTaps> taps {};
    float history[2 * numTaps] {};
    int pos = 0;
};

// Enkel DC-spärr (högpass ~10 Hz).
struct DcBlocker
{
    float x1 = 0.0f, y1 = 0.0f, r = 0.9987f;

    void prepare (float sampleRate) noexcept { r = 1.0f - (twoPi * 10.0f / sampleRate); }
    void reset() noexcept { x1 = y1 = 0.0f; }

    float process (float x) noexcept
    {
        const float y = x - x1 + r * y1;
        x1 = x;
        y1 = std::abs (y) < 1.0e-15f ? 0.0f : y;
        return y1;
    }
};
} // namespace steso::dsp
