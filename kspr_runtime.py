"""KSPR local FastAPI runtime shared by desktop and CLI."""
from __future__ import annotations

import argparse
import asyncio
import json
import os
import secrets
import subprocess
import sys
import tempfile
import zipfile
import sqlite3
import ipaddress
from kspr_engine.network import validate_public_http_url
from urllib.parse import urlparse
from pathlib import Path

import uvicorn
from fastapi import BackgroundTasks, Depends, FastAPI, File, Header, HTTPException, Query, Request, UploadFile
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse
from fastapi.staticfiles import StaticFiles

sys.path.insert(0, str(Path(__file__).resolve().parent / "backend"))
from kspr_engine.analyzer import analyze
from kspr_engine.config import Settings
from kspr_engine.models import AnalysisRequest, JobStatus, MCPServerConfig, SessionEvent
from kspr_engine.inverse_engineering import InverseEngineeringRequest, reconstruct
from kspr_engine.providers import GeminiProvider, ProviderError, get_provider
from kspr_engine.decompiler import DecompilerEngine
from kspr_engine.mcp_client import MCPManager
from kspr_engine.plugin_manager import PluginManager
from kspr_engine.skills import SkillsManager
from kspr_engine.license_service import license_count, normalize_code, verify_license as check_license

ALLOWED = {".py", ".js", ".jsx", ".ts", ".tsx", ".cs", ".java", ".sql", ".html", ".vue", ".php", ".md", ".txt", ".json", ".yaml", ".yml", ".css", ".scss", ".go", ".rs", ".rb", ".java", ".kt", ".sh", ".xml", ".csv"}
MAX_FILE = 2_000_000
MAX_ARCHIVE = 25_000_000
MAX_UPLOAD_FILES = 200
MAX_UPLOAD_TOTAL = 50_000_000
DESKTOP_TOKEN = os.getenv("KSPR_API_TOKEN", "").strip()
LICENSE_UNLOCKED = False
LICENSE_FAILURES: dict[str, tuple[int, float]] = {}
settings = Settings()
KSPR_DIR = Path(os.getenv("KSPR_DATA_DIR", str(Path.home() / ".kspr"))).expanduser()
PROMPTS_FILE = KSPR_DIR / "prompts.json"
PROJECTS_FILE = KSPR_DIR / "projects.json"
DB_FILE = KSPR_DIR / "kspr.db"


def _db() -> sqlite3.Connection:
    KSPR_DIR.mkdir(parents=True, exist_ok=True)
    connection = sqlite3.connect(DB_FILE)
    connection.row_factory = sqlite3.Row
    connection.execute("CREATE TABLE IF NOT EXISTS sessions (id TEXT PRIMARY KEY, title TEXT NOT NULL, payload TEXT NOT NULL, updated_at REAL NOT NULL)")
    connection.execute("CREATE TABLE IF NOT EXISTS projects (id TEXT PRIMARY KEY, name TEXT NOT NULL, path TEXT NOT NULL, updated_at REAL NOT NULL)")
    connection.commit()
    return connection


def _read_json_file(path: Path, fallback):
    try:
        return json.loads(path.read_text(encoding="utf-8")) if path.is_file() else fallback
    except Exception:
        return fallback


def _write_json_file(path: Path, value) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2), encoding="utf-8")


def collect(root: Path) -> list[dict[str, str]]:
    root = root.resolve()
    files: list[dict[str, str]] = []
    for path in root.rglob("*"):
        if not path.is_file() or path.is_symlink() or path.suffix.lower() not in ALLOWED:
            continue
        if any(part in {".git", "node_modules", ".venv", "venv", "dist", "build"} for part in path.parts):
            continue
        try:
            if path.stat().st_size <= MAX_FILE:
                files.append({"path": str(path.relative_to(root)).replace(os.sep, "/"), "content": path.read_text(encoding="utf-8", errors="replace")})
        except OSError:
            continue
    return files[:2000]


def validated_source_path(raw: str) -> Path:
    path = Path(raw).expanduser().resolve()
    if settings.environment.lower() in {"production", "staging"}:
        data_root = KSPR_DIR.resolve()
        if path != data_root and data_root not in path.parents:
            raise HTTPException(403, "El servidor solo puede leer archivos dentro de KSPR_DATA_DIR")
    return path


