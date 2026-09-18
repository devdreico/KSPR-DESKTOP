#include "settingspanel.h"
#include <QScrollArea>
#include <QSettings>
#include <QDir>

SettingsPanel::SettingsPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    loadSettings();
}

SettingsPanel::~SettingsPanel()
{
}

void SettingsPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(24, 16, 24, 16);

    QLabel *titleLabel = new QLabel("Configuracion", header);
    titleLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 16px;"
        "font-weight: 600;"
        "color: #141414;"
    );

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    header->setStyleSheet(
        "background: rgba(255, 254, 251, 0.7);"
        "border-bottom: 1px solid #d8d7d1;"
    );

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("background: transparent; border: none;");

    QWidget *scrollContent = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(24, 24, 24, 24);
    scrollLayout->setSpacing(24);

    scrollLayout->addWidget(createGeneralSection());
    scrollLayout->addWidget(createAppearanceSection());
    scrollLayout->addWidget(createConnectionSection());
    scrollLayout->addWidget(createAdvancedSection());
    scrollLayout->addStretch();

    QWidget *buttonBar = new QWidget(scrollContent);
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonBar);
    buttonLayout->setContentsMargins(0, 16, 0, 0);
    buttonLayout->setSpacing(12);

    m_resetButton = new QPushButton("Restablecer", buttonBar);
    m_resetButton->setMinimumWidth(120);
    m_resetButton->setMinimumHeight(40);
    m_resetButton->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   color: #777873;"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "   border-color: #ef4444;"
        "   color: #ef4444;"
        "}"
    );

    m_applyButton = new QPushButton("Aplicar", buttonBar);
    m_applyButton->setMinimumWidth(120);
    m_applyButton->setMinimumHeight(40);
    m_applyButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6366f1, stop:1 #4f46e5);"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #818cf8, stop:1 #6366f1);"
        "}"
    );

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addWidget(m_applyButton);

    scrollLayout->addWidget(buttonBar);

    scrollArea->setWidget(scrollContent);

    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(scrollArea);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsPanel::onThemeChanged);
    connect(m_applyButton, &QPushButton::clicked, this, &SettingsPanel::onApplyClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsPanel::onResetClicked);
}

QWidget *SettingsPanel::createSection(const QString &title, QWidget *content)
{
    QWidget *section = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(section);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    QLabel *sectionTitle = new QLabel(title, section);
    sectionTitle->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "font-weight: bold;"
        "color: #6366f1;"
        "letter-spacing: 1px;"
        "text-transform: uppercase;"
    );

    layout->addWidget(sectionTitle);
    layout->addWidget(content);

    return section;
}

QWidget *SettingsPanel::createGeneralSection()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    content->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 12px;"
        "}"
    );

    m_autoStartCheck = new QCheckBox("Iniciar KSPR CLI automaticamente", content);
    m_autoStartCheck->setStyleSheet(
        "QCheckBox {"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "   spacing: 8px;"
        "}"
        "QCheckBox::indicator {"
        "   width: 18px;"
        "   height: 18px;"
        "   border-radius: 4px;"
        "   border: 2px solid #d8d7d1;"
        "   background: white;"
        "}"
        "QCheckBox::indicator:checked {"
        "   background: #6366f1;"
        "   border-color: #6366f1;"
        "}"
    );

    m_notificationsCheck = new QCheckBox("Habilitar notificaciones", content);
    m_notificationsCheck->setStyleSheet(m_autoStartCheck->styleSheet());

    m_loggingCheck = new QCheckBox("Habilitar registro de actividad", content);
    m_loggingCheck->setStyleSheet(m_autoStartCheck->styleSheet());

    layout->addWidget(m_autoStartCheck);
    layout->addWidget(m_notificationsCheck);
    layout->addWidget(m_loggingCheck);

    return content;
}

QWidget *SettingsPanel::createAppearanceSection()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    content->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 12px;"
        "}"
    );

    QWidget *themeRow = new QWidget(content);
    QHBoxLayout *themeLayout = new QHBoxLayout(themeRow);
    themeLayout->setContentsMargins(0, 0, 0, 0);
    themeLayout->setSpacing(12);

    QLabel *themeLabel = new QLabel("Tema", themeRow);
    themeLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 13px;"
        "color: #141414;"
    );

    m_themeCombo = new QComboBox(themeRow);
    m_themeCombo->addItems({"Claro", "Oscuro"});
    m_themeCombo->setMinimumHeight(36);
    m_themeCombo->setStyleSheet(
        "QComboBox {"
        "   background: rgba(255, 254, 251, 0.9);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "}"
        "QComboBox:hover {"
        "   border-color: #6366f1;"
        "}"
        "QComboBox::drop-down {"
        "   border: none;"
        "   width: 24px;"
        "}"
    );

    themeLayout->addWidget(themeLabel);
    themeLayout->addStretch();
    themeLayout->addWidget(m_themeCombo);

    QWidget *langRow = new QWidget(content);
    QHBoxLayout *langLayout = new QHBoxLayout(langRow);
    langLayout->setContentsMargins(0, 0, 0, 0);
    langLayout->setSpacing(12);

    QLabel *langLabel = new QLabel("Idioma", langRow);
    langLabel->setStyleSheet(themeLabel->styleSheet());

    m_languageCombo = new QComboBox(langRow);
    m_languageCombo->addItems({"Espanol", "English"});
    m_languageCombo->setMinimumHeight(36);
    m_languageCombo->setStyleSheet(m_themeCombo->styleSheet());

    langLayout->addWidget(langLabel);
    langLayout->addStretch();
    langLayout->addWidget(m_languageCombo);

    layout->addWidget(themeRow);
    layout->addWidget(langRow);

    return content;
}

