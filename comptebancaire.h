#ifndef COMPTEBANCAIRE_H
#define COMPTEBANCAIRE_H

#include <QString>

class CompteBancaire
{
public:
    CompteBancaire(const QString& numero, const QString& titulaire, double solde, int idUtilisateur);

    virtual ~CompteBancaire() = default;

    virtual void deposer(double montant);
    virtual bool retirer(double montant) = 0;

    QString getNumeroCompte() const;
    QString getNomTitulaire() const;
    double getSolde() const;
    QString getDateCreation() const;
    QString getDerniereOperation() const;
    int getIdUtilisateur() const;

    void setDateCreation(const QString& date);
    void setDerniereOperation(const QString& operation);

protected:
    QString m_numeroCompte;
    QString m_nomTitulaire;
    double m_solde;
    QString m_dateCreation;
    QString m_derniereOperation;
    int m_idUtilisateur;
};

#endif // COMPTEBANCAIRE_H