def collect_zip(raw: bytes) -> list[dict[str, str]]:
    if len(raw) > MAX_ARCHIVE:
        raise ValueError("El ZIP supera el límite de 25 MB")
    files: list[dict[str, str]] = []
    total_uncompressed = 0
    with zipfile.ZipFile(__import__("io").BytesIO(raw)) as archive:
        for member in archive.infolist()[:2000]:
            name = member.filename.replace("\\", "/")
            if member.is_dir() or Path(name).suffix.lower() not in ALLOWED or name.startswith("/") or ".." in name.split("/") or member.file_size > MAX_FILE:
                continue
            total_uncompressed += member.file_size
            if total_uncompressed > MAX_FILE * 200:
                raise ValueError("El ZIP supera el límite total de contenido")
            files.append({"path": name, "content": archive.read(member)[:MAX_FILE].decode("utf-8", errors="replace")})
    return files


async def require_desktop_token(x_kspr_desktop_token: str | None = Header(default=None, alias="X-KSPR-Desktop-Token")):
    expected = DESKTOP_TOKEN or settings.api_token
    if expected and not secrets.compare_digest(x_kspr_desktop_token or "", expected):
        raise HTTPException(status_code=401, detail="Token del runtime desktop inválido")
    if not expected and settings.environment.lower() in {"production", "staging"}:
        raise HTTPException(status_code=503, detail="KSPR requiere KSPR_API_TOKEN en modo servidor")


async def require_license():
    if not LICENSE_UNLOCKED:
        raise HTTPException(status_code=403, detail="KSPR está bloqueado. Ejecuta /login e introduce tu código de licencia.")


class LocalJobs:
    def __init__(self):
        self.items: dict[str, JobStatus] = {}

    def create(self) -> JobStatus:
        job = JobStatus(job_id=secrets.token_hex(16), status="queued", progress=0, stage="queued", message="Esperando ejecución")
        self.items[job.job_id] = job
        return job

    async def run(self, job_id: str, request: AnalysisRequest, headers: dict[str, str]):
        job = self.items[job_id]
        async def progress(value: int, stage: str, message: str):
            if job.status == "cancelled": raise asyncio.CancelledError()
            job.status, job.progress, job.stage, job.message = "running", value, stage, message
        try:
            result = await analyze(request, settings, progress, headers.get("X-KSPR-API-Key") or headers.get("X-Gemini-API-Key"), headers.get("X-KSPR-BASE-URL"), headers.get("X-KSPR-AUTH-MODE", "api_key"))
            job.result, job.status, job.progress, job.stage, job.message = result, "completed", 100, "completed", "Análisis terminado"
        except asyncio.CancelledError:
            job.status, job.stage, job.message = "cancelled", "cancelled", "Cancelado por el usuario"
        except Exception as exc:
            job.status, job.stage, job.message, job.error = "failed", "error", "El análisis terminó con error", str(exc)


jobs = LocalJobs()
app = FastAPI(title="KSPR Desktop Runtime", version="0.1.0")
app.add_middleware(CORSMiddleware, allow_origins=settings.allowed_origins + ["tauri://localhost", "http://tauri.localhost"], allow_methods=["GET", "POST", "PUT", "PATCH", "DELETE", "OPTIONS"], allow_headers=["Content-Type", "X-KSPR-Desktop-Token", "X-KSPR-API-Key", "X-KSPR-Provider", "X-KSPR-Base-URL", "X-KSPR-Auth-Mode"])


@app.middleware("http")
async def license_gate(request, call_next):
    public = {"/api/v1/health", "/api/v1/license/status", "/api/v1/license/unlock", "/api/v1/license/verify"}
    if request.url.path.startswith("/api/v1/") and request.url.path not in public and not LICENSE_UNLOCKED:
        from fastapi.responses import JSONResponse
        return JSONResponse(status_code=403, content={"detail": "KSPR está bloqueado. Ejecuta /login e introduce tu código de licencia."})
    return await call_next(request)


@app.get("/api/v1/health")
async def health():
    return {"status": "ok", "version": "0.1.0", "mode": "web", "capabilities": ["analysis", "inverse-engineering", "context-trees", "mcp", "plugins"]}


@app.post("/api/v1/inverse-engineering/reconstruct", dependencies=[Depends(require_desktop_token)])
async def inverse_engineering_reconstruct(request: InverseEngineeringRequest):
    """Reconstruct a digital, physical, process, conceptual or imaginary subject.

    The deterministic graph is the auditable base layer. A provider can enrich
    it later, but no conclusion is allowed to exist without evidence links.
    """
    return reconstruct(request)


@app.get("/api/v1/license/status", dependencies=[Depends(require_desktop_token)])
async def license_status():
    return {"unlocked": LICENSE_UNLOCKED, "available_codes": license_count()}


