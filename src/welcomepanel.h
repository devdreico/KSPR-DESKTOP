#ifndef WELCOMEPANEL_H
#define WELCOMEPANEL_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

class WelcomePanel : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomePanel(QWidget *parent = nullptr);
    ~WelcomePanel();

    void setSetupStep(int step, bool completed);
    void setStatus(const QString &status);

signals:
    void startClicked();
    void skipClicked();

private:
    void setupUI();
    QWidget *createStepWidget(int number, const QString &title, const QString &description);
    void animateEntrance();

    QVBoxLayout *m_mainLayout;
    QLabel *m_statusLabel;
    QWidget *m_stepsContainer;
    QPushButton *m_startButton;
    QPushButton *m_skipButton;

    QWidget *m_step1;
    QWidget *m_step2;
    QWidget *m_step3;
    QLabel *m_step1Indicator;
    QLabel *m_step2Indicator;
    QLabel *m_step3Indicator;
};

#endif // WELCOMEPANEL_H
