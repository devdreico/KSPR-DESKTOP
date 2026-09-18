#include "mainwindow.h"
#include "processmanager.h"
#include "chatpanel.h"
#include "filepanel.h"
#include "dashboardpanel.h"
#include "styles.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QStatusBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow(const QString &cliPath, QWidget *parent)
    : QMainWindow(parent)
    , m_cliPath(cliPath)
    , m_processManager(new ProcessManager(cliPath, this))
{
    setWindowTitle("KSPR Desktop");
    setMinimumSize(1200, 800);
    resize(1440, 900);

    QIcon icon(":/desktop-app-icon.png");
    setWindowIcon(icon);

    KSPRStyles::applyTheme("light");

    setupUI();
    connectSignals();

    m_processManager->start();
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_processManager->isRunning()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Salir de KSPR Desktop",
            "KSPR CLI sigue ejecutándose.\n¿Deseas detenerlo y salir?",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_processManager->stop();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupSidebar();

    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(220);
    sidebar->setProperty("class", "sidebar");
    sidebar->setStyleSheet(
        "background: rgba(243, 242, 238, 0.94); "
        "border-right: 1px solid #d8d7d1;"
    );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(12, 16, 12, 16);
    sidebarLayout->setSpacing(4);

    QWidget *brandWidget = new QWidget(sidebar);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(4, 0, 4, 16);

    QLabel *brandIcon = new QLabel(brandWidget);
    brandIcon->setPixmap(QPixmap(":/desktop-app-icon.png").scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    brandIcon->setFixedSize(28, 28);

    QLabel *brandText = new QLabel(brandWidget);
    brandText->setText("<strong>KSPR</strong><br><span style='font-size: 9px; color: #777873; font-family: DM Mono, monospace;'>Desktop v1.0</span>");

    brandLayout->addWidget(brandIcon);
    brandLayout->addWidget(brandText);
    brandLayout->addStretch();

    sidebarLayout->addWidget(brandWidget);

    QLabel *navLabel = new QLabel("NAVEGACIÓN", sidebar);
    navLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 9px; color: #777873; letter-spacing: 0.1em; padding: 8px 4px;");

    m_chatButton = new QPushButton(" Chat con KSPR I", sidebar);
    m_chatButton->setObjectName("chatButton");
    m_chatButton->setCheckable(true);
    m_chatButton->setChecked(true);
    m_chatButton->setMinimumHeight(40);

    m_filesButton = new QPushButton(" Gestor de Archivos", sidebar);
    m_filesButton->setObjectName("filesButton");
    m_filesButton->setCheckable(true);
    m_filesButton->setMinimumHeight(40);

    m_dashboardButton = new QPushButton(" Dashboard", sidebar);
    m_dashboardButton->setObjectName("dashboardButton");
    m_dashboardButton->setCheckable(true);
    m_dashboardButton->setMinimumHeight(40);

    QString navButtonStyle =
        "QPushButton { "
        "text-align: left; "
        "padding: 10px 12px; "
        "border: none; "
        "border-radius: 8px; "
        "background: transparent; "
        "color: #777873; "
        "font-family: Montserrat, sans-serif; "
        "font-size: 13px; "
        "} "
        "QPushButton:hover { "
        "background: rgba(99, 102, 241, 0.05); "
        "color: #141414; "
        "} "
        "QPushButton:checked { "
        "background: rgba(99, 102, 241, 0.1); "
        "color: #6366f1; "
        "font-weight: 500; "
        "}";

    m_chatButton->setStyleSheet(navButtonStyle);
    m_filesButton->setStyleSheet(navButtonStyle);
    m_dashboardButton->setStyleSheet(navButtonStyle);

    sidebarLayout->addWidget(navLabel);
    sidebarLayout->addWidget(m_chatButton);
    sidebarLayout->addWidget(m_filesButton);
    sidebarLayout->addWidget(m_dashboardButton);
    sidebarLayout->addStretch();

    QWidget *statusWidget = new QWidget(sidebar);
    statusWidget->setStyleSheet("border-top: 1px solid #d8d7d1; padding-top: 12px;");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusWidget);
    statusLayout->setContentsMargins(4, 8, 4, 0);

    m_connectionLabel = new QLabel("Desconectado", statusWidget);
    m_connectionLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873;");

    statusLayout->addWidget(m_connectionLabel);

    sidebarLayout->addWidget(statusWidget);

    m_stackedWidget = new QStackedWidget();

    m_chatPanel = new ChatPanel(m_processManager);
    m_filePanel = new FilePanel(m_processManager);
    m_dashboardPanel = new DashboardPanel(m_processManager);

    m_stackedWidget->addWidget(m_chatPanel);
    m_stackedWidget->addWidget(m_filePanel);
    m_stackedWidget->addWidget(m_dashboardPanel);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(m_stackedWidget, 1);

    setCentralWidget(centralWidget);

    setupStatusBar();
}

