import { AIR_BPM, AIR_PEAKS } from "@/daw/air";
import { createDevice } from "@/daw/devices";
import type { ArrangementClip, Clip, NoteEvent, Project, Track } from "@/daw/types";

function notes(clipId: string, spec: Array<[number, number, number, number?]>): NoteEvent[] {
  return spec.map(([pitch, start, duration, velocity], index) => ({
    id: `${clipId}-n${index}`,
    pitch,
    start,
    duration,
    velocity: velocity ?? 0.84,
  }));
}

function hits(
  clipId: string,
  pitch: number,
  steps: number[],
  velocity = 0.9,
  duration = 0.2,
): NoteEvent[] {
  return steps.map((step, index) => ({
    id: `${clipId}-p${pitch}-${index}`,
    pitch,
    start: step * 0.25,
    duration,
    velocity,
  }));
}

function chord(
  clipId: string,
  pitches: number[],
  start: number,
  duration: number,
  velocity = 0.62,
): Array<[number, number, number, number?]> {
  return pitches.map((pitch) => [pitch, start, duration, velocity]);
}

const HAT_8 = [0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30];

function drumGroove(id: string): NoteEvent[] {
  return [
    ...hits(id, 0, [0, 4, 8, 10, 12, 16, 20, 24, 26, 28], 0.96, 0.2),
    ...hits(id, 1, [4, 12, 20, 28], 0.9, 0.2),
    ...hits(id, 4, [4, 12, 20, 28], 0.48, 0.2),
    ...hits(id, 2, HAT_8.filter((s) => ![6, 14, 22, 30].includes(s)), 0.42, 0.08),
    ...hits(id, 3, [6, 14, 22, 30], 0.5, 0.3),
    ...hits(id, 6, [7, 15, 23], 0.4, 0.1),
    ...hits(id, 5, [29, 30, 31], 0.55, 0.15),
    ...hits(id, 7, [0], 0.35, 0.5),
  ];
}

