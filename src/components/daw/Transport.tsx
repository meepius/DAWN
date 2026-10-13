import { useState } from "react";
import { Circle, Download, Keyboard, LayoutGrid, Play, Repeat, RotateCcw, Rows3, Square } from "lucide-react";
import { toast } from "sonner";
import { getEngine } from "@/daw/engine";
import { useDaw } from "@/daw/store";

const QUANTS: Array<[number, string]> = [
  [0, "Off"],
  [1, "1/4"],
  [2, "1/2"],
  [4, "1 bar"],
];

export function Transport() {
  const project = useDaw((state) => state.project);
  const view = useDaw((state) => state.view);
  const snap = useDaw((state) => state.snap);
  const record = useDaw((state) => state.record);
  const rendering = useDaw((state) => state.rendering);
  const sampleRate = useDaw((state) => state.sampleRate);
  const learn = useDaw((state) => state.learn);
  const [outputs, setOutputs] = useState<MediaDeviceInfo[]>([]);

  const launched = Object.keys(snap.launched).length > 0;

  return (
    <header className="flex w-full min-w-0 shrink-0 flex-col gap-2 border-b border-line bg-surface px-3 py-2">
      <div className="flex min-w-0 items-center gap-2">
        <div className="flex min-w-0 items-center gap-2 pr-1">
          <span className="grid size-9 shrink-0 place-items-center rounded-md bg-accent font-semibold text-accent-ink">
            L
          </span>
          <div className="hidden min-w-0 sm:block">
            <p className="text-sm font-semibold tracking-widest">LATTICE</p>
            <input
              aria-label="Project name"
              value={project.name}
              onChange={(event) => useDaw.getState().setName(event.target.value)}
              className="w-28 truncate bg-transparent text-2xs text-muted outline-none"
            />
          </div>
        </div>
        <div className="flex items-center gap-1">
          <button
            type="button"
            aria-label={snap.playing ? "Pause" : "Play"}
            aria-pressed={snap.playing}
            onClick={() => void getEngine().toggle()}
            className={`inline-flex size-11 items-center justify-center rounded-md ${snap.playing ? "bg-accent text-accent-ink" : "bg-raised text-fg"}`}
          >
            <Play className="size-4" fill="currentColor" />
          </button>
          <button
            type="button"
            aria-label="Stop and return to start"
            onClick={() => getEngine().stop()}
            className="inline-flex size-11 items-center justify-center rounded-md bg-raised text-fg"
          >
            <Square className="size-4" fill="currentColor" />
          </button>
          <button
            type="button"
            aria-label="Record into launched clip"
            aria-pressed={record}
            onClick={() => toggleRecord()}
            className={`inline-flex size-11 items-center justify-center rounded-md ${record ? "bg-rec text-fg" : "bg-raised text-rec"}`}
          >
            <Circle className="size-4" fill="currentColor" />
          </button>
        </div>
        <p data-position="" className="min-w-14 font-mono text-sm tabular-nums text-fg" aria-live="off">
          1.1.1
        </p>
        <label className="flex items-center gap-1 text-2xs text-muted">
          <span className="hidden sm:inline">BPM</span>
          <input
            aria-label="Tempo"
            type="number"
            min={60}
            max={240}
            value={project.bpm}
            onChange={(event) => useDaw.getState().setBpm(Number(event.target.value))}
            className="h-10 w-16 rounded-md border border-line bg-raised px-2 font-mono text-sm text-fg"
          />
        </label>
        {sampleRate ? (
          <span className="hidden font-mono text-2xs text-muted sm:inline">{(sampleRate / 1000).toFixed(1)} kHz</span>
        ) : null}
      </div>
      <div className="tool-row flex min-w-0 max-w-full items-center gap-2 overflow-x-auto">
        <input
          aria-label="Project name"
          value={project.name}
          onChange={(event) => useDaw.getState().setName(event.target.value)}
          className="h-10 w-28 rounded-md border border-line bg-raised px-2 text-sm sm:hidden"
        />
        <div className="flex rounded-md border border-line p-0.5">
          <button
            type="button"
            aria-pressed={view === "session"}
            onClick={() => useDaw.getState().setView("session")}
            className={`inline-flex h-10 items-center gap-1 rounded-sm px-3 text-sm ${view === "session" ? "bg-accent text-accent-ink" : "text-muted"}`}
          >
            <LayoutGrid className="size-4" />
            Session
          </button>
          <button
            type="button"
            aria-pressed={view === "arrangement"}
            onClick={() => useDaw.getState().setView("arrangement")}
            className={`inline-flex h-10 items-center gap-1 rounded-sm px-3 text-sm ${view === "arrangement" ? "bg-accent text-accent-ink" : "text-muted"}`}
          >
            <Rows3 className="size-4" />
            Arrange
          </button>
        </div>

      <label className="flex items-center gap-1 text-2xs text-muted">
        Quantize
        <select
          aria-label="Launch quantize"
          value={project.quantize}
          onChange={(event) => useDaw.getState().setQuantize(Number(event.target.value))}
          className="h-10 rounded-md border border-line bg-raised px-2 text-sm text-fg"
        >
          {QUANTS.map(([value, label]) => (
            <option key={value} value={value}>
              {label}
            </option>
          ))}
        </select>
      </label>

      <label className="flex w-28 items-center gap-2 text-2xs text-muted">
        Swing
        <input
          aria-label="Swing"
          className="daw-slider"
          type="range"
          min={0}
          max={0.55}
          step={0.01}
          value={project.swing}
          onChange={(event) => useDaw.getState().setSwing(Number(event.target.value))}
        />
      </label>

      <button
        type="button"
        aria-pressed={project.metronome}
        onClick={() => useDaw.getState().setMetronome(!project.metronome)}
        className={`h-10 rounded-md px-3 text-sm ${project.metronome ? "bg-accent text-accent-ink" : "bg-raised text-muted"}`}
      >
        Click
      </button>
      <button
        type="button"
        aria-pressed={project.loop}
        onClick={() => useDaw.getState().setLoop(!project.loop)}
        className={`inline-flex h-10 items-center gap-1 rounded-md px-3 text-sm ${project.loop ? "bg-accent text-accent-ink" : "bg-raised text-muted"}`}
      >
        <Repeat className="size-4" />
        Loop
      </button>

      <label className="flex w-28 items-center gap-2 text-2xs text-muted">
        Master
        <input
          aria-label="Master volume"
          className="daw-slider"
          type="range"
          min={0}
          max={1}
          step={0.01}
          value={project.master}
          onChange={(event) => useDaw.getState().setMaster(Number(event.target.value))}
        />
      </label>

      {launched ? (
        <button
          type="button"
          onClick={() => getEngine().releaseAll()}
          className="inline-flex h-10 items-center gap-1 rounded-md bg-raised px-3 text-sm text-fg"
        >
          <RotateCcw className="size-4" />
          Arrangement
        </button>
      ) : null}

      <button
        type="button"
        disabled={rendering}
        onClick={() => void bounce()}
        className="inline-flex h-10 items-center gap-1 rounded-md bg-raised px-3 text-sm disabled:opacity-50"
      >
        <Download className="size-4" />
        {rendering ? "Rendering" : "Bounce"}
      </button>
      <button
        type="button"
        onClick={() => void enableMidi()}
        className={`h-10 rounded-md px-3 text-sm ${learn ? "bg-accent text-accent-ink" : "bg-raised text-fg"}`}
      >
        MIDI
      </button>
      <select
        aria-label="Audio output"
        className="h-10 max-w-36 rounded-md border border-line bg-raised px-2 text-sm text-fg"
        onFocus={() => void loadOutputs(setOutputs)}
        onChange={(event) => void pickOutput(event.target.value)}
        defaultValue=""
      >
        <option value="">Output</option>
        {outputs.map((device) => (
          <option key={device.deviceId} value={device.deviceId}>
            {device.label || "Output"}
          </option>
        ))}
      </select>
      <button
        type="button"
        onClick={() => useDaw.getState().loadDemo()}
        className="h-10 rounded-md bg-raised px-3 text-sm text-muted"
      >
        Demo
      </button>
      <button
        type="button"
        aria-label="How to play"
        onClick={() => useDaw.getState().setHelp(true)}
        className="inline-flex size-11 items-center justify-center rounded-md bg-raised"
      >
        <Keyboard className="size-4" />
      </button>
      </div>
    </header>
  );
}

