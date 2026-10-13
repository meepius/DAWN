import type { ClipColor } from "@/daw/types";

export function formatPosition(beat: number): string {
  const safe = Math.max(0, beat);
  const bar = Math.floor(safe / 4) + 1;
  const beatInBar = Math.floor(safe % 4) + 1;
  const sixteenth = Math.floor((safe % 1) * 4) + 1;
  return `${bar}.${beatInBar}.${sixteenth}`;
}

export function mod(value: number, length: number): number {
  if (length <= 0) return 0;
  return ((value % length) + length) % length;
}

export const NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"] as const;

export function noteName(pitch: number): string {
  const name = NOTE_NAMES[((pitch % 12) + 12) % 12];
  const octave = Math.floor(pitch / 12) - 1;
  return `${name}${octave}`;
}

export function isBlackKey(pitch: number): boolean {
  const pc = ((pitch % 12) + 12) % 12;
  return pc === 1 || pc === 3 || pc === 6 || pc === 8 || pc === 10;
}

export function midiFreq(pitch: number): number {
  return 440 * 2 ** ((pitch - 69) / 12);
}

export const CLIP_FACE: Record<ClipColor, string> = {
  amber: "bg-clip-amber text-ink",
  mint: "bg-clip-mint text-ink",
  blue: "bg-clip-blue text-ink",
  coral: "bg-clip-coral text-ink",
  violet: "bg-clip-violet text-ink",
  teal: "bg-clip-teal text-ink",
};

export const CLIP_SOFT: Record<ClipColor, string> = {
  amber: "bg-clip-amber/20",
  mint: "bg-clip-mint/20",
  blue: "bg-clip-blue/20",
  coral: "bg-clip-coral/20",
  violet: "bg-clip-violet/20",
  teal: "bg-clip-teal/20",
};

export const STRIPE: Record<ClipColor, string> = {
  amber: "bg-clip-amber",
  mint: "bg-clip-mint",
  blue: "bg-clip-blue",
  coral: "bg-clip-coral",
  violet: "bg-clip-violet",
  teal: "bg-clip-teal",
};

export const DRUM_LABELS = ["Kick", "Snare", "Hat", "Open", "Clap", "Tom", "Rim", "Crash"] as const;

export const KEY_ROWS: Array<{ key: string; semi: number }> = [
  { key: "a", semi: 0 },
  { key: "w", semi: 1 },
  { key: "s", semi: 2 },
  { key: "e", semi: 3 },
  { key: "d", semi: 4 },
  { key: "f", semi: 5 },
  { key: "t", semi: 6 },
  { key: "g", semi: 7 },
  { key: "y", semi: 8 },
  { key: "h", semi: 9 },
  { key: "u", semi: 10 },
  { key: "j", semi: 11 },
  { key: "k", semi: 12 },
  { key: "o", semi: 13 },
  { key: "l", semi: 14 },
  { key: "p", semi: 15 },
  { key: ";", semi: 16 },
  { key: "'", semi: 17 },
];

const GM_PAD: Record<number, number> = {
  35: 0,
  36: 0,
  38: 1,
  40: 1,
  42: 2,
  44: 2,
  46: 3,
  39: 4,
  45: 5,
  47: 5,
  37: 6,
  49: 7,
  51: 7,
};

export function drumPad(pitch: number): number | null {
  if (pitch >= 0 && pitch < 8) return pitch;
  return GM_PAD[pitch] ?? null;
}
