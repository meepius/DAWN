#include "juce/InputProfile.h"

namespace dawn {

InputProfile::InputProfile() {
    profile_ = juce::ValueTree{MapId::profile};
    profile_.setProperty(MapId::profileName, "Default", nullptr);
    state_.appendChild(profile_, nullptr);
    bind(0, "key", 65, "note", 60, 1);
    profile_.getChild(0).setProperty(MapId::source, "key", nullptr);
    bind(1, "button", 0, "note", 48, 1);
    bind(2, "axis", 5, "cc", 1, 1);
}

int InputProfile::size() const { return profile_.getNumChildren(); }

juce::ValueTree InputProfile::binding(int index) const { return profile_.getChild(index); }

void InputProfile::bind(int index, const juce::String& source, int code, const juce::String& kind, int number, int channel) {
    auto child = profile_.getChild(index);
    if (!child.isValid()) {
        child = juce::ValueTree{MapId::binding};
        profile_.appendChild(child, nullptr);
    }
    child.setProperty(MapId::source, source, nullptr);
    child.setProperty(MapId::code, code, nullptr);
    child.setProperty(MapId::kind, kind, nullptr);
    child.setProperty(MapId::number, number, nullptr);
    child.setProperty(MapId::channel, channel, nullptr);
}

std::vector<GamepadBinding> InputProfile::gamepadBindings() const {
    std::vector<GamepadBinding> bindings;
    for (int i = 0; i < profile_.getNumChildren(); ++i) {
        const auto child = profile_.getChild(i);
        const auto source = child[MapId::source].toString();
        if (source != "button" && source != "axis") continue;
        GamepadBinding binding;
        binding.control = static_cast<int>(child[MapId::code]);
        binding.axis = source == "axis";
        binding.channel = static_cast<int>(child[MapId::channel]) - 1;
        binding.number = static_cast<int>(child[MapId::number]);
        binding.cc = child[MapId::kind].toString() == "cc";
        bindings.push_back(binding);
    }
    return bindings;
}

bool InputProfile::loadFrom(const juce::File& file) {
    if (!file.existsAsFile()) return false;
    const auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr) return false;
    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.hasType(MapId::profiles) || tree.getNumChildren() == 0) return false;
    state_ = std::move(tree);
    profile_ = state_.getChild(0);
    return profile_.isValid();
}

bool InputProfile::saveTo(const juce::File& file) const {
    const auto xml = state_.createXml();
    if (xml == nullptr) return false;
    file.getParentDirectory().createDirectory();
    return xml->writeTo(file);
}

} // namespace dawn
