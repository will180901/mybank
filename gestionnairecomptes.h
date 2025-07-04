#ifndef GESTIONNAIRECOMPTES_H
#define GESTIONNAIRECOMPTES_H

#include "banque.h"
#include "creationbd.h"
#include <QObject>
#include <QLabel>

class GestionnaireComptes : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireComptes(Banque& banque, CreationBD& creationBD, const QString& utilisateurId, QObject* parent = nullptr);

    void creerCompteCourant();
    void creerCompteEpargne();
    void supprimerCompteCourant();
    void supprimerCompteEpargne();
    void mettreAJourAffichageComptes(
        QLabel* labelSoldeCourant, QLabel* labelDecouvert, QLabel* labelNumeroCourant,
        QLabel* labelDateCreationCourant, QLabel* labelDerniereOperationCourant,
        QLabel* labelSoldeEpargne, QLabel* labelTauxInteret, QLabel* labelNumeroEpargne,
        QLabel* labelDateCreationEpargne, QLabel* labelDerniereOperationEpargne
        );

signals:
    void comptesModifies();

private:
    Banque& m_banque;
    CreationBD& m_creationBD;
    QString m_utilisateurId;

    CompteCourant* getCompteCourant() const;
    CompteEpargne* getCompteEpargne() const;
    QString genererNumeroCompte(const QString& typeCompte);
    void creerCompteCourantEnBD();
    void creerCompteEpargneEnBD();
};

#endif // GESTIONNAIRECOMPTES_H
