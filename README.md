# KSPR Inverse Engineering Web

KSPR Inverse Engineering Web es una aplicación web para descomponer y reconstruir sistemas digitales, físicos, procesos y conceptos a partir de evidencias y nodos de contexto trazables.

La interfaz cubre el flujo de la CLI: carpetas, ZIP y `--git-url`; proveedores Gemini, OpenAI, Groq, DeepSeek, Anthropic, OpenRouter, OpenCode Zen y gateways compatibles; streaming y jobs; sesiones persistentes; MCP; plugins; skills; capacidades CLI-Anything; prompts; decompilación multimodal y Context Trees.

## Ejecutar localmente

En Vercel importa este repositorio. La configuración ya está incluida en `vercel.json`:

- Build command: `npm run build`
- Output directory: `dist`
- Framework: Vite

El navegador y Docker abren la interfaz web completa.

Para Docker local, copia `.env.example` a `.env`, define un token aleatorio y crea el frontend con `VITE_API_TOKEN` si vas a servirlo desde un origen separado. No expongas el puerto directamente a Internet sin TLS y un proxy de autenticación.

```bash
cp .env.example .env
docker compose up --build
```

## Desarrollo

```bash
npm install
npm run dev
```

```bash
python -m pip install -e .
uvicorn kspr_runtime:app --host 127.0.0.1 --port 8000
```

En otra terminal:

```bash
npm ci
npm run dev
```
