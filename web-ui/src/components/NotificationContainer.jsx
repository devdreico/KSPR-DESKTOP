import { useEffect } from 'react'
import { X, CheckCircle, AlertCircle, AlertTriangle, Info } from 'lucide-react'
import { motion, AnimatePresence } from 'framer-motion'
import useStore from '../store/useStore'

const icons = {
  success: CheckCircle,
  error: AlertCircle,
  warning: AlertTriangle,
  info: Info
}

const styles = {
  success: 'bg-success/10 border-success/20 text-success',
  error: 'bg-error/10 border-error/20 text-error',
  warning: 'bg-warning/10 border-warning/20 text-warning',
  info: 'bg-primary/10 border-primary/20 text-primary'
}

export default function NotificationContainer() {
  const { notifications, removeNotification } = useStore()

  return (
    <div className="fixed top-4 right-4 z-50 flex flex-col gap-2 max-w-sm">
      <AnimatePresence>
        {notifications.map((notif) => (
          <Toast key={notif.id} notification={notif} onRemove={removeNotification} />
        ))}
      </AnimatePresence>
    </div>
  )
}

function Toast({ notification, onRemove }) {
  const Icon = icons[notification.type] || Info

  useEffect(() => {
    const timer = setTimeout(() => {
      onRemove(notification.id)
    }, notification.duration || 3000)
    return () => clearTimeout(timer)
  }, [notification, onRemove])

  return (
    <motion.div
      initial={{ opacity: 0, x: 50, scale: 0.95 }}
      animate={{ opacity: 1, x: 0, scale: 1 }}
      exit={{ opacity: 0, x: 50, scale: 0.95 }}
      className={`
        flex items-start gap-3 px-4 py-3 rounded-xl border backdrop-blur-sm shadow-lg
        ${styles[notification.type] || styles.info}
      `}
    >
      <Icon className="w-4 h-4 mt-0.5 shrink-0" />
      <div className="flex-1 min-w-0">
        <p className="text-sm font-medium">{notification.message}</p>
      </div>
      <button
        onClick={() => onRemove(notification.id)}
        className="shrink-0 opacity-60 hover:opacity-100 transition-opacity"
      >
        <X className="w-4 h-4" />
      </button>
    </motion.div>
  )
}
