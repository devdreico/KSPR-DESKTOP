"""Plugin discovery compatible with ~/.kspr/plugins used by the CLI."""

from __future__ import annotations

import importlib.util
import json
import sys
from pathlib import Path
from typing import Any

from .models import MCPTool, PluginInfo


class PluginManager:
    MANIFEST_FILE = "kspr_plugin.json"

    def __init__(self, plugins_dir: Path | None = None):
        self.plugins_dir = plugins_dir or (Path.home() / ".kspr" / "plugins")
        self.plugins_dir.mkdir(parents=True, exist_ok=True)
        self.loaded: dict[str, Any] = {}
        self.meta: dict[str, PluginInfo] = {}

    def scan_plugins(self) -> list[PluginInfo]:
        result: list[PluginInfo] = []
        for folder in sorted(self.plugins_dir.iterdir()):
            if not folder.is_dir():
                continue
            manifest = folder / self.MANIFEST_FILE
            data: dict[str, Any] = {}
            if manifest.is_file():
                try: data = json.loads(manifest.read_text(encoding="utf-8"))
                except Exception: continue
            py = folder / "__init__.py"
            if not py.is_file(): py = folder / f"{folder.name}.py"
            if not py.is_file() and not data: continue
            result.append(PluginInfo(name=data.get("name", folder.name), dir_name=folder.name, version=data.get("version", "0.1.0"), description=data.get("description", ""), enabled=data.get("enabled", True), tools=[MCPTool(name=item.get("name", ""), description=item.get("description", ""), input_schema=item.get("parameters", {}), server=f"plugin:{data.get('name', folder.name)}") for item in data.get("tools", [])]))
        return result

    def load_all(self) -> int:
        count = 0
        for info in self.scan_plugins():
            if not info.enabled: continue
            py = self.plugins_dir / info.dir_name / "__init__.py"
            if not py.is_file(): py = self.plugins_dir / info.dir_name / f"{info.dir_name}.py"
            try:
                spec = importlib.util.spec_from_file_location(f"kspr_plugin_{info.dir_name}", py)
                if spec and spec.loader:
                    module = importlib.util.module_from_spec(spec)
                    sys.modules[spec.name] = module
                    spec.loader.exec_module(module)
                    self.loaded[info.name] = module
                    self.meta[info.name] = info
                    count += 1
            except Exception:
                continue
        return count

    def get_tool_schemas(self) -> list[dict[str, Any]]:
        return [{"type": "function", "function": {"name": tool.name, "description": tool.description, "parameters": tool.input_schema or {"type": "object", "properties": {}}}} for info in self.meta.values() for tool in info.tools]

    def execute_tool(self, name: str, arguments: dict[str, Any]) -> str | None:
        for module in self.loaded.values():
            executor = getattr(module, "execute_tool", None)
            if callable(executor):
                try: return str(executor(name, arguments))
                except Exception as exc: return f"Plugin error: {exc}"
        return None

    def list_loaded(self) -> list[dict[str, Any]]:
        return [{"name": info.name, "version": info.version, "description": info.description, "tools_count": len(info.tools), "loaded": info.name in self.loaded} for info in self.meta.values()]
