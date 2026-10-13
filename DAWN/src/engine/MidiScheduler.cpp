#include "engine/MidiScheduler.h"

#include <algorithm>
#include <cmath>

namespace dawn {

bool eventBefore(const ScheduledEvent& a, const ScheduledEvent& b) noexcept {
    if (a.sampleOffset != b.sampleOffset) return a.sampleOffset < b.sampleOffset;
    const bool aOff = (a.status & 0xF0) == 0x80;
    const bool bOff = (b.status & 0xF0) == 0x80;
    return aOff && !bOff;
}

namespace {

void sortEvents(MidiBlock& midi) noexcept {
    sortMidiBlock(midi);
}

void releaseAll(HeldNote* held, int capacity, MidiBlock& midi) noexcept {
    for (int i = 0; i < capacity; ++i) {
        if (!held[i].used) continue;
        midi.add(0, static_cast<std::uint8_t>(0x80 | (held[i].channel & 0x0F)), held[i].pitch, 0);
        held[i].used = false;
    }
}

void hold(HeldNote* held, int capacity, TrackId track, std::uint8_t pitch, std::uint8_t channel, double offBeat) noexcept {
    for (int i = 0; i < capacity; ++i) {
        if (!held[i].used) {
            held[i] = HeldNote{true, track, pitch, channel, offBeat};
            return;
        }
    }
}

bool clipContains(const PlaybackSnapshot& snapshot, TrackId track) noexcept {
    for (const auto& clip : snapshot.clips) {
        if (clip.track == track) return true;
    }
    return false;
}

int sampleOffset(double beat, double start, double samplesPerBeat, int samples) noexcept {
    const auto offset = static_cast<long long>(std::llround((beat - start) * samplesPerBeat));
    if (offset < 0 || offset >= samples) return -1;
    return static_cast<int>(offset);
}

} // namespace

void sortMidiBlock(MidiBlock& midi) noexcept {
    if (midi.events == nullptr) return;
    for (int i = 1; i < midi.count; ++i) {
        const auto key = midi.events[i];
        int j = i - 1;
        while (j >= 0 && eventBefore(key, midi.events[j])) {
            midi.events[j + 1] = midi.events[j];
            --j;
        }
        midi.events[j + 1] = key;
    }
}

void scheduleMidi(const PlaybackSnapshot& snapshot,
                  TransportClock& clock,
                  HeldNote* held,
                  int heldCapacity,
                  double sampleRate,
                  int samples,
                  MidiBlock& midi) noexcept {
    if (held == nullptr || samples <= 0 || sampleRate <= 0.0) return;

    if (snapshot.commandSerial != clock.appliedSerial) {
        clock.appliedSerial = snapshot.commandSerial;
        if (snapshot.command == TransportCommand::Stop) {
            clock.playing = false;
            releaseAll(held, heldCapacity, midi);
        } else if (snapshot.command == TransportCommand::Play || snapshot.command == TransportCommand::Seek) {
            releaseAll(held, heldCapacity, midi);
            clock.beat = snapshot.commandBeat;
            clock.playing = snapshot.command == TransportCommand::Play;
        }
    }

    if (snapshot.bpm >= 20.0 && snapshot.bpm <= 400.0) clock.bpm = snapshot.bpm;

    for (int i = 0; i < heldCapacity; ++i) {
        if (held[i].used && !clipContains(snapshot, held[i].track)) {
            midi.add(0, static_cast<std::uint8_t>(0x80 | (held[i].channel & 0x0F)), held[i].pitch, 0);
            held[i].used = false;
        }
    }

    if (!clock.playing) {
        sortEvents(midi);
        return;
    }

    const double samplesPerBeat = sampleRate * 60.0 / clock.bpm;
    if (samplesPerBeat <= 0.0) return;
    const double start = clock.beat;
    const double end = start + static_cast<double>(samples) / samplesPerBeat;

    for (const auto& clip : snapshot.clips) {
        if (clip.loopBeats <= 0.0 || clip.noteCount <= 0) continue;
        for (int n = 0; n < clip.noteCount; ++n) {
            const auto& note = snapshot.notes[static_cast<std::size_t>(clip.noteBegin + n)];
            const double origin = clip.launchBeat + note.startBeat;
            const long first = static_cast<long>(std::floor((start - origin) / clip.loopBeats)) - 1;
            for (long step = 0; step < 4; ++step) {
                const double onBeat = origin + static_cast<double>(first + step) * clip.loopBeats;
                if (onBeat >= end) break;
                const int offset = sampleOffset(onBeat, start, samplesPerBeat, samples);
                if (offset < 0) continue;
                const auto status = static_cast<std::uint8_t>(0x90 | (note.channel & 0x0F));
                midi.add(offset, status, note.pitch, note.velocity);
                hold(held, heldCapacity, clip.track, note.pitch, note.channel, onBeat + note.lengthBeats);
            }
        }
    }

    for (int i = 0; i < heldCapacity; ++i) {
        if (!held[i].used) continue;
        const int offset = sampleOffset(held[i].offBeat, start, samplesPerBeat, samples);
        if (offset < 0) continue;
        midi.add(offset, static_cast<std::uint8_t>(0x80 | (held[i].channel & 0x0F)), held[i].pitch, 0);
        held[i].used = false;
    }

    clock.beat = end;
    sortEvents(midi);
}

} // namespace dawn
