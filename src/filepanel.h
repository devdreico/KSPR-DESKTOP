#ifndef FILEPANEL_H
#define FILEPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QJsonObject>
#include <QVector>

class ProcessManager;

struct FileInfo {
    QString path;
    QString content;
    int lineCount;
};

class FilePanel : public QWidget
{
    Q_OBJECT

public:
    explicit FilePanel(ProcessManager *processManager, QWidget *parent = nullptr);
    ~FilePanel();

    void addFile(const QString &path, const QString &content);
    void removeFile(const QString &path);
    void clearFiles();
    QVector<FileInfo> getFiles() const;

signals:
    void fileSelected(const QString &path);
    void fileRemoved(const QString &path);

private slots:
    void onFileClicked(QListWidgetItem *item);
    void onAddFileClicked();
    void onAddFolderClicked();
    void onRemoveFileClicked();
    void onRefreshClicked();

private:
    void setupUI();
    void updateFileList();
    void updatePreview(const QString &content);

    ProcessManager *m_processManager;
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    QListWidget *m_fileList;
    QTextEdit *m_preview;
    QLabel *m_statusLabel;
    QVector<FileInfo> m_files;
};

#endif // FILEPANEL_H
