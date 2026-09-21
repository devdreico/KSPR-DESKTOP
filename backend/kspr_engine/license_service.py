"""Shared license validation for the KSPR CLI, Desktop and Docker modes."""
from __future__ import annotations

import json
import os
from pathlib import Path


def license_paths() -> list[Path]:
    configured = os.getenv("KSPR_LICENSE_FILE")
    paths = [Path(configured).expanduser()] if configured else []
    paths.extend([
        Path(__file__).with_name("licenses.json"),
        Path.home() / ".kspr" / "licenses.json",
    ])
    return paths


def load_license_codes() -> dict[str, dict]:
    for path in license_paths():
        try:
            if path.is_file():
                data = json.loads(path.read_text(encoding="utf-8"))
                codes = data.get("codes", {})
                if isinstance(codes, dict):
                    return codes
        except (OSError, ValueError, TypeError):
            continue
    return {}


def normalize_code(code: str) -> str:
    return "-".join(str(code).strip().upper().split())


def verify_license(code: str) -> bool:
    normalized = normalize_code(code)
    entry = load_license_codes().get(normalized)
    return isinstance(entry, dict) and entry.get("status", "active") == "active"


def license_count() -> int:
    return len(load_license_codes())
