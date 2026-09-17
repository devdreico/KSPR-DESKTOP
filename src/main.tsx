import React from "react";
import ReactDOM from "react-dom/client";
import { invoke } from "@tauri-apps/api/core";
import { Studio } from "./pages/Studio";
import { Landing } from "./pages/Landing";
import "./styles.css";

type RuntimeConfig = { host: string; port: number; token: string };
type TauriWindow = Window & { __TAURI_INTERNALS__?: unknown; __KSPR_RUNTIME_CONFIG__?: RuntimeConfig; __KSPR_SECRETS__?: Record<string, string> };

async function hydrateDesktopStorage() {
  const browser = window as TauriWindow;
  if (!browser.__TAURI_INTERNALS__) return;
  for (let attempt = 0; attempt < 40; attempt += 1) {
    try {
      const config = await invoke<RuntimeConfig>("runtime_config");
      browser.__KSPR_RUNTIME_CONFIG__ = config;
      const gemini = await invoke<string | null>("secret_get", { provider: "gemini" });
      browser.__KSPR_SECRETS__ = gemini ? { gemini } : {};
      const saved = await invoke<Record<string, string>>("storage_load");
      for (const [key, value] of Object.entries(saved)) {
        if (localStorage.getItem(key) === null) localStorage.setItem(key, value);
      }
      const originalSetItem = localStorage.setItem.bind(localStorage);
      localStorage.setItem = (key: string, value: string) => {
        originalSetItem(key, value);
        void invoke("storage_set", { key, value });
      };
      return;
    } catch {
      await new Promise((resolve) => window.setTimeout(resolve, 250));
    }
  }
  throw new Error("KSPR Runtime no pudo iniciar");
}

async function boot() {
  try {
    await hydrateDesktopStorage();
    ReactDOM.createRoot(document.getElementById("root")!).render(<React.StrictMode><Studio /></React.StrictMode>);
  } catch (error) {
    document.getElementById("root")!.innerHTML = `<main style="font-family:system-ui;padding:48px"><h1>KSPR no pudo iniciar</h1><p>${error instanceof Error ? error.message : "Runtime no disponible"}</p><p>Reinicia la aplicación y vuelve a intentarlo.</p></main>`;
  }
}
const browser = window as TauriWindow;
if (browser.__TAURI_INTERNALS__) {
  void boot();
} else {
  ReactDOM.createRoot(document.getElementById("root")!).render(<React.StrictMode><Landing /></React.StrictMode>);
}
