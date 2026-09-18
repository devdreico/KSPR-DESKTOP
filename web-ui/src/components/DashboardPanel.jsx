import { useEffect } from 'react'
import { BarChart3, FileText, Coins, Clock, Activity, Terminal } from 'lucide-react'
import { motion } from 'framer-motion'
import useStore from '../store/useStore'

const metricCards = [
  { id: 'messages', label: 'Mensajes', icon: BarChart3, hint: 'Sesion actual', color: 'text-primary' },
  { id: 'files', label: 'Archivos', icon: FileText, hint: 'En contexto activo', color: 'text-success' },
  { id: 'tokens', label: 'Tokens', icon: Coins, hint: 'Consumidos', color: 'text-warning' },
  { id: 'latency', label: 'Latencia', icon: Clock, hint: 'Promedio', color: 'text-primary-light' },
  { id: 'uptime', label: 'Uptime', icon: Activity, hint: 'Tiempo activo', color: 'text-success' },
]

export default function DashboardPanel() {
  const { metrics, logs, connected } = useStore()

  const formatUptime = (seconds) => {
    const mins = Math.floor(seconds / 60)
    const secs = seconds % 60
    return `${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`
  }

  const formatMetric = (id, value) => {
    switch (id) {
      case 'latency': return `${value}ms`
      case 'uptime': return formatUptime(value)
      default: return value.toLocaleString()
    }
  }

  return (
    <div className="flex flex-col h-full overflow-y-auto">
      {/* Header */}
      <header className="glass-strong border-b border-border px-6 py-3 flex items-center justify-between sticky top-0 z-10">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 rounded-lg gradient-primary flex items-center justify-center">
            <BarChart3 className="w-4 h-4 text-white" />
          </div>
          <div>
            <h2 className="font-semibold text-sm">Dashboard</h2>
            <span className="text-[10px] font-mono text-text-muted">
              Metricas del sistema
            </span>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <span className={`w-2 h-2 rounded-full ${connected ? 'bg-success' : 'bg-error'}`} />
          <span className="text-xs font-mono text-text-muted">
            {connected ? 'En linea' : 'Fuera de linea'}
          </span>
        </div>
      </header>

      {/* Content */}
      <div className="flex-1 p-6 space-y-6">
        {/* Metrics Grid */}
        <section>
          <h3 className="text-[10px] font-mono font-semibold text-text-muted tracking-widest uppercase mb-3">
            Metricas
          </h3>
          <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-5 gap-4">
            {metricCards.map((card, i) => (
              <MetricCard
                key={card.id}
                card={card}
                value={formatMetric(card.id, metrics[card.id])}
                index={i}
              />
            ))}
          </div>
        </section>

        {/* Progress */}
        <section>
          <h3 className="text-[10px] font-mono font-semibold text-text-muted tracking-widest uppercase mb-3">
            Progreso del Analisis
          </h3>
          <div className="glass rounded-xl p-4">
            <div className="flex items-center justify-between mb-2">
              <span className="text-sm text-text-muted">Analisis en curso</span>
              <span className="text-sm font-mono text-primary font-medium">0%</span>
            </div>
            <div className="w-full h-2 bg-border rounded-full overflow-hidden">
              <motion.div
                initial={{ width: 0 }}
                animate={{ width: '0%' }}
                className="h-full gradient-primary rounded-full"
              />
            </div>
          </div>
        </section>

        {/* Logs */}
        <section>
          <h3 className="text-[10px] font-mono font-semibold text-text-muted tracking-widest uppercase mb-3">
            Logs Recientes
          </h3>
          <div className="glass rounded-xl overflow-hidden">
            <div className="p-4 max-h-[300px] overflow-y-auto font-mono text-xs space-y-1">
              {logs.length === 0 ? (
                <div className="text-center py-8 text-text-muted">
                  <Terminal className="w-8 h-8 mx-auto mb-2 opacity-30" />
                  <p>Sin logs recientes</p>
                </div>
              ) : (
                logs.map((log) => (
                  <LogEntry key={log.id} log={log} />
                ))
              )}
            </div>
          </div>
        </section>
      </div>
    </div>
  )
}

function MetricCard({ card, value, index }) {
  const Icon = card.icon

  return (
    <motion.div
      initial={{ opacity: 0, y: 20 }}
      animate={{ opacity: 1, y: 0 }}
      transition={{ delay: index * 0.05 }}
      className="glass rounded-xl p-4 hover:border-primary/30 transition-colors group"
    >
      <div className="flex items-center justify-between mb-3">
        <span className="text-[10px] font-mono text-text-muted uppercase tracking-wide">
          {card.label}
        </span>
        <Icon className={`w-4 h-4 ${card.color} opacity-60 group-hover:opacity-100 transition-opacity`} />
      </div>
      <div className="text-2xl font-bold text-text-primary mb-1">
        {value}
      </div>
      <span className="text-[10px] font-mono text-text-muted">
        {card.hint}
      </span>
    </motion.div>
  )
}

function LogEntry({ log }) {
  const colors = {
    error: 'text-error',
    warning: 'text-warning',
    success: 'text-success',
    info: 'text-text-muted'
  }

  const time = log.timestamp?.toLocaleTimeString('es', { hour: '2-digit', minute: '2-digit', second: '2-digit' })

  return (
    <div className="flex items-start gap-2 py-1">
      <span className="text-text-muted shrink-0">[{time}]</span>
      <span className={`shrink-0 uppercase font-bold ${colors[log.level] || 'text-text-muted'}`}>
        [{log.level}]
      </span>
      <span className="text-text-primary">{log.message}</span>
    </div>
  )
}
