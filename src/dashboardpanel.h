#ifndef DASHBOARDPANEL_H
#define DASHBOARDPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>
#include <QTimer>
#include <QJsonObject>
#include <QVector>

class ProcessManager;

struct MetricData {
    QString label;
    QString value;
    QString hint;
};

class DashboardPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPanel(ProcessManager *processManager, QWidget *parent = nullptr);
    ~DashboardPanel();

    void updateMetrics(const QJsonObject &metrics);
    void addLogEntry(const QString &message, const QString &level = "info");
    void clearLogs();

public slots:
    void refreshMetrics();

signals:
    void refreshRequested();

private:
    void setupUI();
    void createMetricCards();
    void updateMetricCard(int index, const QString &value);
    QWidget* createMetricWidget(const QString &label, const QString &value, const QString &hint);

    ProcessManager *m_processManager;
    QVBoxLayout *m_mainLayout;
    QVector<QLabel*> m_metricValues;
    QProgressBar *m_progressBar;
    QTextEdit *m_logDisplay;
    QLabel *m_statusLabel;
    QTimer *m_refreshTimer;
    QVector<MetricData> m_metrics;
};

#endif // DASHBOARDPANEL_H
