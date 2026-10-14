#include "juce/DawnMainComponent.h"

#include <cmath>

namespace dawn {
namespace {
const juce::Colour kBg{0xff121418};
const juce::Colour kPanel{0xff1a1a1a};
const juce::Colour kLime{0xffccff00};
const juce::Colour kClipColours[] = {juce::Colour(0xffccff00), juce::Colour(0xffff7a18), juce::Colour(0xffb44cff), juce::Colour(0xff3ec1ff)};
const int kPadNotes[] = {36, 38, 42, 46, 39, 45, 37, 49};
const char* kPadNames[] = {"Kick", "Snare", "Hat", "Open", "Clap", "Tom", "Rim", "Crash"};
const char* kMacroNames[] = {"Tune", "Decay", "Snap", "Hat", "Tone", "Level"};
const juce::Identifier kArm{"arm"};
const juce::Identifier kLoop{"loop"};

juce::String positionText(double beats) {
    if (beats < 0.0) beats = 0.0;
    const int bar = static_cast<int>(beats / 4.0) + 1;
    const double inside = std::fmod(beats, 4.0);
    const int beat = static_cast<int>(inside) + 1;
    const int tick = static_cast<int>((inside - std::floor(inside)) * 4.0) + 1;
    return juce::String(bar) + "." + juce::String(beat) + "." + juce::String(tick);
}
} // namespace

class DawnMainComponent::TransportBar : public juce::Component {
public:
    void bind(DawnMainComponent& owner) {
        owner_ = &owner;
        for (auto* button : {&play_, &stop_, &record_, &loop_, &click_}) addAndMakeVisible(*button);
        addAndMakeVisible(bpm_);
        addAndMakeVisible(position_);
        addAndMakeVisible(quantize_);
        addAndMakeVisible(master_);

        play_.setButtonText("Play");
        stop_.setButtonText("Stop");
        record_.setButtonText("Record");
        loop_.setButtonText("Loop");
        click_.setButtonText("Click");
        record_.setClickingTogglesState(true);
        loop_.setClickingTogglesState(true);
        click_.setClickingTogglesState(true);
        loop_.setToggleState(true, juce::dontSendNotification);

        bpm_.setSliderStyle(juce::Slider::LinearBar);
        bpm_.setRange(40.0, 220.0, 0.1);
        bpm_.setValue(static_cast<double>(owner.project_[Id::bpm]), juce::dontSendNotification);
        bpm_.setTextValueSuffix(" BPM");

        position_.setJustificationType(juce::Justification::centred);
        position_.setText("1.1.1", juce::dontSendNotification);
        position_.setColour(juce::Label::backgroundColourId, juce::Colour(0xff07080c));
        position_.setColour(juce::Label::textColourId, kLime);

        quantize_.addItem("None", 1);
        quantize_.addItem("1 Bar", 2);
        quantize_.addItem("1/2", 3);
        quantize_.addItem("1/4", 4);
        quantize_.addItem("1/8", 5);
        quantize_.addItem("1/16", 6);
        quantize_.setSelectedId(2, juce::dontSendNotification);

        master_.setSliderStyle(juce::Slider::LinearHorizontal);
        master_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        master_.setRange(0.0, 1.0, 0.001);
        master_.setValue(0.85, juce::dontSendNotification);
        owner.engine_.setMasterGain(0.85f);

        play_.onClick = [this] {
            const double at = owner_->quantized(owner_->engine_.playhead());
            owner_->state_.play(at);
        };
        stop_.onClick = [this] { owner_->state_.stop(); };
        record_.onClick = [this] {
            const bool armed = record_.getToggleState();
            owner_->engine_.setRecording(armed);
            if (armed) owner_->state_.play(owner_->quantized(owner_->engine_.playhead()));
        };
        loop_.onClick = [this] { owner_->project_.setProperty(kLoop, loop_.getToggleState(), nullptr); };
        click_.onClick = [this] { owner_->engine_.setMetronome(click_.getToggleState()); };
        bpm_.onValueChange = [this] { owner_->project_.setProperty(Id::bpm, bpm_.getValue(), nullptr); };
        master_.onValueChange = [this] { owner_->engine_.setMasterGain(static_cast<float>(master_.getValue())); };
    }

