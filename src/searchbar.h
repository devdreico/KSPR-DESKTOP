#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = nullptr);
    ~SearchBar();

    void setPlaceholderText(const QString &text);
    QString text() const;
    void clear();
    void focus();

signals:
    void searchTextChanged(const QString &text);
    void searchSubmitted(const QString &text);
    void closeClicked();

private slots:
    void onTextChanged(const QString &text);
    void onReturnPressed();
    void onCloseClicked();

private:
    void setupUI();

    QLineEdit *m_searchInput;
    QPushButton *m_searchIcon;
    QPushButton *m_closeButton;
    QLabel *m_resultCountLabel;
};

#endif // SEARCHBAR_H
