#ifndef HIGHLIGHTDELEGATE_H
#define HIGHLIGHTDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QRegularExpression>
#include <QTextDocument>
#include <QWidget>
#include <QLabel>

class HighlightDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit HighlightDelegate(QTableWidget *tableWidget, QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setSearchText(const QString &text);
    void handleSearchChanged(const QString &text);

protected:
    // Gestion des événements pour le redimensionnement
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QString m_searchText;
    QRegularExpression m_regex;
    mutable QTextDocument m_document;
    QTableWidget* m_tableWidget;

    // Widget overlay pour le message "Aucun résultat"
    QWidget* m_overlayWidget;
    QLabel* m_noResultsLabel;

    // Méthodes privées
    QString highlightText(const QString &text) const;
    void createOverlay();
    void showNoResultsMessage();
    void hideNoResultsMessage();
    bool hasVisibleRows() const;
};

#endif // HIGHLIGHTDELEGATE_H
