import { useRef, type PointerEvent as ReactPointerEvent } from "react";
import { CLIP_FACE, STRIPE, formatPosition } from "@/daw/format";
import { useDaw } from "@/daw/store";

export function ArrangementView() {
  const project = useDaw((state) => state.project);
  const px = useDaw((state) => state.pxPerBeat);
  const follow = useDaw((state) => state.follow);
  const selectedTrackId = useDaw((state) => state.selectedTrackId);
  const selectedArrangementId = useDaw((state) => state.selectedArrangementId);
  const selectedClipId = useDaw((state) => state.selectedClipId);
  const scroller = useRef<HTMLDivElement>(null);
  const width = Math.max(project.lengthBeats * px + 80, 640);
  const lanes = project.tracks.filter((track) => track.kind !== "return");

  return (
    <div className="flex h-full min-h-0 flex-col">
      <div className="flex shrink-0 items-center gap-2 border-b border-line px-3 py-2 text-sm">
        <span className="text-muted">Timeline</span>
        <button type="button" className="h-10 rounded-md bg-raised px-3" onClick={() => useDaw.getState().setZoom(Math.max(16, px - 8))}>
          Zoom out
        </button>
        <button type="button" className="h-10 rounded-md bg-raised px-3" onClick={() => useDaw.getState().setZoom(Math.min(72, px + 8))}>
          Zoom in
        </button>
        <button
          type="button"
          aria-pressed={follow}
          className={`h-10 rounded-md px-3 ${follow ? "bg-accent text-accent-ink" : "bg-raised text-muted"}`}
          onClick={() => useDaw.getState().setFollow(!follow)}
        >
          Follow
        </button>
        <span className="hidden text-2xs text-muted lg:inline">Double-click a lane to drop the selected clip. Drag to move.</span>
      </div>
      <div
        ref={scroller}
        data-arrange-scroll=""
        data-follow={follow ? "true" : "false"}
        className="min-h-0 flex-1 overflow-auto"
        onScroll={() => {
          if (!useDaw.getState().snap.playing) return;
        }}
      >
        <div className="relative" style={{ width: width + 112 }} data-arrange="" data-ppb={px} data-origin="112">
          <div className="sticky top-0 z-10 flex h-8 border-b border-line bg-surface text-2xs text-muted">
            <div className="sticky left-0 z-10 w-28 shrink-0 bg-surface" />
            <div
              data-ruler=""
              className="relative h-full flex-1"
              onPointerDown={(event) => startLoop(event, px)}
            >
              {Array.from({ length: Math.ceil(project.lengthBeats / 4) + 1 }, (_, bar) => (
                <span key={bar} className="absolute top-1 font-mono" style={{ left: bar * 4 * px }}>
                  {bar + 1}
                </span>
              ))}
              {project.loop ? (
                <span
                  className="absolute bottom-0 h-1 bg-accent"
                  style={{ left: project.loopStart * px, width: (project.loopEnd - project.loopStart) * px }}
                />
              ) : null}
            </div>
          </div>
          {lanes.map((track) => (
            <div key={track.id} className={`flex h-16 border-b border-line ${track.id === selectedTrackId ? "bg-raised/40" : ""}`}>
              <button
                type="button"
                onClick={() => useDaw.getState().selectTrack(track.id)}
                className="sticky left-0 z-10 flex w-28 shrink-0 items-center gap-2 border-r border-line bg-surface px-2 text-left text-sm"
              >
                <span className={`h-6 w-1 rounded ${STRIPE[track.color]}`} />
                <span className="truncate">{track.name}</span>
              </button>
              <div
                className="relative h-full flex-1"
                onDoubleClick={(event) => {
                  if (!selectedClipId) return;
                  const owns =
                    track.slots.includes(selectedClipId) ||
                    project.arrangement.some((item) => item.trackId === track.id && item.clipId === selectedClipId);
                  if (!owns) return;
                  const rect = event.currentTarget.getBoundingClientRect();
                  const beat = Math.max(0, Math.round((event.clientX - rect.left) / px));
                  useDaw.getState().placeArrangement(track.id, selectedClipId, beat);
                }}
              >
                {project.arrangement
                  .filter((event) => event.trackId === track.id)
                  .map((event) => {
                    const clip = project.clips[event.clipId];
                    if (!clip) return null;
                    const selected = event.id === selectedArrangementId;
                    return (
                      <button
                        key={event.id}
                        type="button"
                        onPointerDown={(pointer) => dragClip(pointer, event.id, event.startBeat, px)}
                        onClick={() => {
                          useDaw.getState().selectArrangement(event.id);
                          useDaw.getState().selectClip(track.id, clip.id);
                        }}
                        className={`absolute top-1 flex h-12 items-center overflow-hidden rounded-sm px-2 text-left text-xs font-semibold ${CLIP_FACE[clip.color]} ${selected ? "outline outline-2 outline-offset-1 outline-fg" : ""}`}
                        style={{ left: event.startBeat * px, width: Math.max(24, clip.lengthBeats * px - 4) }}
                      >
                        <span className="truncate">
                          {clip.name}
                          <span className="ml-2 font-mono font-normal">{formatPosition(event.startBeat)}</span>
                        </span>
                      </button>
                    );
                  })}
              </div>
            </div>
          ))}
          <div data-playhead="" className="pointer-events-none absolute bottom-0 top-8 left-28 z-20 w-px bg-accent" />
        </div>
      </div>
    </div>
  );
}

function dragClip(event: ReactPointerEvent, id: string, origin: number, px: number) {
  const startX = event.clientX;
  let moved = false;
  const move = (pointer: PointerEvent) => {
    const delta = pointer.clientX - startX;
    if (Math.abs(delta) < 3) return;
    moved = true;
    useDaw.getState().moveArrangement(id, origin + delta / px);
  };
  const up = () => {
    window.removeEventListener("pointermove", move);
    window.removeEventListener("pointerup", up);
    if (!moved) useDaw.getState().selectArrangement(id);
  };
  window.addEventListener("pointermove", move);
  window.addEventListener("pointerup", up);
}

function startLoop(event: ReactPointerEvent<HTMLDivElement>, px: number) {
  const rect = event.currentTarget.getBoundingClientRect();
  const beatAt = (clientX: number) => Math.max(0, Math.round((clientX - rect.left) / px));
  const start = beatAt(event.clientX);
  const move = (pointer: PointerEvent) => {
    const end = beatAt(pointer.clientX);
    useDaw.getState().setLoopRegion(Math.min(start, end), Math.max(start, end));
  };
  const up = (pointer: PointerEvent) => {
    window.removeEventListener("pointermove", move);
    window.removeEventListener("pointerup", up);
    const end = beatAt(pointer.clientX);
    if (end === start) useDaw.getState().setLoop(!useDaw.getState().project.loop);
  };
  window.addEventListener("pointermove", move);
  window.addEventListener("pointerup", up);
}
