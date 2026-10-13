import { Plus } from "lucide-react";
import { getEngine } from "@/daw/engine";
import { CLIP_FACE, STRIPE } from "@/daw/format";
import { useDaw } from "@/daw/store";
import type { Clip, Track } from "@/daw/types";

export function SessionView() {
  const project = useDaw((state) => state.project);
  const selectedTrackId = useDaw((state) => state.selectedTrackId);
  const selectedClipId = useDaw((state) => state.selectedClipId);
  const snap = useDaw((state) => state.snap);
  const content = project.tracks.filter((track) => track.kind !== "return");
  const returns = project.tracks.filter((track) => track.kind === "return");

  return (
    <div className="h-full overflow-auto">
      <div className="inline-flex min-w-full items-start">
        <div className="sticky left-0 z-20 w-24 shrink-0 border-r border-line bg-bg">
          <div className="flex h-36 items-end border-b border-line p-2 text-2xs uppercase tracking-wider text-muted">
            Scenes
          </div>
          {project.scenes.map((scene, index) => {
            const hot = content.some((track) => {
              const slot = track.slots[index];
              return !!slot && snap.launched[track.id] === slot;
            });
            return (
              <button
                key={scene.id}
                type="button"
                onClick={() => getEngine().launchScene(index)}
                className={`flex h-14 w-full items-center justify-between border-b border-line px-2 text-left text-sm ${hot ? "bg-accent text-accent-ink" : "bg-surface text-fg"}`}
              >
                <span className="truncate">{scene.name}</span>
                <span className="font-mono text-2xs">{index + 1}</span>
              </button>
            );
          })}
        </div>
        {content.map((track) => (
          <TrackColumn
            key={track.id}
            track={track}
            selected={track.id === selectedTrackId}
            selectedClipId={selectedClipId}
            launchedId={snap.launched[track.id] ?? null}
            launchedSlot={snap.launchedSlot[track.id] ?? -2}
            queuedId={snap.queued[track.id] ?? null}
            queuedSlot={snap.queuedSlot[track.id] ?? -2}
          />
        ))}
        {returns.map((track) => (
          <ReturnColumn key={track.id} track={track} selected={track.id === selectedTrackId} />
        ))}
      </div>
    </div>
  );
}

function TrackColumn({
  track,
  selected,
  selectedClipId,
  launchedId,
  launchedSlot,
  queuedId,
  queuedSlot,
}: {
  track: Track;
  selected: boolean;
  selectedClipId: string | null;
  launchedId: string | null;
  launchedSlot: number;
  queuedId: string | null;
  queuedSlot: number;
}) {
  const project = useDaw((state) => state.project);
  const drums = track.devices.some((device) => device.kind === "drums");
  return (
    <section
      className={`w-40 shrink-0 border-r border-line ${selected ? "bg-raised/60" : "bg-bg"}`}
      onClick={() => useDaw.getState().selectTrack(track.id)}
    >
      <header className="relative flex h-36 flex-col justify-between border-b border-line p-2 pl-3">
        <span className={`absolute bottom-2 left-0 top-2 w-1 rounded-r ${STRIPE[track.color]}`} />
        <input
          aria-label={`${track.name} name`}
          value={track.name}
          onChange={(event) => useDaw.getState().renameTrack(track.id, event.target.value)}
          className="w-full bg-transparent text-sm font-medium outline-none"
        />
        <div data-meter={track.id} className="meter">
          <span />
        </div>
        <div className="flex gap-1">
          <MiniToggle label="Mute" short="M" on={track.mute} onClick={() => useDaw.getState().toggleMute(track.id)} />
          <MiniToggle label="Solo" short="S" on={track.solo} onClick={() => useDaw.getState().toggleSolo(track.id)} />
          {track.kind === "midi" ? (
            <MiniToggle
              label="Arm"
              short="R"
              on={track.arm}
              danger
              onClick={() => useDaw.getState().toggleArm(track.id)}
            />
          ) : null}
        </div>
        <input
          aria-label={`${track.name} volume`}
          className="daw-slider"
          type="range"
          min={0}
          max={1}
          step={0.01}
          value={track.volume}
          onChange={(event) => useDaw.getState().setTrackNumber(track.id, "volume", Number(event.target.value))}
        />
      </header>
      {project.scenes.map((scene, index) => {
        const clipId = track.slots[index] ?? null;
        const clip = clipId ? project.clips[clipId] : null;
        if (!clip) {
          return (
            <div key={scene.id} className="flex h-14 items-center border-b border-line p-1">
              <button
                type="button"
                aria-label={`New clip on ${track.name}, ${scene.name}`}
                onClick={() => useDaw.getState().createClip(track.id, index)}
                className="flex h-11 w-full items-center justify-center rounded-md border border-dashed border-line text-muted"
              >
                <Plus className="size-4" />
              </button>
            </div>
          );
        }
        const live = launchedId === clip.id && launchedSlot === index;
        const queued = (queuedId === clip.id && queuedSlot === index) || (queuedId === "stop" && live);
        return (
          <div key={scene.id} className="flex h-14 items-center border-b border-line p-1">
            <button
              type="button"
              onClick={() => {
                useDaw.getState().selectClip(track.id, clip.id);
                getEngine().launch(track.id, clip.id, index);
              }}
              className={`relative flex h-12 w-full flex-col justify-between overflow-hidden rounded-md px-2 py-1 text-left ${CLIP_FACE[clip.color]} ${live ? "is-live" : ""} ${queued ? "queued" : ""} ${selectedClipId === clip.id ? "outline outline-2 outline-offset-1 outline-fg" : ""}`}
            >
              <span className="truncate text-xs font-semibold">{clip.name}</span>
              <MiniPreview clip={clip} drums={drums} />
              <span data-clip-progress={`${track.id}:${clip.id}`} className="clip-progress" />
            </button>
          </div>
        );
      })}
    </section>
  );
}

