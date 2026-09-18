#include "welcomepanel.h"
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>

WelcomePanel::WelcomePanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    animateEntrance();
}

WelcomePanel::~WelcomePanel()
{
}

void WelcomePanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *centerContainer = new QWidget(this);
    centerContainer->setMaximumWidth(600);
    centerContainer->setMinimumWidth(400);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(40, 60, 40, 40);
    centerLayout->setSpacing(0);

    QLabel *logoContainer = new QLabel(centerContainer);
    logoContainer->setPixmap(QPixmap(":/kspr-main-logo.png").scaled(
        120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoContainer->setAlignment(Qt::AlignCenter);
    logoContainer->setStyleSheet("margin-bottom: 24px;");

    QLabel *titleLabel = new QLabel("Bienvenido a KSPR Desktop", centerContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 28px;"
        "font-weight: 700;"
        "color: #141414;"
        "margin-bottom: 8px;"
    );

    QLabel *subtitleLabel = new QLabel(
        "Tu asistente de IA para análisis de código está a punto de estar listo.",
        centerContainer);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 14px;"
        "color: #777873;"
        "line-height: 1.6;"
        "margin-bottom: 40px;"
    );

    m_stepsContainer = new QWidget(centerContainer);
    QVBoxLayout *stepsLayout = new QVBoxLayout(m_stepsContainer);
    stepsLayout->setContentsMargins(0, 0, 0, 0);
    stepsLayout->setSpacing(16);

    m_step1 = createStepWidget(1, "Verificar KSPR CLI",
        "Comprobando si la línea de comandos de KSPR está instalada en tu sistema.");
    m_step1Indicator = m_step1->findChild<QLabel*>("indicator");

    m_step2 = createStepWidget(2, "Instalar dependencias",
        "Descargando e instalando los componentes necesarios automáticamente.");
    m_step2Indicator = m_step2->findChild<QLabel*>("indicator");

    m_step3 = createStepWidget(3, "Iniciar servidor",
        "Conectándose al servidor local de KSPR para comenzar a trabajar.");
    m_step3Indicator = m_step3->findChild<QLabel*>("indicator");

    stepsLayout->addWidget(m_step1);
    stepsLayout->addWidget(m_step2);
    stepsLayout->addWidget(m_step3);

    m_statusLabel = new QLabel(centerContainer);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 12px;"
        "color: #777873;"
        "padding: 12px 16px;"
        "background: rgba(255, 254, 251, 0.7);"
        "border: 1px solid #d8d7d1;"
        "border-radius: 8px;"
        "margin-top: 24px;"
    );
    m_statusLabel->setText("Preparando entorno...");

    QWidget *buttonContainer = new QWidget(centerContainer);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonContainer);
    buttonLayout->setContentsMargins(0, 24, 0, 0);
    buttonLayout->setSpacing(12);
    buttonLayout->setAlignment(Qt::AlignCenter);

    m_startButton = new QPushButton("Iniciar sesion", buttonContainer);
    m_startButton->setMinimumWidth(160);
    m_startButton->setMinimumHeight(44);
    m_startButton->setEnabled(false);
    m_startButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6366f1, stop:1 #4f46e5);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 10px;"
        "   padding: 12px 24px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #818cf8, stop:1 #6366f1);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #4f46e5, stop:1 #4338ca);"
        "}"
        "QPushButton:disabled {"
        "   background: #d8d7d1;"
        "   color: #777873;"
        "}"
    );

    m_skipButton = new QPushButton("Omitir", buttonContainer);
    m_skipButton->setMinimumWidth(100);
    m_skipButton->setMinimumHeight(44);
    m_skipButton->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #777873;"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 10px;"
        "   padding: 12px 24px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border-color: #6366f1;"
        "   color: #6366f1;"
        "}"
    );

    buttonLayout->addWidget(m_skipButton);
    buttonLayout->addWidget(m_startButton);

    centerLayout->addStretch();
    centerLayout->addWidget(logoContainer);
    centerLayout->addWidget(titleLabel);
    centerLayout->addWidget(subtitleLabel);
    centerLayout->addWidget(m_stepsContainer);
    centerLayout->addWidget(m_statusLabel);
    centerLayout->addLayout(buttonLayout);
    centerLayout->addStretch();

    m_mainLayout->addWidget(centerContainer, 0, Qt::AlignCenter);

    connect(m_startButton, &QPushButton::clicked, this, &WelcomePanel::startClicked);
    connect(m_skipButton, &QPushButton::clicked, this, &WelcomePanel::skipClicked);
}