QWidget *SettingsPanel::createConnectionSection()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    content->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 12px;"
        "}"
    );

    QWidget *cliRow = new QWidget(content);
    QVBoxLayout *cliLayout = new QVBoxLayout(cliRow);
    cliLayout->setContentsMargins(0, 0, 0, 0);
    cliLayout->setSpacing(8);

    QLabel *cliLabel = new QLabel("Ruta de KSPR CLI", cliRow);
    cliLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 13px;"
        "color: #141414;"
    );

    m_cliPathEdit = new QLineEdit(cliRow);
    m_cliPathEdit->setPlaceholderText("./bin/kspr");
    m_cliPathEdit->setMinimumHeight(36);
    m_cliPathEdit->setStyleSheet(
        "QLineEdit {"
        "   background: rgba(255, 254, 251, 0.9);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #6366f1;"
        "}"
    );

    cliLayout->addWidget(cliLabel);
    cliLayout->addWidget(m_cliPathEdit);

    QWidget *portRow = new QWidget(content);
    QHBoxLayout *portLayout = new QHBoxLayout(portRow);
    portLayout->setContentsMargins(0, 0, 0, 0);
    portLayout->setSpacing(12);

    QLabel *portLabel = new QLabel("Puerto del servidor", portRow);
    portLabel->setStyleSheet(cliLabel->styleSheet());

    m_portSpin = new QSpinBox(portRow);
    m_portSpin->setRange(1024, 65535);
    m_portSpin->setValue(8000);
    m_portSpin->setMinimumHeight(36);
    m_portSpin->setStyleSheet(
        "QSpinBox {"
        "   background: rgba(255, 254, 251, 0.9);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "}"
        "QSpinBox:focus {"
        "   border-color: #6366f1;"
        "}"
    );

    portLayout->addWidget(portLabel);
    portLayout->addStretch();
    portLayout->addWidget(m_portSpin);

    layout->addWidget(cliRow);
    layout->addWidget(portRow);

    return content;
}

QWidget *SettingsPanel::createAdvancedSection()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    content->setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.7);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 12px;"
        "}"
    );

    QLabel *logLabel = new QLabel("Directorio de registros", content);
    logLabel->setStyleSheet(
        "font-family: 'Montserrat', sans-serif;"
        "font-size: 13px;"
        "color: #141414;"
    );

    m_logPathEdit = new QLineEdit(content);
    m_logPathEdit->setPlaceholderText(QDir::homePath() + "/.ksrp/logs");
    m_logPathEdit->setMinimumHeight(36);
    m_logPathEdit->setStyleSheet(
        "QLineEdit {"
        "   background: rgba(255, 254, 251, 0.9);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 8px;"
        "   padding: 8px 12px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #6366f1;"
        "}"
    );

    layout->addWidget(logLabel);
    layout->addWidget(m_logPathEdit);

    return content;
}

void SettingsPanel::loadSettings()
{
    QSettings settings("KSRP", "Desktop");

    m_themeCombo->setCurrentText(settings.value("theme", "Claro").toString());
    m_languageCombo->setCurrentText(settings.value("language", "Espanol").toString());
    m_cliPathEdit->setText(settings.value("cliPath", "./bin/kspr").toString());
    m_portSpin->setValue(settings.value("port", 8000).toInt());
    m_autoStartCheck->setChecked(settings.value("autoStart", true).toBool());
    m_notificationsCheck->setChecked(settings.value("notifications", true).toBool());
    m_loggingCheck->setChecked(settings.value("logging", false).toBool());
    m_logPathEdit->setText(settings.value("logPath", QDir::homePath() + "/.ksrp/logs").toString());
}

void SettingsPanel::saveSettings()
{
    QSettings settings("KSRP", "Desktop");

    settings.setValue("theme", m_themeCombo->currentText());
    settings.setValue("language", m_languageCombo->currentText());
    settings.setValue("cliPath", m_cliPathEdit->text());
    settings.setValue("port", m_portSpin->value());
    settings.setValue("autoStart", m_autoStartCheck->isChecked());
    settings.setValue("notifications", m_notificationsCheck->isChecked());
    settings.setValue("logging", m_loggingCheck->isChecked());
    settings.setValue("logPath", m_logPathEdit->text());

    emit settingsChanged();
}

void SettingsPanel::onThemeChanged(int index)
{
    QString theme = (index == 0) ? "light" : "dark";
    emit themeChanged(theme);
}

void SettingsPanel::onApplyClicked()
{
    saveSettings();
}

void SettingsPanel::onResetClicked()
{
    m_themeCombo->setCurrentIndex(0);
    m_languageCombo->setCurrentIndex(0);
    m_cliPathEdit->setText("./bin/kspr");
    m_portSpin->setValue(8000);
    m_autoStartCheck->setChecked(true);
    m_notificationsCheck->setChecked(true);
    m_loggingCheck->setChecked(false);
    m_logPathEdit->setText(QDir::homePath() + "/.ksrp/logs");
}
