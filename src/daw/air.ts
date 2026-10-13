export const AIR_BPM = 122;
export const AIR_BEATS = 16;

export interface AirRender {
  left: Float32Array;
  right: Float32Array;
  peaks: number[];
}

export function airDuration(bpm = AIR_BPM): number {
  return (AIR_BEATS * 60) / bpm;
}

/** Deterministic stereo pad used as the demo audio clip. */
export function synthesizeAir(sampleRate: number, seconds = airDuration()): AirRender {
  const frames = Math.max(1, Math.floor(sampleRate * seconds));
  const left = new Float32Array(frames);
  const right = new Float32Array(frames);
  const freqs = [110, 164.81, 220, 261.63, 329.63];
  const amps = [0.2, 0.15, 0.13, 0.09, 0.07];

  for (let i = 0; i < frames; i++) {
    const t = i / sampleRate;
    let l = 0;
    let r = 0;
    for (let k = 0; k < freqs.length; k++) {
      const trem = 0.78 + 0.22 * Math.sin(2 * Math.PI * (0.07 + k * 0.025) * t + k);
      const f = freqs[k];
      const a = amps[k] * trem;
      l += Math.sin(2 * Math.PI * f * t) * a;
      l += Math.sin(2 * Math.PI * f * 2 * t) * a * 0.12;
      r += Math.sin(2 * Math.PI * f * t + 0.35 + k * 0.2) * a;
      r += Math.sin(2 * Math.PI * f * 2 * t + 0.5) * a * 0.1;
    }
    const hash = Math.sin(i * 12.9898) * 43758.5453;
    const noise = (hash - Math.floor(hash)) * 2 - 1;
    const edge = Math.min(1, t / 0.18, (seconds - t) / 0.28);
    const env = Math.max(0, edge);
    left[i] = Math.tanh((l + noise * 0.012) * env);
    right[i] = Math.tanh((r + noise * 0.01) * env);
  }

  const buckets = 96;
  const size = Math.max(1, Math.floor(frames / buckets));
  const peaks: number[] = [];
  for (let b = 0; b < buckets; b++) {
    let peak = 0;
    const start = b * size;
    const end = Math.min(frames, start + size);
    for (let i = start; i < end; i++) peak = Math.max(peak, Math.abs(left[i]));
    peaks.push(Math.round(peak * 100) / 100);
  }
  return { left, right, peaks };
}

export const AIR_PEAKS: number[] = [
  0.12, 0.28, 0.41, 0.5, 0.55, 0.58, 0.6, 0.62, 0.61, 0.63, 0.64, 0.62, 0.6, 0.63, 0.65,
  0.64, 0.62, 0.6, 0.63, 0.66, 0.64, 0.61, 0.59, 0.62, 0.65, 0.67, 0.64, 0.62, 0.6, 0.63,
  0.66, 0.64, 0.61, 0.63, 0.66, 0.68, 0.65, 0.62, 0.6, 0.63, 0.66, 0.64, 0.62, 0.65, 0.67,
  0.64, 0.61, 0.59, 0.62, 0.65, 0.63, 0.6, 0.62, 0.66, 0.64, 0.61, 0.63, 0.65, 0.62, 0.58,
  0.55, 0.57, 0.6, 0.62, 0.59, 0.56, 0.58, 0.61, 0.63, 0.6, 0.57, 0.54, 0.56, 0.59, 0.57,
  0.53, 0.5, 0.52, 0.55, 0.52, 0.48, 0.45, 0.47, 0.44, 0.4, 0.36, 0.32, 0.27, 0.21, 0.14,
  0.08, 0.04,
];
