import { useEffect } from 'react'
import Sidebar from './components/Sidebar'
import ChatPanel from './components/ChatPanel'
import FilePanel from './components/FilePanel'
import DashboardPanel from './components/DashboardPanel'
import WelcomePanel from './components/WelcomePanel'
import SettingsPanel from './components/SettingsPanel'
import NotificationContainer from './components/NotificationContainer'
import useStore from './store/useStore'

function App() {
  const activePanel = useStore((s) => s.activePanel)

  const panels = {
    chat: ChatPanel,
    files: FilePanel,
    dashboard: DashboardPanel,
    welcome: WelcomePanel,
    settings: SettingsPanel
  }

  const ActivePanel = panels[activePanel] || ChatPanel

  return (
    <div className="flex h-screen w-screen overflow-hidden bg-gradient-to-br from-[var(--color-bg-start)] to-[var(--color-bg-end)]">
      <Sidebar />
      <main className="flex-1 flex flex-col overflow-hidden">
        <ActivePanel />
      </main>
      <NotificationContainer />
    </div>
  )
}

export default App
