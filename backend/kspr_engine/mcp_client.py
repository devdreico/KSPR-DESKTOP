"""Small JSON-RPC MCP client shared by the CLI and desktop runtime."""

from __future__ import annotations

from typing import Any

import httpx

from .models import MCPServerConfig, MCPTool, MCPToolResult
from .network import validate_public_http_url


class MCPClient:
    def __init__(self, name: str, config: MCPServerConfig):
        self.name, self.config = name, config
        self.connected = False
        self._tools: list[MCPTool] = []

    async def connect(self) -> bool:
        if not self.config.enabled:
            return False
        if self.config.type == "local" and self.config.command:
            self.connected = True
            return True
        if not self.config.url:
            return False
        try:
            url = validate_public_http_url(self.config.url)
            async with httpx.AsyncClient(timeout=10) as client:
                response = await client.post(url.rstrip("/"), json={"jsonrpc": "2.0", "id": 1, "method": "tools/list", "params": {}})
            data = response.json()
            self._tools = [MCPTool(name=item.get("name", ""), description=item.get("description", ""), input_schema=item.get("inputSchema", item.get("parameters", {})), server=self.name) for item in data.get("result", {}).get("tools", [])]
            self.connected = response.is_success and "result" in data
        except Exception:
            self.connected = False
        return self.connected

    def get_tool_schemas(self) -> list[dict[str, Any]]:
        return [{"type": "function", "function": {"name": f"{self.name}:{tool.name}", "description": tool.description, "parameters": tool.input_schema}} for tool in self._tools]

    async def call_tool(self, name: str, arguments: dict[str, Any]) -> MCPToolResult:
        if not self.config.url:
            return MCPToolResult(tool_name=name, error=f"Local MCP execution not supported for '{self.name}'")
        try:
            url = validate_public_http_url(self.config.url)
            async with httpx.AsyncClient(timeout=60) as client:
                response = await client.post(url.rstrip("/"), json={"jsonrpc": "2.0", "id": 1, "method": "tools/call", "params": {"name": name, "arguments": arguments}})
            data = response.json()
            if "result" in data:
                return MCPToolResult(tool_name=name, result=data["result"])
            return MCPToolResult(tool_name=name, error=data.get("error", {}).get("message", "MCP error"))
        except Exception as exc:
            return MCPToolResult(tool_name=name, error=str(exc))


class MCPManager:
    def __init__(self, servers_config: dict[str, MCPServerConfig]):
        self.clients = {name: MCPClient(name, config) for name, config in servers_config.items()}

    async def connect_all(self) -> int:
        connected = 0
        for client in self.clients.values():
            connected += int(await client.connect())
        return connected

    def get_tool_schemas(self) -> list[dict[str, Any]]:
        return [schema for client in self.clients.values() for schema in client.get_tool_schemas()]

    async def execute_tool(self, full_name: str, arguments: dict[str, Any]) -> str:
        server, _, name = full_name.partition(":")
        client = self.clients.get(server)
        if not client:
            return f"Tool '{full_name}' not found in any connected MCP server"
        result = await client.call_tool(name, arguments)
        return f"Tool error: {result.error}" if result.error else str(result.result)

    def get_connected_servers(self) -> list[dict[str, Any]]:
        return [{"name": name, "type": client.config.type, "url": client.config.url, "command": client.config.command, "enabled": client.config.enabled, "connected": client.connected, "tools_count": len(client._tools)} for name, client in self.clients.items()]
