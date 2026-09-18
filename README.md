# KSPR Desktop

Interfaz grafica de escritorio para KSPR CLI, construida con Qt 6, C++ y React.

## Caracteristicas

- **Chat con IA**: Interactua con KSPR I directamente desde la interfaz
- **Gestion de archivos**: Sube y gestiona archivos para analisis
- **Dashboard**: Visualiza metricas y estado del sistema
- **Auto-instalacion**: Detecta e instala KSPR CLI automaticamente

## Requisitos

- Qt 6.x (para version de escritorio)
- CMake 3.16+
- C++17
- Python 3.11+ (para KSPR CLI)
- Node.js 18+ (para version web)

## Compilacion

### Version de Escritorio (Qt6)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Version Web (React)
```bash
cd web-ui
npm install
npm run build
```

## Instalacion

### Paquete .deb (Ubuntu/Debian)
```bash
sudo dpkg -i kspr-desktop_1.0.0_amd64.deb
sudo apt-get install -f
```

### Ejecucion
```bash
kspr-desktop
```

## Instalacion de KSPR CLI

La aplicacion detectara si KSPR CLI esta instalado. Si no lo esta, ofrecera instalarlo automaticamente ejecutando:

```bash
curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash
```

## Estructura del Proyecto

```
KSPR-DESKTOP/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── mainwindow.h/cpp
│   ├── processmanager.h/cpp
│   ├── chatpanel.h/cpp
│   ├── filepanel.h/cpp
│   ├── dashboardpanel.h/cpp
│   ├── installer.h/cpp
│   └── styles.h/cpp
├── web-ui/
│   ├── src/
│   │   ├── components/
│   │   └── store/
│   └── dist/
├── resources/
│   ├── resources.qrc
│   ├── desktop-app-icon.png
│   └── kspr-main-logo.png
└── deb-package/
```

## Licencia

MIT