function clips(): Record<string, Clip> {
  const list: Clip[] = [
    {
      id: "clip-hats",
      name: "Hats",
      color: "amber",
      lengthBeats: 4,
      notes: [
        ...hits("clip-hats", 0, [0, 8], 0.8),
        ...hits("clip-hats", 2, [0, 2, 4, 6, 8, 10, 12, 14], 0.46, 0.08),
      ],
    },
    {
      id: "clip-groove",
      name: "Groove",
      color: "amber",
      lengthBeats: 8,
      notes: drumGroove("clip-groove"),
    },
    {
      id: "clip-break",
      name: "Break",
      color: "coral",
      lengthBeats: 8,
      notes: [
        ...hits("clip-break", 0, [0, 8, 16, 24], 0.85),
        ...hits("clip-break", 2, [2, 6, 10, 14, 18, 22, 26, 30], 0.32, 0.08),
        ...hits("clip-break", 3, [14, 30], 0.4, 0.3),
        ...hits("clip-break", 7, [0], 0.7, 0.8),
        ...hits("clip-break", 1, [28], 0.7),
      ],
    },
    {
      id: "clip-bass",
      name: "Bassline",
      color: "mint",
      lengthBeats: 8,
      notes: notes("clip-bass", [
        [33, 0, 0.5, 0.9],
        [33, 0.75, 0.25, 0.7],
        [36, 1.5, 0.5, 0.86],
        [33, 2.5, 0.75, 0.88],
        [31, 3.5, 0.25, 0.72],
        [33, 4, 0.5, 0.9],
        [28, 4.75, 0.25, 0.7],
        [36, 5.5, 0.5, 0.84],
        [33, 6.25, 0.5, 0.86],
        [31, 7, 0.75, 0.78],
      ]),
    },
    {
      id: "clip-bass-long",
      name: "Sub",
      color: "mint",
      lengthBeats: 8,
      notes: notes("clip-bass-long", [
        [33, 0, 1.75, 0.8],
        [31, 2, 1.75, 0.74],
        [28, 4, 1.75, 0.78],
        [36, 6, 1.5, 0.76],
      ]),
    },
    {
      id: "clip-stabs",
      name: "Stabs",
      color: "blue",
      lengthBeats: 8,
      notes: notes("clip-stabs", [
        ...chord("clip-stabs", [57, 60, 64], 0, 0.4, 0.66),
        ...chord("clip-stabs", [57, 60, 64], 1.5, 0.22, 0.5),
        ...chord("clip-stabs", [53, 57, 60], 2, 0.4, 0.64),
        ...chord("clip-stabs", [53, 57, 60], 3.5, 0.22, 0.48),
        ...chord("clip-stabs", [48, 52, 55], 4, 0.4, 0.66),
        ...chord("clip-stabs", [48, 52, 55], 5.5, 0.22, 0.5),
        ...chord("clip-stabs", [55, 59, 62], 6, 0.4, 0.64),
        ...chord("clip-stabs", [55, 59, 62], 7.5, 0.22, 0.5),
      ]),
    },
    {
      id: "clip-pad",
      name: "Pad",
      color: "violet",
      lengthBeats: 8,
      notes: notes("clip-pad", [
        ...chord("clip-pad", [57, 60, 64], 0, 1.9, 0.48),
        ...chord("clip-pad", [53, 57, 60], 2, 1.9, 0.46),
        ...chord("clip-pad", [48, 52, 55], 4, 1.9, 0.48),
        ...chord("clip-pad", [55, 59, 62], 6, 1.9, 0.46),
      ]),
    },
    {
      id: "clip-lead",
      name: "Hook",
      color: "coral",
      lengthBeats: 8,
      notes: notes("clip-lead", [
        [69, 0, 0.5, 0.8],
        [72, 0.5, 0.25, 0.7],
        [74, 0.75, 0.25, 0.72],
        [76, 1, 0.5, 0.84],
        [74, 1.5, 0.25, 0.66],
        [72, 2, 0.5, 0.74],
        [69, 2.5, 0.5, 0.7],
        [67, 3, 0.75, 0.68],
        [69, 4, 0.25, 0.78],
        [69, 4.25, 0.25, 0.66],
        [72, 4.5, 0.5, 0.76],
        [76, 5, 0.75, 0.84],
        [74, 6, 0.5, 0.72],
        [72, 6.5, 0.25, 0.64],
        [69, 7, 0.9, 0.8],
      ]),
    },
    {
      id: "clip-lead-b",
      name: "Lift",
      color: "coral",
      lengthBeats: 8,
      notes: notes("clip-lead-b", [
        [76, 0, 0.25, 0.7],
        [79, 0.5, 0.5, 0.82],
        [76, 1.25, 0.25, 0.66],
        [74, 1.5, 0.5, 0.74],
        [72, 2.25, 0.25, 0.64],
        [76, 2.5, 0.75, 0.8],
        [81, 4, 0.5, 0.84],
        [79, 4.5, 0.25, 0.7],
        [76, 4.75, 0.25, 0.68],
        [74, 5, 0.5, 0.72],
        [72, 5.5, 0.5, 0.7],
        [69, 6, 1.5, 0.78],
      ]),
    },
    {
      id: "clip-air",
      name: "Air",
      color: "teal",
      lengthBeats: 16,
      notes: [],
      audio: { bufferId: "air", sourceBpm: AIR_BPM, warp: true, peaks: AIR_PEAKS },
    },
    {
      id: "clip-mute-d",
      name: "Mute",
      color: "amber",
      lengthBeats: 4,
      notes: [],
    },
    {
      id: "clip-mute-b",
      name: "Mute",
      color: "mint",
      lengthBeats: 4,
      notes: [],
    },
    {
      id: "clip-mute-c",
      name: "Mute",
      color: "blue",
      lengthBeats: 4,
      notes: [],
    },
    {
      id: "clip-mute-l",
      name: "Mute",
      color: "coral",
      lengthBeats: 4,
      notes: [],
    },
    {
      id: "clip-mute-a",
      name: "Mute",
      color: "teal",
      lengthBeats: 4,
      notes: [],
    },
  ];
  return Object.fromEntries(list.map((clip) => [clip.id, clip]));
}