@app.post("/api/v1/license/unlock", dependencies=[Depends(require_desktop_token)])
async def unlock_license(payload: dict, request: Request):
    global LICENSE_UNLOCKED
    now = __import__("time").time()
    client = request.client.host if request.client else "unknown"
    count, since = LICENSE_FAILURES.get(client, (0, now))
    if now - since > 900:
        count, since = 0, now
    if count >= 10:
        raise HTTPException(429, "Demasiados intentos. Espera 15 minutos antes de volver a intentarlo.")
    code = normalize_code(str(payload.get("code", "")))
    if not check_license(code):
        LICENSE_FAILURES[client] = (count + 1, since)
        raise HTTPException(status_code=401, detail="Código de identificación inválido o no reconocido")
    LICENSE_FAILURES.pop(client, None)
    LICENSE_UNLOCKED = True
    return {"valid": True, "unlocked": True, "message": "Código verificado con éxito"}


@app.post("/api/v1/license/lock", dependencies=[Depends(require_desktop_token)])
async def lock_license():
    global LICENSE_UNLOCKED
    LICENSE_UNLOCKED = False
    return {"unlocked": False}


@app.post("/api/v1/license/verify", dependencies=[Depends(require_desktop_token)])
async def verify_license_route(payload: dict):
    valid = check_license(str(payload.get("code", "")))
    return {"valid": valid, "message": "Código verificado con éxito" if valid else "Código de identificación inválido o no reconocido"}


@app.post("/api/v1/ingest/path", dependencies=[Depends(require_desktop_token)])
async def ingest_path(payload: dict):
    path = validated_source_path(str(payload.get("path", "")))
    if path.is_dir(): files = collect(path)
    elif path.is_file() and path.suffix.lower() == ".zip": files = collect_zip(path.read_bytes())
    elif path.is_file() and path.suffix.lower() in ALLOWED: files = [{"path": path.name, "content": path.read_text(encoding="utf-8", errors="replace")[:MAX_FILE]}]
    else: raise HTTPException(400, "La ruta no existe o no es compatible")
    return {"files": files, "count": len(files), "source": "path"}


@app.post("/api/v1/ingest/archive", dependencies=[Depends(require_desktop_token)])
async def ingest_archive(file: UploadFile = File(...)):
    try: files = collect_zip(await file.read(MAX_ARCHIVE + 1))
    except (zipfile.BadZipFile, ValueError) as exc: raise HTTPException(400, str(exc)) from exc
    return {"files": files, "count": len(files), "source": "zip"}


@app.post("/api/v1/ingest/files", dependencies=[Depends(require_desktop_token)])
async def ingest_files(files: list[UploadFile] = File(...)):
    """Previsualiza varios archivos manteniendo la misma regla de lectura segura que la CLI."""
    result: list[dict[str, str]] = []
    total = 0
    for file in files[:MAX_UPLOAD_FILES]:
        name = (file.filename or "").replace("\\", "/")
        if name.startswith("/") or ".." in name.split("/") or Path(name).suffix.lower() not in ALLOWED:
            continue
        raw = await file.read(MAX_FILE + 1)
        if len(raw) > MAX_FILE:
            raise HTTPException(413, "Cada archivo está limitado a 2 MB")
        total += len(raw)
        if total > MAX_UPLOAD_TOTAL:
            raise HTTPException(413, "La carga total supera el límite de 50 MB")
        result.append({"path": name, "content": raw.decode("utf-8", errors="replace")})
    return {"files": result, "count": len(result), "source": "files"}


@app.post("/api/v1/ingest/git", dependencies=[Depends(require_desktop_token)])
async def ingest_git(payload: dict):
    """Clona un repositorio Git temporalmente y devuelve solo evidencia textual."""
    url = str(payload.get("url", "")).strip()
    try:
        url = validate_public_http_url(url)
    except ValueError as exc:
        raise HTTPException(400, str(exc)) from exc
    checkout = Path(tempfile.mkdtemp(prefix="kspr-git-"))
    try:
        subprocess.run(["git", "clone", "--depth", "1", "--no-tags", url, str(checkout)], check=True, capture_output=True, text=True, timeout=120)
        files = collect(checkout)
        if not files:
            raise HTTPException(400, "No se encontraron archivos soportados en el repositorio")
        return {"files": files, "count": len(files), "source": "git", "url": url}
    except subprocess.CalledProcessError as exc:
        raise HTTPException(400, "No se pudo clonar el repositorio Git: " + (exc.stderr or "").strip()[:300]) from exc
    except subprocess.TimeoutExpired as exc:
        raise HTTPException(408, "La clonación Git superó el límite de 120 segundos") from exc
    finally:
        import shutil
        shutil.rmtree(checkout, ignore_errors=True)


