# KSPR Desktop

Tauri shell for the React Studio. The production build expects target-specific
`binaries/kspr-runtime-*` sidecars generated with PyInstaller.

The sidecar exposes the same analysis contract used by KSPR CLI. It starts on a
random localhost port with a per-process token, so the React UI can call the
local engine for analysis, ingestion, provider discovery, MCP inspection,
plugins, skills, decompilation and Context Trees.

The sidecar is generated automatically on Linux and Windows by the
cross-platform builder. It includes the KSPR license registry.

Development:

```bash
python -m pip install -e '.[desktop]'
npm run desktop:dev
```

Release installer:

```bash
npm run desktop:build
```

Linux outputs a `.deb` in `target/release/bundle/deb/`; Windows outputs an
NSIS `.exe` in `target/release/bundle/nsis/`. Linux requires the WebKitGTK
development packages supported by the current Tauri version.