function ReturnColumn({ track, selected }: { track: Track; selected: boolean }) {
  return (
    <section
      className={`w-36 shrink-0 border-r border-line ${selected ? "bg-raised/60" : "bg-bg"}`}
      onClick={() => useDaw.getState().selectTrack(track.id)}
    >
      <header className="relative flex h-36 flex-col gap-2 border-b border-line p-2 pl-3">
        <span className={`absolute bottom-2 left-0 top-2 w-1 ${STRIPE[track.color]}`} />
        <p className="text-sm font-medium">{track.name}</p>
        <p className="text-2xs uppercase tracking-wider text-muted">Return</p>
        <div data-meter={track.id} className="meter">
          <span />
        </div>
        <MiniToggle label="Mute" short="M" on={track.mute} onClick={() => useDaw.getState().toggleMute(track.id)} />
        <input
          aria-label={`${track.name} return level`}
          className="daw-slider"
          type="range"
          min={0}
          max={1}
          step={0.01}
          value={track.volume}
          onChange={(event) => useDaw.getState().setTrackNumber(track.id, "volume", Number(event.target.value))}
        />
      </header>
      <p className="p-3 text-xs leading-relaxed text-muted">Fed by each track’s reverb or delay send.</p>
    </section>
  );
}

function MiniToggle({
  label,
  short,
  on,
  danger,
  onClick,
}: {
  label: string;
  short: string;
  on: boolean;
  danger?: boolean;
  onClick: () => void;
}) {
  return (
    <button
      type="button"
      aria-label={label}
      aria-pressed={on}
      onClick={(event) => {
        event.stopPropagation();
        onClick();
      }}
      className={`inline-flex h-8 min-w-8 flex-1 items-center justify-center rounded-sm text-xs font-semibold ${on ? (danger ? "bg-rec text-fg" : "bg-accent text-accent-ink") : "bg-bg text-muted"}`}
    >
      {short}
    </button>
  );
}

function MiniPreview({ clip, drums }: { clip: Clip; drums: boolean }) {
  if (clip.audio) {
    const peaks = clip.audio.peaks.filter((_, index) => index % 4 === 0);
    return (
      <span className="flex h-4 items-end gap-px">
        {peaks.map((peak, index) => (
          <span
            key={`${clip.id}-pk-${index}`}
            className="block w-px flex-1 bg-ink/70"
            style={{ height: `${Math.max(12, Math.round(peak * 100))}%` }}
          />
        ))}
      </span>
    );
  }
  return (
    <span className="relative block h-4">
      {clip.notes.slice(0, 40).map((note) => (
        <span
          key={note.id}
          className="absolute bottom-0 bg-ink/75"
          style={{
            left: `${(note.start / clip.lengthBeats) * 100}%`,
            width: `${Math.max(4, (note.duration / clip.lengthBeats) * 100)}%`,
            height: drums ? "70%" : `${30 + (note.pitch % 12) * 5}%`,
          }}
        />
      ))}
    </span>
  );
}