async def execute(request: AnalysisRequest, headers: dict[str, str], queue: asyncio.Queue | None = None):
    async def progress(value: int, stage: str, message: str):
        if queue:
            event = SessionEvent(type="tool.progress", status=stage, progress=value, payload={"message": message})
            await queue.put({"type": "progress", "event": event.model_dump(mode="json"), "progress": {"status": "running", "progress": value, "stage": stage, "message": message}})
    async def token(delta: str):
        if queue:
            event = SessionEvent(type="message.delta", payload={"text": delta})
            await queue.put({"type": "delta", "event": event.model_dump(mode="json"), "text": delta})
    return await analyze(request, settings, progress, headers.get("X-KSPR-API-Key") or headers.get("X-Gemini-API-Key"), headers.get("X-KSPR-BASE-URL"), headers.get("X-KSPR-AUTH-MODE", "api_key"), token if queue else None)


@app.post("/api/v1/analyze", dependencies=[Depends(require_desktop_token)])
async def analyze_request(request: AnalysisRequest, x_kspr_api_key: str | None = Header(default=None), x_gemini_api_key: str | None = Header(default=None), x_kspr_base_url: str | None = Header(default=None), x_kspr_auth_mode: str = Header(default="api_key")):
    if sum(len(file.content) for file in request.files) > 250_000 and request.mode.value != "async": raise HTTPException(409, "El contexto requiere un job asíncrono")
    result = await execute(request, {"X-KSPR-API-Key": x_kspr_api_key or "", "X-Gemini-API-Key": x_gemini_api_key or "", "X-KSPR-BASE-URL": x_kspr_base_url or "", "X-KSPR-AUTH-MODE": x_kspr_auth_mode})
    return result


@app.post("/api/v1/analyze/stream", dependencies=[Depends(require_desktop_token)])
async def analyze_stream(request: AnalysisRequest, x_kspr_api_key: str | None = Header(default=None), x_gemini_api_key: str | None = Header(default=None), x_kspr_base_url: str | None = Header(default=None), x_kspr_auth_mode: str = Header(default="api_key")):
    queue: asyncio.Queue = asyncio.Queue()
    headers = {"X-KSPR-API-Key": x_kspr_api_key or "", "X-Gemini-API-Key": x_gemini_api_key or "", "X-KSPR-BASE-URL": x_kspr_base_url or "", "X-KSPR-AUTH-MODE": x_kspr_auth_mode}
    async def events():
        task = asyncio.create_task(execute(request, headers, queue))
        try:
            while True:
                if task.done():
                    try: result = task.result(); yield f"data: {json.dumps({'type':'result','event': SessionEvent(type='session.completed').model_dump(mode='json'),'result':result.model_dump(mode='json')}, ensure_ascii=False)}\n\n"; break
                    except Exception as exc: yield f"data: {json.dumps({'type':'error','event': SessionEvent(type='session.failed', payload={'message': str(exc)}).model_dump(mode='json'),'message':str(exc)}, ensure_ascii=False)}\n\n"; break
                event = await queue.get(); yield f"data: {json.dumps(event, ensure_ascii=False)}\n\n"
        finally:
            if not task.done(): task.cancel()
    return StreamingResponse(events(), media_type="text/event-stream", headers={"Cache-Control":"no-cache","X-Accel-Buffering":"no"})


@app.post("/api/v1/jobs", response_model=JobStatus, status_code=202, dependencies=[Depends(require_desktop_token)])
async def create_job(
    request: AnalysisRequest,
    background_tasks: BackgroundTasks,
    x_kspr_api_key: str | None = Header(default=None, alias="X-KSPR-API-Key"),
    x_gemini_api_key: str | None = Header(default=None, alias="X-Gemini-API-Key"),
    x_kspr_base_url: str | None = Header(default=None, alias="X-KSPR-Base-URL"),
    x_kspr_auth_mode: str = Header(default="api_key", alias="X-KSPR-Auth-Mode"),
):
    job = jobs.create()
    background_tasks.add_task(jobs.run, job.job_id, request, {
        "X-KSPR-API-Key": x_kspr_api_key or "",
        "X-Gemini-API-Key": x_gemini_api_key or "",
        "X-KSPR-BASE-URL": x_kspr_base_url or "",
        "X-KSPR-AUTH-MODE": x_kspr_auth_mode,
    })
    return job


@app.get("/api/v1/jobs/{job_id}", dependencies=[Depends(require_desktop_token)])
async def get_job(job_id: str):
    if job_id not in jobs.items: raise HTTPException(404, "Job no encontrado")
    return jobs.items[job_id]


@app.delete("/api/v1/jobs/{job_id}", dependencies=[Depends(require_desktop_token)])
async def cancel_job(job_id: str):
    job = jobs.items.get(job_id)
    if not job or job.status in {"completed", "failed", "cancelled"}: raise HTTPException(409, "El job no puede cancelarse")
    job.status, job.stage, job.message = "cancelled", "cancelled", "Cancelado por el usuario"; return {"job_id": job_id, "status": "cancelled"}


