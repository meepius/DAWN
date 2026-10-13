const { app, BrowserWindow, Menu, powerSaveBlocker, session, shell } = require("electron");
const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");

const MIME = {
  ".css": "text/css; charset=utf-8",
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".mjs": "text/javascript; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".map": "application/json; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".jpeg": "image/jpeg",
  ".svg": "image/svg+xml",
  ".webp": "image/webp",
  ".ico": "image/x-icon",
  ".woff": "font/woff",
  ".woff2": "font/woff2",
  ".ttf": "font/ttf",
  ".wasm": "application/wasm",
  ".wav": "audio/wav",
  ".mp3": "audio/mpeg",
  ".txt": "text/plain; charset=utf-8",
};

function portableDataDir() {
  if (process.env.PORTABLE_EXECUTABLE_DIR) {
    return path.join(process.env.PORTABLE_EXECUTABLE_DIR, "LATTICE-data");
  }
  if (process.env.APPIMAGE) {
    return path.join(path.dirname(process.env.APPIMAGE), "LATTICE-data");
  }
  return null;
}

const dataDir = portableDataDir();
if (dataDir) {
  fs.mkdirSync(dataDir, { recursive: true });
  app.setPath("userData", dataDir);
}

app.commandLine.appendSwitch("autoplay-policy", "no-user-gesture-required");

const gotLock = app.requestSingleInstanceLock();
if (!gotLock) {
  app.quit();
}

function clientRoot() {
  return path.join(app.getAppPath(), "dist", "client");
}

function startStaticServer(root) {
  const indexPath = path.join(root, "index.html");
  return new Promise((resolve, reject) => {
    const server = http.createServer((req, res) => {
      const url = new URL(req.url ?? "/", "http://127.0.0.1");
      let rel = decodeURIComponent(url.pathname);
      if (rel.endsWith("/")) rel += "index.html";
      const filePath = path.normalize(path.join(root, rel));
      if (!filePath.startsWith(root)) {
        res.writeHead(403);
        res.end("Forbidden");
        return;
      }
      const send = (target, status = 200) => {
        fs.readFile(target, (err, body) => {
          if (err) {
            res.writeHead(404);
            res.end("Not found");
            return;
          }
          const type = MIME[path.extname(target).toLowerCase()] ?? "application/octet-stream";
          res.writeHead(status, {
            "Content-Type": type,
            "Content-Length": body.length,
            "Cache-Control": "no-cache",
          });
          res.end(body);
        });
      };
      fs.stat(filePath, (err, stat) => {
        if (!err && stat.isFile()) {
          send(filePath);
          return;
        }
        if (path.extname(rel)) {
          res.writeHead(404);
          res.end("Not found");
          return;
        }
        send(indexPath);
      });
    });
    server.once("error", reject);
    server.listen(0, "127.0.0.1", () => {
      const address = server.address();
      resolve({ server, port: typeof address === "object" && address ? address.port : 0 });
    });
  });
}

function allowMedia(permission) {
  return (
    permission === "media" ||
    permission === "audioCapture" ||
    permission === "midi" ||
    permission === "midiSysex" ||
    permission === "speaker-selection" ||
    permission === "clipboard-read" ||
    permission === "clipboard-sanitized-write"
  );
}

function createWindow(port) {
  const win = new BrowserWindow({
    width: 1440,
    height: 900,
    minWidth: 960,
    minHeight: 640,
    backgroundColor: "#101114",
    title: "LATTICE",
    autoHideMenuBar: true,
    show: false,
    webPreferences: {
      contextIsolation: true,
      nodeIntegration: false,
      sandbox: true,
      backgroundThrottling: false,
    },
  });
  win.once("ready-to-show", () => win.show());
  win.webContents.setWindowOpenHandler(({ url }) => {
    if (url.startsWith("http://") || url.startsWith("https://")) shell.openExternal(url);
    return { action: "deny" };
  });
  win.webContents.on("will-navigate", (event, url) => {
    const local = `http://127.0.0.1:${port}/`;
    if (!url.startsWith(local)) {
      event.preventDefault();
      if (url.startsWith("http://") || url.startsWith("https://")) shell.openExternal(url);
    }
  });
  void win.loadURL(`http://127.0.0.1:${port}/`);
  return win;
}

if (gotLock) {
  let mainWindow = null;
  app.on("second-instance", () => {
    if (!mainWindow) return;
    if (mainWindow.isMinimized()) mainWindow.restore();
    mainWindow.focus();
  });

  app.whenReady().then(async () => {
    powerSaveBlocker.start("prevent-app-suspension");
    session.defaultSession.setPermissionRequestHandler((_wc, permission, callback) => {
      callback(allowMedia(permission));
    });
    session.defaultSession.setPermissionCheckHandler((_wc, permission) => allowMedia(permission));

    const template = [
      {
        label: "File",
        submenu: [{ role: "quit" }],
      },
      {
        label: "Edit",
        submenu: [
          { role: "undo" },
          { role: "redo" },
          { type: "separator" },
          { role: "cut" },
          { role: "copy" },
          { role: "paste" },
          { role: "selectAll" },
        ],
      },
      {
        label: "View",
        submenu: [
          { role: "reload" },
          { role: "togglefullscreen" },
          { type: "separator" },
          {
            label: "Actual Size",
            accelerator: "CmdOrCtrl+0",
            click: () => mainWindow?.webContents.setZoomLevel(0),
          },
          {
            label: "Zoom In",
            accelerator: "CmdOrCtrl+Plus",
            click: () => mainWindow?.webContents.setZoomLevel((mainWindow?.webContents.getZoomLevel() ?? 0) + 0.5),
          },
          {
            label: "Zoom Out",
            accelerator: "CmdOrCtrl+-",
            click: () => mainWindow?.webContents.setZoomLevel((mainWindow?.webContents.getZoomLevel() ?? 0) - 0.5),
          },
        ],
      },
      {
        label: "Window",
        submenu: [{ role: "minimize" }, { role: "close" }],
      },
    ];
    Menu.setApplicationMenu(Menu.buildFromTemplate(template));

    const root = clientRoot();
    const { port } = await startStaticServer(root);
    mainWindow = createWindow(port);
  });

  app.on("window-all-closed", () => {
    app.quit();
  });
}
