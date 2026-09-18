#ifndef CHATPANEL_H
#define CHATPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QJsonObject>
#include <QVector>

class ProcessManager;

struct ChatMessage {
    QString role;
    QString content;
    QString timestamp;
};

class ChatPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPanel(ProcessManager *processManager, QWidget *parent = nullptr);
    ~ChatPanel();

    void appendMessage(const QString &role, const QString &content);
    void clearChat();
    QString getChatHistory() const;

signals:
    void commandSent(const QString &command);

private slots:
    void onSendClicked();
    void onInputReturnPressed();
    void onResponseReceived(const QJsonObject &response);
    void onProcessOutput(const QString &output);

private:
    void setupUI();
    void scrollToBottom();
    void formatAndAppendMessage(const QString &role, const QString &content);

    ProcessManager *m_processManager;
    QVBoxLayout *m_mainLayout;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_input;
    QPushButton *m_sendButton;
    QLabel *m_statusLabel;
    QVector<ChatMessage> m_messages;
};

#endif // CHATPANEL_H