@app.post("/api/v1/transcribe", dependencies=[Depends(require_desktop_token)])
async def transcribe(file: UploadFile = File(...), x_gemini_api_key: str | None = Header(default=None), x_kspr_api_key: str | None = Header(default=None), x_kspr_auth_mode: str = Header(default="api_key")):
    try: text = await GeminiProvider(settings, api_key=x_kspr_api_key or x_gemini_api_key, auth_mode=x_kspr_auth_mode).transcribe(await file.read(), file.content_type or "audio/webm")
    except ProviderError as exc: raise HTTPException(503, str(exc)) from exc
    return {"text": text, "provider": "gemini"}


@app.get("/api/v1/providers/status", dependencies=[Depends(require_desktop_token)])
async def provider_status(provider: str = Query(default="gemini"), base_url: str | None = None, x_kspr_api_key: str | None = Header(default=None), x_gemini_api_key: str | None = Header(default=None), x_kspr_auth_mode: str = Header(default="api_key")):
    try:
        adapter = get_provider(provider, settings, api_key=x_kspr_api_key or x_gemini_api_key, base_url=base_url, auth_mode=x_kspr_auth_mode)
        models = await adapter.list_models()
        return {"connected": True, "provider": provider, "models": models, "message": f"{provider} conectado"}
    except (ProviderError, ValueError, AttributeError) as exc:
        return {"connected": False, "provider": provider, "models": [], "message": str(exc)}


@app.post("/api/v1/decompilate", dependencies=[Depends(require_desktop_token)])
async def decompilate_sources(
    files: list[UploadFile] = File(default=[]),
    links: str | None = Query(default=None),
    provider: str = Query(default="local"),
    model: str = Query(default="kspr-local"),
    x_kspr_api_key: str | None = Header(default=None, alias="X-KSPR-API-Key"),
    x_kspr_base_url: str | None = Header(default=None, alias="X-KSPR-Base-URL"),
    x_kspr_auth_mode: str = Header(default="api_key", alias="X-KSPR-Auth-Mode"),
):
    """Ingesta fuentes multimodales y genera el mismo Context Tree de la CLI."""
    decompiler = DecompilerEngine()
    ingested = []
    for file in files:
        filename = Path(file.filename or "upload.txt").name
        target = decompiler.staging_dir / f"{secrets.token_hex(8)}-{filename}"
        raw = await file.read(MAX_FILE + 1)
        if len(raw) > MAX_FILE:
            raise HTTPException(413, "Cada archivo de decompilación está limitado a 2 MB")
        target.write_bytes(raw)
        ingested.append(decompiler.ingest_source(str(target)))
    for link in (links or "").split(","):
        candidate = link.strip()
        if not candidate:
            continue
        try:
            candidate = validate_public_http_url(candidate)
        except ValueError as exc:
            raise HTTPException(400, str(exc)) from exc
        ingested.append(await asyncio.to_thread(decompiler.ingest_source, candidate))
    combined = "\n\n".join(f"SOURCE: {item['source']}\n{item.get('content', '')}" for item in ingested if item.get("success"))
    try:
        adapter = get_provider(provider, settings, api_key=x_kspr_api_key, base_url=x_kspr_base_url, auth_mode=x_kspr_auth_mode)
        response = await adapter.complete(f"Analiza estas fuentes y genera un árbol de contexto Markdown temático.\n\n{combined[:250_000]}", model)
        analysis_text = response if isinstance(response, str) else json.dumps(response, ensure_ascii=False)
    except Exception:
        analysis_text = "# Analisis Consolidado\n\nInformación recopilada y estructurada por KSPR Decompiler."
    tree_path = decompiler.generate_context_trees(ingested, analysis_text)
    return {"status": "success", "tree_path": str(tree_path), "concept": tree_path.name, "files": [item.name for item in tree_path.glob("*.md")], "all_trees": decompiler.list_trees()}


@app.get("/api/v1/trees", dependencies=[Depends(require_desktop_token)])
async def list_context_trees():
    return {"trees": DecompilerEngine().list_trees()}


@app.post("/api/v1/mcp/inspect", dependencies=[Depends(require_desktop_token)])
async def inspect_mcp(payload: dict[str, dict]):
    configs = {name: MCPServerConfig(**config) for name, config in payload.items()}
    manager = MCPManager(configs)
    connected = await manager.connect_all()
    return {"connected": connected, "servers": manager.get_connected_servers(), "tools": manager.get_tool_schemas()}


