# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_submodules

hiddenimports = []
hiddenimports += collect_submodules('pypdf')


a = Analysis(
    ['/home/devd/DREI/PROYECTOS/Webs/KSPR AI APP DESKTOP/kspr_runtime.py'],
    pathex=[],
    binaries=[],
    datas=[('/home/devd/DREI/PROYECTOS/Webs/KSPR AI APP DESKTOP/skills', 'skills'), ('/home/devd/DREI/PROYECTOS/Webs/KSPR AI APP DESKTOP/backend/kspr_engine/licenses.json', 'kspr_engine')],
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='kspr-runtime',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)