function slots(ids: Array<string | null>): Array<string | null> {
  return ids;
}

function tracks(): Track[] {
  return [
    {
      id: "tr-drums",
      name: "Drums",
      kind: "midi",
      color: "amber",
      mute: false,
      solo: false,
      arm: true,
      volume: 0.82,
      pan: 0,
      sendRev: 0.08,
      sendDly: 0.02,
      devices: [
        createDevice("dev-drums", "drums"),
        createDevice("dev-drums-eq", "eq", { low: 1.5, mid: -1.2, high: 1.8, lowFreq: 90, highFreq: 6500 }),
        createDevice("dev-drums-comp", "compressor", { threshold: -14, ratio: 3.4, makeup: 2 }),
      ],
      slots: slots([
        "clip-hats",
        "clip-groove",
        "clip-groove",
        "clip-hats",
        "clip-groove",
        "clip-break",
        "clip-break",
        "clip-mute-d",
      ]),
    },
    {
      id: "tr-bass",
      name: "Bass",
      kind: "midi",
      color: "mint",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.78,
      pan: 0,
      sendRev: 0.04,
      sendDly: 0,
      devices: [
        createDevice("dev-bass", "analog", {
          wave: 1,
          wave2: 0,
          mix: 0.18,
          detune: 5,
          cutoff: 240,
          resonance: 1.4,
          env: 0.35,
          attack: 0.005,
          decay: 0.18,
          sustain: 0.45,
          release: 0.12,
          gain: 0.8,
        }),
        createDevice("dev-bass-eq", "eq", { low: 3, mid: -1, high: -2, lowFreq: 120, highFreq: 2500 }),
        createDevice("dev-bass-comp", "compressor", { threshold: -18, ratio: 4, makeup: 3 }),
      ],
      slots: slots([
        null,
        "clip-bass",
        "clip-bass",
        null,
        "clip-bass",
        "clip-bass-long",
        "clip-bass-long",
        "clip-mute-b",
      ]),
    },
    {
      id: "tr-chords",
      name: "Chords",
      kind: "midi",
      color: "blue",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.62,
      pan: -0.15,
      sendRev: 0.28,
      sendDly: 0.08,
      devices: [
        createDevice("dev-chords", "analog", {
          wave: 2,
          wave2: 1,
          mix: 0.4,
          detune: 12,
          cutoff: 1600,
          resonance: 0.7,
          env: 0.38,
          attack: 0.02,
          decay: 0.28,
          sustain: 0.62,
          release: 0.4,
          gain: 0.55,
        }),
        createDevice("dev-chords-eq", "eq", { low: -2, mid: 1, high: 1.5 }),
      ],
      slots: slots([
        null,
        "clip-stabs",
        null,
        "clip-pad",
        "clip-stabs",
        "clip-pad",
        "clip-pad",
        "clip-mute-c",
      ]),
    },
    {
      id: "tr-lead",
      name: "Lead",
      kind: "midi",
      color: "coral",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.58,
      pan: 0.18,
      sendRev: 0.22,
      sendDly: 0.2,
      devices: [
        createDevice("dev-lead", "analog", {
          wave: 1,
          wave2: 1,
          mix: 0.22,
          detune: 7,
          cutoff: 2400,
          resonance: 1.1,
          env: 0.62,
          attack: 0.008,
          decay: 0.16,
          sustain: 0.28,
          release: 0.2,
          gain: 0.62,
        }),
        createDevice("dev-lead-eq", "eq", { low: -3, mid: 1.5, high: 2.2, highFreq: 5000 }),
        createDevice("dev-lead-delay", "delay", { time: 0.75, feedback: 0.34, mix: 0.16, tone: 2800 }),
      ],
      slots: slots([
        null,
        "clip-lead",
        null,
        null,
        "clip-lead-b",
        "clip-lead",
        null,
        "clip-mute-l",
      ]),
    },
    {
      id: "tr-air",
      name: "Air",
      kind: "audio",
      color: "teal",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.42,
      pan: 0,
      sendRev: 0.34,
      sendDly: 0.05,
      devices: [createDevice("dev-air-eq", "eq", { low: -4, mid: 0, high: 2, lowFreq: 200, highFreq: 5000 })],
      slots: slots([
        "clip-air",
        "clip-air",
        "clip-air",
        "clip-air",
        "clip-air",
        "clip-air",
        "clip-air",
        "clip-mute-a",
      ]),
    },
    {
      id: "tr-rev",
      name: "Reverb",
      kind: "return",
      color: "violet",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.7,
      pan: 0,
      sendRev: 0,
      sendDly: 0,
      devices: [createDevice("dev-return-rev", "reverb", { decay: 0.74, tone: 5200, mix: 1 })],
      slots: [],
    },
    {
      id: "tr-dly",
      name: "Delay",
      kind: "return",
      color: "teal",
      mute: false,
      solo: false,
      arm: false,
      volume: 0.55,
      pan: 0,
      sendRev: 0.15,
      sendDly: 0,
      devices: [createDevice("dev-return-dly", "delay", { time: 0.75, feedback: 0.42, tone: 2600, mix: 1 })],
      slots: [],
    },
  ];
}

