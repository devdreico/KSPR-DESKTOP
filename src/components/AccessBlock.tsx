export function AccessBlock() {
  function handlePointerMove(event: React.PointerEvent<HTMLElement>) {
    const bounds = event.currentTarget.getBoundingClientRect();
    const x = ((event.clientX - bounds.left) / bounds.width - 0.5) * 2;
    const y = ((event.clientY - bounds.top) / bounds.height - 0.5) * 2;
    event.currentTarget.style.setProperty("--pointer-x", `${x * 10}px`);
    event.currentTarget.style.setProperty("--pointer-y", `${y * 10}px`);
  }

  return (
    <main className="access-block" onPointerMove={handlePointerMove}>
      <span className="access-block__orb access-block__orb--lime" aria-hidden="true" />
      <span className="access-block__orb access-block__orb--blue" aria-hidden="true" />
      <div className="access-block__content">
        <div className="access-block__logo-wrap">
          <img className="access-block__logo" src="/kspr-main-logo.png" alt="KSPR Desktop" />
        </div>
        <p className="access-block__kicker">KSPR DESKTOP · 2026</p>
        <h1>COMMING <span>SOON</span></h1>
        <p className="access-block__message">Estamos preparando una nueva forma de entender tu código.</p>
        <a className="access-block__back" href="https://kspr.presentto.online/">
          <span>VOLVER</span>
          <span aria-hidden="true">↗</span>
        </a>
      </div>
    </main>
  );
}