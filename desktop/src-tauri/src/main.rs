#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use keyring::Entry;
use rusqlite::{params, Connection};
use serde::{Deserialize, Serialize};
use std::{collections::HashMap, sync::Mutex};
use tauri::{Emitter, Manager, State};
use tauri_plugin_shell::{process::CommandEvent, ShellExt};

const SERVICE: &str = "online.presentto.kspr";

#[derive(Default)]
struct RuntimeState(Mutex<Option<RuntimeConfig>>);

#[derive(Default)]
struct AppState(Mutex<Option<Connection>>);

#[derive(Clone, Debug, Serialize, Deserialize)]
struct RuntimeConfig { host: String, port: u16, token: String }

#[tauri::command]
fn runtime_config(state: State<'_, RuntimeState>) -> Result<RuntimeConfig, String> {
    state.0.lock().map_err(|_| "No se pudo bloquear runtime")?.clone().ok_or_else(|| "El runtime todavía no está listo".into())
}

#[tauri::command]
fn storage_load(state: State<'_, AppState>) -> Result<HashMap<String, String>, String> {
    let guard = state.0.lock().map_err(|_| "No se pudo bloquear SQLite")?;
    let connection = guard.as_ref().ok_or_else(|| "SQLite no está inicializado".to_string())?;
    let mut statement = connection.prepare("SELECT key, value FROM kv") .map_err(|e| e.to_string())?;
    let rows = statement.query_map([], |row| Ok((row.get::<_, String>(0)?, row.get::<_, String>(1)?))).map_err(|e| e.to_string())?;
    rows.collect::<Result<HashMap<_, _>, _>>().map_err(|e| e.to_string())
}

#[tauri::command]
fn storage_set(state: State<'_, AppState>, key: String, value: String) -> Result<(), String> {
    let guard = state.0.lock().map_err(|_| "No se pudo bloquear SQLite")?;
    let connection = guard.as_ref().ok_or_else(|| "SQLite no está inicializado".to_string())?;
    connection.execute("INSERT INTO kv(key,value) VALUES(?1,?2) ON CONFLICT(key) DO UPDATE SET value=excluded.value", params![key, value]).map_err(|e| e.to_string())?;
    Ok(())
}

fn secret_entry(provider: &str) -> Result<Entry, String> { Entry::new(SERVICE, &format!("provider:{provider}")).map_err(|e| e.to_string()) }

#[tauri::command]
fn secret_set(provider: String, value: String) -> Result<(), String> { secret_entry(&provider)?.set_password(&value).map_err(|e| e.to_string()) }

#[tauri::command]
fn secret_get(provider: String) -> Result<Option<String>, String> {
    match secret_entry(&provider)?.get_password() { Ok(value) => Ok(Some(value)), Err(keyring::Error::NoEntry) => Ok(None), Err(error) => Err(error.to_string()) }
}

#[tauri::command]
fn secret_delete(provider: String) -> Result<(), String> { secret_entry(&provider)?.delete_credential().map_err(|e| e.to_string()) }

fn main() {
    tauri::Builder::default()
        .manage(RuntimeState::default())
        .invoke_handler(tauri::generate_handler![runtime_config, storage_load, storage_set, secret_set, secret_get, secret_delete])
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_shell::init())
        .setup(|app| {
            let data_dir = app.path().app_local_data_dir().map_err(|e| e.to_string())?;
            std::fs::create_dir_all(&data_dir).map_err(|e| e.to_string())?;
            let connection = Connection::open(data_dir.join("kspr.sqlite")).map_err(|e| e.to_string())?;
            connection.execute_batch("CREATE TABLE IF NOT EXISTS kv (key TEXT PRIMARY KEY, value TEXT NOT NULL);") .map_err(|e| e.to_string())?;
            app.manage(AppState(Mutex::new(Some(connection))));
            let handle = app.handle().clone();
            let (mut events, _child) = app.shell().sidecar("kspr-runtime").map_err(|e| e.to_string())?.args(["serve", "--host", "127.0.0.1", "--port", "0"]).spawn().map_err(|e| e.to_string())?;
            tauri::async_runtime::spawn(async move {
                while let Some(event) = events.recv().await {
                    if let CommandEvent::Stdout(line) = event {
                        let line = String::from_utf8_lossy(&line);
                        if let Ok(config) = serde_json::from_str::<RuntimeConfig>(line.trim()) {
                            if let Some(state) = handle.try_state::<RuntimeState>() { if let Ok(mut value) = state.0.lock() { *value = Some(config.clone()); } }
                            let _ = handle.emit("runtime-ready", config);
                        }
                    }
                }
            });
            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running KSPR Desktop");
}