function arrange(trackId: string, clipId: string, starts: number[]): ArrangementClip[] {
  return starts.map((startBeat) => ({
    id: `arr-${trackId}-${clipId}-${startBeat}`,
    trackId,
    clipId,
    startBeat,
  }));
}

export function createDemoProject(): Project {
  return {
    version: 1,
    name: "Night Market",
    bpm: AIR_BPM,
    swing: 0.08,
    quantize: 4,
    metronome: false,
    master: 0.84,
    loop: false,
    loopStart: 0,
    loopEnd: 32,
    scenes: [
      { id: "sc-1", name: "Hats" },
      { id: "sc-2", name: "Groove" },
      { id: "sc-3", name: "Rhythm" },
      { id: "sc-4", name: "Keys" },
      { id: "sc-5", name: "Hook" },
      { id: "sc-6", name: "Lift" },
      { id: "sc-7", name: "Break" },
      { id: "sc-8", name: "Mute" },
    ],
    tracks: tracks(),
    clips: clips(),
    arrangement: [
      ...arrange("tr-drums", "clip-groove", [0, 8, 16, 24, 32, 40]),
      ...arrange("tr-drums", "clip-break", [48]),
      ...arrange("tr-drums", "clip-groove", [56]),
      ...arrange("tr-bass", "clip-bass", [8, 16, 24, 32, 40]),
      ...arrange("tr-bass", "clip-bass-long", [48]),
      ...arrange("tr-bass", "clip-bass", [56]),
      ...arrange("tr-chords", "clip-stabs", [16, 24]),
      ...arrange("tr-chords", "clip-pad", [32]),
      ...arrange("tr-chords", "clip-stabs", [40]),
      ...arrange("tr-chords", "clip-pad", [48]),
      ...arrange("tr-chords", "clip-stabs", [56]),
      ...arrange("tr-lead", "clip-lead", [32]),
      ...arrange("tr-lead", "clip-lead-b", [40]),
      ...arrange("tr-lead", "clip-lead", [48]),
      ...arrange("tr-lead", "clip-lead-b", [56]),
      ...arrange("tr-air", "clip-air", [0, 16, 32, 48]),
    ],
    lengthBeats: 64,
    bindings: [],
  };
}

export const STORAGE_KEY = "lattice.project.v1";

export function isProject(value: unknown): value is Project {
  if (!value || typeof value !== "object") return false;
  const project = value as Project;
  return (
    project.version === 1 &&
    Array.isArray(project.tracks) &&
    project.tracks.length > 0 &&
    !!project.clips &&
    Array.isArray(project.scenes) &&
    Array.isArray(project.arrangement)
  );
}
