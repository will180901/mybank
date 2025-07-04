#ifndef GESTIONNAIREINTERFACE_H
#define GESTIONNAIREINTERFACE_H

#include <QObject>
#include <QToolButton>
#include <QLabel>

class AnimationSolde;

class GestionnaireInterface : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireInterface(QObject* parent = nullptr);

    void mettreAJourStyleBoutonsLateraux(
        QToolButton* btnDashboard, QToolButton* btnHistorique, QToolButton* btnParametres,
        QWidget* currentPage, QWidget* pageDashboard, QWidget* pageHistorique, QWidget* pageParametres
        );

    void appliquerStyleBoutonMasquage(QToolButton* button, bool survole);
    void mettreAjourIcone(QToolButton* button, bool visible);
    void appliquerEffetFlou(QLabel* label, bool masquer, AnimationSolde* animation);
};

#endif // GESTIONNAIREINTERFACE_H