    int quantizeId() const { return quantize_.getSelectedId(); }

    void setPlaying(bool playing) { play_.setToggleState(playing, juce::dontSendNotification); }

    void setPosition(const juce::String& text) { position_.setText(text, juce::dontSendNotification); }

    void paint(juce::Graphics& graphics) override {
        graphics.fillAll(kPanel);
        graphics.setColour(kLime.withAlpha(0.85f));
        graphics.fillRect(0, getHeight() - 2, getWidth(), 2);
    }

    void resized() override {
        juce::FlexBox row;
        row.flexDirection = juce::FlexBox::Direction::row;
        row.alignItems = juce::FlexBox::AlignItems::center;
        auto item = [](juce::Component& component, float width) {
            return juce::FlexItem(component).withWidth(width).withHeight(34.0f).withMargin(4.0f);
        };
        row.items.add(item(play_, 72.0f));
        row.items.add(item(stop_, 72.0f));
        row.items.add(item(record_, 84.0f));
        row.items.add(item(loop_, 72.0f));
        row.items.add(item(click_, 72.0f));
        row.items.add(item(bpm_, 150.0f));
        row.items.add(item(position_, 92.0f));
        row.items.add(item(quantize_, 110.0f));
        row.items.add(juce::FlexItem(master_).withFlex(1.0f).withHeight(34.0f).withMargin(4.0f));
        row.performLayout(getLocalBounds().reduced(8, 6).toFloat());
    }

private:
    DawnMainComponent* owner_ = nullptr;
    juce::TextButton play_, stop_, record_, loop_, click_;
    juce::Slider bpm_, master_;
    juce::Label position_;
    juce::ComboBox quantize_;
};

class DawnMainComponent::SessionView : public juce::Component {
public:
    class Slot : public juce::Button {
    public:
        Slot() : juce::Button({}) {}

        void setClip(juce::ValueTree clip, juce::String title, juce::Colour colour) {
            clip_ = std::move(clip);
            title_ = std::move(title);
            colour_ = colour;
            setEnabled(clip_.isValid());
            repaint();
        }

        void paintButton(juce::Graphics& graphics, bool highlighted, bool down) override {
            auto bounds = getLocalBounds().toFloat().reduced(1.0f);
            const bool on = clip_.isValid() && (static_cast<bool>(clip_[Id::launched]) || down);
            graphics.setColour(clip_.isValid() ? (on ? colour_ : colour_.withAlpha(0.16f)) : juce::Colour(0xff101114));
            graphics.fillRoundedRectangle(bounds, 6.0f);
            graphics.setColour(clip_.isValid() ? colour_.withAlpha(on ? 1.0f : 0.7f) : juce::Colour(0xff2a2a2a));
            graphics.drawRoundedRectangle(bounds, 6.0f, highlighted ? 2.0f : 1.0f);
            if (title_.isNotEmpty()) {
                graphics.setColour(on ? juce::Colour(0xff101108) : colour_);
                graphics.setFont(juce::FontOptions(13.0f));
                graphics.drawFittedText(title_, getLocalBounds().reduced(8, 4), juce::Justification::centredLeft, 2);
            }
        }

        juce::ValueTree clip_;
        juce::String title_;
        juce::Colour colour_{kLime};
    };

    class Column : public juce::Component {
    public:
        explicit Column(int index) : index_(index) {
            addAndMakeVisible(name_);
            addAndMakeVisible(toggleRow_);
            toggleRow_.addAndMakeVisible(mute_);
            toggleRow_.addAndMakeVisible(solo_);
            toggleRow_.addAndMakeVisible(arm_);
            addAndMakeVisible(volume_);
            mute_.setButtonText("M");
            solo_.setButtonText("S");
            arm_.setButtonText("R");
            mute_.setClickingTogglesState(true);
            solo_.setClickingTogglesState(true);
            arm_.setClickingTogglesState(true);
            volume_.setSliderStyle(juce::Slider::LinearHorizontal);
            volume_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            volume_.setRange(0.0, 1.0, 0.001);
            name_.setJustificationType(juce::Justification::centredLeft);
            name_.setFont(juce::FontOptions(15.0f));
            for (auto& slot : slots_) addAndMakeVisible(slot);
        }

