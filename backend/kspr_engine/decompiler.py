"""Multi-modal ingestion and Context Trees generation used by the CLI."""

from __future__ import annotations

import re
import os
from pathlib import Path
from typing import Any

import httpx

CONTEXT_TREES_DIR = Path(os.getenv("KSPR_DATA_DIR", str(Path.home() / ".kspr"))) / "Context Trees"


class DecompilerEngine:
    def __init__(self, staging_dir: Path | None = None) -> None:
        self.staging_dir = staging_dir or (Path(os.getenv("KSPR_DATA_DIR", str(Path.home() / ".kspr"))) / "decompile_staging")
        self.staging_dir.mkdir(parents=True, exist_ok=True)

    def ingest_source(self, source_path_or_url: str) -> dict[str, Any]:
        source = source_path_or_url.strip()
        return self._ingest_url(source) if source.startswith(("http://", "https://")) else self._ingest_file(Path(source))

    def _ingest_file(self, path: Path) -> dict[str, Any]:
        if not path.is_file():
            return {"source": str(path), "success": False, "error": "File not found"}
        try:
            suffix = path.suffix.lower()
            if suffix == ".pdf":
                content = f"[PDF Document: {path.name}]\n"
                try:
                    import pypdf
                    reader = pypdf.PdfReader(str(path))
                    content += "\n".join((page.extract_text() or "") for page in reader.pages)
                except Exception:
                    content += path.read_bytes().hex()[:2000]
                kind = "pdf"
            elif suffix in {".jpg", ".jpeg", ".png", ".heif", ".webp", ".bmp"}:
                content = f"[Image Asset: {path.name}] Size: {path.stat().st_size} bytes."
                kind = "image"
            else:
                content = path.read_text(encoding="utf-8", errors="replace")[:2_000_000]
                kind = "text"
            return {"source": str(path), "name": path.name, "type": kind, "content": content, "success": True}
        except Exception as exc:
            return {"source": str(path), "success": False, "error": str(exc)}

    def _ingest_url(self, url: str) -> dict[str, Any]:
        try:
            response = httpx.get(url, timeout=15, follow_redirects=False, headers={"User-Agent": "KSPR/0.1"})
            response.raise_for_status()
            if len(response.content) > 5_000_000:
                return {"source": url, "success": False, "error": "Remote document exceeds 5 MB"}
            text = re.sub(r"<[^>]+>", " ", response.text)
            text = re.sub(r"\s+", " ", text).strip()
            return {"source": url, "name": url.rstrip("/").split("/")[-1] or "web-page", "type": "web_link", "content": f"[Web Link: {url}]\n\n{text[:10000]}", "success": True}
        except Exception as exc:
            return {"source": url, "success": False, "error": str(exc)}

    def generate_context_trees(self, sources: list[dict[str, Any]], response_text: str) -> Path:
        concept = "Analisis General"
        for source in sources:
            if source.get("success"):
                concept = Path(source.get("name", "analisis")).stem.replace("-", " ").title()
                break
        folder = CONTEXT_TREES_DIR / concept
        folder.mkdir(parents=True, exist_ok=True)
        (folder / "Contexto inicial.md").write_text(
            f"# Contexto Inicial — {concept}\n\nAnálisis recopilado de {len(sources)} fuentes indexadas.\n",
            encoding="utf-8",
        )
        sections = self._parse_markdown_sections(response_text) or {"detalles.md": "# Detalles\n\nNo se recibió respuesta estructurada."}
        for filename, content in sections.items():
            safe = Path(filename).name if filename.endswith(".md") else f"{Path(filename).name}.md"
            (folder / safe).write_text(content, encoding="utf-8")
        return folder

    @staticmethod
    def _parse_markdown_sections(text: str) -> dict[str, str]:
        sections: dict[str, str] = {}
        title = "detalles.md"
        lines: list[str] = []
        for line in text.splitlines():
            if line.startswith(("# ", "## ")):
                if lines:
                    sections[title] = "\n".join(lines)
                title = re.sub(r"[^a-z0-9-]+", "-", line.lstrip("# ").strip().lower()).strip("-") + ".md"
                lines = [line]
            else:
                lines.append(line)
        if lines:
            sections[title] = "\n".join(lines)
        return sections

    def list_trees(self) -> list[dict[str, Any]]:
        if not CONTEXT_TREES_DIR.is_dir():
            return []
        return [{"concept": item.name, "path": str(item.resolve()), "files": [file.name for file in item.glob("*.md")]} for item in CONTEXT_TREES_DIR.iterdir() if item.is_dir()]
