import { useState } from 'react'
import { Settings, Monitor, Globe, FolderOpen, Wifi, Bell, Save, RotateCcw } from 'lucide-react'
import { motion } from 'framer-motion'
import useStore from '../store/useStore'

export default function SettingsPanel() {
  const { settings, updateSettings } = useStore()
  const [localSettings, setLocalSettings] = useState({ ...settings })
  const [saved, setSaved] = useState(false)

  const handleChange = (key, value) => {
    setLocalSettings(prev => ({ ...prev, [key]: value }))
    setSaved(false)
  }

  const handleSave = () => {
    updateSettings(localSettings)
    setSaved(true)
    setTimeout(() => setSaved(false), 2000)
  }

  const handleReset = () => {
    const defaults = {
      theme: 'light',
      language: 'es',
      cliPath: './bin/kspr',
      port: 8000,
      autoStart: true,
      notifications: true
    }
    setLocalSettings(defaults)
    setSaved(false)
  }

  return (
    <div className="flex flex-col h-full overflow-y-auto">
      {/* Header */}
      <header className="glass-strong border-b border-border px-6 py-3 flex items-center justify-between sticky top-0 z-10">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 rounded-lg gradient-primary flex items-center justify-center">
            <Settings className="w-4 h-4 text-white" />
          </div>
          <div>
            <h2 className="font-semibold text-sm">Configuracion</h2>
            <span className="text-[10px] font-mono text-text-muted">
              Preferencias de la aplicacion
            </span>
          </div>
        </div>
      </header>

      {/* Content */}
      <div className="flex-1 p-6 max-w-2xl mx-auto w-full space-y-6">
        {/* General */}
        <Section title="General">
          <SettingRow
            icon={Monitor}
            label="Iniciar KSPR CLI automaticamente"
            description="Ejecutar el servidor al abrir la aplicacion"
          >
            <Toggle
              checked={localSettings.autoStart}
              onChange={(v) => handleChange('autoStart', v)}
            />
          </SettingRow>
          <SettingRow
            icon={Bell}
            label="Notificaciones"
            description="Mostrar notificaciones de eventos"
          >
            <Toggle
              checked={localSettings.notifications}
              onChange={(v) => handleChange('notifications', v)}
            />
          </SettingRow>
        </Section>

        {/* Appearance */}
        <Section title="Apariencia">
          <SettingRow
            icon={Monitor}
            label="Tema"
            description="Cambiar apariencia de la interfaz"
          >
            <select
              value={localSettings.theme}
              onChange={(e) => handleChange('theme', e.target.value)}
              className="glass rounded-lg px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-primary/30"
            >
              <option value="light">Claro</option>
              <option value="dark">Oscuro</option>
            </select>
          </SettingRow>
          <SettingRow
            icon={Globe}
            label="Idioma"
            description="Idioma de la interfaz"
          >
            <select
              value={localSettings.language}
              onChange={(e) => handleChange('language', e.target.value)}
              className="glass rounded-lg px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-primary/30"
            >
              <option value="es">Espanol</option>
              <option value="en">English</option>
            </select>
          </SettingRow>
        </Section>

        {/* Connection */}
        <Section title="Conexion">
          <SettingRow
            icon={FolderOpen}
            label="Ruta de KSPR CLI"
            description="Ubicacion del ejecutable"
          >
            <input
              type="text"
              value={localSettings.cliPath}
              onChange={(e) => handleChange('cliPath', e.target.value)}
              placeholder="./bin/kspr"
              className="glass rounded-lg px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-primary/30 w-48"
            />
          </SettingRow>
          <SettingRow
            icon={Wifi}
            label="Puerto del servidor"
            description="Puerto para la conexion con KSPR"
          >
            <input
              type="number"
              value={localSettings.port}
              onChange={(e) => handleChange('port', parseInt(e.target.value) || 8000)}
              min={1024}
              max={65535}
              className="glass rounded-lg px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-primary/30 w-24 text-center"
            />
          </SettingRow>
        </Section>

        {/* Actions */}
        <div className="flex items-center justify-end gap-3 pt-4">
          <button
            onClick={handleReset}
            className="flex items-center gap-2 px-4 py-2.5 rounded-xl text-sm text-text-muted border border-border hover:border-error hover:text-error transition-all"
          >
            <RotateCcw className="w-4 h-4" />
            Restablecer
          </button>
          <motion.button
            whileHover={{ scale: 1.02 }}
            whileTap={{ scale: 0.98 }}
            onClick={handleSave}
            className="gradient-primary text-white px-5 py-2.5 rounded-xl font-medium text-sm
              hover:gradient-primary-hover transition-all shadow-lg shadow-primary/20
              flex items-center gap-2"
          >
            <Save className="w-4 h-4" />
            {saved ? 'Guardado' : 'Aplicar'}
          </motion.button>
        </div>
      </div>
    </div>
  )
}

function Section({ title, children }) {
  return (
    <section>
      <h3 className="text-[10px] font-mono font-semibold text-primary tracking-widest uppercase mb-3">
        {title}
      </h3>
      <div className="glass rounded-xl divide-y divide-border overflow-hidden">
        {children}
      </div>
    </section>
  )
}

function SettingRow({ icon: Icon, label, description, children }) {
  return (
    <div className="flex items-center justify-between px-4 py-3.5">
      <div className="flex items-center gap-3">
        <Icon className="w-4 h-4 text-text-muted" />
        <div>
          <div className="text-sm font-medium">{label}</div>
          <div className="text-[11px] text-text-muted">{description}</div>
        </div>
      </div>
      {children}
    </div>
  )
}

function Toggle({ checked, onChange }) {
  return (
    <button
      onClick={() => onChange(!checked)}
      className={`
        w-10 h-6 rounded-full transition-colors relative
        ${checked ? 'bg-primary' : 'bg-border'}
      `}
    >
      <span
        className={`
          absolute top-1 w-4 h-4 rounded-full bg-white shadow transition-transform
          ${checked ? 'translate-x-5' : 'translate-x-1'}
        `}
      />
    </button>
  )
}
