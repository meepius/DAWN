export type ClipColor = "amber" | "mint" | "blue" | "coral" | "violet" | "teal";

export type DeviceKind =
  | "analog"
  | "drums"
  | "eq"
  | "compressor"
  | "delay"
  | "reverb";

export interface NoteEvent {
  id: string;
  pitch: number;
  start: number;
  duration: number;
  velocity: number;
}

export interface AudioClipData {
  bufferId: string;
  sourceBpm: number;
  warp: boolean;
  peaks: number[];
}

export interface Clip {
  id: string;
  name: string;
  color: ClipColor;
  lengthBeats: number;
  notes: NoteEvent[];
  audio?: AudioClipData;
}

export interface Device {
  id: string;
  kind: DeviceKind;
  name: string;
  bypass: boolean;
  params: Record<string, number>;
}

export type TrackKind = "midi" | "audio" | "return";

export interface Track {
  id: string;
  name: string;
  kind: TrackKind;
  color: ClipColor;
  mute: boolean;
  solo: boolean;
  arm: boolean;
  volume: number;
  pan: number;
  sendRev: number;
  sendDly: number;
  devices: Device[];
  slots: (string | null)[];
}

export interface Scene {
  id: string;
  name: string;
}

export interface ArrangementClip {
  id: string;
  trackId: string;
  clipId: string;
  startBeat: number;
}

export interface MidiBinding {
  cc: number;
  target: string;
}

export interface Project {
  version: 1;
  name: string;
  bpm: number;
  swing: number;
  quantize: number;
  metronome: boolean;
  master: number;
  loop: boolean;
  loopStart: number;
  loopEnd: number;
  scenes: Scene[];
  tracks: Track[];
  clips: Record<string, Clip>;
  arrangement: ArrangementClip[];
  lengthBeats: number;
  bindings: MidiBinding[];
}

export interface TransportSnap {
  playing: boolean;
  launched: Record<string, string>;
  launchedSlot: Record<string, number>;
  queued: Record<string, string>;
  queuedSlot: Record<string, number>;
}