QWidget *WelcomePanel::createStepWidget(int number, const QString &title, const QString &description)
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignLeft);

    widget->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 12px;"
        "}"
        "QWidget:hover {"
        "   border-color: #818cf8;"
        "}"
    );

    QLabel *indicator = new QLabel(QString::number(number), widget);
    indicator->setObjectName("indicator");
    indicator->setFixedSize(32, 32);
    indicator->setAlignment(Qt::AlignCenter);
    indicator->setStyleSheet(
        "background: #d8d7d1;"
        "color: #777873;"
        "border-radius: 16px;"
        "font-family: 'DM Mono', monospace;"
        "font-size: 12px;"
        "font-weight: bold;"
    );

    QLabel *titleLabel = new QLabel(title, widget);
    titleLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 14px;"
        "font-weight: 600;"
        "color: #141414;"
    );

    QLabel *descLabel = new QLabel(description, widget);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 12px;"
        "color: #777873;"
    );

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(4);
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);

    layout->addWidget(indicator);
    layout->addLayout(textLayout);

    return widget;
}

void WelcomePanel::setSetupStep(int step, bool completed)
{
    QLabel *indicator = nullptr;
    if (step == 1) indicator = m_step1Indicator;
    else if (step == 2) indicator = m_step2Indicator;
    else if (step == 3) indicator = m_step3Indicator;

    if (!indicator) return;

    if (completed) {
        indicator->setStyleSheet(
            "background: #10b981;"
            "color: white;"
            "border-radius: 16px;"
            "font-family: 'DM Mono', monospace;"
            "font-size: 14px;"
            "font-weight: bold;"
        );
        indicator->setText("\u2713");
    } else {
        indicator->setStyleSheet(
            "background: #6366f1;"
            "color: white;"
            "border-radius: 16px;"
            "font-family: 'DM Mono', monospace;"
            "font-size: 12px;"
            "font-weight: bold;"
        );
        indicator->setText(QString::number(step));
    }
}

void WelcomePanel::setStatus(const QString &status)
{
    m_statusLabel->setText(status);

    if (status.contains("listo", Qt::CaseInsensitive) ||
        status.contains("completado", Qt::CaseInsensitive)) {
        m_startButton->setEnabled(true);
        m_statusLabel->setStyleSheet(
            "font-family: 'DM Mono', monospace;"
            "font-size: 12px;"
            "color: #10b981;"
            "padding: 12px 16px;"
            "background: rgba(16, 185, 129, 0.1);"
            "border: 1px solid rgba(16, 185, 129, 0.3);"
            "border-radius: 8px;"
            "margin-top: 24px;"
        );
    } else if (status.contains("error", Qt::CaseInsensitive)) {
        m_statusLabel->setStyleSheet(
            "font-family: 'DM Mono', monospace;"
            "font-size: 12px;"
            "color: #ef4444;"
            "padding: 12px 16px;"
            "background: rgba(239, 68, 68, 0.1);"
            "border: 1px solid rgba(239, 68, 68, 0.3);"
            "border-radius: 8px;"
            "margin-top: 24px;"
        );
    }
}

void WelcomePanel::animateEntrance()
{
    setWindowOpacity(0);
    QPropertyAnimation *anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(500);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}
