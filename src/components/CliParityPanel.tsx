import { useState } from "react";
import { Boxes, FileCode2, FolderTree, Library, Network, Package, Plug, Terminal, X } from "lucide-react";
import { decompileSources, ingestGitUrl, inspectMcp, listCapabilities, listContextTrees, listPlugins, listSavedPrompts, listSkills, reconstructSubject, savePrompt, unlockLicense, type ContextTree, type SourceFile } from "../lib/api";

type Props = {
  initialCommand?: string;
  mcpServers: Record<string, unknown>;
  onImportFiles?: (files: SourceFile[]) => void;
  onClose: () => void;
};

const COMMANDS = [
  ["plugins", "/plugins", "Inspecciona y carga plugins instalados"],
  ["login", "/login", "Verifica el código de licencia de KSPR"],
  ["capabilities", "/capabilities", "Descubre harnesses CLI-Anything disponibles"],
  ["skills", "/skills", "Consulta los bundles de skills activos"],
  ["mcp", "/mcp", "Conecta servidores MCP y lista sus herramientas"],
  ["prompts", "/prompts", "Consulta prompts persistentes de la CLI"],
  ["decompilate", "/decompilate", "Genera Context Trees desde archivos o enlaces"],
  ["inverse-engineering", "/inverse", "Reconstruye cualquier sistema a partir de nodos de contexto"],
  ["trees", "/trees", "Lista Context Trees generados"],
  ["update", "/update", "Consulta el comando oficial de actualización"],
] as const;

