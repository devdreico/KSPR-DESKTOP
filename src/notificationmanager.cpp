#include "notificationmanager.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

NotificationManager::NotificationManager(QWidget *parentWidget, QObject *parent)
    : QObject(parent)
    , m_parentWidget(parentWidget)
{
    m_container = new QWidget(parentWidget);
    m_container->setStyleSheet("background: transparent; border: none;");
    m_container->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(16, 16, 16, 16);
    m_containerLayout->setSpacing(8);
    m_containerLayout->addStretch();

    m_container->setGeometry(parentWidget->rect());
    m_container->show();
    m_container->raise();
}

NotificationManager::~NotificationManager()
{
}

void NotificationManager::showNotification(const QString &message, NotificationType type, int durationMs)
{
    QLabel *notification = createNotificationWidget(message, type);

    m_notifications.append(notification);
    m_containerLayout->insertWidget(0, notification);

    notification->setWindowOpacity(0);
    QPropertyAnimation *fadeIn = new QPropertyAnimation(notification, "windowOpacity");
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(durationMs, this, [this, notification]() {
        removeNotification(notification);
    });
}

void NotificationManager::showInfo(const QString &message)
{
    showNotification(message, NotificationType::Info);
}

void NotificationManager::showSuccess(const QString &message)
{
    showNotification(message, NotificationType::Success);
}

void NotificationManager::showWarning(const QString &message)
{
    showNotification(message, NotificationType::Warning);
}

void NotificationManager::showError(const QString &message)
{
    showNotification(message, NotificationType::Error, 5000);
}

void NotificationManager::updateGeometry(const QRect &rect)
{
    if (m_container) {
        m_container->setGeometry(rect);
    }
}

void NotificationManager::removeNotification(QLabel *label)
{
    if (!label) return;

    QPropertyAnimation *fadeOut = new QPropertyAnimation(label, "windowOpacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this, label]() {
        m_containerLayout->removeWidget(label);
        m_notifications.removeOne(label);
        label->deleteLater();
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

QLabel *NotificationManager::createNotificationWidget(const QString &message, NotificationType type)
{
    QLabel *label = new QLabel(m_container);
    label->setText(getIcon(type) + "  " + message);
    label->setWordWrap(true);
    label->setMinimumHeight(48);
    label->setMaximumWidth(400);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setContentsMargins(16, 12, 16, 12);
    label->setStyleSheet(getStyleSheet(type));

    return label;
}

QString NotificationManager::getStyleSheet(NotificationType type)
{
    QString baseStyle =
        "QLabel {"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   border-radius: 10px;"
        "   padding: 12px 16px;"
        "}";

    switch (type) {
        case NotificationType::Info:
            return baseStyle +
                "color: #1e40af;"
                "background: rgba(59, 130, 246, 0.1);"
                "border: 1px solid rgba(59, 130, 246, 0.3);";

        case NotificationType::Success:
            return baseStyle +
                "color: #065f46;"
                "background: rgba(16, 185, 129, 0.1);"
                "border: 1px solid rgba(16, 185, 129, 0.3);";

        case NotificationType::Warning:
            return baseStyle +
                "color: #92400e;"
                "background: rgba(245, 158, 11, 0.1);"
                "border: 1px solid rgba(245, 158, 11, 0.3);";

        case NotificationType::Error:
            return baseStyle +
                "color: #991b1b;"
                "background: rgba(239, 68, 68, 0.1);"
                "border: 1px solid rgba(239, 68, 68, 0.3);";
    }

    return baseStyle;
}

QString NotificationManager::getIcon(NotificationType type)
{
    switch (type) {
        case NotificationType::Info: return "\u2139";
        case NotificationType::Success: return "\u2713";
        case NotificationType::Warning: return "\u26A0";
        case NotificationType::Error: return "\u2717";
    }
    return "";
}
