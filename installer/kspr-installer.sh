#!/bin/bash
# KSPR Desktop - Main Installer Script
# This script downloads and installs the full application

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color
BOLD='\033[1m'

# Configuration
APP_NAME="KSPR Desktop"
APP_VERSION="1.0.0"
INSTALL_DIR="/opt/kspr-desktop"
CONFIG_DIR="$HOME/.kspr"
GITHUB_REPO="devdreiortiz/KSPR-DESKTOP"
GITHUB_API="https://api.github.com/repos/$GITHUB_REPO/releases/latest"
TEMP_DIR="/tmp/kspr-install"

# Create directories
mkdir -p "$INSTALL_DIR/bin"
mkdir -p "$INSTALL_DIR/web-ui"
mkdir -p "$CONFIG_DIR"

# Function to print banner
print_banner() {
    clear
    echo -e "${MAGENTA}"
    echo "  ╔══════════════════════════════════════════════════════════╗"
    echo "  ║                                                          ║"
    echo "  ║     ███╗   ███╗███████╗███████╗ █████╗ ██████╗          ║"
    echo "  ║     ████╗ ████║██╔════╝██╔════╝██╔══██╗██╔══██╗         ║"
    echo "  ║     ██╔████╔██║█████╗  ███████╗███████║██████╔╝         ║"
    echo "  ║     ██║╚██╔╝██║██╔══╝  ╚════██║██╔══██║██╔══██╗         ║"
    echo "  ║     ██║ ╚═╝ ██║███████╗███████║██║  ██║██║  ██║         ║"
    echo "  ║     ╚═╝     ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝         ║"
    echo "  ║                                                          ║"
    echo "  ║          Desktop - Asistente de IA para Codigo           ║"
    echo "  ║                                                          ║"
    echo "  ╚══════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    echo ""
}

