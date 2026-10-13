import tailwindcss from "@tailwindcss/vite";
import viteReact from "@vitejs/plugin-react";
import { fileURLToPath } from "node:url";
import { defineConfig } from "vite";

const root = fileURLToPath(new URL("./desktop", import.meta.url));

/**
 * Static client for the portable desktop shell.
 * The preview / Vercel build keeps using vite.config.ts unchanged.
 */
export default defineConfig({
  root,
  publicDir: fileURLToPath(new URL("./public", import.meta.url)),
  resolve: {
    tsconfigPaths: true,
    alias: {
      "@": fileURLToPath(new URL("./src", import.meta.url)),
    },
  },
  plugins: [tailwindcss(), viteReact()],
  build: {
    outDir: fileURLToPath(new URL("./dist/client", import.meta.url)),
    emptyOutDir: true,
  },
});
