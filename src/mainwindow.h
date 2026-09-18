#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ProcessManager;
class ChatPanel;
class FilePanel;
class DashboardPanel;
class WelcomePanel;
class SettingsPanel;
class NotificationManager;
class LoadingOverlay;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &cliPath, QWidget *parent = nullptr);
    ~MainWindow();

    NotificationManager *notificationManager() const { return m_notificationManager; }
    LoadingOverlay *loadingOverlay() const { return m_loadingOverlay; }

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onChatClicked();
    void onFilesClicked();
    void onDashboardClicked();
    void onWelcomeClicked();
    void onSettingsClicked();
    void onProcessStarted();
    void onProcessFinished(int exitCode);
    void onProcessError(const QString &error);
    void onConnectionEstablished();
    void onThemeChanged(const QString &theme);

private:
    void setupUI();
    void setupSidebar();
    void setupStatusBar();
    void setupWelcomePanel();
    void setupSettingsPanel();
    void connectSignals();
    void updateActiveButton(QPushButton *activeButton);

    QString m_cliPath;
    ProcessManager *m_processManager;
    ChatPanel *m_chatPanel;
    FilePanel *m_filePanel;
    DashboardPanel *m_dashboardPanel;
    WelcomePanel *m_welcomePanel;
    SettingsPanel *m_settingsPanel;
    NotificationManager *m_notificationManager;
    LoadingOverlay *m_loadingOverlay;

    QStackedWidget *m_stackedWidget;
    QPushButton *m_chatButton;
    QPushButton *m_filesButton;
    QPushButton *m_dashboardButton;
    QPushButton *m_welcomeButton;
    QPushButton *m_settingsButton;
    QLabel *m_statusLabel;
    QLabel *m_connectionLabel;
    QLabel *m_versionLabel;

    QWidget *m_sidebar;
};

#endif // MAINWINDOW_H
