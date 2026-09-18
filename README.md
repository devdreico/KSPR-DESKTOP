# KSPR Desktop

Interfaz gráfica de escritorio para KSPR CLI, construida con Qt 6 y C++.

## Características

- **Chat con IA**: Interactúa con KSPR I directamente desde la interfaz
- **Gestión de archivos**: Sube y gestiona archivos para análisis
- **Dashboard**: Visualiza métricas y estado del sistema
- **Auto-instalación**: Detecta e instala KSPR CLI automáticamente

## Requisitos

- Qt 6.x
- CMake 3.16+
- C++17
- Python 3.11+ (para KSPR CLI)

## Compilación

```bash
# Crear directorio de build
mkdir build && cd build

# Configurar con CMake
cmake ..

# Compilar
make -j$(nproc)
```

## Ejecución

```bash
./build/src/ksrp-desktop
```

## Instalación de KSPR CLI

La aplicación detectará si KSPR CLI está instalado. Si no lo está, ofrecerá instalarlo automáticamente ejecutando:

```bash
curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash
```

## Estructura del Proyecto

```
KSRP-DESKTOP/
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
├── resources/
│   ├── resources.qrc
│   ├── desktop-app-icon.png
│   └── kspr-main-logo.png
└── build/
```

## Licencia

MIT
