#include "mainwindow.h"
#include "processmanager.h"
#include "chatpanel.h"
#include "filepanel.h"
#include "dashboardpanel.h"
#include "welcomepanel.h"
#include "settingspanel.h"
#include "notificationmanager.h"
#include "loadingoverlay.h"
#include "styles.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QStatusBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QShortcut>

MainWindow::MainWindow(const QString &cliPath, QWidget *parent)
    : QMainWindow(parent)
    , m_cliPath(cliPath)
    , m_processManager(new ProcessManager(cliPath, this))
    , m_notificationManager(nullptr)
    , m_loadingOverlay(nullptr)
{
    setWindowTitle("KSPR Desktop");
    setMinimumSize(1200, 800);
    resize(1440, 900);

    QIcon icon(":/desktop-app-icon.png");
    setWindowIcon(icon);

    KSPRStyles::applyTheme("light");

    setupUI();
    connectSignals();

    m_loadingOverlay = new LoadingOverlay(centralWidget());

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

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_notificationManager) {
        m_notificationManager->updateGeometry(rect());
    }
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_sidebar = new QWidget();
    m_sidebar->setFixedWidth(240);
    m_sidebar->setProperty("class", "sidebar");
    m_sidebar->setStyleSheet(
        "background: rgba(243, 242, 238, 0.94); "
        "border-right: 1px solid #d8d7d1;"
    );

    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(12, 16, 12, 16);
    sidebarLayout->setSpacing(4);

    QWidget *brandWidget = new QWidget(m_sidebar);
    QHBoxLayout *brandLayout = new QHBoxLayout(brandWidget);
    brandLayout->setContentsMargins(4, 0, 4, 20);

    QLabel *brandIcon = new QLabel(brandWidget);
    brandIcon->setPixmap(QPixmap(":/desktop-app-icon.png").scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    brandIcon->setFixedSize(32, 32);

    QLabel *brandText = new QLabel(brandWidget);
    brandText->setText(
        "<strong style='font-size: 16px; color: #141414;'>KSPR</strong><br>"
        "<span style='font-size: 9px; color: #777873; font-family: DM Mono, monospace;'>Desktop v1.0.0</span>"
    );

    brandLayout->addWidget(brandIcon);
    brandLayout->addWidget(brandText);
    brandLayout->addStretch();

    sidebarLayout->addWidget(brandWidget);

    QLabel *navLabel = new QLabel("NAVEGACIÓN", m_sidebar);
    navLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 9px;"
        "color: #777873;"
        "letter-spacing: 0.1em;"
        "padding: 8px 4px;"
    );

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

    m_chatButton = new QPushButton("  \u2709  Chat con KSPR I", m_sidebar);
    m_chatButton->setObjectName("chatButton");
    m_chatButton->setCheckable(true);
    m_chatButton->setChecked(true);
    m_chatButton->setMinimumHeight(42);
    m_chatButton->setToolTip("Abrir chat con KSPR I (Ctrl+1)");
    m_chatButton->setStyleSheet(navButtonStyle);

    m_filesButton = new QPushButton("  \u2630  Gestor de Archivos", m_sidebar);
    m_filesButton->setObjectName("filesButton");
    m_filesButton->setCheckable(true);
    m_filesButton->setMinimumHeight(42);
    m_filesButton->setToolTip("Gestionar archivos (Ctrl+2)");
    m_filesButton->setStyleSheet(navButtonStyle);

    m_dashboardButton = new QPushButton("  \u25A3  Dashboard", m_sidebar);
    m_dashboardButton->setObjectName("dashboardButton");
    m_dashboardButton->setCheckable(true);
    m_dashboardButton->setMinimumHeight(42);
    m_dashboardButton->setToolTip("Ver metricas (Ctrl+3)");
    m_dashboardButton->setStyleSheet(navButtonStyle);

    sidebarLayout->addWidget(navLabel);
    sidebarLayout->addWidget(m_chatButton);
    sidebarLayout->addWidget(m_filesButton);
    sidebarLayout->addWidget(m_dashboardButton);
    sidebarLayout->addStretch();

    QLabel *toolsLabel = new QLabel("HERRAMIENTAS", m_sidebar);
    toolsLabel->setStyleSheet(navLabel->styleSheet());
    sidebarLayout->addWidget(toolsLabel);

    m_welcomeButton = new QPushButton("  \u2302  Bienvenida", m_sidebar);
    m_welcomeButton->setCheckable(true);
    m_welcomeButton->setMinimumHeight(42);
    m_welcomeButton->setToolTip("Pantalla de bienvenida");
    m_welcomeButton->setStyleSheet(navButtonStyle);

    m_settingsButton = new QPushButton("  \u2699  Configuracion", m_sidebar);
    m_settingsButton->setCheckable(true);
    m_settingsButton->setMinimumHeight(42);
    m_settingsButton->setToolTip("Configuracion (Ctrl+,)");
    m_settingsButton->setStyleSheet(navButtonStyle);

    sidebarLayout->addWidget(m_welcomeButton);
    sidebarLayout->addWidget(m_settingsButton);

    QWidget *statusWidget = new QWidget(m_sidebar);
    statusWidget->setStyleSheet("border-top: 1px solid #d8d7d1; padding-top: 12px;");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusWidget);
    statusLayout->setContentsMargins(4, 8, 4, 0);

    m_connectionLabel = new QLabel("\u25CF Desconectado", statusWidget);
    m_connectionLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #777873;"
    );

    statusLayout->addWidget(m_connectionLabel);

    sidebarLayout->addWidget(statusWidget);

    m_stackedWidget = new QStackedWidget();

    m_chatPanel = new ChatPanel(m_processManager);
    m_filePanel = new FilePanel(m_processManager);
    m_dashboardPanel = new DashboardPanel(m_processManager);
    m_welcomePanel = new WelcomePanel();
    m_settingsPanel = new SettingsPanel();

    m_stackedWidget->addWidget(m_chatPanel);
    m_stackedWidget->addWidget(m_filePanel);
    m_stackedWidget->addWidget(m_dashboardPanel);
    m_stackedWidget->addWidget(m_welcomePanel);
    m_stackedWidget->addWidget(m_settingsPanel);

    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(m_stackedWidget, 1);

    setCentralWidget(centralWidget);

    setupStatusBar();
    setupShortcuts();

    m_notificationManager = new NotificationManager(centralWidget, this);
}

