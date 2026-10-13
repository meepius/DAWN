import { deflateSync } from "node:zlib";
import { mkdirSync, writeFileSync } from "node:fs";
import { dirname } from "node:path";

function crc32(buf) {
  let c = ~0;
  for (let i = 0; i < buf.length; i++) {
    c ^= buf[i];
    for (let k = 0; k < 8; k++) c = (c >>> 1) ^ (0xedb88320 & -(c & 1));
  }
  return ~c >>> 0;
}

function chunk(type, data) {
  const len = Buffer.alloc(4);
  len.writeUInt32BE(data.length);
  const body = Buffer.concat([Buffer.from(type), data]);
  const crc = Buffer.alloc(4);
  crc.writeUInt32BE(crc32(body));
  return Buffer.concat([len, body, crc]);
}

/** 256px LATTICE mark, matching public/favicon.svg. No image libraries. */
export function writeLatticeIcon(outPath) {
  const size = 256;
  const stride = size * 4 + 1;
  const raw = Buffer.alloc(stride * size);
  const paint = (x, y, color) => {
    if (x < 0 || y < 0 || x >= size || y >= size) return;
    const i = y * stride + 1 + x * 4;
    raw[i] = color[0];
    raw[i + 1] = color[1];
    raw[i + 2] = color[2];
    raw[i + 3] = 255;
  };
  const bg = [0x10, 0x11, 0x14];
  for (let y = 0; y < size; y++) {
    raw[y * stride] = 0;
    for (let x = 0; x < size; x++) paint(x, y, bg);
  }
  const s = (n) => Math.round((n * size) / 32);
  const roundRect = (x, y, w, h, radius, color) => {
    const r = Math.max(1, radius);
    for (let yy = y; yy < y + h; yy++) {
      for (let xx = x; xx < x + w; xx++) {
        const dx = xx < x + r ? x + r - xx : xx >= x + w - r ? xx - (x + w - r - 1) : 0;
        const dy = yy < y + r ? y + r - yy : yy >= y + h - r ? yy - (y + h - r - 1) : 0;
        if (dx * dx + dy * dy > r * r) continue;
        paint(xx, yy, color);
      }
    }
  };
  roundRect(s(4), s(6), s(11), s(8), s(1.6), [0xf3, 0xf2, 0xec]);
  roundRect(s(17), s(6), s(11), s(8), s(1.6), [0x2c, 0x30, 0x38]);
  roundRect(s(4), s(17), s(7), s(8), s(1.6), [0x2c, 0x30, 0x38]);
  roundRect(s(13), s(17), s(15), s(8), s(1.6), [0xd6, 0xff, 0x4a]);

  const ihdr = Buffer.alloc(13);
  ihdr.writeUInt32BE(size, 0);
  ihdr.writeUInt32BE(size, 4);
  ihdr[8] = 8;
  ihdr[9] = 6;
  const png = Buffer.concat([
    Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]),
    chunk("IHDR", ihdr),
    chunk("IDAT", deflateSync(raw)),
    chunk("IEND", Buffer.alloc(0)),
  ]);
  mkdirSync(dirname(outPath), { recursive: true });
  writeFileSync(outPath, png);
}

if (import.meta.url === `file://${process.argv[1]}`) {
  writeLatticeIcon(process.argv[2] ?? "desktop/build/icon.png");
}
