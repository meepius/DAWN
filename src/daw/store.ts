import { create } from "zustand";
import { DEFAULT_PARAMS, PARAMS, createDevice } from "@/daw/devices";
import { AIR_BPM, AIR_PEAKS } from "@/daw/air";
import { STORAGE_KEY, createDemoProject, isProject } from "@/daw/demo";
import { getEngine } from "@/daw/engine";
import type { ArrangementClip, Clip, DeviceKind, NoteEvent, Project, Track, TransportSnap } from "@/daw/types";

export type DawView = "session" | "arrangement";
export type PanelTab = "devices" | "clip";

interface DawState {
  ready: boolean;
  project: Project;
  view: DawView;
  panel: PanelTab;
  selectedTrackId: string;
  selectedClipId: string | null;
  selectedArrangementId: string | null;
  learn: boolean;
  learnTarget: string | null;
  record: boolean;
  octave: number;
  follow: boolean;
  help: boolean;
  pxPerBeat: number;
  snap: TransportSnap;
  midiIns: number | null;
  sampleRate: number | null;
  rendering: boolean;
  hydrate: () => void;
  setSnap: (snap: TransportSnap) => void;
  setView: (view: DawView) => void;
  setPanel: (panel: PanelTab) => void;
  setHelp: (help: boolean) => void;
  setFollow: (follow: boolean) => void;
  setZoom: (px: number) => void;
  selectTrack: (trackId: string) => void;
  selectClip: (trackId: string, clipId: string | null) => void;
  selectArrangement: (id: string | null) => void;
  setName: (name: string) => void;
  setBpm: (bpm: number) => void;
  setSwing: (swing: number) => void;
  setQuantize: (quantize: number) => void;
  setMetronome: (on: boolean) => void;
  setMaster: (master: number) => void;
  setLoop: (loop: boolean) => void;
  setLoopRegion: (start: number, end: number) => void;
  setRecord: (record: boolean) => void;
  setOctave: (octave: number) => void;
  setLearn: (learn: boolean) => void;
  armLearn: (target: string) => void;
  learnCc: (cc: number) => void;
  applyControl: (target: string, normalized: number) => void;
  toggleMute: (trackId: string) => void;
  toggleSolo: (trackId: string) => void;
  toggleArm: (trackId: string) => void;
  setTrackNumber: (trackId: string, key: "volume" | "pan" | "sendRev" | "sendDly", value: number) => void;
  renameTrack: (trackId: string, name: string) => void;
  setDeviceParam: (deviceId: string, key: string, value: number) => void;
  toggleBypass: (deviceId: string) => void;
  addDevice: (trackId: string, kind: DeviceKind) => void;
  removeDevice: (deviceId: string) => void;
  createClip: (trackId: string, sceneIndex: number) => void;
  toggleNote: (clipId: string, pitch: number, start: number, duration: number) => void;
  upsertNote: (clipId: string, note: NoteEvent) => void;
  renameClip: (clipId: string, name: string) => void;
  setClipLength: (clipId: string, beats: number) => void;
  setWarp: (clipId: string, warp: boolean) => void;
  adoptAudio: (clipId: string, bufferId: string, peaks: number[], duration: number) => void;
  moveArrangement: (id: string, startBeat: number) => void;
  placeArrangement: (trackId: string, clipId: string, startBeat: number) => void;
  deleteArrangement: (id: string) => void;
  setRendering: (rendering: boolean) => void;
  setMidiIns: (count: number) => void;
  setSampleRate: (rate: number) => void;
  loadDemo: () => void;
}

function uid(prefix: string): string {
  return `${prefix}-${Math.random().toString(36).slice(2, 8)}`;
}

function patchProject(project: Project, patch: Partial<Project>): Project {
  return { ...project, ...patch };
}

function mapTrack(project: Project, trackId: string, update: (track: Track) => Track): Project {
  return {
    ...project,
    tracks: project.tracks.map((track) => (track.id === trackId ? update(track) : track)),
  };
}

function mapClip(project: Project, clipId: string, update: (clip: Clip) => Clip): Project {
  const clip = project.clips[clipId];
  if (!clip) return project;
  return { ...project, clips: { ...project.clips, [clipId]: update(clip) } };
}

