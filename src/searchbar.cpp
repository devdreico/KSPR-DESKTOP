#include "searchbar.h"

SearchBar::SearchBar(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

SearchBar::~SearchBar()
{
}

void SearchBar::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    setStyleSheet(
        "QWidget {"
        "   background: rgba(255, 254, 251, 0.9);"
        "   border: 1px solid #d8d7d1;"
        "   border-radius: 10px;"
        "}"
    );

    m_searchIcon = new QPushButton("\u2315", this);
    m_searchIcon->setFixedSize(32, 32);
    m_searchIcon->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   color: #777873;"
        "   font-size: 16px;"
        "}"
        "QPushButton:hover {"
        "   color: #6366f1;"
        "}"
    );

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Buscar...");
    m_searchInput->setStyleSheet(
        "QLineEdit {"
        "   background: transparent;"
        "   border: none;"
        "   padding: 8px 4px;"
        "   font-family: 'Montserrat', sans-serif;"
        "   font-size: 13px;"
        "   color: #141414;"
        "}"
        "QLineEdit:focus {"
        "   outline: none;"
        "}"
    );

    m_resultCountLabel = new QLabel(this);
    m_resultCountLabel->setStyleSheet(
        "font-family: 'DM Mono', monospace;"
        "font-size: 10px;"
        "color: #777873;"
        "padding: 0 4px;"
    );
    m_resultCountLabel->hide();

    m_closeButton = new QPushButton("\u2715", this);
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "   background: transparent;"
        "   border: none;"
        "   color: #777873;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "   color: #ef4444;"
        "}"
    );

    layout->addWidget(m_searchIcon);
    layout->addWidget(m_searchInput, 1);
    layout->addWidget(m_resultCountLabel);
    layout->addWidget(m_closeButton);

    connect(m_searchInput, &QLineEdit::textChanged, this, &SearchBar::onTextChanged);
    connect(m_searchInput, &QLineEdit::returnPressed, this, &SearchBar::onReturnPressed);
    connect(m_closeButton, &QPushButton::clicked, this, &SearchBar::onCloseClicked);
}

void SearchBar::setPlaceholderText(const QString &text)
{
    m_searchInput->setPlaceholderText(text);
}

QString SearchBar::text() const
{
    return m_searchInput->text();
}

void SearchBar::clear()
{
    m_searchInput->clear();
    m_resultCountLabel->hide();
}

void SearchBar::focus()
{
    m_searchInput->setFocus();
}

void SearchBar::onTextChanged(const QString &text)
{
    emit searchTextChanged(text);
}

void SearchBar::onReturnPressed()
{
    emit searchSubmitted(m_searchInput->text());
}

void SearchBar::onCloseClicked()
{
    clear();
    emit closeClicked();
}
