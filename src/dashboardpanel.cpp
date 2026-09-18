#include "dashboardpanel.h"
#include "processmanager.h"
#include <QGridLayout>
#include <QScrollArea>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

DashboardPanel::DashboardPanel(ProcessManager *processManager, QWidget *parent)
    : QWidget(parent)
    , m_processManager(processManager)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();

    connect(m_refreshTimer, &QTimer::timeout,
            this, &DashboardPanel::refreshMetrics);
    m_refreshTimer->start(5000);

    connect(m_processManager, &ProcessManager::responseReceived,
            this, [this](const QJsonObject &response) {
                if (response.contains("metrics")) {
                    updateMetrics(response["metrics"].toObject());
                }
            });
}

DashboardPanel::~DashboardPanel()
{
}

void DashboardPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *titleLabel = new QLabel("Dashboard", header);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #141414;");

    m_statusLabel = new QLabel("En línea", header);
    m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #10b981;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel);

    header->setStyleSheet("background: rgba(255, 254, 251, 0.7); border-bottom: 1px solid #d8d7d1;");

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *contentWidget = new QWidget();
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(16, 16, 16, 16);
    contentLayout->setSpacing(16);

    QWidget *metricsGrid = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(metricsGrid);
    gridLayout->setSpacing(12);

    m_metrics = {
        {"Mensajes", "0", "Total en sesión"},
        {"Archivos", "0", "Contexto activo"},
        {"Tokens", "0", "Consumidos"},
        {"Latencia", "0ms", "Promedio"},
        {"Uptime", "00:00", "Tiempo activo"}
    };

    for (int i = 0; i < m_metrics.size(); ++i) {
        QWidget *card = createMetricWidget(
            m_metrics[i].label,
            m_metrics[i].value,
            m_metrics[i].hint
        );
        m_metricValues.append(card->findChild<QLabel*>("value"));
        gridLayout->addWidget(card, i / 3, i % 3);
    }

    QLabel *metricsTitle = new QLabel("MÉTRICAS");
    metricsTitle->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873; letter-spacing: 0.08em; margin-bottom: 8px;");

    contentLayout->addWidget(metricsTitle);
    contentLayout->addWidget(metricsGrid);

    QLabel *progressTitle = new QLabel("PROGRESO DEL ANÁLISIS");
    progressTitle->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873; letter-spacing: 0.08em; margin-top: 8px; margin-bottom: 8px;");

    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat("%p%");
    m_progressBar->setFixedHeight(24);
    m_progressBar->setProperty("class", "progress-bar");

    contentLayout->addWidget(progressTitle);
    contentLayout->addWidget(m_progressBar);

    QLabel *logsTitle = new QLabel("LOGS RECIENTES");
    logsTitle->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873; letter-spacing: 0.08em; margin-top: 16px; margin-bottom: 8px;");

    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setMaximumHeight(200);
    m_logDisplay->setStyleSheet(
        "background: rgba(255, 254, 251, 0.5); "
        "border: 1px solid #d8d7d1; "
        "border-radius: 8px; "
        "font-family: 'DM Mono', monospace; "
        "font-size: 11px; "
        "color: #141414; "
        "padding: 12px;"
    );

    contentLayout->addWidget(logsTitle);
    contentLayout->addWidget(m_logDisplay);
    contentLayout->addStretch();

    scrollArea->setWidget(contentWidget);

    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(scrollArea, 1);
}

void DashboardPanel::createMetricCards()
{
}

QWidget* DashboardPanel::createMetricWidget(const QString &label, const QString &value, const QString &hint)
{
    QWidget *card = new QWidget();
    card->setProperty("class", "metric-card");
    card->setStyleSheet(
        "QWidget[class='metric-card'] { "
        "background: rgba(255, 254, 251, 0.7); "
        "border: 1px solid rgba(255, 255, 255, 0.3); "
        "border-radius: 12px; "
        "padding: 16px; "
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setSpacing(4);

    QLabel *labelWidget = new QLabel(label, card);
    labelWidget->setStyleSheet(
        "font-family: 'DM Mono', monospace; "
        "font-size: 10px; "
        "color: #777873; "
        "letter-spacing: 0.05em;"
    );

    QLabel *valueWidget = new QLabel(value, card);
    valueWidget->setObjectName("value");
    valueWidget->setStyleSheet(
        "font-size: 28px; "
        "font-weight: bold; "
        "color: #141414;"
    );

    QLabel *hintWidget = new QLabel(hint, card);
    hintWidget->setStyleSheet(
        "font-family: 'DM Mono', monospace; "
        "font-size: 10px; "
        "color: #aaa9a4;"
    );

    layout->addWidget(labelWidget);
    layout->addWidget(valueWidget);
    layout->addWidget(hintWidget);

    return card;
}

void DashboardPanel::updateMetrics(const QJsonObject &metrics)
{
    if (metrics.contains("messages")) {
        updateMetricCard(0, metrics["messages"].toString());
    }
    if (metrics.contains("files")) {
        updateMetricCard(1, metrics["files"].toString());
    }
    if (metrics.contains("tokens")) {
        updateMetricCard(2, metrics["tokens"].toString());
    }
    if (metrics.contains("latency")) {
        updateMetricCard(3, metrics["latency"].toString());
    }
    if (metrics.contains("uptime")) {
        updateMetricCard(4, metrics["uptime"].toString());
    }
}

void DashboardPanel::updateMetricCard(int index, const QString &value)
{
    if (index >= 0 && index < m_metricValues.size()) {
        m_metricValues[index]->setText(value);
    }
}

void DashboardPanel::addLogEntry(const QString &message, const QString &level)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString color;

    if (level == "error") {
        color = "#ef4444";
    } else if (level == "warning") {
        color = "#f59e0b";
    } else if (level == "success") {
        color = "#10b981";
    } else {
        color = "#777873";
    }

    QString html = QString(
        "<span style='color: #777873;'>[%1]</span> "
        "<span style='color: %2;'>[%3]</span> "
        "<span style='color: #141414;'>%4</span>"
    ).arg(timestamp, color, level.toUpper(), message.toHtmlEscaped());

    m_logDisplay->append(html);
}

void DashboardPanel::clearLogs()
{
    m_logDisplay->clear();
}

void DashboardPanel::refreshMetrics()
{
    if (m_processManager->isRunning()) {
        m_processManager->sendCommand("status");
        m_statusLabel->setText("En línea");
        m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #10b981;");
    } else {
        m_statusLabel->setText("Desconectado");
        m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #ef4444;");
    }

    emit refreshRequested();
}