@app.get("/api/v1/plugins", dependencies=[Depends(require_desktop_token)])
async def list_plugins(load: bool = Query(default=False)):
    manager = PluginManager()
    if load:
        manager.load_all()
    return {"plugins": [item.model_dump() for item in manager.scan_plugins()], "loaded": manager.list_loaded()}


@app.get("/api/v1/skills", dependencies=[Depends(require_desktop_token)])
async def list_skills(include_context: bool = Query(default=False)):
    manager = SkillsManager()
    payload = {"skills": [item.to_dict() for item in manager.list_bundles()]}
    if include_context:
        payload["context"] = manager.get_skill_context()
    return payload


@app.get("/api/v1/capabilities", dependencies=[Depends(require_desktop_token)])
async def list_capabilities(query: str = Query(default="")):
    """Lista harnesses CLI-Anything disponibles sin ejecutar comandos del repositorio."""
    import shutil
    candidates = []
    for directory in [Path("/usr/local/bin"), Path.home() / ".local" / "bin"]:
        if directory.is_dir():
            candidates.extend({"id": item.name, "name": item.name, "description": "CLI-Anything harness detectado en el sistema", "command": [str(item)], "source_type": "cli-anything"} for item in directory.glob("cli-anything-*") if item.is_file())
    if query:
        candidates = [item for item in candidates if query.lower() in item["name"].lower()]
    return {"capabilities": candidates, "count": len(candidates)}


@app.get("/api/v1/prompts", dependencies=[Depends(require_desktop_token)])
async def list_prompts():
    data = _read_json_file(PROMPTS_FILE, {"version": "1.0.0", "prompts": {}})
    return {"prompts": list(data.get("prompts", {}).values())}


@app.post("/api/v1/prompts", dependencies=[Depends(require_desktop_token)])
async def save_prompt(payload: dict):
    data = _read_json_file(PROMPTS_FILE, {"version": "1.0.0", "prompts": {}})
    name = str(payload.get("name", "")).strip()
    if not name or not str(payload.get("content", "")).strip():
        raise HTTPException(400, "Un prompt requiere nombre y contenido")
    data.setdefault("prompts", {})[name] = {"name": name, "description": payload.get("description", ""), "content": payload["content"], "tags": payload.get("tags", [])}
    _write_json_file(PROMPTS_FILE, data)
    return data["prompts"][name]


@app.delete("/api/v1/prompts/{name}", dependencies=[Depends(require_desktop_token)])
async def delete_prompt(name: str):
    data = _read_json_file(PROMPTS_FILE, {"version": "1.0.0", "prompts": {}})
    removed = data.get("prompts", {}).pop(name, None) is not None
    _write_json_file(PROMPTS_FILE, data)
    return {"removed": removed, "name": name}


@app.get("/api/v1/projects", dependencies=[Depends(require_desktop_token)])
async def list_projects():
    connection = _db()
    rows = [dict(row) for row in connection.execute("SELECT id,name,path,updated_at FROM projects ORDER BY updated_at DESC")]
    connection.close()
    return {"projects": rows or _read_json_file(PROJECTS_FILE, [])}


@app.post("/api/v1/projects", dependencies=[Depends(require_desktop_token)])
async def create_project(payload: dict):
    name = str(payload.get("name", "")).strip()
    raw_path = str(payload.get("path", "")).strip()
    if not name and not raw_path:
        raise HTTPException(400, "Indica un nombre o ruta de proyecto")
    path = validated_source_path(raw_path) if raw_path else validated_source_path(str((KSPR_DIR / "workspace" / name).resolve()))
    path.mkdir(parents=True, exist_ok=True)
    projects = _read_json_file(PROJECTS_FILE, [])
    entry = {"name": name or path.name, "path": str(path)}
    if entry not in projects:
        projects.append(entry)
        _write_json_file(PROJECTS_FILE, projects)
    connection = _db()
    existing = connection.execute("SELECT id FROM projects WHERE path=?", (entry["path"],)).fetchone()
    project_id = existing["id"] if existing else secrets.token_hex(12)
    connection.execute("INSERT INTO projects(id,name,path,updated_at) VALUES(?,?,?,?) ON CONFLICT(id) DO UPDATE SET name=excluded.name,updated_at=excluded.updated_at", (project_id, entry["name"], entry["path"], __import__("time").time()))
    connection.commit(); connection.close()
    return {**entry, "id": project_id}


@app.get("/api/v1/sessions", dependencies=[Depends(require_desktop_token)])
async def list_sessions():
    connection = _db()
    rows = [dict(row) for row in connection.execute("SELECT id,title,updated_at FROM sessions ORDER BY updated_at DESC")]
    connection.close()
    return {"sessions": rows}


