#pragma once

#include "engine/GamepadMap.h"

#include <juce_data_structures/juce_data_structures.h>
#include <vector>

namespace dawn {

namespace MapId {
inline const juce::Identifier profiles{"inputProfiles"};
inline const juce::Identifier profile{"profile"};
inline const juce::Identifier binding{"binding"};
inline const juce::Identifier profileName{"name"};
inline const juce::Identifier source{"source"};
inline const juce::Identifier code{"code"};
inline const juce::Identifier kind{"kind"};
inline const juce::Identifier number{"number"};
inline const juce::Identifier channel{"channel"};
} // namespace MapId

// Separate from the project tree so a mapping edit does not rebuild the mixer.
class InputProfile {
public:
    InputProfile();

    juce::ValueTree& state() noexcept { return state_; }
    int size() const;
    juce::ValueTree binding(int index) const;
    void bind(int index, const juce::String& source, int code, const juce::String& kind, int number, int channel);
    std::vector<GamepadBinding> gamepadBindings() const;
    bool loadFrom(const juce::File& file);
    bool saveTo(const juce::File& file) const;

private:
    juce::ValueTree state_{MapId::profiles};
    juce::ValueTree profile_;
};

} // namespace dawn
