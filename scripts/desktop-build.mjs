import { spawn } from "node:child_process";
import { rmSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { writeLatticeIcon } from "./make-desktop-icon.mjs";

const root = fileURLToPath(new URL("..", import.meta.url));
const node = process.execPath;
rmSync(fileURLToPath(new URL("../dist", import.meta.url)), { recursive: true, force: true });
writeLatticeIcon(fileURLToPath(new URL("../desktop/build/icon.png", import.meta.url)));

const child = spawn(
  node,
  [
    fileURLToPath(new URL("./with-app-env.mjs", import.meta.url)),
    node,
    fileURLToPath(new URL("../node_modules/vite/bin/vite.js", import.meta.url)),
    "build",
    "--config",
    "vite.desktop.config.ts",
  ],
  {
    stdio: "inherit",
    cwd: root,
    env: { ...process.env, VITE_AUTH_ENABLED: "false" },
  },
);

child.on("exit", (code, signal) => {
  if (signal) process.exit(1);
  process.exit(code ?? 1);
});
