#ifndef INSTALLER_H
#define INSTALLER_H

#include <QObject>
#include <QProcess>
#include <QString>

class Installer : public QObject
{
    Q_OBJECT

public:
    explicit Installer(QObject *parent = nullptr);
    ~Installer();

    bool isInstalled(const QString &cliPath) const;
    void setInstallDir(const QString &dir);
    void install();
    bool isInstalling() const;

signals:
    void installStarted();
    void installProgress(const QString &message);
    void installCompleted(bool success);
    void installError(const QString &error);

private slots:
    void onProcessStarted();
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);

private:
    QProcess *m_process;
    QString m_installDir;
    bool m_installing;
};

#endif // INSTALLER_H
