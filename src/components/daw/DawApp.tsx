import { useEffect } from "react";
import { Group, Panel, Separator } from "react-resizable-panels";
import { Toaster } from "sonner";
import { X } from "lucide-react";
import { ArrangementView } from "@/components/daw/ArrangementView";
import { BottomPanel } from "@/components/daw/BottomPanel";
import { SessionView } from "@/components/daw/SessionView";
import { Transport } from "@/components/daw/Transport";
import { STORAGE_KEY } from "@/daw/demo";
import { getEngine } from "@/daw/engine";
import { KEY_ROWS } from "@/daw/format";
import { useDaw } from "@/daw/store";

function isTyping(target: EventTarget | null): boolean {
  if (!(target instanceof HTMLElement)) return false;
  const tag = target.tagName;
  return tag === "INPUT" || tag === "TEXTAREA" || tag === "SELECT" || target.isContentEditable;
}

export function DawApp() {
  const view = useDaw((state) => state.view);
  const help = useDaw((state) => state.help);
  const ready = useDaw((state) => state.ready);
  const project = useDaw((state) => state.project);

  useEffect(() => {
    const engine = getEngine();
    engine.bind({
      getProject: () => useDaw.getState().project,
      getUi: () => {
        const state = useDaw.getState();
        return {
          selectedTrackId: state.selectedTrackId,
          record: state.record,
          octave: state.octave,
          learn: state.learn,
          learnTarget: state.learnTarget,
        };
      },
      onChange: (snap) => useDaw.getState().setSnap(snap),
      onRecord: (clipId, note) => useDaw.getState().upsertNote(clipId, note),
      onLearn: (cc) => useDaw.getState().learnCc(cc),
      onControl: (target, normalized) => useDaw.getState().applyControl(target, normalized),
      onMidiIns: (count) => useDaw.getState().setMidiIns(count),
      onSampleRate: (rate) => useDaw.getState().setSampleRate(rate),
    });
    useDaw.getState().hydrate();

    const down = (event: KeyboardEvent) => {
      if (event.metaKey || event.ctrlKey || event.altKey) return;
      if (isTyping(event.target)) return;
      if (event.code === "Space") {
        event.preventDefault();
        void engine.toggle();
        return;
      }
      if (event.key === "[") {
        useDaw.getState().setOctave(useDaw.getState().octave - 1);
        return;
      }
      if (event.key === "]") {
        useDaw.getState().setOctave(useDaw.getState().octave + 1);
        return;
      }
      if (event.key === "Backspace" || event.key === "Delete") {
        const state = useDaw.getState();
        if (state.view === "arrangement" && state.selectedArrangementId) {
          event.preventDefault();
          state.deleteArrangement(state.selectedArrangementId);
        }
        return;
      }
      if (event.repeat) return;
      const state = useDaw.getState();
      const track = state.project.tracks.find((item) => item.id === state.selectedTrackId);
      const drums = track?.devices.some((device) => device.kind === "drums") ?? false;
      if (drums && /^[1-8]$/.test(event.key)) {
        void engine.noteOn(Number(event.key) - 1, 0.92);
        return;
      }
      if (drums) return;
      const row = KEY_ROWS.find((item) => item.key === event.key.toLowerCase());
      if (!row) return;
      void engine.noteOn(12 * (state.octave + 1) + row.semi, 0.88);
    };
    const up = (event: KeyboardEvent) => {
      const state = useDaw.getState();
      const track = state.project.tracks.find((item) => item.id === state.selectedTrackId);
      const drums = track?.devices.some((device) => device.kind === "drums") ?? false;
      if (drums && /^[1-8]$/.test(event.key)) {
        engine.noteOff(Number(event.key) - 1);
        return;
      }
      const row = KEY_ROWS.find((item) => item.key === event.key.toLowerCase());
      if (!row) return;
      engine.noteOff(12 * (state.octave + 1) + row.semi);
    };
    const blur = () => engine.releaseAllNotes();
    window.addEventListener("keydown", down);
    window.addEventListener("keyup", up);
    window.addEventListener("blur", blur);
    return () => {
      window.removeEventListener("keydown", down);
      window.removeEventListener("keyup", up);
      window.removeEventListener("blur", blur);
      engine.dispose();
    };
  }, []);

  useEffect(() => {
    if (!ready) return;
    const timer = window.setTimeout(() => {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(project));
    }, 280);
    return () => window.clearTimeout(timer);
  }, [project, ready]);

  return (
    <div className="flex h-dvh w-full max-w-full flex-col overflow-hidden bg-bg text-fg">
      <Transport />
      <div className="min-h-0 flex-1">
        <Group orientation="vertical" className="h-full min-h-0">
          <Panel id="stage" defaultSize="62%" minSize="34%" className="min-h-0">
            <div className="h-full min-h-0 overflow-hidden">
              {view === "session" ? <SessionView /> : <ArrangementView />}
            </div>
          </Panel>
          <Separator className="h-2 shrink-0 bg-line" />
          <Panel id="rack" defaultSize="38%" minSize="22%" className="min-h-0">
            <div className="h-full min-h-0 overflow-hidden">
              <BottomPanel />
            </div>
          </Panel>
        </Group>
      </div>
      {help ? <Help /> : null}
      <Toaster theme="dark" position="top-center" />
    </div>
  );
}

function Help() {
  return (
    <div className="fixed inset-0 z-40 flex items-end justify-center bg-bg/70 p-3 sm:items-center">
      <div className="max-h-[85dvh] w-full max-w-lg overflow-auto rounded-md border border-line bg-surface p-4 shadow-none">
        <div className="mb-3 flex items-start justify-between gap-3">
          <div>
            <h2 className="text-lg font-semibold">Play LATTICE</h2>
            <p className="text-sm text-muted">Session on top, devices underneath. One screen.</p>
          </div>
          <button
            type="button"
            className="inline-flex size-11 items-center justify-center rounded-md border border-line bg-raised"
            onClick={() => useDaw.getState().setHelp(false)}
            aria-label="Close help"
          >
            <X className="size-4" />
          </button>
        </div>
        <ul className="space-y-2 text-sm leading-relaxed text-fg">
          <li>Press play to run the arrangement from the playhead. Stop returns to bar 1.</li>
          <li>Launch a clip or a scene to take that track over. Quantize waits for the next division. Mute scene silences the session.</li>
          <li>Back to arrangement releases every launched clip.</li>
          <li>Keys A–K play the selected instrument (black keys on W E T Y U). [ and ] shift octave. 1–8 hit drum pads when Drums is selected.</li>
          <li>Arm a track, launch its clip, then Record. Incoming notes write into the looping clip.</li>
          <li>MIDI Learn: arm Learn, click a control name, move a hardware knob. Note input plays the selected instrument.</li>
          <li>Warp on an audio clip keeps pitch when tempo changes. Off is tape-style repitch. Bounce renders the arrangement to WAV.</li>
        </ul>
        <p className="mt-4 text-sm leading-relaxed text-muted">
          Audio runs in the browser, on whatever device the system already uses. Instruments, meters, sends, and the bounce are real audio-graph nodes, kept separate from this interface. VST3 and CLAP plug-ins need a native host, so this edition ships the built-in devices only. MIT licensed.
        </p>
      </div>
    </div>
  );
}
