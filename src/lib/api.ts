export type SourceFile = { path: string; content: string };

export type AnalysisRequest = {
  project_name: string;
  files: SourceFile[];
  provider: string;
  model?: string;
  variant?: string;
  effort?: string;
  iterations: number;
  mode: "auto" | "direct" | "async";
  instruction?: string;
};

export type Artifact = { path: string; title: string; content: string; kind: string };
export type ProviderAuth = {
  provider?: string;
  apiKey?: string;
  baseUrl?: string;
  authMode?: "api_key" | "bearer";
};
export type AnalysisResult = {
  analysis_id: string;
  summary: {
    project_name: string;
    status: string;
    provider: string;
    model: string;
    files_analyzed: number;
    total_lines: number;
    ui_elements: number;
    flows: number;
    endpoints: number;
    risks: number;
    contradictions: number;
    iterations_completed: number;
  };
  report: Record<string, unknown>;
  artifacts: Artifact[];
  response_text: string;
};

export type AnalysisProgress = {
  status: string;
  progress: number;
  stage: string;
  message: string;
};

export type InverseEvidence = {
  id: string;
  kind: "text" | "source" | "image" | "document" | "observation" | "constraint" | "hypothesis";
  title: string;
  content: string;
  source_ref?: string;
  confidence?: number;
  metadata?: Record<string, unknown>;
};

export type InverseEngineeringRequest = {
  subject: string;
  domain: "digital" | "physical" | "process" | "conceptual" | "imaginary" | "mixed";
  objective?: string;
  evidence: InverseEvidence[];
  depth?: number;
  include_hypotheses?: boolean;
  include_unknowns?: boolean;
};

export type InverseEngineeringResult = {
  reconstruction_id: string;
  subject: string;
  domain: string;
  status: string;
  nodes: Array<{ id: string; label: string; node_type: string; description: string; evidence_ids: string[]; confidence: number; properties: Record<string, unknown> }>;
  relations: Array<{ id: string; source: string; target: string; relation: string; confidence: number; evidence_ids: string[] }>;
  hypotheses: Array<{ id: string; statement: string; confidence: number; supporting_evidence: string[]; missing_evidence: string[]; verification_steps: string[] }>;
  unknowns: string[];
  reconstruction_plan: string[];
  evidence_map: Record<string, string[]>;
  metrics: Record<string, number>;
};

export async function reconstructSubject(payload: InverseEngineeringRequest): Promise<InverseEngineeringResult> {
  return parse(await fetch(apiUrl() + "/api/v1/inverse-engineering/reconstruct", { method: "POST", headers: providerHeaders(), body: JSON.stringify(payload) }));
}

function apiUrl() {
  const runtime = (window as Window & { __KSPR_RUNTIME_CONFIG__?: { host: string; port: number } }).__KSPR_RUNTIME_CONFIG__;
  return import.meta.env.VITE_API_URL || (runtime ? `http://${runtime.host}:${runtime.port}` : "");
}

async function parse<T>(response: Response): Promise<T> {
  const text = await response.text();
  if (text.trim().toLowerCase().startsWith("<!doctype") || text.trim().toLowerCase().startsWith("<html")) {
    throw new Error("El servidor backend no está respondiendo en /api (comprueba que uvicorn esté corriendo en el puerto 8000).");
  }
  let payload: any = {};
  try {
    payload = JSON.parse(text);
  } catch {
    payload = { detail: text.slice(0, 200) };
  }
  if (!response.ok) {
    throw new Error(payload.detail || "KSPR API error " + response.status);
  }
  return payload;
}

function providerHeaders(auth?: ProviderAuth): Record<string, string> {
  const headers: Record<string, string> = { "Content-Type": "application/json" };
  const desktopRuntime = (window as Window & { __KSPR_RUNTIME_CONFIG__?: { token: string } }).__KSPR_RUNTIME_CONFIG__;
  const configuredToken = import.meta.env.VITE_API_TOKEN as string | undefined;
  if (desktopRuntime?.token || configuredToken) headers["X-KSPR-Desktop-Token"] = desktopRuntime?.token || configuredToken || "";
  if (auth?.apiKey) headers["X-KSPR-API-Key"] = auth.apiKey;
  if (auth?.provider) headers["X-KSPR-Provider"] = auth.provider;
  if (auth?.baseUrl) headers["X-KSPR-Base-URL"] = auth.baseUrl;
  if (auth?.authMode) headers["X-KSPR-Auth-Mode"] = auth.authMode;
  return headers;
}

