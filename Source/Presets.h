#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace steso
{
struct Preset
{
    juce::String name;
    juce::String category;
    // Bara avvikelser från standardvärdena (i riktiga enheter: Hz, sekunder, 0..1, index).
    std::vector<std::pair<const char*, float>> values;
};

const std::vector<Preset>& getFactoryPresets();

// Sätter alla parametrar: först standardvärden, sedan presetens värden.
void applyPreset (juce::AudioProcessorValueTreeState& apvts, const Preset& preset);
} // namespace steso
