"""Loads skill bundles available to the KSPR agent."""

from __future__ import annotations

import json
from pathlib import Path

SKILLS_DIR = Path(__file__).parent.parent.parent / "skills"


class SkillBundle:
    def __init__(self, name: str, description: str, version: str, skill_path: str, skill_content: str, enabled: bool = True, metadata: dict | None = None):
        self.name, self.description, self.version, self.skill_path, self.skill_content, self.enabled, self.metadata = name, description, version, skill_path, skill_content, enabled, metadata or {}

    def to_dict(self) -> dict:
        return {"name": self.name, "description": self.description, "version": self.version, "skill_path": self.skill_path, "skill_content": self.skill_content, "enabled": self.enabled, "metadata": self.metadata}


class SkillsManager:
    def __init__(self, skills_dir: str | None = None):
        self.skills_dir = Path(skills_dir) if skills_dir else SKILLS_DIR
        self.bundles: dict[str, SkillBundle] = {}
        self.reload()

    def reload(self) -> None:
        self.bundles.clear()
        manifests = [self.skills_dir / "manifest.json"] + list(self.skills_dir.glob("*/manifest.json"))
        for manifest in manifests:
            if not manifest.is_file(): continue
            try: data = json.loads(manifest.read_text(encoding="utf-8"))
            except Exception: continue
            skills = data.get("skills", {manifest.parent.name: data})
            for key, info in skills.items():
                path = manifest.parent / info.get("skill_path", "SKILL.md")
                if not path.is_file(): continue
                self.bundles[key] = SkillBundle(info.get("name", key), info.get("description", ""), info.get("version", "0.0.1"), str(path), path.read_text(encoding="utf-8"), True, info.get("metadata", {}))

    def list_bundles(self) -> list[SkillBundle]: return list(self.bundles.values())
    def get_bundle(self, name: str) -> SkillBundle | None: return self.bundles.get(name)
    def get_skill_context(self) -> str: return "\n".join(f"=== Skill: {bundle.name} ===\n{bundle.skill_content}\n=== End Skill: {bundle.name} ===" for bundle in self.bundles.values() if bundle.enabled)
    def enable(self, name: str) -> bool:
        if name not in self.bundles: return False
        self.bundles[name].enabled = True
        return True
    def disable(self, name: str) -> bool:
        if name not in self.bundles: return False
        self.bundles[name].enabled = False
        return True