async function runAnalysisStream(payload: AnalysisRequest, auth: ProviderAuth | undefined, signal: AbortSignal | undefined, onProgress?: (progress: AnalysisProgress) => void, onToken?: (delta: string) => void): Promise<AnalysisResult | undefined> {
  const response = await fetch(apiUrl() + "/api/v1/analyze/stream", {
    method: "POST",
    headers: providerHeaders(auth),
    signal,
    body: JSON.stringify(payload),
  });
  if (response.status === 404) return undefined;
  if (!response.ok) return parse<AnalysisResult>(response);
  if (!response.body) throw new Error("KSPR no abrió el canal de eventos de análisis.");

  const reader = response.body.getReader();
  const decoder = new TextDecoder();
  let buffer = "";
  let result: AnalysisResult | undefined;
  const consume = (chunk: string) => {
    buffer += chunk;
    const events = buffer.split("\n\n");
    buffer = events.pop() || "";
    for (const event of events) {
      const data = event.split("\n").find((line) => line.startsWith("data: "))?.slice(6);
      if (!data) continue;
      const payloadEvent = JSON.parse(data) as { type: string; progress?: AnalysisProgress; result?: AnalysisResult; message?: string; text?: string };
      if (payloadEvent.type === "progress" && payloadEvent.progress) onProgress?.(payloadEvent.progress);
      if (payloadEvent.type === "delta" && payloadEvent.text) onToken?.(payloadEvent.text);
      if (payloadEvent.type === "error") throw new Error(payloadEvent.message || "KSPR no pudo completar el análisis");
      if (payloadEvent.type === "result" && payloadEvent.result) result = payloadEvent.result;
    }
  };
  while (!result) {
    const read = await reader.read();
    if (read.done) break;
    consume(decoder.decode(read.value, { stream: true }));
  }
  if (!result) throw new Error("El canal de eventos de KSPR terminó sin resultado.");
  return result;
}

export async function runAnalysis(payload: AnalysisRequest, auth?: ProviderAuth, signal?: AbortSignal, onProgress?: (progress: AnalysisProgress) => void, onToken?: (delta: string) => void): Promise<AnalysisResult> {
  const contextSize = payload.files.reduce((total, file) => total + file.content.length, 0);
  if (payload.mode !== "async" && contextSize <= 250_000) {
    const streamed = await runAnalysisStream(payload, auth, signal, onProgress, onToken);
    if (streamed) return streamed;
  }
  const headers = providerHeaders(auth);
  const response = await fetch(apiUrl() + "/api/v1/analyze", {
    method: "POST",
    headers,
    signal,
    body: JSON.stringify(payload),
  });
  if (response.status !== 409) return parse<AnalysisResult>(response);

  const jobResponse = await fetch(apiUrl() + "/api/v1/jobs", {
    method: "POST",
    headers,
    signal,
    body: JSON.stringify({ ...payload, mode: "async" }),
  });
  const job = await parse<{ job_id: string }>(jobResponse);
  onProgress?.({ status: "queued", progress: 0, stage: "queued", message: "Esperando ejecución" });
  const cancelJob = () => {
    void fetch(apiUrl() + "/api/v1/jobs/" + job.job_id, { method: "DELETE", headers }).catch(() => undefined);
  };
  signal?.addEventListener("abort", cancelJob, { once: true });
  try {
    for (;;) {
      await new Promise((resolve) => setTimeout(resolve, 700));
      const status = await parse<AnalysisProgress & { result?: AnalysisResult; error?: string }>(
        await fetch(apiUrl() + "/api/v1/jobs/" + job.job_id, { headers, signal }),
      );
      onProgress?.(status);
      if (status.status === "completed" && status.result) return status.result;
      if (status.status === "failed") throw new Error(status.error || "El job falló");
      if (status.status === "cancelled") throw new DOMException("Job cancelado", "AbortError");
    }
  } finally {
    signal?.removeEventListener("abort", cancelJob);
  }
}

export async function ingestArchive(file: File): Promise<SourceFile[]> {
  const form = new FormData();
  form.append("file", file);
  const headers = providerHeaders();
  delete headers["Content-Type"];
  const response = await fetch(apiUrl() + "/api/v1/ingest/archive", { method: "POST", headers, body: form });
  const payload = await parse<{ files: SourceFile[] }>(response);
  return payload.files;
}

export async function ingestPath(path: string): Promise<SourceFile[]> {
  const response = await fetch(apiUrl() + "/api/v1/ingest/path", {
    method: "POST",
    headers: providerHeaders(),
    body: JSON.stringify({ path }),
  });
  const payload = await parse<{ files: SourceFile[] }>(response);
  return payload.files;
}

export async function ingestGitUrl(url: string): Promise<SourceFile[]> {
  const response = await fetch(apiUrl() + "/api/v1/ingest/git", { method: "POST", headers: providerHeaders(), body: JSON.stringify({ url }) });
  const payload = await parse<{ files: SourceFile[] }>(response);
  return payload.files;
}

export async function transcribeAudio(blob: Blob, auth?: ProviderAuth): Promise<string> {
  const form = new FormData();
  form.append("file", blob, "kspr-recording.webm");
  const headers = providerHeaders(auth);
  delete headers["Content-Type"];
  const response = await fetch(apiUrl() + "/api/v1/transcribe", { method: "POST", headers, body: form });
  const payload = await parse<{ text: string }>(response);
  return payload.text;
}

export type GeminiModel = {
  id: string;
  name: string;
  description: string;
  input_token_limit?: number;
  output_token_limit?: number;
};