        void bind(DawnMainComponent& owner) {
            owner_ = &owner;
            const bool live = index_ < owner.trackCount_;
            setVisible(live);
            if (!live) return;
            auto track = owner.tracks_[index_];
            name_.setText(track[Id::name].toString(), juce::dontSendNotification);
            mute_.setToggleState(static_cast<bool>(track[Id::mute]), juce::dontSendNotification);
            solo_.setToggleState(static_cast<bool>(track[Id::solo]), juce::dontSendNotification);
            arm_.setToggleState(static_cast<bool>(track[kArm]), juce::dontSendNotification);
            volume_.setValue(static_cast<double>(track[Id::volume]), juce::dontSendNotification);
            mute_.onClick = [this] { owner_->tracks_[index_].setProperty(Id::mute, mute_.getToggleState(), nullptr); };
            solo_.onClick = [this] { owner_->tracks_[index_].setProperty(Id::solo, solo_.getToggleState(), nullptr); };
            arm_.onClick = [this] { owner_->tracks_[index_].setProperty(kArm, arm_.getToggleState(), nullptr); };
            volume_.onValueChange = [this] {
                owner_->selectTrack(index_);
                owner_->tracks_[index_].setProperty(Id::volume, static_cast<float>(volume_.getValue()), nullptr);
                owner_->devices_->pullTrack(index_);
            };
            for (int scene = 0; scene < kScenes; ++scene) {
                auto clip = owner.clips_[index_][scene];
                juce::String title;
                if (clip.isValid()) {
                    title = clip[Id::userNotes].toString();
                    if (title.isEmpty()) title = "Clip " + juce::String(scene + 1);
                }
                slots_[scene].setClip(clip, title, kClipColours[scene % 4]);
                slots_[scene].onClick = [this, scene] {
                    owner_->selectTrack(index_);
                    owner_->launchClip(owner_->clips_[index_][scene]);
                    slots_[scene].repaint();
                };
            }
        }

        void setSelected(bool selected) {
            selected_ = selected;
            repaint();
        }

        void pullVolume() {
            if (owner_ == nullptr || index_ >= owner_->trackCount_) return;
            volume_.setValue(static_cast<double>(owner_->tracks_[index_][Id::volume]), juce::dontSendNotification);
        }

        void paint(juce::Graphics& graphics) override {
            graphics.setColour(kPanel);
            graphics.fillRect(getLocalBounds());
            graphics.setColour(selected_ ? kLime : juce::Colour(0xff2c3038));
            graphics.fillRect(0, 0, getWidth(), 3);
        }

        void resized() override {
            juce::Grid grid;
            grid.templateColumns = {juce::Grid::TrackInfo(juce::Grid::Fr(1))};
            grid.templateRows = {juce::Grid::TrackInfo(juce::Grid::Px(26)), juce::Grid::TrackInfo(juce::Grid::Px(28)), juce::Grid::TrackInfo(juce::Grid::Px(24))};
            for (int i = 0; i < kScenes; ++i) grid.templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
            grid.rowGap = juce::Grid::Px(6);
            grid.items = {juce::GridItem(name_), juce::GridItem(toggleRow_), juce::GridItem(volume_)};
            for (auto& slot : slots_) grid.items.add(juce::GridItem(slot));
            grid.performLayout(getLocalBounds().reduced(4, 6));
            juce::FlexBox toggles;
            toggles.flexDirection = juce::FlexBox::Direction::row;
            toggles.items.add(juce::FlexItem(mute_).withFlex(1.0f).withMargin(2.0f));
            toggles.items.add(juce::FlexItem(solo_).withFlex(1.0f).withMargin(2.0f));
            toggles.items.add(juce::FlexItem(arm_).withFlex(1.0f).withMargin(2.0f));
            toggles.performLayout(toggleRow_.getLocalBounds().toFloat());
        }

        void mouseDown(const juce::MouseEvent&) override {
            if (owner_ != nullptr) owner_->selectTrack(index_);
        }