# Function to print status
print_status() {
    echo -e "${BLUE}[*]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Function to check dependencies
check_dependencies() {
    print_status "Verificando dependencias..."
    
    local missing=0
    
    for cmd in curl wget dpkg; do
        if ! command -v $cmd &> /dev/null; then
            print_error "Falta: $cmd"
            missing=1
        fi
    done
    
    if [ $missing -eq 1 ]; then
        print_warning "Instalando dependencias faltantes..."
        sudo apt-get update -qq
        sudo apt-get install -y -qq curl wget dpkg 2>/dev/null
    fi
    
    print_success "Dependencias verificadas"
}

# Function to download file with progress
download_file() {
    local url=$1
    local output=$2
    local description=$3
    
    print_status "Descargando $description..."
    
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "$output" "$url" 2>&1
    else
        curl -L --progress-bar -o "$output" "$url"
    fi
    
    if [ $? -eq 0 ]; then
        print_success "$description descargado"
        return 0
    else
        print_error "Error descargando $description"
        return 1
    fi
}

# Function to get latest release info from GitHub
get_latest_release() {
    local api_url=$1
    local response=$(curl -s "$api_url" 2>/dev/null)
    
    if [ -z "$response" ]; then
        echo ""
        return 1
    fi
    
    # Extract download URL for .deb package
    local deb_url=$(echo "$response" | grep -o '"browser_download_url": "[^"]*\.deb"' | head -1 | cut -d'"' -f4)
    
    if [ -z "$deb_url" ]; then
        # Try to get tag name for manual URL construction
        local tag=$(echo "$response" | grep -o '"tag_name": "[^"]*"' | head -1 | cut -d'"' -f4)
        if [ -n "$tag" ]; then
            deb_url="https://github.com/$GITHUB_REPO/releases/download/$tag/ksrp-desktop_${tag}_amd64.deb"
        fi
    fi
    
    echo "$deb_url"
}

# Function to download application assets
download_assets() {
    print_status "Descargando activos de la aplicacion..."
    
    mkdir -p "$TEMP_DIR"
    
    # Download Qt binary
    local binary_url="https://github.com/$GITHUB_REPO/releases/download/v${APP_VERSION}/kspr-desktop"
    download_file "$binary_url" "$TEMP_DIR/kspr-desktop" "binario Qt" || true
    
    # Download web UI
    local webui_url="https://github.com/$GITHUB_REPO/releases/download/v${APP_VERSION}/web-ui.tar.gz"
    download_file "$webui_url" "$TEMP_DIR/web-ui.tar.gz" "interfaz web" || true
    
    # Download KSPR CLI
    print_status "Descargando KSPR CLI..."
    curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash 2>/dev/null || true
    
    return 0
}

# Function to install application
install_application() {
    print_status "Instalando $APP_NAME..."
    
    # Copy binary if downloaded
    if [ -f "$TEMP_DIR/kspr-desktop" ]; then
        cp "$TEMP_DIR/kspr-desktop" "$INSTALL_DIR/bin/"
        chmod +x "$INSTALL_DIR/bin/kspr-desktop"
        print_success "Binario Qt instalado"
    fi
    
    # Extract web UI if downloaded
    if [ -f "$TEMP_DIR/web-ui.tar.gz" ]; then
        tar -xzf "$TEMP_DIR/web-ui.tar.gz" -C "$INSTALL_DIR/web-ui/" 2>/dev/null || true
        print_success "Interfaz web instalada"
    fi
    
    # Copy launcher script
    cat > "$INSTALL_DIR/kspr-launcher.sh" << 'LAUNCHER_EOF'
#!/bin/bash
# KSPR Desktop Launcher

APP_DIR="/opt/kspr-desktop"
WEB_DIR="$APP_DIR/web-ui"
CLI_PATH="$APP_DIR/bin/kspr"
CONFIG_DIR="$HOME/.kspr"

# Create config directory
mkdir -p "$CONFIG_DIR"

# Check if first run
if [ ! -f "$CONFIG_DIR/.installed" ]; then
    echo "Primera ejecucion – configurando KSPR Desktop..."
    
    # Install KSPR CLI if not present
    if [ ! -f "$CLI_PATH" ]; then
        echo "Instalando KSPR CLI..."
        curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash
    fi
    
    # Mark as installed
    touch "$CONFIG_DIR/.installed"
fi

# Start KSPR CLI if binary exists
if [ -f "$CLI_PATH" ]; then
    echo "Iniciando KSPR CLI..."
    "$CLI_PATH" serve --host 127.0.0.1 --port 8000 &
    CLI_PID=$!
    sleep 2
fi

# Launch application
if [ -f "$APP_DIR/bin/kspr-desktop" ]; then
    echo "Iniciando KSPR Desktop..."
    "$APP_DIR/bin/kspr-desktop" &
else
    echo "Iniciando interfaz web..."
    if command -v python3 &> /dev/null; then
        cd "$WEB_DIR"
        python3 -m http.server 3000 &
        echo ""
        echo "  KSPR Desktop disponible en: http://localhost:3000"
        echo ""
    elif command -v npx &> /dev/null; then
        cd "$WEB_DIR"
        npx serve -l 3000 &
        echo ""
        echo "  KSPR Desktop disponible en: http://localhost:3000"
        echo ""
    else
        echo "Error: No se encontro Python o Node.js"
        exit 1
    fi
fi

# Handle cleanup on exit
cleanup() {
    echo ""
    echo "Cerrando KSPR Desktop..."
    if [ -n "$CLI_PID" ]; then
        kill $CLI_PID 2>/dev/null
    fi
    exit 0
}

trap cleanup SIGINT SIGTERM

echo ""
echo "  Presiona Ctrl+C para cerrar"
echo ""
wait
LAUNCHER_EOF
    chmod +x "$INSTALL_DIR/kspr-launcher.sh"
    print_success "Script de lanzamiento instalado"
    
    # Create desktop entry
    cat > /usr/share/applications/kspr-desktop.desktop << 'DESKTOP_EOF'
[Desktop Entry]
Name=KSPR Desktop
Comment=AI Code Analysis Assistant
Exec=/opt/kspr-desktop/kspr-launcher.sh
Icon=kspr-desktop
Terminal=false
Type=Application
Categories=Development;IDE;
Keywords=kspr;ai;code;analysis;
StartupWMClass=kspr-desktop
DESKTOP_EOF
    print_success "Entrada de escritorio creada"
    
    # Create command line symlink
    ln -sf "$INSTALL_DIR/kspr-launcher.sh" /usr/local/bin/kspr-desktop
    print_success "Comando 'kspr-desktop' disponible"
    
    # Update desktop database
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database /usr/share/applications 2>/dev/null || true
    fi
    
    # Update icon cache
    if command -v gtk-update-icon-cache &> /dev/null; then
        gtk-update-icon-cache /usr/share/icons/hicolor 2>/dev/null || true
    fi
    
    print_success "$APP_NAME instalado correctamente"
}

# Function to cleanup temp files
cleanup_temp() {
    if [ -d "$TEMP_DIR" ]; then
        rm -rf "$TEMP_DIR"
    fi
}

# Function to show installation summary
show_summary() {
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                                                          ║${NC}"
    echo -e "${GREEN}║         Instalacion completada exitosamente              ║${NC}"
    echo -e "${GREEN}║                                                          ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "  ${BOLD}Para iniciar:${NC}"
    echo -e "    Terminal:     ${CYAN}kspr-desktop${NC}"
    echo -e "    Aplicacion:   Busca ${CYAN}KSPR Desktop${NC} en el menu"
    echo ""
    echo -e "  ${BOLD}Ubicacion:${NC}"
    echo -e "    Aplicacion:   ${CYAN}$INSTALL_DIR${NC}"
    echo -e "    Configuracion:${CYAN}$CONFIG_DIR${NC}"
    echo ""
    echo -e "  ${BOLD}Desinstalar:${NC}"
    echo -e "    ${CYAN}sudo dpkg -r ksrp-desktop${NC}"
    echo ""
}

# Main installation flow
main() {
    print_banner
    
    echo -e "${BOLD}Instalador de $APP_NAME v$APP_VERSION${NC}"
    echo ""
    
    # Check if running as root for installation
    if [ "$EUID" -eq 0 ]; then
        print_warning "Ejecutando como root (instalacion global)"
    else
        print_status "Se necesitan permisos de administrador para instalar"
        echo ""
    fi
    
    # Check dependencies
    check_dependencies
    echo ""
    
    # Download assets
    download_assets
    echo ""
    
    # Install application
    install_application
    echo ""
    
    # Cleanup
    cleanup_temp
    
    # Show summary
    show_summary
}

# Run main function
main "$@"
