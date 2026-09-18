import { create } from 'zustand'

const useStore = create((set, get) => ({
  // Navigation
  activePanel: 'chat',
  setActivePanel: (panel) => set({ activePanel: panel }),

  // Connection
  connected: false,
  connecting: false,
  setConnected: (connected) => set({ connected, connecting: false }),
  setConnecting: (connecting) => set({ connecting }),

  // Chat
  messages: [],
  addMessage: (message) => set((state) => ({
    messages: [...state.messages, { ...message, id: Date.now(), timestamp: new Date() }]
  })),
  clearMessages: () => set({ messages: [] }),

  // Files
  files: [],
  addFile: (file) => set((state) => ({
    files: [...state.files, { ...file, id: Date.now() }]
  })),
  removeFile: (id) => set((state) => ({
    files: state.files.filter(f => f.id !== id)
  })),
  clearFiles: () => set({ files: [] }),

  // Dashboard
  metrics: {
    messages: 0,
    files: 0,
    tokens: 0,
    latency: 0,
    uptime: 0
  },
  updateMetrics: (metrics) => set((state) => ({
    metrics: { ...state.metrics, ...metrics }
  })),

  // Logs
  logs: [],
  addLog: (log) => set((state) => ({
    logs: [...state.logs, { ...log, id: Date.now(), timestamp: new Date() }].slice(-100)
  })),

  // Settings
  settings: {
    theme: 'light',
    language: 'es',
    cliPath: './bin/kspr',
    port: 8000,
    autoStart: true,
    notifications: true
  },
  updateSettings: (settings) => set((state) => ({
    settings: { ...state.settings, ...settings }
  })),

  // Notifications
  notifications: [],
  addNotification: (notification) => set((state) => ({
    notifications: [...state.notifications, { ...notification, id: Date.now() }]
  })),
  removeNotification: (id) => set((state) => ({
    notifications: state.notifications.filter(n => n.id !== id)
  })),

  // Search
  searchQuery: '',
  setSearchQuery: (query) => set({ searchQuery: query }),
  searchOpen: false,
  setSearchOpen: (open) => set({ searchOpen: open })
}))

export default useStore