        int index_ = 0;
        bool selected_ = false;
        DawnMainComponent* owner_ = nullptr;
        juce::Label name_;
        juce::Component toggleRow_;
        juce::TextButton mute_, solo_, arm_;
        juce::Slider volume_;
        Slot slots_[DawnMainComponent::kScenes];
    };

    SessionView() {
        for (int i = 0; i < kColumns; ++i) {
            columns_[i] = std::make_unique<Column>(i);
            addAndMakeVisible(*columns_[i]);
        }
    }

    void bind(DawnMainComponent& owner) {
        for (auto& column : columns_) column->bind(owner);
        if (owner.trackCount_ > 0) columns_[0]->setSelected(true);
    }

    void select(int index) {
        for (int i = 0; i < kColumns; ++i) columns_[i]->setSelected(i == index);
    }

    void pullVolume(int index) {
        if (index >= 0 && index < kColumns) columns_[index]->pullVolume();
    }

    void paint(juce::Graphics& graphics) override { graphics.fillAll(kBg); }

    void resized() override {
        int count = 0;
        for (auto& column : columns_)
            if (column->isVisible()) ++count;
        if (count == 0) count = 1;
        juce::Grid grid;
        for (int i = 0; i < count; ++i) grid.templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
        grid.templateRows = {juce::Grid::TrackInfo(juce::Grid::Fr(1))};
        grid.columnGap = juce::Grid::Px(8);
        int placed = 0;
        for (auto& column : columns_) {
            if (!column->isVisible()) continue;
            grid.items.add(juce::GridItem(*column));
            ++placed;
        }
        grid.performLayout(getLocalBounds().reduced(8));
        juce::ignoreUnused(placed);
    }

