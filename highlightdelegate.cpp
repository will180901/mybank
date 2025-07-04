#include "highlightdelegate.h"
#include <QPainter>
#include <QTextDocument>
#include <QStyleOptionViewItem>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QRegularExpressionMatchIterator>
#include <QEvent>

HighlightDelegate::HighlightDelegate(QTableWidget *tableWidget, QObject *parent)
    : QStyledItemDelegate(parent),
    m_tableWidget(tableWidget),
    m_regex(QRegularExpression()),
    m_overlayWidget(nullptr),
    m_noResultsLabel(nullptr)
{
    m_document.setDocumentMargin(2);
    createOverlay();
}

void HighlightDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_searchText.isEmpty() || !index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // Configuration de base
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QString text = index.data(Qt::DisplayRole).toString();

    // Préparation du rendu
    painter->save();

    // Fond pour élément sélectionné
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    } else {
        painter->fillRect(opt.rect, opt.palette.base());
    }

    // Configuration exacte du document
    m_document.setHtml(highlightText(text));
    m_document.setDefaultFont(opt.font);
    m_document.setTextWidth(opt.rect.width() - 4); // Marge pour éviter débordement

    // Alignement précis
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap); // Pas de retour à la ligne
    textOption.setAlignment(opt.displayAlignment);
    m_document.setDefaultTextOption(textOption);

    // Positionnement exact
    QRect textRect = opt.rect;
    textRect.adjust(2, 0, -2, 0); // Petite marge horizontale

    painter->translate(textRect.topLeft());

    // Clipping pour éviter débordement
    painter->setClipRect(QRect(0, 0, textRect.width(), textRect.height()));

    m_document.drawContents(painter);

    painter->restore();
}

QString HighlightDelegate::highlightText(const QString &text) const
{
    if (m_searchText.isEmpty() || !m_regex.isValid())
        return text.toHtmlEscaped();

    QString highlighted = text.toHtmlEscaped();

    // Utiliser QRegularExpressionMatchIterator pour un remplacement précis
    QRegularExpressionMatchIterator iterator = m_regex.globalMatch(highlighted);
    QString result = highlighted;
    int offset = 0;

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString matchedText = match.captured(0);
        QString replacement = QString("<span style=\"background-color:#FFFFCC;color:black;\">%1</span>").arg(matchedText);

        result.replace(match.capturedStart() + offset, match.capturedLength(), replacement);
        offset += replacement.length() - match.capturedLength();
    }

    return result;
}

void HighlightDelegate::setSearchText(const QString &text)
{
    m_searchText = text;

    // Optimisation : précompiler l'expression régulière
    if (text.isEmpty()) {
        m_regex = QRegularExpression();
    } else {
        // Échapper les caractères spéciaux et créer la regex
        QString escapedText = QRegularExpression::escape(text);
        m_regex = QRegularExpression(
            escapedText,
            QRegularExpression::CaseInsensitiveOption
            );
    }
}

void HighlightDelegate::handleSearchChanged(const QString &text)
{
    setSearchText(text);

    // Filtrage des lignes
    const bool hasSearch = !text.isEmpty();
    const QString searchLower = text.toLower();

    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        bool matchFound = false;

        for (int col = 0; col < m_tableWidget->columnCount(); ++col) {
            QTableWidgetItem *item = m_tableWidget->item(row, col);
            if (!item) continue;

            if (item->text().toLower().contains(searchLower)) {
                matchFound = true;
                break;
            }
        }

        m_tableWidget->setRowHidden(row, hasSearch && !matchFound);
    }

    // Gérer l'affichage du message "Aucun résultat"
    if (hasSearch && !hasVisibleRows()) {
        showNoResultsMessage();
    } else {
        hideNoResultsMessage();
    }

    // Forcer la mise à jour de l'affichage
    m_tableWidget->viewport()->update();
}

void HighlightDelegate::createOverlay()
{
    // Créer le widget overlay
    m_overlayWidget = new QWidget(m_tableWidget);
    m_overlayWidget->setStyleSheet(
        "QWidget { "
        "background-color: rgba(255, 255, 255, 180); " // Rideau transparent
        "border-radius: 8px; "
        "}"
        );

    // Créer le label pour le message
    m_noResultsLabel = new QLabel("Aucun résultat trouvé pour cet élément recherché", m_overlayWidget);
    m_noResultsLabel->setAlignment(Qt::AlignCenter);
    m_noResultsLabel->setStyleSheet(
        "QLabel { "
        "color: #666666; "
        "font-size: 16px; "
        "font-weight: bold; "
        "background-color: transparent; "
        "padding: 20px; "
        "}"
        );

    // Layout pour centrer le message
    QVBoxLayout* layout = new QVBoxLayout(m_overlayWidget);
    layout->addWidget(m_noResultsLabel);
    layout->setAlignment(Qt::AlignCenter);

    // Initialement caché
    m_overlayWidget->hide();

    // CORRECTION : Installer un event filter pour détecter le redimensionnement
    m_tableWidget->installEventFilter(const_cast<HighlightDelegate*>(this));
}

void HighlightDelegate::showNoResultsMessage()
{
    if (!m_overlayWidget) return;

    // Positionner l'overlay sur tout le viewport du tableau
    QRect tableRect = m_tableWidget->viewport()->rect();
    m_overlayWidget->setGeometry(tableRect);

    // Afficher avec une animation douce
    m_overlayWidget->show();
    m_overlayWidget->raise(); // Mettre au premier plan
}

void HighlightDelegate::hideNoResultsMessage()
{
    if (m_overlayWidget) {
        m_overlayWidget->hide();
    }
}

bool HighlightDelegate::eventFilter(QObject *watched, QEvent *event)
{
    // Gérer le redimensionnement du tableau
    if (watched == m_tableWidget && event->type() == QEvent::Resize) {
        if (m_overlayWidget && m_overlayWidget->isVisible()) {
            QRect tableRect = m_tableWidget->viewport()->rect();
            m_overlayWidget->setGeometry(tableRect);
        }
    }

    // Continuer le traitement normal des événements
    return QStyledItemDelegate::eventFilter(watched, event);
}

bool HighlightDelegate::hasVisibleRows() const
{
    for (int row = 0; row < m_tableWidget->rowCount(); ++row) {
        if (!m_tableWidget->isRowHidden(row)) {
            return true;
        }
    }
    return false;
}
