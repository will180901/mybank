#ifndef GESTIONNAIREHISTORIQUE_H
#define GESTIONNAIREHISTORIQUE_H

#include "creationbd.h"
#include <QTableWidget>
#include <QObject>

class GestionnaireHistorique : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireHistorique(CreationBD& creationBD, const QString& utilisateurId, QObject* parent = nullptr);

    void chargerHistoriqueComplet(QTableWidget* table);
    void afficherTop5DernieresTransactions(QTableWidget* table);

private:
    CreationBD& m_creationBD;
    QString m_utilisateurId;
    QStringList m_mesComptes; // Ajouter cette ligne
    void chargerComptesUtilisateur(); // Ajouter cette déclaration

    void configurerTableHistorique(QTableWidget* table);
    void peuplerLigneHistorique(QTableWidget* table, int row, const QSqlQuery& query);
    QString determinerClasseBadgeMontant(const QString& typeOp, const QString& compteDest, const QStringList& mesComptes) const;
};

#endif // GESTIONNAIREHISTORIQUE_H
