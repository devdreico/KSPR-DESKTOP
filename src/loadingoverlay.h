#ifndef LOADINGOVERLAY_H
#define LOADINGOVERLAY_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class LoadingOverlay : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal rotation READ rotation WRITE setRotation)

public:
    explicit LoadingOverlay(QWidget *parent = nullptr);
    ~LoadingOverlay();

    void showOverlay(const QString &message = "Cargando...");
    void hideOverlay();
    bool isShowing() const { return m_showing; }

    qreal rotation() const { return m_rotation; }
    void setRotation(qreal rotation);

signals:
    void shown();
    void hidden();

private slots:
    void animateRotation();

private:
    void setupUI();

    bool m_showing;
    qreal m_rotation;
    QLabel *m_messageLabel;
    QLabel *m_spinnerLabel;
    QTimer *m_animationTimer;
};

#endif // LOADINGOVERLAY_H
