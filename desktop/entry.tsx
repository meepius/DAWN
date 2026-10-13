import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import { DawApp } from "@/components/daw/DawApp";
import "./app.css";

const root = document.getElementById("root");
if (!root) throw new Error("Missing #root");

createRoot(root).render(
  <StrictMode>
    <DawApp />
  </StrictMode>,
);
