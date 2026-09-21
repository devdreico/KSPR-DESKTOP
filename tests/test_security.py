import socket

import pytest

from backend.kspr_engine.network import validate_public_http_url
from backend.kspr_engine.config import Settings


def test_rejects_private_ip():
    with pytest.raises(ValueError, match="privados"):
        validate_public_http_url("http://127.0.0.1:8000", allow_http=True)


def test_rejects_credentials():
    with pytest.raises(ValueError, match="credenciales"):
        validate_public_http_url("https://user:password@example.com")


def test_rejects_dns_resolving_to_private_address(monkeypatch):
    monkeypatch.setattr(socket, "getaddrinfo", lambda *args, **kwargs: [(socket.AF_INET, socket.SOCK_STREAM, 6, "", ("10.0.0.8", 443))])
    with pytest.raises(ValueError, match="privados"):
        validate_public_http_url("https://internal.example.com")


def test_accepts_public_url(monkeypatch):
    monkeypatch.setattr(socket, "getaddrinfo", lambda *args, **kwargs: [(socket.AF_INET, socket.SOCK_STREAM, 6, "", ("93.184.216.34", 443))])
    assert validate_public_http_url("https://example.com") == "https://example.com"


def test_production_requires_explicit_jwt_secret():
    with pytest.raises(ValueError, match="KSPR_JWT_SECRET_KEY"):
        Settings(environment="production")