import { useRef } from "react";
import { toast } from "sonner";
import { getEngine } from "@/daw/engine";
import { DRUM_LABELS, isBlackKey, noteName } from "@/daw/format";
import { useDaw } from "@/daw/store";

const LENGTHS = [4, 8, 16, 32];

export function ClipEditor() {
  const project = useDaw((state) => state.project);
  const clipId = useDaw((state) => state.selectedClipId);
  const trackId = useDaw((state) => state.selectedTrackId);
  const octave = useDaw((state) => state.octave);
  const clip = clipId ? project.clips[clipId] : null;
  const track = project.tracks.find((item) => item.id === trackId);
  const fileRef = useRef<HTMLInputElement>(null);

  if (!clip || !track) {
    return <p className="p-4 text-sm text-muted">Select a clip in the session or arrangement.</p>;
  }

  const drums = track.devices.some((device) => device.kind === "drums");
  const steps = Math.round(clip.lengthBeats * 4);

  return (
    <div className="p-3">
      <div className="mb-3 flex flex-wrap items-center gap-2">
        <input
          aria-label="Clip name"
          value={clip.name}
          onChange={(event) => useDaw.getState().renameClip(clip.id, event.target.value)}
          className="h-10 w-36 rounded-md border border-line bg-raised px-2 text-sm"
        />
        {LENGTHS.map((beats) => (
          <button
            key={beats}
            type="button"
            onClick={() => useDaw.getState().setClipLength(clip.id, beats)}
            className={`h-10 rounded-md px-3 text-sm ${clip.lengthBeats === beats ? "bg-accent text-accent-ink" : "bg-raised text-muted"}`}
          >
            {beats / 4} bar{beats / 4 === 1 ? "" : "s"}
          </button>
        ))}
        {clip.audio ? (
          <>
            <button
              type="button"
              aria-pressed={clip.audio.warp}
              onClick={() => useDaw.getState().setWarp(clip.id, !clip.audio?.warp)}
              className={`h-10 rounded-md px-3 text-sm ${clip.audio.warp ? "bg-accent text-accent-ink" : "bg-raised text-muted"}`}
            >
              Warp
            </button>
            <button type="button" className="h-10 rounded-md bg-raised px-3 text-sm" onClick={() => fileRef.current?.click()}>
              Import audio
            </button>
            <input
              ref={fileRef}
              type="file"
              accept="audio/*"
              className="hidden"
              onChange={(event) => {
                const file = event.target.files?.[0];
                event.target.value = "";
                if (file) void importFile(clip.id, file);
              }}
            />
          </>
        ) : (
          <p className="text-2xs text-muted">Click a cell to paint or erase. Octave buttons match the keyboard.</p>
        )}
      </div>

      {clip.audio ? (
        <div className="flex h-24 items-end gap-px rounded-md bg-surface p-2">
          {clip.audio.peaks.map((peak, index) => (
            <span
              key={`${clip.id}-wave-${index}`}
              className="flex-1 rounded-sm bg-clip-teal"
              style={{ height: `${Math.max(6, Math.round(peak * 100))}%` }}
            />
          ))}
        </div>
      ) : (
        <div className="overflow-auto rounded-md border border-line" data-roll="" data-clip={clip.id} data-track={track.id}>
          <div className="relative inline-block min-w-full">
            <div className="flex">
              <div className="sticky left-0 z-10 w-14 shrink-0 bg-bg" />
              {Array.from({ length: steps }, (_, step) => (
                <div key={step} className={`w-8 shrink-0 py-1 text-center font-mono text-2xs md:w-6 ${step % 4 === 0 ? "text-fg" : "text-muted"}`}>
                  {step % 4 === 0 ? step / 4 + 1 : ""}
                </div>
              ))}
            </div>
            {(drums ? drumRows() : pitchRows(octave)).map((row) => (
              <div key={row.pitch} className="flex">
                <div
                  className={`sticky left-0 z-10 flex w-14 shrink-0 items-center px-1 font-mono text-2xs ${row.black ? "bg-bg text-muted" : "bg-surface text-fg"}`}
                >
                  {row.label}
                </div>
                {Array.from({ length: steps }, (_, step) => {
                  const start = step * 0.25;
                  const on = clip.notes.some((note) => note.pitch === row.pitch && Math.abs(note.start - start) < 0.02);
                  return (
                    <button
                      key={step}
                      type="button"
                      aria-label={`${row.label} step ${step + 1}`}
                      onClick={() => useDaw.getState().toggleNote(clip.id, row.pitch, start, drums ? 0.2 : 0.25)}
                      className={`h-11 w-8 shrink-0 border-b border-r border-line md:h-7 md:w-6 ${on ? "bg-accent" : step % 4 === 0 ? "bg-raised" : row.black ? "bg-bg" : "bg-surface"}`}
                    />
                  );
                })}
              </div>
            ))}
            <div data-roll-play="" hidden className="pointer-events-none absolute bottom-0 top-6 z-20 w-px bg-rec" />
          </div>
        </div>
      )}
    </div>
  );
}

function drumRows() {
  return DRUM_LABELS.map((label, pitch) => ({ pitch, label, black: false }));
}

function pitchRows(octave: number) {
  const base = 12 * (octave + 1);
  const rows = [];
  for (let pitch = base + 12; pitch >= base; pitch--) {
    rows.push({ pitch, label: noteName(pitch), black: isBlackKey(pitch) });
  }
  return rows;
}

async function importFile(clipId: string, file: File) {
  try {
    const data = await file.arrayBuffer();
    const id = `user-${Date.now()}`;
    const result = await getEngine().importBuffer(id, data);
    useDaw.getState().adoptAudio(clipId, id, result.peaks, result.duration);
    toast.success("Audio clip replaced");
  } catch (error) {
    toast.error(error instanceof Error ? error.message : "Could not read that file");
  }
}
