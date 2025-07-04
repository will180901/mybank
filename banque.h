#ifndef BANQUE_H
#define BANQUE_H

#include <QString>
#include <QList>
#include "CompteBancaire.h"
#include "creationbd.h"
#include "CompteCourant.h"
#include "CompteEpargne.h"

class Banque
{
private:
    QString m_nom;
    QList<CompteBancaire*> m_comptes;  // Source de vérité pendant l'exécution
    CreationBD& m_creationBD;
    bool m_comptesCharges;             // Flag pour éviter les rechargements multiples

public:
    explicit Banque(const QString& nom, CreationBD& creationBD);
    ~Banque();

    // Gestion mémoire des comptes
    void ajouterCompte(CompteBancaire* compte);
    CompteBancaire* trouverCompte(const QString& numeroCompte) const;
    void viderComptes();
    void supprimerCompte(const QString& numeroCompte);

    // Synchronisation avec BDD - 3 moments clés
    void chargerTousLesComptes();                              // 1. Chargement initial
    bool creerCompteBD(CompteBancaire* compte);                // 2a. Création avec sync BDD
    bool supprimerCompteBD(const QString& numeroCompte);       // 2b. Suppression avec sync BDD
    bool sauvegarderToutesLesModifications();                  // 3. Sauvegarde finale

    // Opérations métier (uniquement sur m_comptes)
    bool effectuerVirement(const QString& compteSource, const QString& compteDest, double montant);
    bool deposer(const QString& numeroCompte, double montant);
    bool retirer(const QString& numeroCompte, double montant);
    void calculerInterets();  // Pour les comptes épargne

    // Getters
    QString getNom() const;
    QList<CompteBancaire*> getComptes() const;
    int getNombreComptes() const;
    bool sontComptesCharges() const;

private:
    // Méthodes utilitaires privées
    bool mettreAJourCompteBD(CompteBancaire* compte);
    void marquerDerniereOperation(CompteBancaire* compte, const QString& operation);
};

#endif // BANQUE_H