@app.get("/api/v1/sessions/{session_id}", dependencies=[Depends(require_desktop_token)])
async def get_session(session_id: str):
    connection = _db(); row = connection.execute("SELECT payload FROM sessions WHERE id=?", (session_id,)).fetchone(); connection.close()
    if not row: raise HTTPException(404, "Sesión no encontrada")
    return json.loads(row["payload"])


@app.post("/api/v1/sessions", dependencies=[Depends(require_desktop_token)])
async def save_session(payload: dict):
    session_id = str(payload.get("id") or secrets.token_hex(12))
    title = str(payload.get("title") or "Nueva sesión")
    connection = _db()
    connection.execute("INSERT INTO sessions(id,title,payload,updated_at) VALUES(?,?,?,?) ON CONFLICT(id) DO UPDATE SET title=excluded.title,payload=excluded.payload,updated_at=excluded.updated_at", (session_id, title, json.dumps({**payload, "id": session_id}, ensure_ascii=False), __import__("time").time()))
    connection.commit(); connection.close()
    return {**payload, "id": session_id, "title": title}


@app.delete("/api/v1/sessions/{session_id}", dependencies=[Depends(require_desktop_token)])
async def delete_session(session_id: str):
    connection = _db(); cursor = connection.execute("DELETE FROM sessions WHERE id=?", (session_id,)); connection.commit(); connection.close()
    return {"removed": cursor.rowcount > 0, "id": session_id}


WEB_DIST = Path(__file__).resolve().parent / "dist"
if WEB_DIST.is_dir():
    app.mount("/", StaticFiles(directory=WEB_DIST, html=True), name="web")


async def interactive_shell() -> None:
    """Terminal counterpart of the desktop session, backed by the same engine."""
    config_file = KSPR_DIR / "config.json"
    config = _read_json_file(config_file, {})
    for provider, key in config.get("api_keys", {}).items():
        if key:
            os.environ[f"KSPR_{provider.upper().replace('-', '_')}_API_KEY"] = key
    provider_name = config.get("active_provider", "local")
    model = config.get("active_model", "kspr-local" if provider_name == "local" else "gemini-2.5-flash")
    workspace = Path(config.get("current_workspace", Path.cwd())).expanduser()
    attached: dict[str, str] = {}
    history: list[dict[str, str]] = []

    print("KSPR AI — Interactive Shell")
    print(f"Provider: {provider_name} · Model: {model} · Workspace: {workspace}")
    print("Escribe /help para ver los comandos disponibles.")
    while True:
        try:
            prompt = input("\nkspr ❯ ").strip()
        except (EOFError, KeyboardInterrupt):
            print("\nSaliendo de la sesión de KSPR CLI.")
            return
        if not prompt:
            continue
        if prompt.startswith("/"):
            command, _, argument = prompt.partition(" ")
            command = command.lower()
            if command in {"/exit", "/quit"}:
                print("Saliendo de la sesión de KSPR CLI.")
                return
            if command == "/help":
                print("/login /api /project /model /provider /mcp /plugins /capabilities /prompts /skills /decompilate /trees /context /compact /new /sessions /clear /update /exit")
            elif command == "/provider":
                if argument.strip():
                    provider_name = argument.strip()
                    config["active_provider"] = provider_name
                    _write_json_file(config_file, config)
                print(f"Proveedor activo: {provider_name}")
            elif command == "/model":
                if argument.strip():
                    model = argument.strip()
                    config["active_model"] = model
                    _write_json_file(config_file, config)
                print(f"Modelo activo: {model}")
            elif command == "/context":
                print("Archivos adjuntos:")
                print("\n".join(f"- {path}" for path in attached) or "(vacío)")
            elif command == "/compact":
                history = history[-6:]
                print("Contexto compactado.")
            elif command == "/new":
                attached.clear(); history.clear(); print("Nueva sesión iniciada.")
            elif command == "/clear":
                os.system("cls" if os.name == "nt" else "clear")
            elif command == "/mcp":
                raw = _read_json_file(KSPR_DIR / "mcp_servers.json", {})
                manager = MCPManager({name: MCPServerConfig(**value) for name, value in raw.items()})
                await manager.connect_all()
                print(json.dumps(manager.get_connected_servers(), ensure_ascii=False, indent=2))
            elif command == "/plugins":
                manager = PluginManager()
                manager.load_all()
                print(json.dumps(manager.list_loaded() or [item.model_dump() for item in manager.scan_plugins()], ensure_ascii=False, indent=2))
            elif command == "/skills":
                manager = SkillsManager()
                print(json.dumps([item.to_dict() for item in manager.list_bundles()], ensure_ascii=False, indent=2))
            elif command == "/trees":
                print(json.dumps(DecompilerEngine().list_trees(), ensure_ascii=False, indent=2))
            elif command == "/update":
                print("Actualiza KSPR ejecutando el instalador oficial: curl -fsSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash")
            else:
                print(f"Comando disponible en la interfaz desktop o aún no parametrizado: {command}")
            continue

        for token in prompt.split():
            if token.startswith("@"):
                candidate = workspace / token[1:]
                if candidate.is_file():
                    attached[token[1:]] = candidate.read_text(encoding="utf-8", errors="replace")
        source = [dict(path=path, content=content) for path, content in attached.items()]
        source.append({"path": "chat-request.md", "content": prompt})
        history.extend([{"role": "user", "content": prompt}])
        try:
            request = AnalysisRequest(project_name=workspace.name, files=source, provider=provider_name, model=model, iterations=3, mode="direct", instruction=prompt)
            result = await analyze(request, settings, gemini_api_key=config.get("api_keys", {}).get(provider_name))
            print(result.response_text or "KSPR terminó el análisis sin texto visible.")
            history.append({"role": "assistant", "content": result.response_text})
        except Exception as exc:
            print(f"[!] {exc}")


