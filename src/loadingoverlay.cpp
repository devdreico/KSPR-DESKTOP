#include "loadingoverlay.h"
#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>

LoadingOverlay::LoadingOverlay(QWidget *parent)
    : QWidget(parent)
    , m_showing(false)
    , m_rotation(0)
{
    setupUI();
}

LoadingOverlay::~LoadingOverlay()
{
}

void LoadingOverlay::setupUI()
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    hide();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignCenter);

    QWidget *card = new QWidget(this);
    card->setFixedSize(200, 120);
    card->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.95);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 16px;"
        "}"
    );

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 50));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setAlignment(Qt::AlignCenter);
    cardLayout->setSpacing(12);

    m_spinnerLabel = new QLabel(card);
    m_spinnerLabel->setFixedSize(32, 32);
    m_spinnerLabel->setAlignment(Qt::AlignCenter);
    m_spinnerLabel->setText("\u25CB");
    m_spinnerLabel->setStyleSheet(
        "font-size: 28px;"
        "color: #6366f1;"
    );

    m_messageLabel = new QLabel("Cargando...", card);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 13px;"
        "color: #777873;"
    );

    cardLayout->addWidget(m_spinnerLabel, 0, Qt::AlignCenter);
    cardLayout->addWidget(m_messageLabel, 0, Qt::AlignCenter);

    layout->addWidget(card, 0, Qt::AlignCenter);

    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, &LoadingOverlay::animateRotation);
}

void LoadingOverlay::showOverlay(const QString &message)
{
    m_messageLabel->setText(message);
    m_showing = true;
    show();
    raise();
    m_animationTimer->start(50);
    emit shown();
}

void LoadingOverlay::hideOverlay()
{
    m_showing = false;
    hide();
    m_animationTimer->stop();
    emit hidden();
}

void LoadingOverlay::animateRotation()
{
    m_rotation += 10;
    if (m_rotation >= 360) m_rotation = 0;

    m_spinnerLabel->setStyleSheet(
        QString("font-size: 28px; color: #6366f1; transform: rotate(%1deg);").arg(m_rotation)
    );
}

void LoadingOverlay::setRotation(qreal rotation)
{
    m_rotation = rotation;
}
