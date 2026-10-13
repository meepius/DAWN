import type { Device, DeviceKind } from "@/daw/types";

export interface ParamDef {
  key: string;
  label: string;
  min: number;
  max: number;
  step: number;
  format: (value: number) => string;
}

const hz = (value: number) =>
  value >= 1000 ? `${(value / 1000).toFixed(1)}k` : `${Math.round(value)}`;
const db = (value: number) => `${value > 0 ? "+" : ""}${value.toFixed(1)}`;
const pct = (value: number) => `${Math.round(value * 100)}%`;
const num = (digits: number) => (value: number) => value.toFixed(digits);
const beats = (value: number) => `${value.toFixed(2)}`;

export const PARAMS: Record<DeviceKind, ParamDef[]> = {
  analog: [
    { key: "wave", label: "Osc A", min: 0, max: 3, step: 1, format: (v) => WAVE[v] ?? "Saw" },
    { key: "wave2", label: "Osc B", min: 0, max: 3, step: 1, format: (v) => WAVE[v] ?? "Saw" },
    { key: "mix", label: "Mix", min: 0, max: 1, step: 0.01, format: pct },
    { key: "detune", label: "Detune", min: -40, max: 40, step: 1, format: (v) => `${Math.round(v)} ct` },
    { key: "cutoff", label: "Cutoff", min: 40, max: 9000, step: 1, format: hz },
    { key: "resonance", label: "Reso", min: 0.2, max: 14, step: 0.1, format: num(1) },
    { key: "env", label: "Env", min: 0, max: 1, step: 0.01, format: pct },
    { key: "attack", label: "Attack", min: 0.005, max: 1.5, step: 0.005, format: (v) => `${v.toFixed(2)}s` },
    { key: "decay", label: "Decay", min: 0.02, max: 1.5, step: 0.01, format: (v) => `${v.toFixed(2)}s` },
    { key: "sustain", label: "Sustain", min: 0, max: 1, step: 0.01, format: pct },
    { key: "release", label: "Release", min: 0.02, max: 2, step: 0.01, format: (v) => `${v.toFixed(2)}s` },
    { key: "gain", label: "Level", min: 0, max: 1, step: 0.01, format: pct },
  ],
  drums: [
    { key: "tune", label: "Tune", min: 0, max: 1, step: 0.01, format: pct },
    { key: "decay", label: "Decay", min: 0, max: 1, step: 0.01, format: pct },
    { key: "snap", label: "Snap", min: 0, max: 1, step: 0.01, format: pct },
    { key: "hat", label: "Hat", min: 0, max: 1, step: 0.01, format: pct },
    { key: "tone", label: "Tone", min: 0, max: 1, step: 0.01, format: pct },
    { key: "gain", label: "Level", min: 0, max: 1, step: 0.01, format: pct },
  ],
  eq: [
    { key: "low", label: "Low", min: -12, max: 12, step: 0.1, format: db },
    { key: "mid", label: "Mid", min: -12, max: 12, step: 0.1, format: db },
    { key: "high", label: "High", min: -12, max: 12, step: 0.1, format: db },
    { key: "lowFreq", label: "Low Hz", min: 60, max: 400, step: 1, format: hz },
    { key: "highFreq", label: "High Hz", min: 1500, max: 8000, step: 10, format: hz },
  ],
  compressor: [
    { key: "threshold", label: "Thresh", min: -40, max: 0, step: 0.5, format: db },
    { key: "ratio", label: "Ratio", min: 1, max: 12, step: 0.1, format: (v) => `${v.toFixed(1)}:1` },
    { key: "attack", label: "Attack", min: 0.001, max: 0.2, step: 0.001, format: (v) => `${Math.round(v * 1000)}ms` },
    { key: "release", label: "Release", min: 0.02, max: 0.6, step: 0.01, format: (v) => `${Math.round(v * 1000)}ms` },
    { key: "makeup", label: "Makeup", min: 0, max: 12, step: 0.1, format: db },
  ],
  delay: [
    { key: "time", label: "Time", min: 0.125, max: 2, step: 0.125, format: beats },
    { key: "feedback", label: "Feed", min: 0, max: 0.85, step: 0.01, format: pct },
    { key: "tone", label: "Tone", min: 800, max: 8000, step: 10, format: hz },
    { key: "mix", label: "Mix", min: 0, max: 1, step: 0.01, format: pct },
  ],
  reverb: [
    { key: "decay", label: "Decay", min: 0.2, max: 0.92, step: 0.01, format: pct },
    { key: "tone", label: "Tone", min: 800, max: 9000, step: 10, format: hz },
    { key: "mix", label: "Mix", min: 0, max: 1, step: 0.01, format: pct },
  ],
};

const WAVE = ["Sine", "Saw", "Square", "Tri"];

export const DEFAULT_PARAMS: Record<DeviceKind, Record<string, number>> = {
  analog: {
    wave: 1,
    wave2: 1,
    mix: 0.28,
    detune: 8,
    cutoff: 1400,
    resonance: 0.8,
    env: 0.42,
    attack: 0.01,
    decay: 0.22,
    sustain: 0.55,
    release: 0.25,
    gain: 0.75,
  },
  drums: { tune: 0.42, decay: 0.48, snap: 0.62, hat: 0.4, tone: 0.55, gain: 0.85 },
  eq: { low: 0, mid: 0, high: 0, lowFreq: 180, highFreq: 4200 },
  compressor: { threshold: -16, ratio: 3, attack: 0.012, release: 0.16, makeup: 1.5 },
  delay: { time: 0.75, feedback: 0.32, tone: 3200, mix: 0.18 },
  reverb: { decay: 0.72, tone: 4800, mix: 0.22 },
};

const NAMES: Record<DeviceKind, string> = {
  analog: "Analog",
  drums: "Drum Rack",
  eq: "EQ Three",
  compressor: "Compressor",
  delay: "Delay",
  reverb: "Reverb",
};

export function createDevice(id: string, kind: DeviceKind, params?: Record<string, number>): Device {
  return {
    id,
    kind,
    name: NAMES[kind],
    bypass: false,
    params: { ...DEFAULT_PARAMS[kind], ...params },
  };
}

export function paramNumber(params: Record<string, number>, key: string, fallback: number): number {
  const value = params[key];
  return typeof value === "number" && Number.isFinite(value) ? value : fallback;
}