void MainWindow::setupSidebar()
{
}

void MainWindow::setupWelcomePanel()
{
}

void MainWindow::setupSettingsPanel()
{
}

void MainWindow::setupShortcuts()
{
    QShortcut *chatShortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
    connect(chatShortcut, &QShortcut::activated, this, &MainWindow::onChatClicked);

    QShortcut *filesShortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
    connect(filesShortcut, &QShortcut::activated, this, &MainWindow::onFilesClicked);

    QShortcut *dashboardShortcut = new QShortcut(QKeySequence("Ctrl+3"), this);
    connect(dashboardShortcut, &QShortcut::activated, this, &MainWindow::onDashboardClicked);

    QShortcut *welcomeShortcut = new QShortcut(QKeySequence("Ctrl+0"), this);
    connect(welcomeShortcut, &QShortcut::activated, this, &MainWindow::onWelcomeClicked);

    QShortcut *settingsShortcut = new QShortcut(QKeySequence("Ctrl+,"), this);
    connect(settingsShortcut, &QShortcut::activated, this, &MainWindow::onSettingsClicked);

    QShortcut *quitShortcut = new QShortcut(QKeySequence("Ctrl+Q"), this);
    connect(quitShortcut, &QShortcut::activated, this, &MainWindow::close);
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
    m_statusLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #777873;"
    );

    statusBar->addWidget(m_statusLabel);

    QLabel *securityLabel = new QLabel("\u26BF  Secure static analysis \u00B7 No code execution");
    securityLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #777873;"
    );
    statusBar->addPermanentWidget(securityLabel);
}

