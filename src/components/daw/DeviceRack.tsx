import { X } from "lucide-react";
import { PARAMS } from "@/daw/devices";
import { useDaw } from "@/daw/store";
import type { Device, DeviceKind, Track } from "@/daw/types";

const ADDABLE: Array<[DeviceKind, string]> = [
  ["eq", "EQ Three"],
  ["compressor", "Compressor"],
  ["delay", "Delay"],
  ["reverb", "Reverb"],
];

export function DeviceRack() {
  const project = useDaw((state) => state.project);
  const selectedTrackId = useDaw((state) => state.selectedTrackId);
  const learnTarget = useDaw((state) => state.learnTarget);
  const track = project.tracks.find((item) => item.id === selectedTrackId) ?? project.tracks[0];
  if (!track) return null;

  return (
    <div className="flex gap-3 overflow-x-auto p-3">
      {track.kind !== "return" ? <Sends track={track} learnTarget={learnTarget} /> : null}
      {track.devices.map((device) => (
        <DeviceCard key={device.id} device={device} learnTarget={learnTarget} />
      ))}
      {track.kind !== "return" ? (
        <label className="flex w-40 shrink-0 flex-col justify-center gap-2 rounded-md border border-dashed border-line p-3 text-sm text-muted">
          Add device
          <select
            aria-label="Add device"
            className="h-11 rounded-md border border-line bg-raised px-2 text-fg"
            value=""
            onChange={(event) => {
              const kind = event.target.value as DeviceKind;
              if (kind) useDaw.getState().addDevice(track.id, kind);
            }}
          >
            <option value="">Choose</option>
            {ADDABLE.map(([kind, label]) => (
              <option key={kind} value={kind}>
                {label}
              </option>
            ))}
          </select>
        </label>
      ) : null}
    </div>
  );
}

function Sends({ track, learnTarget }: { track: Track; learnTarget: string | null }) {
  return (
    <section className="w-56 shrink-0 rounded-md border border-line bg-surface p-3">
      <h3 className="mb-2 text-sm font-medium">Sends</h3>
      <Send
        label="Reverb"
        target={`sendR:${track.id}`}
        value={track.sendRev}
        learn={learnTarget === `sendR:${track.id}`}
        onChange={(value) => useDaw.getState().setTrackNumber(track.id, "sendRev", value)}
      />
      <Send
        label="Delay"
        target={`sendD:${track.id}`}
        value={track.sendDly}
        learn={learnTarget === `sendD:${track.id}`}
        onChange={(value) => useDaw.getState().setTrackNumber(track.id, "sendDly", value)}
      />
      <Send
        label="Pan"
        target={`pan:${track.id}`}
        value={(track.pan + 1) / 2}
        learn={learnTarget === `pan:${track.id}`}
        display={track.pan.toFixed(2)}
        onChange={(value) => useDaw.getState().setTrackNumber(track.id, "pan", value * 2 - 1)}
      />
    </section>
  );
}

function Send({
  label,
  target,
  value,
  display,
  learn,
  onChange,
}: {
  label: string;
  target: string;
  value: number;
  display?: string;
  learn: boolean;
  onChange: (value: number) => void;
}) {
  const binding = useDaw((state) => state.project.bindings.find((item) => item.target === target));
  return (
    <label className={`mb-2 block ${learn ? "rounded-sm outline outline-2 outline-accent" : ""}`}>
      <span className="mb-1 flex items-center justify-between text-2xs text-muted">
        <button type="button" className="text-left" onClick={() => useDaw.getState().armLearn(target)}>
          {label}
          {binding ? <span className="ml-1 font-mono text-accent">CC{binding.cc}</span> : null}
        </button>
        <span className="font-mono">{display ?? `${Math.round(value * 100)}`}</span>
      </span>
      <input
        aria-label={label}
        className="daw-slider"
        type="range"
        min={0}
        max={1}
        step={0.01}
        value={value}
        onChange={(event) => onChange(Number(event.target.value))}
      />
    </label>
  );
}

function DeviceCard({ device, learnTarget }: { device: Device; learnTarget: string | null }) {
  const locked = device.kind === "analog" || device.kind === "drums";
  return (
    <section className={`w-72 shrink-0 rounded-md border border-line bg-surface p-3 ${device.bypass ? "opacity-50" : ""}`}>
      <div className="mb-2 flex items-center justify-between gap-2">
        <h3 className="text-sm font-medium">{device.name}</h3>
        <div className="flex gap-1">
          <button
            type="button"
            onClick={() => useDaw.getState().toggleBypass(device.id)}
            className="h-8 rounded-sm bg-raised px-2 text-2xs"
          >
            {device.bypass ? "Bypassed" : "On"}
          </button>
          {locked ? null : (
            <button
              type="button"
              aria-label={`Remove ${device.name}`}
              onClick={() => useDaw.getState().removeDevice(device.id)}
              className="inline-flex size-8 items-center justify-center rounded-sm bg-raised"
            >
              <X className="size-3" />
            </button>
          )}
        </div>
      </div>
      <div className="grid grid-cols-2 gap-x-3 gap-y-2">
        {PARAMS[device.kind].map((spec) => {
          const target = `dev:${device.id}:${spec.key}`;
          const value = device.params[spec.key] ?? spec.min;
          const learn = learnTarget === target;
          return (
            <ParamSlider
              key={spec.key}
              label={spec.label}
              target={target}
              min={spec.min}
              max={spec.max}
              step={spec.step}
              value={value}
              shown={spec.format(value)}
              learn={learn}
              onChange={(next) => useDaw.getState().setDeviceParam(device.id, spec.key, next)}
            />
          );
        })}
      </div>
    </section>
  );
}

function ParamSlider({
  label,
  target,
  min,
  max,
  step,
  value,
  shown,
  learn,
  onChange,
}: {
  label: string;
  target: string;
  min: number;
  max: number;
  step: number;
  value: number;
  shown: string;
  learn: boolean;
  onChange: (value: number) => void;
}) {
  const binding = useDaw((state) => state.project.bindings.find((item) => item.target === target));
  return (
    <label className={learn ? "rounded-sm outline outline-2 outline-accent" : ""}>
      <span className="mb-1 flex items-center justify-between gap-2 text-2xs text-muted">
        <button type="button" className="truncate text-left" onClick={() => useDaw.getState().armLearn(target)}>
          {label}
          {binding ? <span className="ml-1 font-mono text-accent">CC{binding.cc}</span> : null}
        </button>
        <span className="font-mono">{shown}</span>
      </span>
      <input
        aria-label={label}
        className="daw-slider"
        type="range"
        min={min}
        max={max}
        step={step}
        value={value}
        onChange={(event) => onChange(Number(event.target.value))}
      />
    </label>
  );
}
