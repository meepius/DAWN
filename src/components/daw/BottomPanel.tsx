import { ClipEditor } from "@/components/daw/ClipEditor";
import { DeviceRack } from "@/components/daw/DeviceRack";
import { PlayKeys } from "@/components/daw/PlayKeys";
import { useDaw } from "@/daw/store";

export function BottomPanel() {
  const panel = useDaw((state) => state.panel);
  return (
    <div className="flex h-full min-h-0 flex-col bg-bg">
      <div className="flex shrink-0 items-center gap-1 border-b border-line px-2 py-1">
        <Tab id="devices" label="Devices" />
        <Tab id="clip" label="Clip" />
      </div>
      <div className="min-h-0 flex-1 overflow-auto">
        {panel === "devices" ? <DeviceRack /> : <ClipEditor />}
        <PlayKeys />
      </div>
    </div>
  );
}

function Tab({ id, label }: { id: "devices" | "clip"; label: string }) {
  const active = useDaw((state) => state.panel === id);
  return (
    <button
      type="button"
      aria-pressed={active}
      onClick={() => useDaw.getState().setPanel(id)}
      className={`h-10 rounded-md px-3 text-sm ${active ? "bg-raised text-fg" : "text-muted"}`}
    >
      {label}
    </button>
  );
}
