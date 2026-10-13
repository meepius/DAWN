import { getEngine } from "@/daw/engine";
import { DRUM_LABELS, KEY_ROWS, noteName } from "@/daw/format";
import { useDaw } from "@/daw/store";

export function PlayKeys() {
  const trackId = useDaw((state) => state.selectedTrackId);
  const octave = useDaw((state) => state.octave);
  const track = useDaw((state) => state.project.tracks.find((item) => item.id === trackId));
  if (!track || track.kind !== "midi") return null;
  const drums = track.devices.some((device) => device.kind === "drums");

  return (
    <div className="border-t border-line px-3 py-3">
      <div className="mb-2 flex items-center justify-between gap-2">
        <p className="text-2xs uppercase tracking-wider text-muted">Play {track.name}</p>
        {drums ? null : (
          <div className="flex items-center gap-1">
            <button type="button" className="h-10 rounded-md bg-raised px-3 text-sm" onClick={() => useDaw.getState().setOctave(octave - 1)}>
              Octave −
            </button>
            <span className="w-12 text-center font-mono text-sm">{noteName(12 * (octave + 1))}</span>
            <button type="button" className="h-10 rounded-md bg-raised px-3 text-sm" onClick={() => useDaw.getState().setOctave(octave + 1)}>
              Octave +
            </button>
          </div>
        )}
      </div>
      {drums ? (
        <div className="grid grid-cols-4 gap-2 sm:grid-cols-8">
          {DRUM_LABELS.map((label, pad) => (
            <KeyButton key={label} label={label} hint={String(pad + 1)} onDown={() => void getEngine().noteOn(pad, 0.94)} onUp={() => getEngine().noteOff(pad)} />
          ))}
        </div>
      ) : (
        <div className="flex gap-1 overflow-x-auto pb-1">
          {KEY_ROWS.map((row) => {
            const pitch = 12 * (octave + 1) + row.semi;
            const black = [1, 3, 6, 8, 10, 13, 15].includes(row.semi);
            return (
              <KeyButton
                key={row.key}
                label={noteName(pitch)}
                hint={row.key.toUpperCase()}
                black={black}
                onDown={() => void getEngine().noteOn(pitch, 0.9)}
                onUp={() => getEngine().noteOff(pitch)}
              />
            );
          })}
        </div>
      )}
    </div>
  );
}

function KeyButton({
  label,
  hint,
  black,
  onDown,
  onUp,
}: {
  label: string;
  hint: string;
  black?: boolean;
  onDown: () => void;
  onUp: () => void;
}) {
  return (
    <button
      type="button"
      onPointerDown={(event) => {
        event.preventDefault();
        event.currentTarget.setPointerCapture(event.pointerId);
        onDown();
      }}
      onPointerUp={onUp}
      onPointerCancel={onUp}
      className={`flex h-16 min-w-14 flex-1 flex-col items-center justify-center rounded-md text-sm ${black ? "bg-bg text-fg" : "bg-raised text-fg"}`}
    >
      <span>{label}</span>
      <span className="font-mono text-2xs text-muted">{hint}</span>
    </button>
  );
}