    std::unique_ptr<Column> columns_[DawnMainComponent::kColumns];
};

class DawnMainComponent::DevicePanel : public juce::Component {
public:
    explicit DevicePanel(VisualizerDataQueue& queue) : visuals_(queue) {
        addAndMakeVisible(sendHost_);
        addAndMakeVisible(macroHost_);
        addAndMakeVisible(visuals_);
        addAndMakeVisible(padHost_);
        for (auto* slider : {&reverb_, &delay_, &pan_}) {
            sendHost_.addAndMakeVisible(*slider);
            slider->setSliderStyle(juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        }
        reverb_.setRange(0.0, 1.0, 0.001);
        delay_.setRange(0.0, 1.0, 0.001);
        pan_.setRange(-1.0, 1.0, 0.001);
        for (int i = 0; i < 6; ++i) {
            macroHost_.addAndMakeVisible(macroName_[i]);
            macroHost_.addAndMakeVisible(macro_[i]);
            macroName_[i].setText(kMacroNames[i], juce::dontSendNotification);
            macroName_[i].setFont(juce::FontOptions(12.0f));
            macro_[i].setSliderStyle(juce::Slider::LinearHorizontal);
            macro_[i].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            macro_[i].setRange(i == 5 ? 0.0 : 0.0, 1.0, 0.001);
        }
        for (int i = 0; i < 8; ++i) {
            padHost_.addAndMakeVisible(pads_[i]);
            pads_[i].setButtonText(kPadNames[i]);
        }
    }

    void bind(DawnMainComponent& owner) {
        owner_ = &owner;
        reverb_.onValueChange = [this] { pushSend(owner_->trackNamed("reverb").isValid() ? owner_->trackNamed("reverb") : owner_->trackNamed("air"),
                                                   static_cast<float>(reverb_.getValue())); };
        delay_.onValueChange = [this] { pushSend(owner_->trackNamed("delay"), static_cast<float>(delay_.getValue())); };
        pan_.onValueChange = [this] {
            if (auto track = current()) track.setProperty(Id::pan, static_cast<float>(pan_.getValue()), nullptr);
        };
        for (int i = 0; i < 5; ++i) {
            macro_[i].onValueChange = [this, i] {
                if (auto track = current()) track.setProperty(kMacroNames[i], macro_[i].getValue(), nullptr);
            };
        }
        macro_[5].onValueChange = [this] {
            if (!current().isValid()) return;
            owner_->tracks_[owner_->selected_].setProperty(Id::volume, static_cast<float>(macro_[5].getValue()), nullptr);
            owner_->session_->pullVolume(owner_->selected_);
        };
        for (int i = 0; i < 8; ++i) {
            pads_[i].onStateChange = [this, i] {
                const auto note = kPadNotes[i];
                if (pads_[i].isDown()) owner_->live_.push(juce::MidiMessage::noteOn(1, note, static_cast<juce::uint8>(112)));
                else owner_->live_.push(juce::MidiMessage::noteOff(1, note));
            };
        }
        pullTrack(0);
    }

    void pullTrack(int) {
        if (owner_ == nullptr) return;
        auto track = current();
        if (!track.isValid()) return;
        pan_.setValue(static_cast<double>(track[Id::pan]), juce::dontSendNotification);
        macro_[5].setValue(static_cast<double>(track[Id::volume]), juce::dontSendNotification);
        auto readSend = [&](juce::ValueTree destination, juce::Slider& slider) {
            if (!destination.isValid()) return;
            const auto id = destination[Id::trackId];
            for (int i = 0; i < track.getNumChildren(); ++i) {
                auto child = track.getChild(i);
                if (child.hasType(Id::send) && child[Id::destination] == id) {
                    slider.setValue(static_cast<double>(child[Id::level]), juce::dontSendNotification);
                    return;
                }
            }
        };
        readSend(owner_->trackNamed("reverb").isValid() ? owner_->trackNamed("reverb") : owner_->trackNamed("air"), reverb_);
        readSend(owner_->trackNamed("delay"), delay_);
    }

    void paint(juce::Graphics& graphics) override {
        graphics.fillAll(juce::Colour(0xff14161c));
        graphics.setColour(juce::Colour(0xff2c3038));
        graphics.fillRect(0, 0, getWidth(), 1);
        graphics.setColour(kLime.withAlpha(0.8f));
        graphics.setFont(juce::FontOptions(12.0f));
        graphics.drawText("SENDS", 12, 6, 80, 16, juce::Justification::centredLeft);
        graphics.drawText("DRUM RACK", sendsArea_.getRight() + 12, 6, 120, 16, juce::Justification::centredLeft);
    }

    void resized() override {
        juce::Grid grid;
        grid.templateColumns = {juce::Grid::TrackInfo(juce::Grid::Px(240)), juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                                juce::Grid::TrackInfo(juce::Grid::Px(280))};
        grid.templateRows = {juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Px(72))};
        grid.columnGap = juce::Grid::Px(8);
        grid.rowGap = juce::Grid::Px(6);
        grid.items = {juce::GridItem(sendHost_), juce::GridItem(macroHost_), juce::GridItem(visuals_), juce::GridItem(padHost_).withArea(2, 1, 3, 4)};
        grid.performLayout(getLocalBounds().reduced(8, 24));
        sendsArea_ = sendHost_.getBounds();

        juce::FlexBox sendBox;
        sendBox.flexDirection = juce::FlexBox::Direction::column;
        sendBox.items.add(juce::FlexItem(reverb_).withFlex(1.0f).withMargin(2.0f));
        sendBox.items.add(juce::FlexItem(delay_).withFlex(1.0f).withMargin(2.0f));
        sendBox.items.add(juce::FlexItem(pan_).withFlex(1.0f).withMargin(2.0f));
        sendBox.performLayout(sendHost_.getLocalBounds().toFloat());

        auto macroArea = macroHost_.getLocalBounds();
        const int rowH = juce::jmax(1, macroArea.getHeight() / 6);
        for (int i = 0; i < 6; ++i) {
            auto row = macroArea.removeFromTop(rowH);
            macroName_[i].setBounds(row.removeFromLeft(58));
            macro_[i].setBounds(row);
        }
        juce::FlexBox pads;
        pads.flexDirection = juce::FlexBox::Direction::row;
        for (auto& pad : pads_) pads.items.add(juce::FlexItem(pad).withFlex(1.0f).withMargin(3.0f));
        pads.performLayout(padHost_.getLocalBounds().toFloat());
    }

private:
    juce::ValueTree current() const {
        if (owner_ == nullptr || owner_->selected_ < 0 || owner_->selected_ >= owner_->trackCount_) return {};
        return owner_->tracks_[owner_->selected_];
    }

