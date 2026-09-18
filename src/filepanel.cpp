#include "filepanel.h"
#include "processmanager.h"
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FilePanel::FilePanel(ProcessManager *processManager, QWidget *parent)
    : QWidget(parent)
    , m_processManager(processManager)
{
    setupUI();
}

FilePanel::~FilePanel()
{
}

void FilePanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    QWidget *header = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 12, 16, 12);

    QLabel *titleLabel = new QLabel("Gestor de Archivos", header);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #141414;");

    m_statusLabel = new QLabel("0 archivos", header);
    m_statusLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873;");

    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_statusLabel);

    header->setStyleSheet("background: rgba(255, 254, 251, 0.7); border-bottom: 1px solid #d8d7d1;");

    QWidget *toolbar = new QWidget(this);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(12, 8, 12, 8);
    toolbarLayout->setSpacing(6);

    QPushButton *addFileBtn = new QPushButton("+ Archivo", toolbar);
    addFileBtn->setProperty("class", "secondary");
    addFileBtn->setFixedHeight(32);

    QPushButton *addFolderBtn = new QPushButton("+ Carpeta", toolbar);
    addFolderBtn->setProperty("class", "secondary");
    addFolderBtn->setFixedHeight(32);

    QPushButton *removeBtn = new QPushButton("Quitar", toolbar);
    removeBtn->setProperty("class", "ghost");
    removeBtn->setFixedHeight(32);

    QPushButton *refreshBtn = new QPushButton("Actualizar", toolbar);
    refreshBtn->setProperty("class", "ghost");
    refreshBtn->setFixedHeight(32);

    toolbarLayout->addWidget(addFileBtn);
    toolbarLayout->addWidget(addFolderBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(removeBtn);
    toolbarLayout->addWidget(refreshBtn);

    toolbar->setStyleSheet("background: rgba(255, 254, 251, 0.5); border-bottom: 1px solid #d8d7d1;");

    m_splitter = new QSplitter(Qt::Horizontal, this);

    QWidget *listWidget = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(listWidget);
    listLayout->setContentsMargins(12, 12, 12, 12);

    m_fileList = new QListWidget(listWidget);
    m_fileList->setProperty("class", "file-list");
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);

    listLayout->addWidget(m_fileList);

    QWidget *previewWidget = new QWidget();
    QVBoxLayout *previewLayout = new QVBoxLayout(previewWidget);
    previewLayout->setContentsMargins(12, 12, 12, 12);

    QLabel *previewLabel = new QLabel("Vista previa", previewWidget);
    previewLabel->setStyleSheet("font-family: 'DM Mono', monospace; font-size: 10px; color: #777873; margin-bottom: 8px;");

    m_preview = new QTextEdit(previewWidget);
    m_preview->setReadOnly(true);
    m_preview->setProperty("class", "file-preview");
    m_preview->setPlaceholderText("Selecciona un archivo para ver su contenido...");

    previewLayout->addWidget(previewLabel);
    previewLayout->addWidget(m_preview);

    m_splitter->addWidget(listWidget);
    m_splitter->addWidget(previewWidget);
    m_splitter->setSizes({200, 400});

    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(toolbar);
    m_mainLayout->addWidget(m_splitter, 1);

    connect(addFileBtn, &QPushButton::clicked, this, &FilePanel::onAddFileClicked);
    connect(addFolderBtn, &QPushButton::clicked, this, &FilePanel::onAddFolderClicked);
    connect(removeBtn, &QPushButton::clicked, this, &FilePanel::onRemoveFileClicked);
    connect(refreshBtn, &QPushButton::clicked, this, &FilePanel::onRefreshClicked);
    connect(m_fileList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current, QListWidgetItem *) {
        if (current) {
            onFileClicked(current);
        }
    });
}

void FilePanel::addFile(const QString &path, const QString &content)
{
    for (const FileInfo &existing : m_files) {
        if (existing.path == path) {
            return;
        }
    }

    FileInfo info;
    info.path = path;
    info.content = content;
    info.lineCount = content.count('\n') + 1;
    m_files.append(info);

    updateFileList();
    m_statusLabel->setText(QString("%1 archivos").arg(m_files.size()));
}

void FilePanel::removeFile(const QString &path)
{
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files[i].path == path) {
            m_files.removeAt(i);
            updateFileList();
            m_statusLabel->setText(QString("%1 archivos").arg(m_files.size()));
            emit fileRemoved(path);
            return;
        }
    }
}

void FilePanel::clearFiles()
{
    m_files.clear();
    updateFileList();
    m_statusLabel->setText("0 archivos");
    m_preview->clear();
}

QVector<FileInfo> FilePanel::getFiles() const
{
    return m_files;
}

void FilePanel::onFileClicked(QListWidgetItem *item)
{
    if (!item) return;

    QString path = item->data(Qt::UserRole).toString();

    for (const FileInfo &info : m_files) {
        if (info.path == path) {
            updatePreview(info.content);
            emit fileSelected(path);
            return;
        }
    }
}

void FilePanel::onAddFileClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Seleccionar archivos",
        QDir::homePath(),
        "Archivos de código (*.py *.js *.ts *.jsx *.tsx *.java *.c *.cpp *.h *.hpp *.cs *.php *.html *.css *.json *.yaml *.yml *.md *.txt);;Todos los archivos (*)"
    );

    for (const QString &filePath : files) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QString content = stream.readAll();
            file.close();

            QString relativePath = QDir::current().relativeFilePath(filePath);
            addFile(relativePath, content);

            QJsonObject params;
            params["path"] = relativePath;
            params["content"] = content;
            m_processManager->sendCommand("ingest_file", params);
        }
    }
}

void FilePanel::onAddFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory(
        this,
        "Seleccionar carpeta",
        QDir::homePath()
    );

    if (folder.isEmpty()) return;

    QDir dir(folder);
    QStringList filters;
    filters << "*.py" << "*.js" << "*.ts" << "*.jsx" << "*.tsx"
            << "*.java" << "*.c" << "*.cpp" << "*.h" << "*.hpp"
            << "*.cs" << "*.php" << "*.html" << "*.css" << "*.json"
            << "*.yaml" << "*.yml" << "*.md" << "*.txt";

    QStringList files = dir.entryList(filters, QDir::Files);

    for (const QString &file : files) {
        QString filePath = dir.absoluteFilePath(file);
        QFile f(filePath);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&f);
            QString content = stream.readAll();
            f.close();

            addFile(file, content);
        }
    }

    QJsonObject params;
    params["path"] = folder;
    m_processManager->sendCommand("ingest_path", params);
}

void FilePanel::onRemoveFileClicked()
{
    QListWidgetItem *current = m_fileList->currentItem();
    if (current) {
        QString path = current->data(Qt::UserRole).toString();
        removeFile(path);
    }
}

void FilePanel::onRefreshClicked()
{
    updateFileList();
}

void FilePanel::updateFileList()
{
    m_fileList->clear();

    for (const FileInfo &info : m_files) {
        QListWidgetItem *item = new QListWidgetItem(m_fileList);
        item->setText(info.path);
        item->setData(Qt::UserRole, info.path);
        item->setToolTip(QString("%1 líneas").arg(info.lineCount));
    }
}

void FilePanel::updatePreview(const QString &content)
{
    m_preview->setPlainText(content);
}
