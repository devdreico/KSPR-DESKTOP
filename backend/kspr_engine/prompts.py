from typing import Any

MASTER_PROMPT_VERSION = "kspr-master-v4"

KSPR_I_SYSTEM_PROMPT = """# SYSTEM INITIALIZATION CORE - KSPR AI (v4.0)

You are KSPR I, an abstract systems engineer and static reverse-engineering specialist. Disassemble complexity, map data flows and maximize useful signal.

Responses must be direct technical plain text. Avoid greetings, filler and unsupported assumptions. Cite the supplied files and lines.
"""


def build_master_prompt(
    project_name: str,
    context: dict[str, Any],
    iteration: int,
    prior_review: str = "",
    instruction: str = "",
    personality_content: str = "",
) -> str:
    """Construye el prompt optimizado con la directiva estricta de KSPR I."""
    personality_block = f"\nBLOQUE DE PERSONALIDAD ACTIVA:\n{personality_content}\n" if personality_content else ""
    
    return f"""Eres KSPR I, motor hiper-eficiente de ingeniería inversa estática.
Directivas Absolutas de Eficiencia y Restricción:
1. Redacta en texto plano, con ortografía técnica seca, sin relleno ni caos de caracteres.
2. Elimina saludos, cortesías, preámbulos y cierres conversacionales.
3. Trabaja estrictamente con la evidencia estática proporcionada y cita archivos y líneas.

{personality_block}
PROYECTO: {project_name}
ITERACIÓN: {iteration}
REQUERIMIENTO:
{instruction or 'Extrae la arquitectura, endpoints, esquemas de datos y flujos críticos en texto plano sin asteriscos.'}

EVIDENCIA ESTÁTICA:
{context}

REVISIÓN PREVIA:
{prior_review or 'Ninguna.'}
"""


def output_schema() -> dict[str, Any]:
    return {
        "analisis_cinetico": {
            "pantalla_origen": "string",
            "flujos_detectados": [
                {"accion_usuario": "string", "funcion_interna_gatillada": "string", "impacto_en_estado": "string"}
            ],
        },
        "propuesta_endpoints_api": [
            {
                "ruta": "string", "metodo_http": "GET | POST | PUT | DELETE",
                "descripcion_funcional": "string", "requiere_parametros_body_o_query": {},
                "accion_equivalente_backend_viejo": "string",
            }
        ],
        "codigo_conector_sugerido": {"lenguaje": "string", "framework": "string", "archivo_destino": "string", "codigo_fuente_limpio": "string"},
    }
