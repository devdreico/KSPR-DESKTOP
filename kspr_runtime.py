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
from pathlib import Path

import uvicorn
from fastapi import BackgroundTasks, Depends, FastAPI, File, Header, HTTPException, Query, UploadFile
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse

sys.path.insert(0, str(Path(__file__).resolve().parent / "backend"))
from kspr_engine.analyzer import analyze
from kspr_engine.config import Settings
from kspr_engine.models import AnalysisRequest, JobStatus
from kspr_engine.providers import GeminiProvider, ProviderError, get_provider

ALLOWED = {".py", ".js", ".jsx", ".ts", ".tsx", ".cs", ".java", ".sql", ".html", ".vue", ".php", ".md", ".txt", ".json", ".yaml", ".yml"}
MAX_FILE = 2_000_000
MAX_ARCHIVE = 25_000_000
DESKTOP_TOKEN = ""
settings = Settings()


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


def collect_zip(raw: bytes) -> list[dict[str, str]]:
    if len(raw) > MAX_ARCHIVE:
        raise ValueError("El ZIP supera el límite de 25 MB")
    files: list[dict[str, str]] = []
    with zipfile.ZipFile(__import__("io").BytesIO(raw)) as archive:
        for member in archive.infolist()[:2000]:
            name = member.filename.replace("\\", "/")
            if member.is_dir() or Path(name).suffix.lower() not in ALLOWED or name.startswith("/") or ".." in name.split("/") or member.file_size > MAX_FILE:
                continue
            files.append({"path": name, "content": archive.read(member)[:MAX_FILE].decode("utf-8", errors="replace")})
    return files


async def require_desktop_token(x_kspr_desktop_token: str | None = Header(default=None, alias="X-KSPR-Desktop-Token")):
    if DESKTOP_TOKEN and not secrets.compare_digest(x_kspr_desktop_token or "", DESKTOP_TOKEN):
        raise HTTPException(status_code=401, detail="Token del runtime desktop inválido")


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
app.add_middleware(CORSMiddleware, allow_origins=["http://localhost:5173", "tauri://localhost", "http://tauri.localhost"], allow_methods=["*"], allow_headers=["*"])


@app.get("/api/v1/health")
async def health():
    return {"status": "ok", "version": "0.1.0", "mode": "desktop-local"}


@app.post("/api/v1/ingest/path", dependencies=[Depends(require_desktop_token)])
async def ingest_path(payload: dict):
    path = Path(str(payload.get("path", ""))).expanduser().resolve()
    if path.is_dir(): files = collect(path)
    elif path.is_file() and path.suffix.lower() == ".zip": files = collect_zip(path.read_bytes())
    elif path.is_file() and path.suffix.lower() in ALLOWED: files = [{"path": path.name, "content": path.read_text(encoding="utf-8", errors="replace")[:MAX_FILE]}]
    else: raise HTTPException(400, "La ruta no existe o no es compatible")
    return {"files": files, "count": len(files), "source": "path"}


@app.post("/api/v1/ingest/archive", dependencies=[Depends(require_desktop_token)])
async def ingest_archive(file: UploadFile = File(...)):
    try: files = collect_zip(await file.read())
    except (zipfile.BadZipFile, ValueError) as exc: raise HTTPException(400, str(exc)) from exc
    return {"files": files, "count": len(files), "source": "zip"}


async def execute(request: AnalysisRequest, headers: dict[str, str], queue: asyncio.Queue | None = None):
    async def progress(value: int, stage: str, message: str):
        if queue: await queue.put({"type": "progress", "progress": {"status": "running", "progress": value, "stage": stage, "message": message}})
    async def token(delta: str):
        if queue: await queue.put({"type": "delta", "text": delta})
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
                    try: result = task.result(); yield f"data: {json.dumps({'type':'result','result':result.model_dump(mode='json')}, ensure_ascii=False)}\n\n"; break
                    except Exception as exc: yield f"data: {json.dumps({'type':'error','message':str(exc)}, ensure_ascii=False)}\n\n"; break
                event = await queue.get(); yield f"data: {json.dumps(event, ensure_ascii=False)}\n\n"
        finally:
            if not task.done(): task.cancel()
    return StreamingResponse(events(), media_type="text/event-stream", headers={"Cache-Control":"no-cache","X-Accel-Buffering":"no"})


@app.post("/api/v1/jobs", response_model=JobStatus, status_code=202, dependencies=[Depends(require_desktop_token)])
async def create_job(request: AnalysisRequest, background_tasks: BackgroundTasks, x_kspr_api_key: str | None = Header(default=None), x_gemini_api_key: str | None = Header(default=None)):
    job = jobs.create(); background_tasks.add_task(jobs.run, job.job_id, request, {"X-KSPR-API-Key": x_kspr_api_key or "", "X-Gemini-API-Key": x_gemini_api_key or ""}); return job


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
    parser = argparse.ArgumentParser(prog="kspr"); parser.add_argument("source", type=Path, nargs="?"); parser.add_argument("--output", type=Path, default=Path("kspr-context")); parser.add_argument("--iterations", type=int, default=3, choices=range(1, 9)); parser.add_argument("--provider", default="local", choices=["local", "gemini"]); parser.add_argument("--model"); parser.add_argument("--version", action="store_true"); args = parser.parse_args()
    if args.version: print("kspr 0.1.0"); return
    if not args.source: parser.print_help(); return
    request = AnalysisRequest(project_name=args.source.name, files=collect(args.source), provider=args.provider, model=args.model, iterations=args.iterations, mode="direct")
    result = asyncio.run(analyze(request, settings)); args.output.mkdir(parents=True, exist_ok=True)
    for artifact in result.artifacts:
        target = args.output / artifact.path; target.parent.mkdir(parents=True, exist_ok=True); target.write_text(artifact.content, encoding="utf-8")
    print(f"KSPR completó el análisis: {len(request.files)} archivos -> {args.output.resolve()}")


if __name__ == "__main__": main()
