#include "processmanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ProcessManager::ProcessManager(const QString &cliPath, QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_cliPath(cliPath)
    , m_connected(false)
{
}

ProcessManager::~ProcessManager()
{
    stop();
}

void ProcessManager::start()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        return;
    }

    m_process = new QProcess(this);

    connect(m_process, &QProcess::started,
            this, &ProcessManager::onProcessStarted);
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &ProcessManager::onReadyReadStdout);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &ProcessManager::onReadyReadStderr);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessManager::onFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &ProcessManager::onError);

    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    QStringList arguments;
    arguments << "serve" << "--host" << "127.0.0.1" << "--port" << "8000";

    m_process->start(m_cliPath, arguments);

    emit statusChanged("Iniciando KSPR CLI...");
}

void ProcessManager::stop()
{
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->terminate();
            if (!m_process->waitForFinished(3000)) {
                m_process->kill();
                m_process->waitForFinished(1000);
            }
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_connected = false;
    emit statusChanged("Detenido");
}

void ProcessManager::restart()
{
    stop();
    start();
}

bool ProcessManager::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

void ProcessManager::sendCommand(const QString &action, const QJsonObject &params)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        emit errorReceived("El proceso no está ejecutándose");
        return;
    }

    QJsonObject command;
    command["action"] = action;
    if (!params.isEmpty()) {
        command["params"] = params;
    }

    QJsonDocument doc(command);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

    m_process->write(data);
}

void ProcessManager::sendRawInput(const QString &input)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        emit errorReceived("El proceso no está ejecutándose");
        return;
    }

    m_process->write(input.toUtf8() + "\n");
}

void ProcessManager::onProcessStarted()
{
    emit statusChanged("KSPR CLI iniciado");
    qDebug() << "KSPR CLI process started";
}

void ProcessManager::onReadyReadStdout()
{
    QByteArray data = m_process->readAllStandardOutput();
    parseOutput(data);
}

void ProcessManager::onReadyReadStderr()
{
    QByteArray data = m_process->readAllStandardError();
    QString errorStr = QString::fromUtf8(data).trimmed();
    if (!errorStr.isEmpty()) {
        emit errorReceived(errorStr);
    }
}

void ProcessManager::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_connected = false;
    emit finished(exitCode);
    emit statusChanged("Proceso finalizado");

    qDebug() << "KSPR CLI finished with code:" << exitCode;
}

void ProcessManager::onError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "No se pudo iniciar KSPR CLI";
        break;
    case QProcess::Crashed:
        errorMsg = "KSPR CLI se cerró inesperadamente";
        break;
    case QProcess::Timedout:
        errorMsg = "KSPR CLI excedió el tiempo límite";
        break;
    case QProcess::WriteError:
        errorMsg = "Error al escribir en KSPR CLI";
        break;
    case QProcess::ReadError:
        errorMsg = "Error al leer de KSPR CLI";
        break;
    default:
        errorMsg = "Error desconocido en KSPR CLI";
        break;
    }

    emit errorReceived(errorMsg);
    emit statusChanged("Error");
}

void ProcessManager::parseOutput(const QByteArray &data)
{
    m_outputBuffer.append(data);

    while (m_outputBuffer.contains('\n')) {
        int idx = m_outputBuffer.indexOf('\n');
        QByteArray line = m_outputBuffer.left(idx);
        m_outputBuffer.remove(0, idx + 1);

        line = line.trimmed();
        if (line.isEmpty()) continue;

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(line, &parseError);

        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject response = doc.object();

            if (response.contains("status") && response["status"].toString() == "ready") {
                m_connected = true;
                emit connectionEstablished();
            }

            emit responseReceived(response);
        } else {
            emit outputReceived(QString::fromUtf8(line));
        }
    }
}
