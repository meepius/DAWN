#include "juce/DawnMainComponent.h"

#include "juce/AppPaths.h"

#include <cmath>
#include <cstring>
#include <vector>

namespace dawn {
namespace {
const juce::Identifier kArm{"arm"};

juce::String mimeFor(const juce::String& path) {
    if (path.endsWithIgnoreCase(".css")) return "text/css";
    if (path.endsWithIgnoreCase(".js")) return "text/javascript";
    if (path.endsWithIgnoreCase(".svg")) return "image/svg+xml";
    if (path.endsWithIgnoreCase(".json")) return "application/json";
    return "text/html";
}

juce::var objectArg(const juce::var& payload, const juce::Identifier& key) {
    if (auto* object = payload.getDynamicObject()) return object->getProperty(key);
    return {};
}
} // namespace

DawnMainComponent::DawnMainComponent(ProjectState& state, AudioEngine& engine, VisualizerDataQueue&, LiveMidiInjector& live)
    : state_(state), engine_(engine), live_(live), project_(state.state()) {
    int count = 0;
    for (auto child : project_) {
        if (!child.hasType(Id::track) || count >= kColumns) continue;
        tracks_[count] = child;
        int scene = 0;
        for (auto clip : child) {
            if (!clip.hasType(Id::clip) || scene >= kScenes) continue;
            clips_[count][scene++] = clip;
        }
        ++count;
    }
    trackCount_ = count;
    engine_.setMasterGain(0.85f);

    masterRelay_.addListener(this);
    bpmRelay_.addListener(this);
    browser_ = std::make_unique<juce::WebBrowserComponent>(browserOptions());
    addAndMakeVisible(*browser_);
    const auto index = webRoot().getChildFile("index.html");
    if (index.existsAsFile())
        browser_->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    startTimerHz(15);
}

DawnMainComponent::~DawnMainComponent() {
    stopTimer();
    masterRelay_.removeListener(this);
    bpmRelay_.removeListener(this);
    browser_ = nullptr;
}

void DawnMainComponent::resized() {
    if (browser_ != nullptr) browser_->setBounds(getLocalBounds());
}

juce::File DawnMainComponent::webRoot() const {
    auto absolute = [](juce::File directory) { return juce::File(directory.getFullPathName()); };
    const auto candidates = {
        absolute(juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("DAWN_DATA/web")),
        absolute(AppPaths::current().root().getChildFile("web")),
        absolute(AppPaths::current().root().getChildFile("DAWN_DATA/web")),
#if defined(DAWN_WEB_ROOT)
        absolute(juce::File(DAWN_WEB_ROOT)),
#endif
    };
    for (const auto& directory : candidates)
        if (directory.getChildFile("index.html").existsAsFile()) return directory;
    return candidates.begin() != candidates.end() ? *candidates.begin() : juce::File();
}

std::optional<juce::WebBrowserComponent::Resource> DawnMainComponent::resourceFor(const juce::String& path) const {
    if (path.contains("..")) return std::nullopt;
    auto relative = path == "/" ? juce::String("index.html") : path;
    if (relative.startsWithChar('/')) relative = relative.substring(1);
    const auto root = webRoot();
    const auto file = juce::File(root.getChildFile(relative).getFullPathName());
    if (!file.existsAsFile() || !file.isAChildOf(root)) return std::nullopt;
    juce::MemoryBlock block;
    if (!file.loadFileAsData(block)) return std::nullopt;
    std::vector<std::byte> bytes(block.getSize());
    if (!bytes.empty()) std::memcpy(bytes.data(), block.getData(), block.getSize());
    return juce::WebBrowserComponent::Resource{std::move(bytes), mimeFor(file.getFileName())};
}

juce::String DawnMainComponent::sessionJson() const {
    juce::Array<juce::var> tracks;
    for (int i = 0; i < trackCount_; ++i) {
        auto* track = new juce::DynamicObject();
        track->setProperty("name", tracks_[i][Id::name].toString());
        juce::Array<juce::var> clips;
        for (int scene = 0; scene < kScenes; ++scene) {
            auto* clip = new juce::DynamicObject();
            const auto tree = clips_[i][scene];
            clip->setProperty("title", tree.isValid() ? tree[Id::userNotes].toString() : juce::String());
            clip->setProperty("launched", tree.isValid() && static_cast<bool>(tree[Id::launched]));
            clips.add(juce::var(clip));
        }
        track->setProperty("clips", clips);
        tracks.add(juce::var(track));
    }
    return juce::JSON::toString(juce::var(tracks), true);
}

juce::WebBrowserComponent::Options DawnMainComponent::browserOptions() {
    auto options = juce::WebBrowserComponent::Options{};
#if JUCE_WINDOWS
    options = options.withBackend(juce::WebBrowserComponent::Options::Backend::webview2);
#endif
    options = options.withNativeIntegrationEnabled()
                       .withOptionsFrom(masterRelay_)
                       .withOptionsFrom(bpmRelay_)
                       .withResourceProvider([this](const juce::String& path) { return resourceFor(path); })
                       .withInitialisationData("session", sessionJson())
                       .withInitialisationData("bpm", project_[Id::bpm])
                       .withInitialisationData("master", 0.85)
                       .withNativeFunction("togglePlay", [this](const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion done) {
                           const bool playing = static_cast<int>(project_[Id::command]) == static_cast<int>(TransportCommand::Play);
                           if (playing) state_.stop();
                           else state_.play(quantized(engine_.playhead()));
                           if (done != nullptr) done(!playing);
                       })
                       .withEventListener("play", [this](juce::var) { state_.play(quantized(engine_.playhead())); })
                       .withEventListener("stop", [this](juce::var) { state_.stop(); })
                       .withEventListener("playClicked", [this](juce::var) { state_.play(quantized(engine_.playhead())); })
                       .withEventListener("stopClicked", [this](juce::var) { state_.stop(); })
                       .withEventListener("slider_moved", [this](juce::var payload) {
                           const auto id = objectArg(payload, "id").toString();
                           const auto value = static_cast<double>(objectArg(payload, "value"));
                           if (id == "master") engine_.setMasterGain(static_cast<float>(value));
                           else if (id == "bpm") project_.setProperty(Id::bpm, value, nullptr);
                           else if (id == "level" && trackCount_ > 0) tracks_[0].setProperty(Id::volume, static_cast<float>(value), nullptr);
                       })
                       .withEventListener("launchClip", [this](juce::var payload) {
                           launchClip(static_cast<int>(objectArg(payload, "track")), static_cast<int>(objectArg(payload, "scene")));
                       })
                       .withEventListener("trackFlag", [this](juce::var payload) {
                           setTrackFlag(static_cast<int>(objectArg(payload, "track")), objectArg(payload, "flag").toString(),
                                        static_cast<bool>(objectArg(payload, "value")));
                       })
                       .withEventListener("pad", [this](juce::var payload) {
                           const int note = static_cast<int>(objectArg(payload, "note"));
                           if (static_cast<bool>(objectArg(payload, "down")))
                               live_.push(juce::MidiMessage::noteOn(1, note, static_cast<juce::uint8>(112)));
                           else live_.push(juce::MidiMessage::noteOff(1, note));
                       });
    return options;
}

void DawnMainComponent::sliderValueChanged(juce::WebSliderRelay* relay) {
    if (relay == &masterRelay_) engine_.setMasterGain(relay->getValue());
    else if (relay == &bpmRelay_) project_.setProperty(Id::bpm, static_cast<double>(relay->getValue()), nullptr);
}

void DawnMainComponent::initialUpdateRequested(juce::WebSliderRelay* relay) {
    if (relay == &masterRelay_) relay->setValue(0.85f);
    else if (relay == &bpmRelay_) relay->setValue(static_cast<float>(static_cast<double>(project_[Id::bpm])));
}

void DawnMainComponent::timerCallback() {
    if (browser_ == nullptr) return;
    const bool playing = static_cast<int>(project_[Id::command]) == static_cast<int>(TransportCommand::Play);
    juce::String script;
    script << "window.dawn && window.dawn.setTransport(" << (playing ? "true" : "false") << "," << engine_.playhead() << ")";
    browser_->evaluateJavascript(script);
}

void DawnMainComponent::launchClip(int track, int scene) {
    if (track < 0 || track >= trackCount_ || scene < 0 || scene >= kScenes) return;
    auto clip = clips_[track][scene];
    if (!clip.isValid()) return;
    const bool next = !static_cast<bool>(clip[Id::launched]);
    const double at = quantized(engine_.playhead());
    state_.setLaunched(clip, next, at);
    if (next && static_cast<int>(project_[Id::command]) != static_cast<int>(TransportCommand::Play)) state_.play(at);
}

void DawnMainComponent::setTrackFlag(int track, const juce::String& flag, bool value) {
    if (track < 0 || track >= trackCount_) return;
    juce::Identifier id;
    if (flag == "mute") id = Id::mute;
    else if (flag == "solo") id = Id::solo;
    else if (flag == "arm") id = kArm;
    else return;
    tracks_[track].setProperty(id, value, nullptr);
}

double DawnMainComponent::quantized(double beat) const {
    const double grid = 4.0;
    if (beat < 0.0) beat = 0.0;
    return std::floor(beat / grid) * grid;
}

} // namespace dawn
