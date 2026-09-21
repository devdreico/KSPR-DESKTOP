import { cpSync, existsSync, mkdirSync, rmSync } from "node:fs";
import { join, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { platform } from "node:os";
import { spawnSync } from "node:child_process";

const root = resolve(fileURLToPath(new URL("..", import.meta.url)));
const isWindows = platform() === "win32";
const python = isWindows ? "python" : "python3";
const separator = isWindows ? ";" : ":";
const sidecarDir = join(root, "desktop", "src-tauri", "binaries");
const buildDir = join(root, ".build", "pyinstaller");
const licenseFile = join(root, "backend", "kspr_engine", "licenses.json");

function run(command, args) {
  const result = spawnSync(command, args, { cwd: root, stdio: "inherit", shell: isWindows });
  if (result.error) throw result.error;
  if (result.status !== 0) process.exit(result.status ?? 1);
}

if (!existsSync(licenseFile)) {
  throw new Error(`No se encontró el registro de licencias: ${licenseFile}`);
}

mkdirSync(sidecarDir, { recursive: true });
rmSync(buildDir, { recursive: true, force: true });

const rust = spawnSync("rustc", ["-vV"], { cwd: root, encoding: "utf8", shell: isWindows });
if (rust.status !== 0) throw new Error("Rust no está instalado o no está disponible en PATH.");
const target = rust.stdout.split(/\r?\n/).find((line) => line.startsWith("host:"))?.split(/\s+/)[1];
if (!target) throw new Error("No se pudo detectar el target de Rust.");

run(python, ["-m", "PyInstaller", "--noconfirm", "--clean", "--onefile", "--name", "kspr-runtime",
  "--distpath", join(buildDir, "dist"), "--workpath", join(buildDir, "work"), "--specpath", join(buildDir, "spec"),
  "--add-data", `${join(root, "skills")}${separator}skills`,
  "--add-data", `${licenseFile}${separator}kspr_engine`,
  "--collect-submodules", "pypdf", join(root, "kspr_runtime.py")]);

const extension = isWindows ? ".exe" : "";
const output = join(buildDir, "dist", `kspr-runtime${extension}`);
if (!existsSync(output)) throw new Error(`PyInstaller no produjo el sidecar esperado: ${output}`);
const targetOutput = join(sidecarDir, `kspr-runtime-${target}${extension}`);
cpSync(output, targetOutput);
console.log(`Sidecar generado: ${targetOutput}`);