def main():
    global DESKTOP_TOKEN
    if len(sys.argv) > 1 and sys.argv[1] == "serve":
        parser = argparse.ArgumentParser(); parser.add_argument("serve"); parser.add_argument("--host", default="127.0.0.1"); parser.add_argument("--port", type=int, default=0); parser.add_argument("--token", default=None); args = parser.parse_args()
        DESKTOP_TOKEN = args.token or secrets.token_urlsafe(32)
        config = uvicorn.Config(app, host=args.host, port=args.port, log_level="warning")
        server = uvicorn.Server(config)
        async def start():
            sock = config.bind_socket(); print(json.dumps({"ready": True, "host": args.host, "port": sock.getsockname()[1], "token": DESKTOP_TOKEN}), flush=True); await server.serve(sockets=[sock])
        asyncio.run(start()); return
    parser = argparse.ArgumentParser(prog="kspr", description="KSPR AI - Empresarial CLI Engine")
    parser.add_argument("source", type=Path, nargs="?", help="Directorio o ZIP a analizar")
    parser.add_argument("--interactive", "-i", action="store_true", help="Inicia el shell interactivo")
    parser.add_argument("--git-url", help="Clona un repositorio Git en modo lectura para analizarlo")
    parser.add_argument("--output", type=Path, default=Path("kspr-context"))
    parser.add_argument("--project-name", default=None)
    parser.add_argument("--iterations", type=int, default=3, choices=range(1, 9))
    parser.add_argument("--provider", default="gemini", choices=["local", "gemini", "openai", "groq", "deepseek", "anthropic", "openrouter", "opencode-zen", "openai-compatible"])
    parser.add_argument("--model", default=None)
    parser.add_argument("--version", action="store_true")
    parser.add_argument("--uninstall", action="store_true", help="Elimina la instalación local de KSPR")
    args = parser.parse_args()

    if args.version:
        print("kspr 0.1.0")
        return
    if args.uninstall:
        install_dir = Path.home() / ".kspr"
        if install_dir.is_dir():
            import shutil
            shutil.rmtree(install_dir)
            print("KSPR desinstalado completamente del sistema.")
        else:
            print("No hay una instalación de KSPR encontrada en ~/.kspr/.")
        return
    if args.interactive or (args.source is None and not args.git_url):
        asyncio.run(interactive_shell())
        return

    workspace: Path | None = None
    try:
        if args.git_url:
            workspace = Path(tempfile.mkdtemp(prefix="kspr-git-"))
            subprocess.run(["git", "clone", "--depth", "1", "--no-tags", args.git_url, str(workspace)], check=True, capture_output=True, text=True)
            files = collect(workspace)
            default_name = args.git_url.rstrip("/").rsplit("/", 1)[-1].removesuffix(".git")
        elif args.source:
            files = collect(args.source)
            default_name = args.source.stem
        else:
            parser.error("indica una fuente local o --git-url")
        if not files:
            parser.error("No se encontraron archivos soportados en la fuente")
        request = AnalysisRequest(project_name=args.project_name or default_name, files=files, provider=args.provider, model=args.model, iterations=args.iterations, mode="direct")
        result = asyncio.run(analyze(request, settings))
        args.output.mkdir(parents=True, exist_ok=True)
        for artifact in result.artifacts:
            target = args.output / artifact.path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text(artifact.content, encoding="utf-8")
        print(f"KSPR completó el análisis: {len(request.files)} archivos -> {args.output.resolve()}")
    finally:
        if workspace and workspace.is_dir():
            import shutil
            shutil.rmtree(workspace, ignore_errors=True)


if __name__ == "__main__": main()
