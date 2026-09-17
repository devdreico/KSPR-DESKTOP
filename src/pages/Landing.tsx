import { ArrowDownToLine, Github, ShieldCheck } from "lucide-react";
import "../landing.css";

const downloads = {
  debian: "https://github.com/devdreiortiz/KSPR/releases/latest/download/kspr-linux-x64.deb",
  windows: "https://github.com/devdreiortiz/KSPR/releases/latest/download/kspr-windows-x64.exe",
};

function DebianIcon() {
  return <svg aria-hidden="true" viewBox="0 0 48 48" className="platform-icon"><circle cx="24" cy="24" r="21" fill="#d70a53"/><path fill="#fff" d="M29.4 11.4c-5.8-2.4-12.8.5-15.2 6.3-2.1 5.2.2 11.5 5.3 14.1 4.1 2.1 9.1 1.1 12.2-2.2-2.1.9-4.7.8-6.8-.3-3.9-2-5.5-6.8-3.5-10.7 1.8-3.6 6.1-5.2 9.9-3.9-.5-1.4-1.1-2.5-1.9-3.3Zm3.9 5.7c-1.3-.9-3-1.3-4.5-.9 1.1.8 1.8 2.1 1.8 3.5 0 2.4-2 4.4-4.4 4.4 1.3 1.1 3.1 1.7 4.9 1.3 3.4-.7 5.2-4.8 2.2-8.3Z"/></svg>;
}

function WindowsIcon() {
  return <svg aria-hidden="true" viewBox="0 0 48 48" className="platform-icon"><path fill="#08a6f0" d="M5 9.1 21.5 6.8v16.4H5V9.1Zm19.2-2.7L43 3.5v19.7H24.2V6.4ZM5 25.1h16.5v16.3L5 39.1V25.1Zm19.2 0H43v19.4l-18.8-2.7V25.1Z"/></svg>;
}

export function Landing() {
  return <main className="landing-shell">
    <nav className="landing-nav"><div className="brand"><span className="brand-mark">K</span><span>KSPR</span></div><a href="https://github.com/devdreiortiz/KSPR" target="_blank" rel="noreferrer"><Github size={17}/> GitHub</a></nav>
    <section className="landing-hero">
      <div className="eyebrow"><span/> KSPR DESKTOP · PRIMERA RELEASE</div>
      <h1>Entiende cualquier código.<br/><em>Más rápido.</em></h1>
      <p className="hero-copy">KSPR convierte repositorios complejos en mapas técnicos, contexto auditable y documentación accionable. Analiza localmente, sin ejecutar el código.</p>
      <div className="landing-actions"><a className="primary-action" href="#downloads">Descargar KSPR <ArrowDownToLine size={18}/></a><a className="text-action" href="https://kspr.presentto.online" target="_blank" rel="noreferrer">Conocer la herramienta ↗</a></div>
    </section>
    <section id="downloads" className="download-section">
      <div className="section-heading"><div><div className="eyebrow">DISPONIBLE PARA TU EQUIPO</div><h2>Descarga KSPR Desktop</h2></div><span className="release-pill">v0.1.0 · x64</span></div>
      <div className="download-grid">
        <a className="download-card" href={downloads.debian}><DebianIcon/><span className="download-info"><strong>Debian / Ubuntu</strong><small>Instalador .deb · Linux x64</small></span><ArrowDownToLine className="download-arrow" size={20}/></a>
        <a className="download-card" href={downloads.windows}><WindowsIcon/><span className="download-info"><strong>Windows</strong><small>Instalador .exe · Windows x64</small></span><ArrowDownToLine className="download-arrow" size={20}/></a>
      </div>
      <p className="download-note"><ShieldCheck size={15}/> Descargas publicadas desde GitHub Releases. Consulta los checksums para verificar tu instalador.</p>
    </section>
    <footer className="landing-footer"><span>© 2026 KSPR</span><a href="https://github.com/devdreiortiz/KSPR/releases" target="_blank" rel="noreferrer">Ver todas las versiones ↗</a></footer>
  </main>;
}
