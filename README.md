# KSPR Desktop

KSPR convierte repositorios complejos en mapas técnicos, contexto auditable y documentación accionable. Este proyecto contiene el Studio desktop con Tauri y la landing pública desplegable en Vercel.

## Publicar la página

En Vercel importa este repositorio. La configuración ya está incluida en `vercel.json`:

- Build command: `npm run build`
- Output directory: `dist`
- Framework: Vite

En navegador se muestra la página de descargas; dentro de Tauri se inicia el Studio.

## Publicar instaladores

El workflow de GitHub Actions construye Linux x64 y Windows x64. Para crear la primera release:

```bash
git add .
git commit -m "chore: prepare KSPR desktop release"
git push origin main
git tag v0.1.0-rc.1
git push origin v0.1.0-rc.1
```

La release generará estos nombres estables, usados por la landing:

- `kspr-linux-x64.deb`
- `kspr-windows-x64.exe`
- `SHA256SUMS`

Los enlaces de descarga apuntan a `devdreiortiz/KSPR/releases/latest`. Si el repositorio se publica bajo otra cuenta, actualiza las URLs en `src/pages/Landing.tsx`.

## Desarrollo

```bash
npm install
npm run dev
```

Para el desktop se requiere Rust, las dependencias de Tauri y el sidecar Python generado por PyInstaller.
