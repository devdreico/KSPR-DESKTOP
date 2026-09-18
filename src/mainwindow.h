#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class ProcessManager;
class ChatPanel;
class FilePanel;
class DashboardPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &cliPath, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onChatClicked();
    void onFilesClicked();
    void onDashboardClicked();
    void onProcessStarted();
    void onProcessFinished(int exitCode);
    void onProcessError(const QString &error);
    void onConnectionEstablished();

private:
    void setupUI();
    void setupSidebar();
    void setupStatusBar();
    void connectSignals();

    QString m_cliPath;
    ProcessManager *m_processManager;
    ChatPanel *m_chatPanel;
    FilePanel *m_filePanel;
    DashboardPanel *m_dashboardPanel;

    QStackedWidget *m_stackedWidget;
    QPushButton *m_chatButton;
    QPushButton *m_filesButton;
    QPushButton *m_dashboardButton;
    QLabel *m_statusLabel;
    QLabel *m_connectionLabel;
};

#endif // MAINWINDOW_H
