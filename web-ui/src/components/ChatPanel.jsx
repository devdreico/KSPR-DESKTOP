import { useState, useRef, useEffect } from 'react'
import { Send, Bot, User, AlertCircle } from 'lucide-react'
import { motion, AnimatePresence } from 'framer-motion'
import useStore from '../store/useStore'

export default function ChatPanel() {
  const { messages, addMessage, connected } = useStore()
  const [input, setInput] = useState('')
  const [sending, setSending] = useState(false)
  const messagesEndRef = useRef(null)
  const inputRef = useRef(null)

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [messages])

  useEffect(() => {
    inputRef.current?.focus()
  }, [])

  const handleSend = () => {
    const text = input.trim()
    if (!text || sending) return

    addMessage({ role: 'user', content: text })
    setInput('')
    setSending(true)

    // Simulate response (will be replaced with real backend)
    setTimeout(() => {
      addMessage({
        role: 'assistant',
        content: `Received: "${text}". Connecting to KSPR CLI backend...`
      })
      setSending(false)
    }, 800)
  }

  const handleKeyDown = (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault()
      handleSend()
    }
  }

  return (
    <div className="flex flex-col h-full">
      {/* Header */}
      <header className="glass-strong border-b border-border px-6 py-3 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 rounded-lg gradient-primary flex items-center justify-center">
            <Bot className="w-4 h-4 text-white" />
          </div>
          <div>
            <h2 className="font-semibold text-sm">Chat con KSPR I</h2>
            <span className="text-[10px] font-mono text-text-muted">
              {connected ? 'En linea' : 'Fuera de linea'}
            </span>
          </div>
        </div>
        <div className="flex items-center gap-2">
          <span className="text-xs font-mono text-text-muted">
            {messages.length} mensajes
          </span>
        </div>
      </header>

      {/* Messages */}
      <div className="flex-1 overflow-y-auto px-6 py-4 space-y-4">
        {messages.length === 0 && (
          <div className="flex flex-col items-center justify-center h-full text-center animate-fade-in">
            <div className="w-20 h-20 rounded-2xl gradient-primary flex items-center justify-center mb-6 shadow-xl shadow-primary/20">
              <Bot className="w-10 h-10 text-white" />
            </div>
            <h3 className="text-xl font-semibold mb-2">Hola, soy KSPR I</h3>
            <p className="text-text-muted text-sm max-w-md">
              Tu asistente de IA para analisis de codigo. Preguntame sobre tu codigo,
              funciones, o cualquier cosa relacionada con programacion.
            </p>
          </div>
        )}

        <AnimatePresence>
          {messages.map((msg) => (
            <MessageBubble key={msg.id} message={msg} />
          ))}
        </AnimatePresence>

        {sending && (
          <motion.div
            initial={{ opacity: 0, y: 10 }}
            animate={{ opacity: 1, y: 0 }}
            className="flex items-center gap-3 px-4"
          >
            <div className="w-7 h-7 rounded-lg gradient-primary flex items-center justify-center">
              <Bot className="w-3.5 h-3.5 text-white" />
            </div>
            <div className="glass rounded-2xl rounded-tl-sm px-4 py-3 flex gap-1.5">
              <span className="w-2 h-2 bg-text-muted rounded-full animate-bounce" style={{ animationDelay: '0ms' }} />
              <span className="w-2 h-2 bg-text-muted rounded-full animate-bounce" style={{ animationDelay: '150ms' }} />
              <span className="w-2 h-2 bg-text-muted rounded-full animate-bounce" style={{ animationDelay: '300ms' }} />
            </div>
          </motion.div>
        )}

        <div ref={messagesEndRef} />
      </div>

      {/* Input */}
      <div className="glass-strong border-t border-border px-6 py-4">
        <div className="flex items-center gap-3">
          <input
            ref={inputRef}
            type="text"
            value={input}
            onChange={(e) => setInput(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="Escribe tu mensaje a KSPR I..."
            className="flex-1 glass rounded-xl px-4 py-3 text-sm outline-none focus:ring-2 focus:ring-primary/30 transition-all placeholder:text-text-muted"
          />
          <motion.button
            whileHover={{ scale: 1.05 }}
            whileTap={{ scale: 0.95 }}
            onClick={handleSend}
            disabled={!input.trim() || sending}
            className="gradient-primary text-white px-5 py-3 rounded-xl font-medium text-sm
              disabled:opacity-40 disabled:cursor-not-allowed
              hover:gradient-primary-hover transition-all shadow-lg shadow-primary/20"
          >
            <Send className="w-4 h-4" />
          </motion.button>
        </div>
      </div>
    </div>
  )
}

function MessageBubble({ message }) {
  const isUser = message.role === 'user'
  const isError = message.role === 'error'

  const time = message.timestamp?.toLocaleTimeString('es', { hour: '2-digit', minute: '2-digit' })

  if (isError) {
    return (
      <motion.div
        initial={{ opacity: 0, scale: 0.95 }}
        animate={{ opacity: 1, scale: 1 }}
        className="flex justify-center"
      >
        <div className="flex items-center gap-2 bg-error/10 text-error px-4 py-2 rounded-lg text-xs font-mono">
          <AlertCircle className="w-3.5 h-3.5" />
          {message.content}
        </div>
      </motion.div>
    )
  }

  return (
    <motion.div
      initial={{ opacity: 0, y: 10 }}
      animate={{ opacity: 1, y: 0 }}
      className={`flex ${isUser ? 'justify-end' : 'justify-start'}`}
    >
      <div className={`flex items-start gap-2.5 max-w-[75%] ${isUser ? 'flex-row-reverse' : ''}`}>
        {!isUser && (
          <div className="w-7 h-7 rounded-lg gradient-primary flex items-center justify-center shrink-0 mt-1">
            <Bot className="w-3.5 h-3.5 text-white" />
          </div>
        )}

        <div className={isUser ? 'text-right' : ''}>
          {!isUser && (
            <span className="text-[10px] font-mono text-text-muted ml-1">KSPR I</span>
          )}
          <div
            className={`
              px-4 py-3 text-sm leading-relaxed
              ${isUser
                ? 'gradient-primary text-white rounded-2xl rounded-tr-sm'
                : 'glass rounded-2xl rounded-tl-sm'
              }
            `}
          >
            {message.content}
          </div>
          <span className="text-[10px] font-mono text-text-muted mt-1 block px-1">
            {time}
          </span>
        </div>

        {isUser && (
          <div className="w-7 h-7 rounded-lg bg-primary-dark flex items-center justify-center shrink-0 mt-1">
            <User className="w-3.5 h-3.5 text-white" />
          </div>
        )}
      </div>
    </motion.div>
  )
}
