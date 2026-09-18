#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

class ProcessManager : public QObject
{
    Q_OBJECT

public:
    explicit ProcessManager(const QString &cliPath, QObject *parent = nullptr);
    ~ProcessManager();

    void start();
    void stop();
    void restart();
    bool isRunning() const;

    void sendCommand(const QString &action, const QJsonObject &params = QJsonObject());
    void sendRawInput(const QString &input);

signals:
    void started();
    void finished(int exitCode);
    void connectionEstablished();
    void responseReceived(const QJsonObject &response);
    void outputReceived(const QString &output);
    void errorReceived(const QString &error);
    void statusChanged(const QString &status);

private slots:
    void onProcessStarted();
    void onReadyReadStdout();
    void onReadyReadStderr();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onError(QProcess::ProcessError error);

private:
    void parseOutput(const QByteArray &data);

    QProcess *m_process;
    QString m_cliPath;
    QByteArray m_outputBuffer;
    bool m_connected;
};

#endif // PROCESSMANAGER_H
