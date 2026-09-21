"""Universal inverse-engineering domain model and deterministic reconstruction pass.

The pass is deliberately provider-independent: it turns any describable subject
into a graph of evidence before an LLM is invited to interpret it. This keeps
KSPR useful for software, physical objects, processes and abstract concepts,
and makes every conclusion traceable to a source node.
"""
from __future__ import annotations

import hashlib
import re
from enum import StrEnum
from typing import Any

from pydantic import BaseModel, Field, field_validator


class InverseDomain(StrEnum):
    digital = "digital"
    physical = "physical"
    process = "process"
    conceptual = "conceptual"
    imaginary = "imaginary"
    mixed = "mixed"


class EvidenceKind(StrEnum):
    text = "text"
    source = "source"
    image = "image"
    document = "document"
    observation = "observation"
    constraint = "constraint"
    hypothesis = "hypothesis"


class InverseEvidence(BaseModel):
    id: str = Field(min_length=1, max_length=120)
    kind: EvidenceKind = EvidenceKind.text
    title: str = Field(min_length=1, max_length=240)
    content: str = Field(min_length=1, max_length=2_000_000)
    source_ref: str | None = Field(default=None, max_length=500)
    confidence: float = Field(default=1.0, ge=0, le=1)
    metadata: dict[str, Any] = Field(default_factory=dict)


class ReconstructionNode(BaseModel):
    id: str
    label: str
    node_type: str
    description: str = ""
    evidence_ids: list[str] = Field(default_factory=list)
    confidence: float = Field(default=0.5, ge=0, le=1)
    properties: dict[str, Any] = Field(default_factory=dict)


class ReconstructionRelation(BaseModel):
    id: str
    source: str
    target: str
    relation: str
    confidence: float = Field(default=0.5, ge=0, le=1)
    evidence_ids: list[str] = Field(default_factory=list)


class ReconstructionHypothesis(BaseModel):
    id: str
    statement: str
    confidence: float = Field(default=0.5, ge=0, le=1)
    supporting_evidence: list[str] = Field(default_factory=list)
    missing_evidence: list[str] = Field(default_factory=list)
    verification_steps: list[str] = Field(default_factory=list)


class InverseEngineeringRequest(BaseModel):
    subject: str = Field(min_length=1, max_length=500)
    domain: InverseDomain = InverseDomain.mixed
    objective: str = Field(default="Reconstruir la estructura, funcionamiento y dependencias del sujeto.", max_length=10_000)
    evidence: list[InverseEvidence] = Field(default_factory=list, max_length=2_000)
    depth: int = Field(default=3, ge=1, le=8)
    include_hypotheses: bool = True
    include_unknowns: bool = True

    @field_validator("evidence")
    @classmethod
    def require_evidence_or_objective(cls, value: list[InverseEvidence]) -> list[InverseEvidence]:
        if not value:
            raise ValueError("Se requiere al menos una evidencia para iniciar la reconstrucción")
        return value


class InverseEngineeringResult(BaseModel):
    reconstruction_id: str
    subject: str
    domain: InverseDomain
    status: str
    nodes: list[ReconstructionNode]
    relations: list[ReconstructionRelation]
    hypotheses: list[ReconstructionHypothesis]
    unknowns: list[str]
    reconstruction_plan: list[str]
    evidence_map: dict[str, list[str]]
    metrics: dict[str, int | float]


def _id(prefix: str, value: str) -> str:
    digest = hashlib.sha1(value.encode("utf-8", "ignore")).hexdigest()[:12]
    return f"{prefix}_{digest}"


def _add_node(nodes: dict[str, ReconstructionNode], label: str, node_type: str, description: str, evidence_id: str, confidence: float = 0.65, **properties: Any) -> str:
    node_id = _id(node_type, f"{label}:{evidence_id}")
    existing = nodes.get(node_id)
    if existing:
        if evidence_id not in existing.evidence_ids:
            existing.evidence_ids.append(evidence_id)
        existing.confidence = max(existing.confidence, confidence)
        return node_id
    nodes[node_id] = ReconstructionNode(id=node_id, label=label[:240], node_type=node_type, description=description[:2_000], evidence_ids=[evidence_id], confidence=confidence, properties=properties)
    return node_id