function toggleRecord() {
  const state = useDaw.getState();
  const next = !state.record;
  state.setRecord(next);
  if (!next) return;
  const track =
    state.project.tracks.find((item) => item.arm && item.kind === "midi") ??
    state.project.tracks.find((item) => item.id === state.selectedTrackId);
  if (!track || track.kind !== "midi") return;
  if (state.snap.launched[track.id]) return;
  const fallback = track.slots.find((slot): slot is string => Boolean(slot)) ?? null;
  const clipId =
    state.selectedClipId && track.slots.includes(state.selectedClipId) ? state.selectedClipId : fallback;
  if (!clipId) return;
  getEngine().launch(track.id, clipId, track.slots.indexOf(clipId));
}

async function bounce() {
  const state = useDaw.getState();
  state.setRendering(true);
  try {
    const blob = await getEngine().bounce();
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `${state.project.name.replace(/\s+/g, "-").toLowerCase() || "lattice"}.wav`;
    link.click();
    URL.revokeObjectURL(url);
    toast.success("Arrangement bounced");
  } catch (error) {
    toast.error(error instanceof Error ? error.message : "Bounce failed");
  } finally {
    useDaw.getState().setRendering(false);
  }
}

async function enableMidi() {
  try {
    await getEngine().enableMidi();
    const count = useDaw.getState().midiIns ?? 0;
    useDaw.getState().setLearn(true);
    toast.message(count > 0 ? `${count} MIDI input${count === 1 ? "" : "s"} ready. Click a control name, then move a knob.` : "No MIDI inputs. You can still play the on-screen keys.");
  } catch (error) {
    toast.error(error instanceof Error ? error.message : "MIDI unavailable");
  }
}

async function loadOutputs(setOutputs: (devices: MediaDeviceInfo[]) => void) {
  try {
    const devices = await getEngine().listOutputs();
    setOutputs(devices);
  } catch {
    setOutputs([]);
  }
}

async function pickOutput(deviceId: string) {
  if (!deviceId) return;
  try {
    await getEngine().setOutput(deviceId);
  } catch {
    toast.error("This browser cannot switch audio outputs.");
  }
}