void MainWindow::setupSidebar()
{
}

void MainWindow::setupStatusBar()
{
    QStatusBar *statusBar = this->statusBar();
    statusBar->setStyleSheet(
        "QStatusBar { "
        "background: rgba(243, 242, 238, 0.94); "
        "border-top: 1px solid #d8d7d1; "
        "padding: 4px 16px; "
        "} "
        "QStatusBar::item { border: none; }"
    );

    m_statusLabel = new QLabel("KSPR Desktop v1.0.0");
    m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873;");

    statusBar->addWidget(m_statusLabel);
    statusBar->addPermanentWidget(new QLabel("Secure static analysis · No code execution"));
}

void MainWindow::connectSignals()
{
    connect(m_chatButton, &QPushButton::clicked, this, &MainWindow::onChatClicked);
    connect(m_filesButton, &QPushButton::clicked, this, &MainWindow::onFilesClicked);
    connect(m_dashboardButton, &QPushButton::clicked, this, &MainWindow::onDashboardClicked);

    connect(m_processManager, &ProcessManager::started,
            this, &MainWindow::onProcessStarted);
    connect(m_processManager, &ProcessManager::finished,
            this, &MainWindow::onProcessFinished);
    connect(m_processManager, &ProcessManager::errorReceived,
            this, &MainWindow::onProcessError);
    connect(m_processManager, &ProcessManager::connectionEstablished,
            this, &MainWindow::onConnectionEstablished);
}

void MainWindow::onChatClicked()
{
    m_stackedWidget->setCurrentWidget(m_chatPanel);
    m_chatButton->setChecked(true);
    m_filesButton->setChecked(false);
    m_dashboardButton->setChecked(false);
}

void MainWindow::onFilesClicked()
{
    m_stackedWidget->setCurrentWidget(m_filePanel);
    m_chatButton->setChecked(false);
    m_filesButton->setChecked(true);
    m_dashboardButton->setChecked(false);
}

void MainWindow::onDashboardClicked()
{
    m_stackedWidget->setCurrentWidget(m_dashboardPanel);
    m_chatButton->setChecked(false);
    m_filesButton->setChecked(false);
    m_dashboardButton->setChecked(true);
}

void MainWindow::onProcessStarted()
{
    m_connectionLabel->setText("Iniciando...");
    m_connectionLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #f59e0b;");
    m_dashboardPanel->addLogEntry("KSPR CLI iniciado", "info");
}

void MainWindow::onProcessFinished(int exitCode)
{
    m_connectionLabel->setText("Detenido");
    m_connectionLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #ef4444;");
    m_dashboardPanel->addLogEntry(QString("KSPR CLI finalizado con código: %1").arg(exitCode), "warning");
}

void MainWindow::onProcessError(const QString &error)
{
    m_connectionLabel->setText("Error");
    m_connectionLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #ef4444;");
    m_dashboardPanel->addLogEntry(error, "error");
}

void MainWindow::onConnectionEstablished()
{
    m_connectionLabel->setText("● Conectado");
    m_connectionLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #10b981;");
    m_dashboardPanel->addLogEntry("Conexión establecida con KSPR CLI", "success");
}