def reconstruct(request: InverseEngineeringRequest) -> InverseEngineeringResult:
    """Build a traceable first reconstruction from heterogeneous evidence."""
    nodes: dict[str, ReconstructionNode] = {}
    relations: dict[str, ReconstructionRelation] = {}
    hypotheses: list[ReconstructionHypothesis] = []
    unknowns: list[str] = []
    root_evidence = "evidence_subject"
    root_id = _add_node(nodes, request.subject, "subject", request.objective, root_evidence, 0.9, domain=request.domain.value)

    for evidence in request.evidence:
        evidence_node = _add_node(nodes, evidence.title, evidence.kind.value, evidence.content[:1_000], evidence.id, evidence.confidence, source_ref=evidence.source_ref)
        relation_id = _id("rel", f"{evidence_node}:supports:{root_id}")
        relations[relation_id] = ReconstructionRelation(id=relation_id, source=evidence_node, target=root_id, relation="supports", confidence=evidence.confidence, evidence_ids=[evidence.id])

        content = evidence.content
        # Headings and labelled statements are high-value structural signals.
        candidates = re.findall(r"^\s{0,3}(?:#{1,4}|[-*])\s+(.{3,180})$|^\s*([A-Za-zÁÉÍÓÚÜÑ][\w ÁÉÍÓÚÜÑáéíóúüñ/-]{2,80})\s*:\s*(.{3,240})$", content, flags=re.MULTILINE)
        for group in candidates[:80]:
            label = next((part.strip() for part in group if part and len(part.strip()) > 2), "")
            if not label or label.lower() == evidence.title.lower():
                continue
            child_type = "component" if evidence.kind in {EvidenceKind.source, EvidenceKind.document} else "stage"
            child_id = _add_node(nodes, label, child_type, f"Señal extraída de {evidence.title}.", evidence.id, max(0.45, evidence.confidence - 0.1))
            rid = _id("rel", f"{root_id}:contains:{child_id}")
            relations[rid] = ReconstructionRelation(id=rid, source=root_id, target=child_id, relation="contains", confidence=0.62, evidence_ids=[evidence.id])

        # Recognize common digital/process signals without pretending they are certainty.
        for match in re.findall(r"(?:https?://[^\s)]+|/[A-Za-z0-9_./-]+|\b(?:GET|POST|PUT|PATCH|DELETE)\s+/[^\s]+)", content, flags=re.I)[:40]:
            endpoint_id = _add_node(nodes, match, "interface", "Interfaz o punto de entrada detectado en la evidencia.", evidence.id, max(0.5, evidence.confidence - 0.05))
            rid = _id("rel", f"{root_id}:exposes:{endpoint_id}")
            relations[rid] = ReconstructionRelation(id=rid, source=root_id, target=endpoint_id, relation="exposes", confidence=0.7, evidence_ids=[evidence.id])

    if request.include_hypotheses:
        evidence_ids = [item.id for item in request.evidence]
        hypotheses.append(ReconstructionHypothesis(id=_id("hyp", request.subject), statement=f"{request.subject} puede modelarse como un sistema de nodos relacionados dentro del dominio {request.domain.value}.", confidence=0.58 if len(evidence_ids) < 3 else 0.72, supporting_evidence=evidence_ids[:8], missing_evidence=["Observaciones de comportamiento o una fuente independiente de validación."], verification_steps=["Comparar la hipótesis contra una segunda fuente.", "Validar las relaciones con una observación o ejecución controlada."]))

    if request.include_unknowns:
        unknowns.extend(["Propósito completo de los nodos sin evidencia operacional.", "Relaciones no observables entre componentes que solo aparecen en una fuente.", "Restricciones y casos límite todavía no descritos."])

    reconstruction_plan = [
        "Inventariar y clasificar todas las evidencias disponibles.",
        "Validar nodos críticos con una fuente independiente.",
        "Confirmar relaciones mediante observación, pruebas o simulación segura.",
        "Separar hechos observados, inferencias y preguntas abiertas.",
        "Generar una especificación reconstruida versionada y trazable.",
    ]
    return InverseEngineeringResult(
        reconstruction_id=_id("reconstruction", f"{request.subject}:{len(request.evidence)}"), subject=request.subject, domain=request.domain, status="completed", nodes=list(nodes.values()), relations=list(relations.values()), hypotheses=hypotheses, unknowns=unknowns, reconstruction_plan=reconstruction_plan, evidence_map={item.id: [node.id for node in nodes.values() if item.id in node.evidence_ids] for item in request.evidence}, metrics={"evidence": len(request.evidence), "nodes": len(nodes), "relations": len(relations), "hypotheses": len(hypotheses), "coverage": round(min(1.0, len(nodes) / max(4, len(request.evidence) * 3)), 3)},
    )
