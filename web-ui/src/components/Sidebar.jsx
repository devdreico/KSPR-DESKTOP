import { MessageSquare, FolderOpen, LayoutDashboard, Home, Settings, Circle } from 'lucide-react'
import { motion } from 'framer-motion'
import useStore from '../store/useStore'

const navItems = [
  { id: 'chat', label: 'Chat con KSPR I', icon: MessageSquare, shortcut: 'Ctrl+1' },
  { id: 'files', label: 'Gestor de Archivos', icon: FolderOpen, shortcut: 'Ctrl+2' },
  { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard, shortcut: 'Ctrl+3' },
]

const toolItems = [
  { id: 'welcome', label: 'Bienvenida', icon: Home, shortcut: 'Ctrl+0' },
  { id: 'settings', label: 'Configuracion', icon: Settings, shortcut: 'Ctrl+,' },
]

export default function Sidebar() {
  const { activePanel, setActivePanel, connected, connecting } = useStore()

  const statusColor = connected
    ? 'text-success'
    : connecting
    ? 'text-warning animate-pulse-dot'
    : 'text-text-muted'

  const statusText = connected
    ? 'Conectado'
    : connecting
    ? 'Conectando...'
    : 'Desconectado'

  return (
    <aside className="w-[240px] h-full flex flex-col glass-strong border-r border-border">
      {/* Brand */}
      <div className="px-4 pt-5 pb-4 flex items-center gap-3">
        <div className="w-9 h-9 rounded-xl gradient-primary flex items-center justify-center shadow-lg shadow-primary/20">
          <span className="text-white font-bold text-sm">K</span>
        </div>
        <div>
          <h1 className="font-bold text-base text-text-primary leading-tight">KSPR</h1>
          <span className="text-[10px] font-mono text-text-muted">Desktop v1.0.0</span>
        </div>
      </div>

      {/* Navigation */}
      <div className="px-3 flex-1 flex flex-col gap-0.5 overflow-y-auto">
        <span className="px-2 py-2 text-[9px] font-mono font-semibold text-text-muted tracking-widest uppercase">
          Navegacion
        </span>

        {navItems.map((item) => (
          <NavItem
            key={item.id}
            item={item}
            active={activePanel === item.id}
            onClick={() => setActivePanel(item.id)}
          />
        ))}

        <span className="px-2 py-2 mt-4 text-[9px] font-mono font-semibold text-text-muted tracking-widest uppercase">
          Herramientas
        </span>

        {toolItems.map((item) => (
          <NavItem
            key={item.id}
            item={item}
            active={activePanel === item.id}
            onClick={() => setActivePanel(item.id)}
          />
        ))}
      </div>

      {/* Status */}
      <div className="px-4 py-3 border-t border-border">
        <div className="flex items-center gap-2">
          <Circle className={`w-2 h-2 fill-current ${statusColor}`} />
          <span className={`text-xs font-mono ${statusColor}`}>
            {statusText}
          </span>
        </div>
      </div>
    </aside>
  )
}

function NavItem({ item, active, onClick }) {
  const Icon = item.icon

  return (
    <motion.button
      whileHover={{ x: 2 }}
      whileTap={{ scale: 0.98 }}
      onClick={onClick}
      className={`
        w-full flex items-center gap-3 px-3 py-2.5 rounded-lg text-left
        transition-all duration-150 group relative
        ${active
          ? 'bg-primary/10 text-primary font-medium'
          : 'text-text-muted hover:bg-primary/5 hover:text-text-primary'
        }
      `}
    >
      {active && (
        <motion.div
          layoutId="activeIndicator"
          className="absolute left-0 top-1/2 -translate-y-1/2 w-[3px] h-5 rounded-r-full bg-primary"
          transition={{ type: 'spring', stiffness: 500, damping: 30 }}
        />
      )}
      <Icon className={`w-[18px] h-[18px] ${active ? 'text-primary' : 'text-text-muted group-hover:text-text-primary'}`} />
      <span className="text-[13px] flex-1 truncate">{item.label}</span>
      <kbd className="hidden group-hover:inline text-[9px] font-mono text-text-muted bg-white/50 px-1.5 py-0.5 rounded border border-border">
        {item.shortcut}
      </kbd>
    </motion.button>
  )
}
