import { useState } from 'react'
import { FolderOpen, FileCode, Trash2, RefreshCw, Eye, Upload } from 'lucide-react'
import { motion, AnimatePresence } from 'framer-motion'
import useStore from '../store/useStore'

const SUPPORTED_EXTENSIONS = ['.py', '.js', '.ts', '.jsx', '.tsx', '.java', '.c', '.cpp', '.h', '.hpp', '.cs', '.php', '.html', '.css', '.json', '.yaml', '.yml', '.md', '.txt']

export default function FilePanel() {
  const { files, addFile, removeFile, clearFiles } = useStore()
  const [selectedFile, setSelectedFile] = useState(null)
  const [dragOver, setDragOver] = useState(false)

  const handleFileSelect = (e) => {
    const selectedFiles = Array.from(e.target.files || [])
    selectedFiles.forEach(file => {
      const ext = '.' + file.name.split('.').pop()
      if (SUPPORTED_EXTENSIONS.includes(ext)) {
        const reader = new FileReader()
        reader.onload = (ev) => {
          addFile({
            name: file.name,
            path: file.webkitRelativePath || file.name,
            content: ev.target.result,
            size: file.size,
            lines: ev.target.result.split('\n').length
          })
        }
        reader.readAsText(file)
      }
    })
  }

  const handleDrop = (e) => {
    e.preventDefault()
    setDragOver(false)
    const droppedFiles = Array.from(e.dataTransfer.files)
    droppedFiles.forEach(file => {
      const ext = '.' + file.name.split('.').pop()
      if (SUPPORTED_EXTENSIONS.includes(ext)) {
        const reader = new FileReader()
        reader.onload = (ev) => {
          addFile({
            name: file.name,
            path: file.webkitRelativePath || file.name,
            content: ev.target.result,
            size: file.size,
            lines: ev.target.result.split('\n').length
          })
        }
        reader.readAsText(file)
      }
    })
  }

  const formatSize = (bytes) => {
    if (bytes < 1024) return bytes + ' B'
    if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB'
    return (bytes / (1024 * 1024)).toFixed(1) + ' MB'
  }

  return (
    <div className="flex flex-col h-full">
      {/* Header */}
      <header className="glass-strong border-b border-border px-6 py-3 flex items-center justify-between">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 rounded-lg gradient-primary flex items-center justify-center">
            <FolderOpen className="w-4 h-4 text-white" />
          </div>
          <div>
            <h2 className="font-semibold text-sm">Gestor de Archivos</h2>
            <span className="text-[10px] font-mono text-text-muted">
              {files.length} archivos en contexto
            </span>
          </div>
        </div>
      </header>

      {/* Toolbar */}
      <div className="glass-strong border-b border-border px-6 py-3 flex items-center gap-3">
        <label className="flex items-center gap-2 glass rounded-lg px-4 py-2 text-sm font-medium cursor-pointer hover:border-primary transition-colors">
          <Upload className="w-4 h-4" />
          <span>+ Archivo</span>
          <input
            type="file"
            multiple
            className="hidden"
            accept={SUPPORTED_EXTENSIONS.join(',')}
            onChange={handleFileSelect}
          />
        </label>

        {files.length > 0 && (
          <>
            <button
              onClick={() => setSelectedFile(null)}
              className="flex items-center gap-2 text-text-muted hover:text-error text-sm px-3 py-2 rounded-lg hover:bg-error/5 transition-colors"
            >
              <Trash2 className="w-4 h-4" />
              Limpiar
            </button>
            <button
              onClick={clearFiles}
              className="flex items-center gap-2 text-text-muted hover:text-text-primary text-sm px-3 py-2 rounded-lg hover:bg-primary/5 transition-colors"
            >
              <RefreshCw className="w-4 h-4" />
              Actualizar
            </button>
          </>
        )}
      </div>

      {/* Content */}
      <div className="flex-1 flex overflow-hidden">
        {/* File List */}
        <div
          className={`w-[320px] border-r border-border overflow-y-auto transition-colors ${
            dragOver ? 'bg-primary/5 border-primary' : ''
          }`}
          onDragOver={(e) => { e.preventDefault(); setDragOver(true) }}
          onDragLeave={() => setDragOver(false)}
          onDrop={handleDrop}
        >
          {files.length === 0 ? (
            <div className="flex flex-col items-center justify-center h-full text-center p-6 animate-fade-in">
              <div className="w-16 h-16 rounded-2xl bg-primary/10 flex items-center justify-center mb-4">
                <FolderOpen className="w-8 h-8 text-primary" />
              </div>
              <h3 className="font-semibold mb-1">Sin archivos</h3>
              <p className="text-text-muted text-xs">
                Arrastra archivos aqui o usa el boton + Archivo
              </p>
            </div>
          ) : (
            <div className="p-2">
              <AnimatePresence>
                {files.map((file) => (
                  <motion.button
                    key={file.id}
                    initial={{ opacity: 0, x: -10 }}
                    animate={{ opacity: 1, x: 0 }}
                    exit={{ opacity: 0, x: -10 }}
                    onClick={() => setSelectedFile(file)}
                    className={`w-full flex items-center gap-3 px-3 py-2.5 rounded-lg text-left transition-all group ${
                      selectedFile?.id === file.id
                        ? 'bg-primary/10 border border-primary/20'
                        : 'hover:bg-primary/5 border border-transparent'
                    }`}
                  >
                    <FileCode className={`w-4 h-4 shrink-0 ${
                      selectedFile?.id === file.id ? 'text-primary' : 'text-text-muted'
                    }`} />
                    <div className="flex-1 min-w-0">
                      <div className="text-sm font-medium truncate">{file.name}</div>
                      <div className="text-[10px] font-mono text-text-muted">
                        {file.lines} lineas · {formatSize(file.size)}
                      </div>
                    </div>
                    <button
                      onClick={(e) => { e.stopPropagation(); removeFile(file.id) }}
                      className="opacity-0 group-hover:opacity-100 p-1 rounded hover:bg-error/10 text-text-muted hover:text-error transition-all"
                    >
                      <Trash2 className="w-3.5 h-3.5" />
                    </button>
                  </motion.button>
                ))}
              </AnimatePresence>
            </div>
          )}
        </div>

        {/* Preview */}
        <div className="flex-1 flex flex-col overflow-hidden">
          {selectedFile ? (
            <>
              <div className="glass-strong border-b border-border px-6 py-3 flex items-center justify-between">
                <div className="flex items-center gap-2">
                  <Eye className="w-4 h-4 text-primary" />
                  <span className="text-sm font-medium">{selectedFile.name}</span>
                </div>
                <span className="text-[10px] font-mono text-text-muted">
                  {selectedFile.lines} lineas
                </span>
              </div>
              <div className="flex-1 overflow-auto p-6">
                <pre className="text-sm font-mono text-text-primary whitespace-pre-wrap leading-relaxed">
                  {selectedFile.content}
                </pre>
              </div>
            </>
          ) : (
            <div className="flex items-center justify-center h-full text-text-muted">
              <div className="text-center">
                <Eye className="w-8 h-8 mx-auto mb-2 opacity-30" />
                <span className="text-sm">Selecciona un archivo para previsualizar</span>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
