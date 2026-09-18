#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget *parent = nullptr);
    ~SettingsPanel();

    void loadSettings();
    void saveSettings();

signals:
    void themeChanged(const QString &theme);
    void settingsChanged();

private slots:
    void onThemeChanged(int index);
    void onResetClicked();
    void onApplyClicked();

private:
    void setupUI();
    QWidget *createSection(const QString &title, QWidget *content);
    QWidget *createGeneralSection();
    QWidget *createAppearanceSection();
    QWidget *createConnectionSection();
    QWidget *createAdvancedSection();

    QVBoxLayout *m_mainLayout;
    QComboBox *m_themeCombo;
    QComboBox *m_languageCombo;
    QLineEdit *m_cliPathEdit;
    QSpinBox *m_portSpin;
    QCheckBox *m_autoStartCheck;
    QCheckBox *m_notificationsCheck;
    QCheckBox *m_loggingCheck;
    QLineEdit *m_logPathEdit;
    QPushButton *m_applyButton;
    QPushButton *m_resetButton;
};

#endif // SETTINGSPANEL_H