function findDevice(project: Project, deviceId: string): { track: Track; deviceIndex: number } | null {
  for (const track of project.tracks) {
    const deviceIndex = track.devices.findIndex((device) => device.id === deviceId);
    if (deviceIndex >= 0) return { track, deviceIndex };
  }
  return null;
}

const demo = createDemoProject();

export const useDaw = create<DawState>((set, get) => ({
  ready: false,
  project: demo,
  view: "session",
  panel: "devices",
  selectedTrackId: demo.tracks[0]?.id ?? "tr-drums",
  selectedClipId: "clip-groove",
  selectedArrangementId: null,
  learn: false,
  learnTarget: null,
  record: false,
  octave: 3,
  follow: true,
  help: false,
  pxPerBeat: 36,
  snap: { playing: false, launched: {}, launchedSlot: {}, queued: {}, queuedSlot: {} },
  midiIns: null,
  sampleRate: null,
  rendering: false,

  hydrate: () => {
    try {
      const raw = localStorage.getItem(STORAGE_KEY);
      if (!raw) {
        set({ ready: true });
        return;
      }
      const parsed: unknown = JSON.parse(raw);
      if (!isProject(parsed)) {
        set({ ready: true });
        return;
      }
      set({
        project: parsed,
        ready: true,
        selectedTrackId: parsed.tracks[0]?.id ?? get().selectedTrackId,
        selectedClipId: null,
      });
    } catch {
      set({ ready: true });
    }
  },

  setSnap: (snap) => set({ snap }),
  setView: (view) => set({ view }),
  setPanel: (panel) => set({ panel }),
  setHelp: (help) => set({ help }),
  setFollow: (follow) => set({ follow }),
  setZoom: (pxPerBeat) => set({ pxPerBeat }),
  selectArrangement: (selectedArrangementId) => set({ selectedArrangementId }),
  setRendering: (rendering) => set({ rendering }),
  setMidiIns: (midiIns) => set({ midiIns }),
  setSampleRate: (sampleRate) => set({ sampleRate }),

  selectTrack: (trackId) => {
    const track = get().project.tracks.find((item) => item.id === trackId);
    let octave = get().octave;
    if (track?.name === "Bass") octave = 1;
    else if (track?.name === "Lead") octave = 4;
    else if (track?.kind === "midi") octave = 3;
    set({ selectedTrackId: trackId, octave });
  },

  selectClip: (trackId, clipId) => {
    get().selectTrack(trackId);
    set({ selectedClipId: clipId, panel: clipId ? "clip" : get().panel });
  },

  setName: (name) => set({ project: patchProject(get().project, { name }) }),
  setBpm: (bpm) => {
    set({ project: patchProject(get().project, { bpm: Math.min(240, Math.max(60, Math.round(bpm))) }) });
    getEngine().sync();
  },
  setSwing: (swing) => set({ project: patchProject(get().project, { swing }) }),
  setQuantize: (quantize) => set({ project: patchProject(get().project, { quantize }) }),
  setMetronome: (metronome) => set({ project: patchProject(get().project, { metronome }) }),
  setMaster: (master) => {
    set({ project: patchProject(get().project, { master }) });
    getEngine().sync();
  },
  setLoop: (loop) => set({ project: patchProject(get().project, { loop }) }),
  setLoopRegion: (loopStart, loopEnd) =>
    set({
      project: patchProject(get().project, {
        loopStart: Math.max(0, loopStart),
        loopEnd: Math.max(loopStart + 4, loopEnd),
        loop: true,
      }),
    }),
  setRecord: (record) => set({ record }),
  setOctave: (octave) => set({ octave: Math.min(6, Math.max(0, octave)) }),
  setLearn: (learn) => set({ learn, learnTarget: learn ? get().learnTarget : null }),
  armLearn: (learnTarget) => set({ learn: true, learnTarget }),

  learnCc: (cc) => {
    const target = get().learnTarget;
    if (!target) return;
    const bindings = get().project.bindings.filter((binding) => binding.cc !== cc && binding.target !== target);
    bindings.push({ cc, target });
    set({ project: patchProject(get().project, { bindings }), learnTarget: null });
  },

  applyControl: (target, normalized) => {
    const project = get().project;
    const value = Math.min(1, Math.max(0, normalized));
    if (target === "master") {
      get().setMaster(value);
      return;
    }
    const [kind, id, param] = target.split(":");
    if (kind === "vol" && id) get().setTrackNumber(id, "volume", value);
    else if (kind === "pan" && id) get().setTrackNumber(id, "pan", value * 2 - 1);
    else if (kind === "sendR" && id) get().setTrackNumber(id, "sendRev", value);
    else if (kind === "sendD" && id) get().setTrackNumber(id, "sendDly", value);
    else if (kind === "dev" && id && param) {
      const found = findDevice(project, id);
      const device = found ? found.track.devices[found.deviceIndex] : undefined;
      if (!device) return;
      const spec = PARAMS[device.kind].find((item) => item.key === param);
      if (!spec) return;
      get().setDeviceParam(id, param, spec.min + value * (spec.max - spec.min));
    }
  },

  toggleMute: (trackId) => {
    set({
      project: mapTrack(get().project, trackId, (track) => ({ ...track, mute: !track.mute })),
    });
    getEngine().sync();
  },
  toggleSolo: (trackId) => {
    set({
      project: mapTrack(get().project, trackId, (track) => ({ ...track, solo: !track.solo })),
    });
    getEngine().sync();
  },
  toggleArm: (trackId) => {
    set({
      project: {
        ...get().project,
        tracks: get().project.tracks.map((track) => ({
          ...track,
          arm: track.id === trackId ? !track.arm : false,
        })),
      },
    });
  },
  setTrackNumber: (trackId, key, value) => {
    set({
      project: mapTrack(get().project, trackId, (track) => ({ ...track, [key]: value })),
    });
    getEngine().sync();
  },
  renameTrack: (trackId, name) =>
    set({ project: mapTrack(get().project, trackId, (track) => ({ ...track, name })) }),

  setDeviceParam: (deviceId, key, value) => {
    const project = get().project;
    const found = findDevice(project, deviceId);
    if (!found) return;
    const devices = found.track.devices.map((device, index) =>
      index === found.deviceIndex ? { ...device, params: { ...device.params, [key]: value } } : device,
    );
    set({ project: mapTrack(project, found.track.id, (track) => ({ ...track, devices })) });
    getEngine().sync();
  },
  toggleBypass: (deviceId) => {
    const project = get().project;
    const found = findDevice(project, deviceId);
    if (!found) return;
    const devices = found.track.devices.map((device, index) =>
      index === found.deviceIndex ? { ...device, bypass: !device.bypass } : device,
    );
    set({ project: mapTrack(project, found.track.id, (track) => ({ ...track, devices })) });
    getEngine().sync();
  },
  addDevice: (trackId, kind) => {
    if (kind === "analog" || kind === "drums") return;
    const track = get().project.tracks.find((item) => item.id === trackId);
    if (!track || track.devices.some((device) => device.kind === kind)) return;
    const device = createDevice(uid(kind), kind, DEFAULT_PARAMS[kind]);
    set({
      project: mapTrack(get().project, trackId, (current) => ({
        ...current,
        devices: [...current.devices, device],
      })),
    });
    getEngine().sync();
  },
  removeDevice: (deviceId) => {
    const project = get().project;
    const found = findDevice(project, deviceId);
    if (!found) return;
    const device = found.track.devices[found.deviceIndex];
    if (!device || device.kind === "analog" || device.kind === "drums") return;
    set({
      project: mapTrack(project, found.track.id, (track) => ({
        ...track,
        devices: track.devices.filter((item) => item.id !== deviceId),
      })),
    });
    getEngine().sync();
  },

  createClip: (trackId, sceneIndex) => {
    const track = get().project.tracks.find((item) => item.id === trackId);
    if (!track || track.kind === "return") return;
    const id = uid("clip");
    const clip: Clip = {
      id,
      name: "Clip",
      color: track.color,
      lengthBeats: 4,
      notes: [],
      audio:
        track.kind === "audio"
          ? { bufferId: "air", sourceBpm: AIR_BPM, warp: true, peaks: AIR_PEAKS }
          : undefined,
    };
    const slots = [...track.slots];
    while (slots.length < get().project.scenes.length) slots.push(null);
    slots[sceneIndex] = id;
    set({
      project: {
        ...mapTrack(get().project, trackId, (current) => ({ ...current, slots })),
        clips: { ...get().project.clips, [id]: clip },
      },
      selectedTrackId: trackId,
      selectedClipId: id,
      panel: "clip",
    });
  },

  toggleNote: (clipId, pitch, start, duration) => {
    set({
      project: mapClip(get().project, clipId, (clip) => {
        const existing = clip.notes.find((note) => note.pitch === pitch && Math.abs(note.start - start) < 0.02);
        const notes = existing
          ? clip.notes.filter((note) => note !== existing)
          : [...clip.notes, { id: uid("n"), pitch, start, duration, velocity: 0.85 }];
        return { ...clip, notes };
      }),
    });
  },

  upsertNote: (clipId, note) => {
    set({
      project: mapClip(get().project, clipId, (clip) => {
        const notes = clip.notes.filter(
          (item) => !(item.pitch === note.pitch && Math.abs(item.start - note.start) < 0.02),
        );
        notes.push(note);
        return { ...clip, notes };
      }),
    });
  },

  renameClip: (clipId, name) =>
    set({ project: mapClip(get().project, clipId, (clip) => ({ ...clip, name })) }),

  setClipLength: (clipId, beats) =>
    set({
      project: mapClip(get().project, clipId, (clip) => ({
        ...clip,
        lengthBeats: beats,
        notes: clip.notes.filter((note) => note.start < beats),
      })),
    }),

  setWarp: (clipId, warp) =>
    set({
      project: mapClip(get().project, clipId, (clip) =>
        clip.audio ? { ...clip, audio: { ...clip.audio, warp } } : clip,
      ),
    }),

  adoptAudio: (clipId, bufferId, peaks, duration) => {
    const bpm = get().project.bpm;
    const beats = Math.max(4, Math.round((duration * bpm) / 60 / 4) * 4);
    set({
      project: mapClip(get().project, clipId, (clip) => ({
        ...clip,
        lengthBeats: beats,
        notes: [],
        audio: { bufferId, sourceBpm: bpm, warp: true, peaks },
      })),
    });
  },

  moveArrangement: (id, startBeat) => {
    const start = Math.max(0, Math.round(startBeat * 4) / 4);
    const project = get().project;
    const event = project.arrangement.find((item) => item.id === id);
    const clip = event ? project.clips[event.clipId] : undefined;
    const end = start + (clip?.lengthBeats ?? 0);
    set({
      project: patchProject(project, {
        arrangement: project.arrangement.map((item) => (item.id === id ? { ...item, startBeat: start } : item)),
        lengthBeats: Math.max(project.lengthBeats, end),
      }),
    });
  },

  placeArrangement: (trackId, clipId, startBeat) => {
    const start = Math.max(0, Math.round(startBeat * 4) / 4);
    const project = get().project;
    const clip = project.clips[clipId];
    const event: ArrangementClip = { id: uid("arr"), trackId, clipId, startBeat: start };
    set({
      project: patchProject(project, {
        arrangement: [...project.arrangement, event],
        lengthBeats: Math.max(project.lengthBeats, start + (clip?.lengthBeats ?? 4)),
      }),
      selectedArrangementId: event.id,
    });
  },

  deleteArrangement: (id) =>
    set({
      project: patchProject(get().project, {
        arrangement: get().project.arrangement.filter((event) => event.id !== id),
      }),
      selectedArrangementId: get().selectedArrangementId === id ? null : get().selectedArrangementId,
    }),

  loadDemo: () => {
    const project = createDemoProject();
    localStorage.removeItem(STORAGE_KEY);
    getEngine().stop();
    getEngine().releaseAll();
    set({
      project,
      selectedTrackId: project.tracks[0]?.id ?? "tr-drums",
      selectedClipId: "clip-groove",
      selectedArrangementId: null,
      record: false,
      view: "session",
    });
    getEngine().sync();
  },
}));
