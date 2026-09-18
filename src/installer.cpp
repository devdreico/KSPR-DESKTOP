#include "installer.h"
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

Installer::Installer(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_installing(false)
{
}

Installer::~Installer()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

bool Installer::isInstalled(const QString &cliPath) const
{
    QFileInfo fileInfo(cliPath);
    return fileInfo.exists() && fileInfo.isExecutable();
}

void Installer::setInstallDir(const QString &dir)
{
    m_installDir = dir;
}

void Installer::install()
{
    if (m_installing) return;

    m_installing = true;
    emit installStarted();

    QDir().mkpath(m_installDir);

    m_process = new QProcess(this);

    connect(m_process, &QProcess::started,
            this, &Installer::onProcessStarted);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &Installer::onProcessReadyRead);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &Installer::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Installer::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &Installer::onProcessError);

    QString script = QString(
        "curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | "
        "INSTALL_DIR=%1 bash"
    ).arg(m_installDir);

    m_process->start("bash", {"-c", script});
}

bool Installer::isInstalling() const
{
    return m_installing;
}

void Installer::onProcessStarted()
{
    emit installProgress("Descargando KSPR CLI...");
}

void Installer::onProcessReadyRead()
{
    QByteArray output = m_process->readAllStandardOutput();
    QByteArray error = m_process->readAllStandardError();

    QString message = QString::fromUtf8(output).trimmed();
    if (!message.isEmpty()) {
        emit installProgress(message);
    }

    QString errorMsg = QString::fromUtf8(error).trimmed();
    if (!errorMsg.isEmpty()) {
        emit installProgress(errorMsg);
    }
}

void Installer::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_installing = false;

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        emit installCompleted(true);
    } else {
        emit installError(QString("La instalación falló con código: %1").arg(exitCode));
    }

    m_process->deleteLater();
    m_process = nullptr;
}

void Installer::onProcessError(QProcess::ProcessError error)
{
    m_installing = false;

    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "No se pudo iniciar el proceso de instalación";
        break;
    case QProcess::Crashed:
        errorMsg = "El proceso de instalación falló";
        break;
    case QProcess::Timedout:
        errorMsg = "La instalación excedió el tiempo límite";
        break;
    case QProcess::WriteError:
        errorMsg = "Error al escribir en el proceso";
        break;
    case QProcess::ReadError:
        errorMsg = "Error al leer la salida del proceso";
        break;
    default:
        errorMsg = "Error desconocido durante la instalación";
        break;
    }

    emit installError(errorMsg);

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
}
