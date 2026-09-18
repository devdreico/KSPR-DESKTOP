#!/bin/bash
# KSPR Desktop - Application Launcher
# This script handles launching the application properly

set -e

# Application paths
APP_DIR="/opt/kspr-desktop"
WEB_DIR="$APP_DIR/web-ui"
CLI_PATH="$APP_DIR/bin/kspr"
QT_APP="$APP_DIR/bin/kspr-desktop"
CONFIG_DIR="$HOME/.kspr"
LOG_DIR="$CONFIG_DIR/logs"
PID_FILE="$CONFIG_DIR/kspr.pid"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${BLUE}[*]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

# Create necessary directories
setup_directories() {
    mkdir -p "$CONFIG_DIR"
    mkdir -p "$LOG_DIR"
}

# Check if application is already running
check_running() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            return 0
        fi
    fi
    return 1
}

# Save PID of running process
save_pid() {
    echo $! > "$PID_FILE"
}

# Cleanup on exit
cleanup() {
    if [ -f "$PID_FILE" ]; then
        local pid=$(cat "$PID_FILE")
        if kill -0 "$pid" 2>/dev/null; then
            kill "$pid" 2>/dev/null || true
        fi
        rm -f "$PID_FILE"
    fi
}

# Install KSPR CLI if not present
install_cli() {
    if [ ! -f "$CLI_PATH" ]; then
        print_info "Instalando KSPR CLI..."
        curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash 2>&1 | tee -a "$LOG_DIR/install.log"
        
        if [ -f "$CLI_PATH" ]; then
            print_success "KSPR CLI instalado"
        else
            print_warning "No se pudo instalar KSPR CLI automaticamente"
            return 1
        fi
    fi
    return 0
}

# Start KSPR CLI server
start_cli() {
    if [ -f "$CLI_PATH" ]; then
        # Check if CLI is already running
        if pgrep -f "kspr serve" > /dev/null 2>&1; then
            print_info "KSPR CLI ya esta ejecutandose"
            return 0
        fi
        
        print_info "Iniciando KSPR CLI server..."
        "$CLI_PATH" serve --host 127.0.0.1 --port 8000 > "$LOG_DIR/cli.log" 2>&1 &
        local cli_pid=$!
        
        # Wait for server to start
        sleep 2
        
        if kill -0 "$cli_pid" 2>/dev/null; then
            print_success "KSPR CLI iniciado (PID: $cli_pid)"
            return 0
        else
            print_warning "KSPR CLI fallo al iniciar"
            return 1
        fi
    fi
    return 1
}

# Launch Qt desktop application
launch_qt_app() {
    if [ -f "$QT_APP" ]; then
        print_info "Iniciando KSPR Desktop (Qt)..."
        "$QT_APP" "$@" > "$LOG_DIR/app.log" 2>&1 &
        save_pid
        print_success "KSPR Desktop iniciado"
        return 0
    fi
    return 1
}

# Launch web interface
launch_web_app() {
    print_info "Iniciando interfaz web..."
    
    local port=3000
    local started=false
    
    # Try Python HTTP server
    if command -v python3 &> /dev/null; then
        cd "$WEB_DIR"
        python3 -m http.server $port > "$LOG_DIR/web.log" 2>&1 &
        save_pid
        started=true
        print_success "Servidor web iniciado en http://localhost:$port"
    # Try Node.js serve
    elif command -v npx &> /dev/null; then
        cd "$WEB_DIR"
        npx serve -l $port > "$LOG_DIR/web.log" 2>&1 &
        save_pid
        started=true
        print_success "Servidor web iniciado en http://localhost:$port"
    # Try Node.js http-server
    elif command -v http-server &> /dev/null; then
        cd "$WEB_DIR"
        http-server -p $port > "$LOG_DIR/web.log" 2>&1 &
        save_pid
        started=true
        print_success "Servidor web iniciado en http://localhost:$port"
    fi
    
    if [ "$started" = false ]; then
        print_error "No se encontro Python o Node.js para servir la interfaz web"
        print_error "Instala python3 o nodejs: sudo apt install python3 nodejs"
        return 1
    fi
    
    # Try to open browser
    if command -v xdg-open &> /dev/null; then
        sleep 2
        xdg-open "http://localhost:$port" 2>/dev/null || true
    elif command -v sensible-browser &> /dev/null; then
        sleep 2
        sensible-browser "http://localhost:$port" 2>/dev/null || true
    fi
    
    return 0
}

# Main launch function
main() {
    # Setup
    setup_directories
    
    # Check if already running
    if check_running; then
        print_warning "KSPR Desktop ya esta ejecutandose"
        print_info "Ventana existente enfocada"
        
        # Try to focus existing window
        if command -v wmctrl &> /dev/null; then
            wmctrl -a "KSPR Desktop" 2>/dev/null || true
        fi
        return 0
    fi
    
    # Install CLI if needed
    install_cli || true
    
    # Start CLI server
    start_cli || true
    
    # Try Qt app first, then web app
    if launch_qt_app "$@"; then
        return 0
    elif launch_web_app; then
        return 0
    else
        print_error "No se pudo iniciar KSPR Desktop"
        return 1
    fi
}

# Handle signals
trap cleanup EXIT INT TERM

# Run main function
main "$@"
