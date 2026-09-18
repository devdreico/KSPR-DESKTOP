#!/bin/bash
# KSPR Desktop Launcher Script

APP_DIR="/opt/ksrp-desktop"
WEB_DIR="$APP_DIR/web-ui"
CLI_PATH="$APP_DIR/bin/kspr"

# Check if KSPR CLI exists
if [ ! -f "$CLI_PATH" ]; then
    echo "KSPR CLI no encontrado. Instalando..."
    cd "$APP_DIR/bin"
    curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash
fi

# Start KSPR CLI in background
if [ -f "$CLI_PATH" ]; then
    echo "Iniciando KSPR CLI..."
    "$CLI_PATH" serve --host 127.0.0.1 --port 8000 &
    CLI_PID=$!
fi

# Launch Qt application or serve web UI
if [ -f "$APP_DIR/bin/ksrp-desktop" ]; then
    echo "Iniciando KSPR Desktop..."
    "$APP_DIR/bin/ksrp-desktop" &
else
    echo "Iniciando interfaz web..."
    if command -v python3 &> /dev/null; then
        cd "$WEB_DIR"
        python3 -m http.server 3000 &
        echo "Interfaz web disponible en http://localhost:3000"
    elif command -v npx &> /dev/null; then
        cd "$WEB_DIR"
        npx serve -l 3000 &
        echo "Interfaz web disponible en http://localhost:3000"
    else
        echo "Error: No se encontró Python o Node.js para servir la interfaz web"
        exit 1
    fi
fi

# Wait for user to close
echo "Presiona Ctrl+C para cerrar..."
wait $CLI_PID 2>/dev/null
