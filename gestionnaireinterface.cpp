#include "gestionnaireinterface.h"
#include "animationsolde.h"
#include <QStyle>
#include <QIcon>
#include <QApplication>

GestionnaireInterface::GestionnaireInterface(QObject* parent)
    : QObject(parent)
{
}

void GestionnaireInterface::mettreAJourStyleBoutonsLateraux(
    QToolButton* btnDashboard, QToolButton* btnHistorique, QToolButton* btnParametres,
    QWidget* currentPage, QWidget* pageDashboard, QWidget* pageHistorique, QWidget* pageParametres)
{
    QString styleBase =
        "QToolButton { "
        "padding: 6px 12px; "
        "border-radius: 4px; "
        "border: none; "
        "color: #4A4A4A; "
        "font-weight: 500; "
        "min-height: 18px; "
        "text-align: left; "
        "background-color: transparent; "
        "}"
        "QToolButton:hover{ "
        "background-color: rgba(41, 98, 255,0.2); "
        "}";

    QString styleActif =
        "QToolButton { "
        "background-color: rgb(41, 98, 255); "
        "padding: 6px 12px; "
        "border-radius: 4px; "
        "border: none; "
        "color: rgb(255, 255, 255); "
        "font-weight: 500; "
        "min-height: 18px; "
        "}";

    btnDashboard->setStyleSheet(styleBase);
    btnHistorique->setStyleSheet(styleBase);
    btnParametres->setStyleSheet(styleBase);

    if (currentPage == pageDashboard) {
        btnDashboard->setStyleSheet(styleActif);
    }
    else if (currentPage == pageHistorique) {
        btnHistorique->setStyleSheet(styleActif);
    }
    else if (currentPage == pageParametres) {
        btnParametres->setStyleSheet(styleActif);
    }
}

void GestionnaireInterface::appliquerStyleBoutonMasquage(QToolButton* button, bool survole)
{
    if (!button) return;

    QString style;
    if (survole) {
        style = "QToolButton { "
                "background-color: rgb(41, 98, 255); "
                "border-radius: 4px; "
                "border: none; "
                "padding: 6px 12px; "
                "}";
    } else {
        style = "QToolButton { "
                "background-color: transparent; "
                "border: none; "
                "border-radius: 4px; "
                "padding: 6px 12px; "
                "}";
    }

    button->setStyleSheet(style);
}

void GestionnaireInterface::mettreAjourIcone(QToolButton* button, bool visible)
{
    if (!button) return;

    bool isHovered = button->underMouse();
    QString basePath = isHovered ? ":/icon_blanc/" : ":/icon_gris/";
    QString iconName = visible ? "eye-off.svg" : "eye.svg";

    QIcon icon(basePath + iconName);
    button->setIcon(icon);
    button->setIconSize(QSize(15, 15));
    button->setToolTip(visible ? "Masquer le solde" : "Afficher le solde");

    button->style()->unpolish(button);
    button->style()->polish(button);
    button->update();
}

void GestionnaireInterface::appliquerEffetFlou(QLabel* label, bool masquer, AnimationSolde* animation)
{
    if (animation) {
        animation->appliquerAvecLabel(label, masquer);
    }
}
