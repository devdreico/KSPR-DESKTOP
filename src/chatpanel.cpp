#include "chatpanel.h"
#include "processmanager.h"
#include <QScrollBar>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>

ChatPanel::ChatPanel(ProcessManager *processManager, QWidget *parent)
    : QWidget(parent)
    , m_processManager(processManager)
{
    setupUI();

    connect(m_processManager, &ProcessManager::responseReceived,
            this, &ChatPanel::onResponseReceived);
    connect(m_processManager, &ProcessManager::outputReceived,
            this, &ChatPanel::onProcessOutput);
}

ChatPanel::~ChatPanel()
{
}

void ChatPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *titleLabel = new QLabel("Chat con KSPR I", header);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #141414;");

    m_statusLabel = new QLabel("Desconectado", header);
    m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel);

    header->setProperty("class", "glass");
    header->setStyleSheet("background: rgba(255, 254, 251, 0.7); border-bottom: 1px solid #d8d7d1;");

    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setAcceptRichText(true);
    m_chatDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_chatDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_chatDisplay->setProperty("class", "chat-display");

    QWidget *inputArea = new QWidget(this);
    QHBoxLayout *inputLayout = new QHBoxLayout(inputArea);
    inputLayout->setContentsMargins(16, 12, 16, 12);
    inputLayout->setSpacing(8);

    m_input = new QLineEdit(inputArea);
    m_input->setPlaceholderText("Escribe tu mensaje a KSPR I...");
    m_input->setProperty("class", "chat-input");

    m_sendButton = new QPushButton("Enviar", inputArea);
    m_sendButton->setProperty("class", "send-button");
    m_sendButton->setFixedWidth(80);

    inputLayout->addWidget(m_input);
    inputLayout->addWidget(m_sendButton);

    inputArea->setProperty("class", "glass");
    inputArea->setStyleSheet("background: rgba(255, 254, 251, 0.7); border-top: 1px solid #d8d7d1;");

    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(m_chatDisplay, 1);
    m_mainLayout->addWidget(inputArea);

    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatPanel::onSendClicked);
    connect(m_input, &QLineEdit::returnPressed,
            this, &ChatPanel::onInputReturnPressed);
}

void ChatPanel::onSendClicked()
{
    QString text = m_input->text().trimmed();
    if (text.isEmpty()) return;

    appendMessage("user", text);

    if (text.startsWith("/")) {
        emit commandSent(text);
    } else {
        QJsonObject params;
        params["message"] = text;
        m_processManager->sendCommand("chat", params);
    }

    m_input->clear();
    m_statusLabel->setText("Procesando...");
    m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #6366f1;");
}

void ChatPanel::onInputReturnPressed()
{
    onSendClicked();
}

void ChatPanel::onResponseReceived(const QJsonObject &response)
{
    if (response.contains("response")) {
        QString content = response["response"].toString();
        appendMessage("assistant", content);
        m_statusLabel->setText("Listo");
        m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #10b981;");
    } else if (response.contains("error")) {
        QString error = response["error"].toString();
        appendMessage("error", error);
        m_statusLabel->setText("Error");
        m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #ef4444;");
    }
}

void ChatPanel::onProcessOutput(const QString &output)
{
    if (!output.trimmed().isEmpty()) {
        m_statusLabel->setText("Recibiendo...");
    }
}

void ChatPanel::appendMessage(const QString &role, const QString &content)
{
    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = QDateTime::currentDateTime().toString("HH:mm");
    m_messages.append(msg);

    formatAndAppendMessage(role, content);
    scrollToBottom();
}

void ChatPanel::formatAndAppendMessage(const QString &role, const QString &content)
{
    QString html;
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm");

    if (role == "user") {
        html = QString(
            "<div style='margin: 8px 0; text-align: right;'>"
            "<span style='background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6366f1, stop:1 #4f46e5); "
            "color: white; padding: 10px 14px; border-radius: 12px 12px 4px 12px; "
            "display: inline-block; max-width: 70%%; text-align: left; "
            "font-family: Montserrat, sans-serif; font-size: 14px;'>"
            "%2"
            "</span>"
            "<br><span style='font-size: 10px; color: #777873; font-family: DM Mono, monospace;'>%3</span>"
            "</div>"
        ).arg(content.toHtmlEscaped(), timestamp);
    } else if (role == "assistant") {
        html = QString(
            "<div style='margin: 8px 0;'>"
            "<div style='display: flex; align-items: flex-start; gap: 8px;'>"
            "<div style='width: 28px; height: 28px; border-radius: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6366f1, stop:1 #4f46e5); "
            "display: flex; align-items: center; justify-content: center; color: white; font-weight: bold; font-size: 12px;'>K</div>"
            "<div>"
            "<span style='font-weight: 500; font-size: 11px; color: #141414; font-family: DM Mono, monospace;'>KSPR I</span>"
            "<div style='background: rgba(255, 254, 251, 0.7); border: 1px solid #d8d7d1; "
            "border-radius: 4px 12px 12px 12px; padding: 12px 14px; margin-top: 4px; "
            "font-family: Montserrat, sans-serif; font-size: 14px; line-height: 1.6;'>"
            "%2"
            "</div>"
            "<span style='font-size: 10px; color: #777873; font-family: DM Mono, monospace;'>%3</span>"
            "</div>"
            "</div>"
            "</div>"
        ).arg(content.toHtmlEscaped(), timestamp);
    } else if (role == "error") {
        html = QString(
            "<div style='margin: 8px 0; text-align: center;'>"
            "<span style='background: rgba(239, 68, 68, 0.1); color: #ef4444; "
            "padding: 8px 12px; border-radius: 8px; font-size: 12px; "
            "font-family: DM Mono, monospace;'>"
            "Error: %1"
            "</span>"
            "</div>"
        ).arg(content.toHtmlEscaped());
    }

    m_chatDisplay->append(html);
}

void ChatPanel::clearChat()
{
    m_chatDisplay->clear();
    m_messages.clear();
}

QString ChatPanel::getChatHistory() const
{
    QString history;
    for (const ChatMessage &msg : m_messages) {
        history += QString("[%1] %2: %3\n")
            .arg(msg.timestamp, msg.role, msg.content);
    }
    return history;
}

void ChatPanel::scrollToBottom()
{
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
