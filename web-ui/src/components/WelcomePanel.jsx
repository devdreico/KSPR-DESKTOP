import { useState, useEffect } from 'react'
import { Check, Loader2, ChevronRight, Shield, Cpu, Zap } from 'lucide-react'
import { motion } from 'framer-motion'
import useStore from '../store/useStore'

const steps = [
  {
    id: 1,
    title: 'Verificar KSPR CLI',
    description: 'Comprobando si la linea de comandos esta instalada.',
    icon: Shield,
  },
  {
    id: 2,
    title: 'Instalar dependencias',
    description: 'Descargando componentes necesarios.',
    icon: Cpu,
  },
  {
    id: 3,
    title: 'Iniciar servidor',
    description: 'Conectandose al servidor local de KSPR.',
    icon: Zap,
  },
]

export default function WelcomePanel() {
  const { setActivePanel, connected, setConnecting } = useStore()
  const [currentStep, setCurrentStep] = useState(0)
  const [completedSteps, setCompletedSteps] = useState(new Set())
  const [status, setStatus] = useState('Preparando entorno...')

  useEffect(() => {
    const runSetup = async () => {
      for (let i = 0; i < steps.length; i++) {
        setCurrentStep(i)
        await new Promise(r => setTimeout(r, 1200))
        setCompletedSteps(prev => new Set([...prev, i]))
      }
      setStatus('Listo para comenzar')
    }
    runSetup()
  }, [])

  const handleStart = () => {
    setActivePanel('chat')
  }

  return (
    <div className="flex items-center justify-center h-full p-8">
      <motion.div
        initial={{ opacity: 0, scale: 0.95 }}
        animate={{ opacity: 1, scale: 1 }}
        className="max-w-lg w-full text-center space-y-8"
      >
        {/* Logo */}
        <motion.div
          initial={{ scale: 0 }}
          animate={{ scale: 1 }}
          transition={{ type: 'spring', stiffness: 200, damping: 15 }}
          className="w-28 h-28 mx-auto rounded-3xl gradient-primary flex items-center justify-center shadow-2xl shadow-primary/30"
        >
          <span className="text-white text-4xl font-bold">K</span>
        </motion.div>

        {/* Title */}
        <div>
          <h1 className="text-3xl font-bold mb-2">Bienvenido a KSPR Desktop</h1>
          <p className="text-text-muted">
            Tu asistente de IA para analisis de codigo esta casi listo.
          </p>
        </div>

        {/* Steps */}
        <div className="space-y-3 text-left">
          {steps.map((step, i) => {
            const Icon = step.icon
            const isCompleted = completedSteps.has(i)
            const isCurrent = currentStep === i

            return (
              <motion.div
                key={step.id}
                initial={{ opacity: 0, x: -20 }}
                animate={{ opacity: 1, x: 0 }}
                transition={{ delay: i * 0.1 }}
                className={`
                  glass rounded-xl p-4 flex items-center gap-4 transition-all
                  ${isCurrent ? 'border-primary/30 shadow-lg shadow-primary/5' : ''}
                  ${isCompleted ? 'border-success/30' : ''}
                `}
              >
                <div className={`
                  w-8 h-8 rounded-full flex items-center justify-center shrink-0
                  ${isCompleted
                    ? 'bg-success text-white'
                    : isCurrent
                    ? 'gradient-primary text-white'
                    : 'bg-border text-text-muted'
                  }
                `}>
                  {isCompleted ? (
                    <Check className="w-4 h-4" />
                  ) : isCurrent ? (
                    <Loader2 className="w-4 h-4 animate-spin" />
                  ) : (
                    <span className="text-xs font-mono font-bold">{step.id}</span>
                  )}
                </div>
                <div className="flex-1">
                  <div className="text-sm font-medium">{step.title}</div>
                  <div className="text-xs text-text-muted">{step.description}</div>
                </div>
                <Icon className="w-4 h-4 text-text-muted" />
              </motion.div>
            )
          })}
        </div>

        {/* Status */}
        <div className={`
          px-4 py-3 rounded-lg text-xs font-mono
          ${status.includes('Listo')
            ? 'bg-success/10 text-success border border-success/20'
            : 'glass text-text-muted'
          }
        `}>
          {status}
        </div>

        {/* Actions */}
        <div className="flex items-center justify-center gap-3">
          <button
            onClick={() => setActivePanel('settings')}
            className="px-5 py-2.5 rounded-xl text-sm text-text-muted border border-border hover:border-primary hover:text-primary transition-all"
          >
            Configurar
          </button>
          <motion.button
            whileHover={{ scale: 1.02 }}
            whileTap={{ scale: 0.98 }}
            onClick={handleStart}
            disabled={!status.includes('Listo')}
            className="gradient-primary text-white px-6 py-2.5 rounded-xl font-medium text-sm
              disabled:opacity-40 disabled:cursor-not-allowed
              hover:gradient-primary-hover transition-all shadow-lg shadow-primary/20
              flex items-center gap-2"
          >
            Iniciar sesion
            <ChevronRight className="w-4 h-4" />
          </motion.button>
        </div>
      </motion.div>
    </div>
  )
}