void MainWindow::connectSignals()
{
    connect(m_chatButton, &QPushButton::clicked, this, &MainWindow::onChatClicked);
    connect(m_filesButton, &QPushButton::clicked, this, &MainWindow::onFilesClicked);
    connect(m_dashboardButton, &QPushButton::clicked, this, &MainWindow::onDashboardClicked);
    connect(m_welcomeButton, &QPushButton::clicked, this, &MainWindow::onWelcomeClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);

    connect(m_processManager, &ProcessManager::started,
            this, &MainWindow::onProcessStarted);
    connect(m_processManager, &ProcessManager::finished,
            this, &MainWindow::onProcessFinished);
    connect(m_processManager, &ProcessManager::errorReceived,
            this, &MainWindow::onProcessError);
    connect(m_processManager, &ProcessManager::connectionEstablished,
            this, &MainWindow::onConnectionEstablished);

    connect(m_settingsPanel, &SettingsPanel::themeChanged,
            this, &MainWindow::onThemeChanged);
}

void MainWindow::updateActiveButton(QPushButton *activeButton)
{
    QList<QPushButton*> buttons = {m_chatButton, m_filesButton, m_dashboardButton,
                                   m_welcomeButton, m_settingsButton};
    for (QPushButton *btn : buttons) {
        if (btn == activeButton) {
            btn->setChecked(true);
        } else {
            btn->setChecked(false);
        }
    }
}

void MainWindow::onChatClicked()
{
    m_stackedWidget->setCurrentWidget(m_chatPanel);
    updateActiveButton(m_chatButton);
}

void MainWindow::onFilesClicked()
{
    m_stackedWidget->setCurrentWidget(m_filePanel);
    updateActiveButton(m_filesButton);
}

void MainWindow::onDashboardClicked()
{
    m_stackedWidget->setCurrentWidget(m_dashboardPanel);
    updateActiveButton(m_dashboardButton);
}

void MainWindow::onWelcomeClicked()
{
    m_stackedWidget->setCurrentWidget(m_welcomePanel);
    updateActiveButton(m_welcomeButton);
}

void MainWindow::onSettingsClicked()
{
    m_stackedWidget->setCurrentWidget(m_settingsPanel);
    updateActiveButton(m_settingsButton);
}

void MainWindow::onProcessStarted()
{
    m_connectionLabel->setText("\u25CF Iniciando...");
    m_connectionLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #f59e0b;"
    );
    m_dashboardPanel->addLogEntry("KSPR CLI iniciado", "info");
    if (m_notificationManager) {
        m_notificationManager->showInfo("KSPR CLI iniciado");
    }
}

void MainWindow::onProcessFinished(int exitCode)
{
    m_connectionLabel->setText("\u25CF Detenido");
    m_connectionLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #ef4444;"
    );
    m_dashboardPanel->addLogEntry(QString("KSPR CLI finalizado con codigo: %1").arg(exitCode), "warning");
    if (m_notificationManager) {
        m_notificationManager->showWarning("KSPR CLI se ha detenido");
    }
}

void MainWindow::onProcessError(const QString &error)
{
    m_connectionLabel->setText("\u25CF Error");
    m_connectionLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #ef4444;"
    );
    m_dashboardPanel->addLogEntry(error, "error");
    if (m_notificationManager) {
        m_notificationManager->showError(error);
    }
}

void MainWindow::onConnectionEstablished()
{
    m_connectionLabel->setText("\u25CF Conectado");
    m_connectionLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #10b981;"
    );
    m_dashboardPanel->addLogEntry("Conexion establecida con KSPR CLI", "success");
    if (m_notificationManager) {
        m_notificationManager->showSuccess("Conexion establecida con KSPR CLI");
    }
}

void MainWindow::onThemeChanged(const QString &theme)
{
    KSPRStyles::applyTheme(theme);
    if (m_notificationManager) {
        m_notificationManager->showInfo(QString("Tema cambiado a: %1").arg(
            theme == "light" ? "Claro" : "Oscuro"));
    }
}
