#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QVector>

enum class NotificationType {
    Info,
    Success,
    Warning,
    Error
};

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    explicit NotificationManager(QWidget *parentWidget, QObject *parent = nullptr);
    ~NotificationManager();

    void showNotification(const QString &message, NotificationType type = NotificationType::Info,
                         int durationMs = 3000);
    void showInfo(const QString &message);
    void showSuccess(const QString &message);
    void showWarning(const QString &message);
    void showError(const QString &message);
    void updateGeometry(const QRect &rect);

private slots:
    void removeNotification(QLabel *label);

private:
    QWidget *m_parentWidget;
    QVBoxLayout *m_containerLayout;
    QWidget *m_container;
    QVector<QLabel*> m_notifications;

    QLabel *createNotificationWidget(const QString &message, NotificationType type);
    QString getStyleSheet(NotificationType type);
    QString getIcon(NotificationType type);
};

#endif // NOTIFICATIONMANAGER_H
