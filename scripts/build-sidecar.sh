#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET_TRIPLE="$(rustc -vV | awk '/^host: / {print $2}')"
SIDECAR_DIR="$ROOT_DIR/desktop/src-tauri/binaries"
BUILD_DIR="${TMPDIR:-/tmp}/kspr-pyinstaller-build"

mkdir -p "$SIDECAR_DIR"
rm -rf "$BUILD_DIR"
python -m PyInstaller \
  --noconfirm \
  --clean \
  --onefile \
  --name kspr-runtime \
  --distpath "$BUILD_DIR/dist" \
  --workpath "$BUILD_DIR/work" \
  --specpath "$BUILD_DIR/spec" \
  --add-data "$ROOT_DIR/skills:skills" \
  --collect-submodules pypdf \
  "$ROOT_DIR/kspr_runtime.py"

cp "$BUILD_DIR/dist/kspr-runtime" "$SIDECAR_DIR/kspr-runtime-$TARGET_TRIPLE"
chmod +x "$SIDECAR_DIR/kspr-runtime-$TARGET_TRIPLE"
echo "Sidecar generado en $SIDECAR_DIR/kspr-runtime-$TARGET_TRIPLE"
