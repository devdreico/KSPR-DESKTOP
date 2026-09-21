import { FileText, Copy, Check, Download } from "lucide-react";
import { useState } from "react";
import type { Artifact } from "../lib/api";

export function ArtifactPanel({ artifacts }: { artifacts: Artifact[] }) {
  const [selected, setSelected] = useState(artifacts[0]?.path || "");
  const [copied, setCopied] = useState(false);
  const artifact = artifacts.find((item) => item.path === selected) || artifacts[0];

  async function copy() {
    if (!artifact) return;
    await navigator.clipboard?.writeText(artifact.content);
    setCopied(true);
    setTimeout(() => setCopied(false), 1600);
  }

  function download() {
    if (!artifact) return;
    const link = document.createElement("a");
    link.href = URL.createObjectURL(new Blob([artifact.content], { type: "text/plain;charset=utf-8" }));
    link.download = artifact.path.split("/").pop() || "kspr-artifact.md";
    link.click();
    URL.revokeObjectURL(link.href);
  }

  return (
    <section className="artifact-layout">
      <aside className="artifact-nav">
        <div className="section-kicker">Context package</div>
        {artifacts.map((item) => (
          <button className={item.path === selected ? "artifact-link active" : "artifact-link"} key={item.path} onClick={() => setSelected(item.path)}>
            <FileText size={14} />
            <span>{item.path}</span>
          </button>
        ))}
      </aside>
      <div className="artifact-viewer">
        {artifact && (
          <>
            <div className="viewer-head">
              <div><span className="eyebrow">Generated artifact</span><h3>{artifact.title}</h3></div>
              <div style={{ display: "flex", gap: "6px" }}><button className="icon-button" onClick={download} title="Descargar artefacto"><Download size={16} /></button><button className="icon-button" onClick={copy} title="Copiar contenido">{copied ? <Check size={16} /> : <Copy size={16} />}</button></div>
            </div>
            <pre>{artifact.content}</pre>
          </>
        )}
      </div>
    </section>
  );
}