export function CliParityPanel({ initialCommand = "plugins", mcpServers, onImportFiles, onClose }: Props) {
  const [active, setActive] = useState(initialCommand);
  const [output, setOutput] = useState<unknown>(null);
  const [busy, setBusy] = useState(false);
  const [links, setLinks] = useState("");
  const [gitUrl, setGitUrl] = useState("");
  const [promptName, setPromptName] = useState("");
  const [promptContent, setPromptContent] = useState("");
  const [licenseCode, setLicenseCode] = useState("");
  const [decompileFiles, setDecompileFiles] = useState<File[]>([]);
  const [error, setError] = useState("");
  const [inverseSubject, setInverseSubject] = useState("");
  const [inverseDomain, setInverseDomain] = useState("mixed");
  const [inverseObjective, setInverseObjective] = useState("Descompón la estructura, funcionamiento, dependencias y preguntas abiertas.");
  const [inverseEvidence, setInverseEvidence] = useState("");

  async function execute(command: string) {
    setActive(command);
    setBusy(true);
    setError("");
    try {
      if (command === "plugins") setOutput(await listPlugins(true));
      if (command === "capabilities") setOutput(await listCapabilities());
      if (command === "skills") setOutput(await listSkills(true));
      if (command === "mcp") setOutput(await inspectMcp(mcpServers));
      if (command === "prompts") setOutput(await listSavedPrompts());
      if (command === "trees") setOutput(await listContextTrees());
      if (command === "update") setOutput({ command: "curl -fsSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash", note: "La actualización se deja como comando explícito para evitar mutar el sistema desde la interfaz." });
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "KSPR no pudo ejecutar el comando");
    } finally {
      setBusy(false);
    }
  }

  async function decompile() {
    setBusy(true);
    setError("");
    try {
      setOutput(await decompileSources(decompileFiles, links, { provider: "local" }));
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "No se pudo generar el Context Tree");
    } finally {
      setBusy(false);
    }
  }

  async function createPrompt() {
    if (!promptName.trim() || !promptContent.trim()) return;
    setBusy(true);
    setError("");
    try {
      setOutput(await savePrompt({ name: promptName.trim(), content: promptContent }));
      setPromptName("");
      setPromptContent("");
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "No se pudo guardar el prompt");
    } finally {
      setBusy(false);
    }
  }

  async function login() {
    if (!licenseCode.trim()) return;
    setBusy(true);
    setError("");
    try {
      setOutput(await unlockLicense(licenseCode));
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "No se pudo verificar el código");
    } finally {
      setBusy(false);
    }
  }

  async function importGit() {
    if (!gitUrl.trim()) return;
    setBusy(true);
    setError("");
    try {
      const files = await ingestGitUrl(gitUrl);
      onImportFiles?.(files);
      setOutput({ source: "git", url: gitUrl, files: files.length, message: "Repositorio cargado al contexto de la sesión." });
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "No se pudo cargar el repositorio Git");
    } finally {
      setBusy(false);
    }
  }

  async function reconstruct() {
    if (!inverseSubject.trim() || !inverseEvidence.trim()) return;
    setBusy(true);
    setError("");
    try {
      setOutput(await reconstructSubject({
        subject: inverseSubject.trim(),
        domain: inverseDomain as "digital" | "physical" | "process" | "conceptual" | "imaginary" | "mixed",
        objective: inverseObjective,
        evidence: [{ id: `evidence-${Date.now()}`, kind: "text", title: "Observación inicial", content: inverseEvidence.trim(), confidence: 0.75 }],
        depth: 4,
        include_hypotheses: true,
        include_unknowns: true,
      }));
    } catch (cause) {
      setError(cause instanceof Error ? cause.message : "No se pudo reconstruir el sujeto");
    } finally {
      setBusy(false);
    }
  }

  const iconFor = (command: string) => command === "mcp" ? <Network size={14} /> : command === "plugins" ? <Plug size={14} /> : command === "skills" ? <Library size={14} /> : command === "capabilities" ? <Boxes size={14} /> : command === "decompilate" ? <FileCode2 size={14} /> : command === "trees" ? <FolderTree size={14} /> : <Package size={14} />;

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal-box cli-parity-modal" onClick={(event) => event.stopPropagation()}>
        <div className="modal-header">
          <h2><Terminal size={18} /> Paridad con KSPR CLI <small>mismos comandos y runtime</small></h2>
          <button className="modal-close" onClick={onClose}><X size={18} /></button>
        </div>
        <div className="cli-parity-layout">
          <nav className="cli-command-list">
            {COMMANDS.map(([id, slash, description]) => <button className={active === id ? "active" : ""} key={id} onClick={() => id === "decompilate" || id === "inverse-engineering" ? setActive(id) : void execute(id)}>{iconFor(id)}<span><strong>{slash}</strong><small>{description}</small></span></button>)}
          </nav>
          <section className="cli-command-output">
            <div className="cli-output-head"><span>{active === "decompilate" ? "Context Tree Generator" : `Resultado de /${active}`}</span>{busy && <span className="cli-running">ejecutando…</span>}</div>
            {active === "login" ? <div className="decompile-form">
              <p>Equivalente visual de <code>/login</code>. El código se valida en el runtime local y nunca se envía a un proveedor de IA.</p>
              <label className="modal-field"><span>Código de licencia</span><input type="password" value={licenseCode} onChange={(event) => setLicenseCode(event.target.value)} placeholder="KSPR-XXXX-XXXX-XXXX" /></label>
              <button className="modal-button primary" disabled={busy || !licenseCode.trim()} onClick={() => void login()}>Verificar código</button>
              {Boolean(output) && <pre className="cli-json-output">{JSON.stringify(output, null, 2)}</pre>}
            </div> : active === "prompts" ? <div className="decompile-form">
              <p>Equivalente visual de <code>/prompts add|select|remove|info</code>. Los prompts se guardan en <code>~/.kspr/prompts.json</code>.</p>
              <label className="modal-field"><span>Nombre del prompt</span><input value={promptName} onChange={(event) => setPromptName(event.target.value)} placeholder="inventario" /></label>
              <label className="modal-field"><span>Contenido</span><textarea value={promptContent} onChange={(event) => setPromptContent(event.target.value)} placeholder="Recupera botones, eventos y contratos con evidencia." /></label>
              <button className="modal-button primary" disabled={busy || !promptName.trim() || !promptContent.trim()} onClick={() => void createPrompt()}>Guardar prompt</button>
              {Boolean(output) && <pre className="cli-json-output">{JSON.stringify(output, null, 2)}</pre>}
            </div> : active === "inverse-engineering" ? <div className="decompile-form">
              <p><strong>KSPR Inverse Engineering</strong> transforma evidencias heterogéneas en un grafo trazable de nodos, relaciones, hipótesis, incógnitas y pasos de verificación. No presupone que el sujeto sea software.</p>
              <label className="modal-field"><span>Sujeto a reconstruir</span><input value={inverseSubject} onChange={(event) => setInverseSubject(event.target.value)} placeholder="Ej. un sistema de pagos, una máquina, un proceso o una teoría" /></label>
              <label className="modal-field"><span>Dominio</span><select value={inverseDomain} onChange={(event) => setInverseDomain(event.target.value)}><option value="mixed">Mixto</option><option value="digital">Digital</option><option value="physical">Físico</option><option value="process">Proceso</option><option value="conceptual">Conceptual</option><option value="imaginary">Imaginario</option></select></label>
              <label className="modal-field"><span>Objetivo de reconstrucción</span><textarea value={inverseObjective} onChange={(event) => setInverseObjective(event.target.value)} /></label>
              <label className="modal-field"><span>Evidencia o nodos de contexto</span><textarea value={inverseEvidence} onChange={(event) => setInverseEvidence(event.target.value)} placeholder="Describe observaciones, componentes, reglas, síntomas, medidas, documentos o cualquier señal disponible." rows={7} /></label>
              <button className="modal-button primary" disabled={busy || !inverseSubject.trim() || !inverseEvidence.trim()} onClick={() => void reconstruct()}>Construir reconstrucción trazable</button>
              {Boolean(output) && <pre className="cli-json-output">{JSON.stringify(output, null, 2)}</pre>}
            </div> : active === "decompilate" ? <div className="decompile-form">
              <p>Equivalente visual de <code>kspr /decompilate</code>. El motor solo lee las fuentes; no ejecuta el proyecto.</p>
              <label className="modal-field"><span>Repositorio Git (equivalente a --git-url)</span><input value={gitUrl} onChange={(event) => setGitUrl(event.target.value)} placeholder="https://github.com/org/repo.git" /><button type="button" className="modal-button" disabled={busy || !gitUrl.trim()} onClick={() => void importGit()}>Cargar al contexto</button></label>
              <label className="modal-field"><span>Archivos multimodales</span><input type="file" multiple onChange={(event) => setDecompileFiles(Array.from(event.target.files || []))} /></label>
              <label className="modal-field"><span>Enlaces (separados por coma)</span><input value={links} onChange={(event) => setLinks(event.target.value)} placeholder="https://docs.example.com" /></label>
              <button className="modal-button primary" disabled={busy || (!decompileFiles.length && !links.trim())} onClick={() => void decompile()}>Generar Context Tree</button>
              {Boolean(output) && <pre className="cli-json-output">{JSON.stringify(output, null, 2)}</pre>}
            </div> : output ? <pre className="cli-json-output">{JSON.stringify(output, null, 2)}</pre> : <div className="cli-empty"><Terminal size={22} /><p>Selecciona un comando para ejecutar su operación real.</p></div>}
            {error && <div className="composer-error">{error}</div>}
            {active === "trees" && Boolean(output) && <div className="tree-summary"><FolderTree size={14} />{((output as { trees?: ContextTree[] }).trees || []).length} árboles disponibles</div>}
          </section>
        </div>
      </div>
    </div>
  );
}