    void pushSend(juce::ValueTree destination, float level) {
        if (owner_ == nullptr || !destination.isValid()) return;
        owner_->setSend(current(), destination, level);
    }

    DawnMainComponent* owner_ = nullptr;
    juce::Component sendHost_, macroHost_, padHost_;
    juce::Slider reverb_, delay_, pan_;
    juce::Label macroName_[6];
    juce::Slider macro_[6];
    juce::TextButton pads_[8];
    MasterVisualizers visuals_;
    juce::Rectangle<int> sendsArea_;
};

DawnMainComponent::DawnMainComponent(ProjectState& state, AudioEngine& engine, VisualizerDataQueue& visuals, LiveMidiInjector& live)
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
    transportHolder_ = std::make_unique<TransportBar>();
    sessionHolder_ = std::make_unique<SessionView>();
    deviceHolder_ = std::make_unique<DevicePanel>(visuals);
    transport_ = transportHolder_.get();
    session_ = sessionHolder_.get();
    devices_ = deviceHolder_.get();
    addAndMakeVisible(*transport_);
    addAndMakeVisible(*session_);
    addAndMakeVisible(*devices_);
    transport_->bind(*this);
    session_->bind(*this);
    devices_->bind(*this);
    startTimerHz(20);
}

DawnMainComponent::~DawnMainComponent() { stopTimer(); }

void DawnMainComponent::paint(juce::Graphics& graphics) { graphics.fillAll(kBg); }

void DawnMainComponent::resized() {
    juce::Grid grid;
    grid.templateColumns = {juce::Grid::TrackInfo(juce::Grid::Fr(1))};
    grid.templateRows = {juce::Grid::TrackInfo(juce::Grid::Px(64)), juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Px(236))};
    grid.items = {juce::GridItem(*transport_), juce::GridItem(*session_), juce::GridItem(*devices_)};
    grid.performLayout(getLocalBounds());
}

void DawnMainComponent::timerCallback() {
    const bool playing = static_cast<int>(project_[Id::command]) == static_cast<int>(TransportCommand::Play);
    transport_->setPlaying(playing);
    transport_->setPosition(positionText(engine_.playhead()));
}

void DawnMainComponent::launchClip(juce::ValueTree clip) {
    if (!clip.isValid()) return;
    const bool next = !static_cast<bool>(clip[Id::launched]);
    const double at = quantized(engine_.playhead());
    state_.setLaunched(clip, next, at);
    if (next) {
        const auto command = static_cast<TransportCommand>(static_cast<int>(project_[Id::command]));
        if (command != TransportCommand::Play) state_.play(at);
    }
}

void DawnMainComponent::selectTrack(int index) {
    if (index < 0 || index >= trackCount_) return;
    selected_ = index;
    session_->select(index);
    devices_->pullTrack(index);
}

double DawnMainComponent::quantizeBeats() const {
    switch (transport_->quantizeId()) {
    case 2: return 4.0;
    case 3: return 2.0;
    case 4: return 1.0;
    case 5: return 0.5;
    case 6: return 0.25;
    default: return 0.0;
    }
}

double DawnMainComponent::quantized(double beat) const {
    const double grid = quantizeBeats();
    if (grid <= 0.0) return beat;
    return std::floor(beat / grid) * grid;
}

void DawnMainComponent::setSend(juce::ValueTree track, juce::ValueTree destination, float level) {
    if (!track.isValid() || !destination.isValid()) return;
    const auto id = destination[Id::trackId];
    for (int i = 0; i < track.getNumChildren(); ++i) {
        auto child = track.getChild(i);
        if (child.hasType(Id::send) && child[Id::destination] == id) {
            child.setProperty(Id::level, level, nullptr);
            return;
        }
    }
    juce::ValueTree send{Id::send};
    send.setProperty(Id::destination, id, nullptr);
    send.setProperty(Id::level, level, nullptr);
    track.appendChild(send, nullptr);
}

juce::ValueTree DawnMainComponent::trackNamed(const juce::String& fragment) const {
    for (int i = 0; i < trackCount_; ++i) {
        if (tracks_[i][Id::name].toString().containsIgnoreCase(fragment)) return tracks_[i];
    }
    return {};
}

} // namespace dawn
