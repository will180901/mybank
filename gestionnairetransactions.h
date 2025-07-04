#ifndef GESTIONNAIRETRANSACTIONS_H
#define GESTIONNAIRETRANSACTIONS_H

#include "banque.h"
#include "creationbd.h"
#include <QObject>

class CompteBancaire;

class GestionnaireTransactions : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireTransactions(Banque& banque, CreationBD& creationBD, QObject* parent = nullptr);

    void effectuerDepot(CompteBancaire* compte, double montant, const QString& motif);
    void effectuerRetrait(CompteBancaire* compte, double montant, const QString& motif);
    void effectuerVirement(const QString& compteSource, const QString& compteDest, double montant, const QString& motif);

signals:
    void transactionEffectuee();

private:
    Banque& m_banque;
    CreationBD& m_creationBD;

    bool enregistrerTransaction(const QString& typeOperation, double montant,
                                const QString& compteSource, const QString& compteDest,
                                const QString& motif,
                                double soldeAvant, double soldeApres);
    CompteBancaire* chargerCompteDepuisBD(const QString& numeroCompte);
};

#endif // GESTIONNAIRETRANSACTIONS_H