export async function checkProviderStatus(auth?: ProviderAuth): Promise<{ connected: boolean; provider: string; models: GeminiModel[]; message: string }> {
  const headers = providerHeaders(auth);
  const params = new URLSearchParams({ provider: auth?.provider || "gemini" });
  if (auth?.baseUrl) params.set("base_url", auth.baseUrl);
  const response = await fetch(apiUrl() + "/api/v1/providers/status?" + params.toString(), { headers });
  return parse(response);
}

export const checkGeminiStatus = (apiKey?: string) => checkProviderStatus({ provider: "gemini", apiKey });

export type ContextTree = { concept: string; path: string; files: string[] };

export async function decompileSources(files: File[], links: string, auth?: ProviderAuth): Promise<{ status: string; tree_path: string; concept: string; files: string[]; all_trees: ContextTree[] }> {
  const form = new FormData();
  files.forEach((file) => form.append("files", file, file.name));
  const params = new URLSearchParams({ provider: auth?.provider || "local", model: "kspr-local" });
  if (links.trim()) params.set("links", links.trim());
  const headers = providerHeaders(auth);
  delete headers["Content-Type"];
  return parse(await fetch(apiUrl() + "/api/v1/decompilate?" + params.toString(), { method: "POST", headers, body: form }));
}

export async function listContextTrees(): Promise<{ trees: ContextTree[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/trees", { headers: providerHeaders() }));
}

export async function inspectMcp(servers: Record<string, unknown>): Promise<{ connected: number; servers: unknown[]; tools: unknown[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/mcp/inspect", { method: "POST", headers: providerHeaders(), body: JSON.stringify(servers) }));
}

export async function listPlugins(load = false): Promise<{ plugins: unknown[]; loaded: unknown[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/plugins?load=" + String(load), { headers: providerHeaders() }));
}

export async function listSkills(includeContext = false): Promise<{ skills: unknown[]; context?: string }> {
  return parse(await fetch(apiUrl() + "/api/v1/skills?include_context=" + String(includeContext), { headers: providerHeaders() }));
}

export async function listCapabilities(query = ""): Promise<{ capabilities: unknown[]; count: number }> {
  return parse(await fetch(apiUrl() + "/api/v1/capabilities?query=" + encodeURIComponent(query), { headers: providerHeaders() }));
}

export async function listSavedPrompts(): Promise<{ prompts: unknown[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/prompts", { headers: providerHeaders() }));
}

export async function savePrompt(prompt: { name: string; content: string; description?: string; tags?: string[] }): Promise<unknown> {
  return parse(await fetch(apiUrl() + "/api/v1/prompts", { method: "POST", headers: providerHeaders(), body: JSON.stringify(prompt) }));
}

export async function deleteSavedPrompt(name: string): Promise<{ removed: boolean; name: string }> {
  return parse(await fetch(apiUrl() + "/api/v1/prompts/" + encodeURIComponent(name), { method: "DELETE", headers: providerHeaders() }));
}

export async function verifyLicense(code: string): Promise<{ valid: boolean; message: string }> {
  return parse(await fetch(apiUrl() + "/api/v1/license/verify", { method: "POST", headers: providerHeaders(), body: JSON.stringify({ code }) }));
}

export async function getLicenseStatus(): Promise<{ unlocked: boolean; available_codes: number }> {
  return parse(await fetch(apiUrl() + "/api/v1/license/status", { headers: providerHeaders() }));
}

export async function unlockLicense(code: string): Promise<{ valid: boolean; unlocked: boolean; message: string }> {
  return parse(await fetch(apiUrl() + "/api/v1/license/unlock", { method: "POST", headers: providerHeaders(), body: JSON.stringify({ code }) }));
}

export async function lockLicense(): Promise<{ unlocked: boolean }> {
  return parse(await fetch(apiUrl() + "/api/v1/license/lock", { method: "POST", headers: providerHeaders() }));
}

export type PersistedSession = { id: string; title: string; [key: string]: unknown };
export type ProjectRecord = { id?: string; name: string; path: string; updated_at?: number };

export async function listSessions(): Promise<{ sessions: PersistedSession[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/sessions", { headers: providerHeaders() }));
}

export async function saveSession(session: PersistedSession): Promise<PersistedSession> {
  return parse(await fetch(apiUrl() + "/api/v1/sessions", { method: "POST", headers: providerHeaders(), body: JSON.stringify(session) }));
}

export async function deletePersistedSession(id: string): Promise<{ removed: boolean; id: string }> {
  return parse(await fetch(apiUrl() + "/api/v1/sessions/" + encodeURIComponent(id), { method: "DELETE", headers: providerHeaders() }));
}

export async function listProjects(): Promise<{ projects: ProjectRecord[] }> {
  return parse(await fetch(apiUrl() + "/api/v1/projects", { headers: providerHeaders() }));
}

export async function createProject(project: { name: string; path?: string }): Promise<ProjectRecord> {
  return parse(await fetch(apiUrl() + "/api/v1/projects", { method: "POST", headers: providerHeaders(), body: JSON.stringify(project) }));
}
