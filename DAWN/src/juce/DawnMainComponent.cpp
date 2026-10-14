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
    startTimerHz(15);
}

void DawnMainComponent::openFrontend() {
    if (browser_ == nullptr || opened_ || !isShowing()) return;
    const auto index = webRoot().getChildFile("index.html");
    if (navAttempts_ == 0) {
        const auto line = "DAWN frontend " + index.getFullPathName()
                        + (index.existsAsFile() ? " exists" : " MISSING")
                        + " url=" + juce::WebBrowserComponent::getResourceProviderRoot();
        DBG(line);
        juce::Logger::writeToLog(line);
        const auto logFile = index.getParentDirectory().getParentDirectory().getChildFile("webview.log");
        logFile.getParentDirectory().createDirectory();
        logFile.appendText(line + "\n");
    }
    browser_->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    opened_ = ++navAttempts_ >= 8;
}

DawnMainComponent::~DawnMainComponent() {
    stopTimer();
    masterRelay_.removeListener(this);
    bpmRelay_.removeListener(this);
    browser_ = nullptr;
}

void DawnMainComponent::resized() {
    if (browser_ != nullptr) browser_->setBounds(getLocalBounds());
    openFrontend();
}

void DawnMainComponent::visibilityChanged() { openFrontend(); }

juce::File DawnMainComponent::webRoot() const {
    const auto executable = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    const auto beside = juce::File(executable.getSiblingFile("DAWN_DATA").getChildFile("web").getFullPathName());
    if (beside.getChildFile("index.html").existsAsFile()) return beside;
    const auto portable = juce::File(AppPaths::current().root().getChildFile("DAWN_DATA").getChildFile("web").getFullPathName());
    if (portable.getChildFile("index.html").existsAsFile()) return portable;
#if defined(DAWN_WEB_ROOT)
    const auto source = juce::File(juce::File(DAWN_WEB_ROOT).getFullPathName());
    if (source.getChildFile("index.html").existsAsFile()) return source;
#endif
    return beside;
}

std::optional<juce::WebBrowserComponent::Resource> DawnMainComponent::resourceFor(const juce::String& path) const {
    auto relative = path;
    if (relative.containsIgnoreCase("juce.backend"))
        relative = relative.fromFirstOccurrenceOf("juce.backend", false, false);
    if (relative.isEmpty() || relative == "/") relative = "index.html";
    if (relative.startsWithChar('/')) relative = relative.substring(1);
    if (relative.contains("..")) return std::nullopt;

    const auto root = webRoot();
    const auto file = juce::File(root.getChildFile(relative).getFullPathName());
    const auto rootPrefix = root.getFullPathName() + juce::File::getSeparatorString();
    juce::MemoryBlock block;
    if (file.existsAsFile() && file.getFullPathName().startsWithIgnoreCase(rootPrefix) && file.loadFileAsData(block)) {
        std::vector<std::byte> bytes(block.getSize());
        if (!bytes.empty()) std::memcpy(bytes.data(), block.getData(), block.getSize());
        return juce::WebBrowserComponent::Resource{std::move(bytes), mimeFor(file.getFileName())};
    }

    const auto message = juce::String("<!DOCTYPE html><html><body style=\"background:#1a1a1a;color:#00ff00;font-family:sans-serif\">")
                       + "<h1>DAWN WEB UI MISSING</h1><p>"
                       + root.getChildFile("index.html").getFullPathName()
                       + "</p></body></html>";
    std::vector<std::byte> bytes(static_cast<std::size_t>(message.getNumBytesAsUTF8()));
    std::memcpy(bytes.data(), message.toRawUTF8(), bytes.size());
    return juce::WebBrowserComponent::Resource{std::move(bytes), juce::String("text/html")};
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
    auto options = juce::WebBrowserComponent::Options{}.withKeepPageLoadedWhenBrowserIsHidden();
#if JUCE_WINDOWS
    const auto userData = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                              .getParentDirectory()
                              .getChildFile("DAWN_DATA/webview2");
    userData.createDirectory();
    options = options.withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
                  .withWinWebView2Options(juce::WebBrowserComponent::Options::WinWebView2{}
                                              .withUserDataFolder(userData)
                                              .withBackgroundColour(juce::Colour(0xff1a1a1a)));
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
    openFrontend();
    if (browser_ == nullptr || !opened_) return;
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
