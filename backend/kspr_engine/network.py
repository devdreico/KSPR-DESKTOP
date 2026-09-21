from __future__ import annotations

import ipaddress
import socket
from urllib.parse import urlparse


def validate_public_http_url(value: str, *, allow_http: bool = False) -> str:
    parsed = urlparse(value.strip())
    allowed_schemes = {"https"} | ({"http"} if allow_http else set())
    if parsed.scheme not in allowed_schemes or not parsed.hostname or parsed.username or parsed.password:
        raise ValueError("La URL debe ser HTTP(S) pública y no incluir credenciales")
    try:
        addresses = {item[4][0] for item in socket.getaddrinfo(parsed.hostname, parsed.port or (443 if parsed.scheme == "https" else 80), type=socket.SOCK_STREAM)}
    except socket.gaierror as exc:
        raise ValueError("No se pudo resolver el host remoto") from exc
    for raw_address in addresses:
        address = ipaddress.ip_address(raw_address)
        if not address.is_global:
            raise ValueError("No se permiten hosts privados o reservados")
    return value.strip()