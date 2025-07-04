#include "gestionnairetransactions.h"
#include "comptebancaire.h"
#include "comptecourant.h"
#include "compteepargne.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QDebug>

GestionnaireTransactions::GestionnaireTransactions(Banque& banque, CreationBD& creationBD, QObject* parent)
    : QObject(parent), m_banque(banque), m_creationBD(creationBD)
{
}

bool GestionnaireTransactions::enregistrerTransaction(const QString& typeOperation, double montant,
                                                      const QString& compteSource, const QString& compteDest,
                                                      const QString& motif,
                                                      double soldeAvant, double soldeApres)
{
    if (!m_creationBD.estOuverte()) {
        return false;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Récupérer les IDs des comptes
    query.prepare("SELECT id FROM comptes WHERE numero_compte = ?");
    query.addBindValue(compteSource);

    if (!query.exec() || !query.next()) {
        qWarning() << "Compte source non trouvé pour transaction:" << compteSource;
        return false;
    }
    int idCompteSource = query.value(0).toInt();

    int idCompteBeneficiaire = -1;
    if (!compteDest.isEmpty()) {
        query.prepare("SELECT id FROM comptes WHERE numero_compte = ?");
        query.addBindValue(compteDest);

        if (query.exec() && query.next()) {
            idCompteBeneficiaire = query.value(0).toInt();
        } else {
            qWarning() << "Compte bénéficiaire non trouvé pour transaction:" << compteDest;
        }
    }

    // Insérer la transaction
    if (idCompteBeneficiaire > 0) {
        query.prepare("INSERT INTO transactions (type_operation, montant, id_compte_source, "
                      "id_compte_beneficiaire, date_operation, libelle, statut, solde_avant, solde_apres) "
                      "VALUES (?, ?, ?, ?, ?, ?, 'valide', ?, ?)");
        query.addBindValue(typeOperation);
        query.addBindValue(montant);
        query.addBindValue(idCompteSource);
        query.addBindValue(idCompteBeneficiaire);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(motif);
        query.addBindValue(soldeAvant);
        query.addBindValue(soldeApres);
    } else {
        query.prepare("INSERT INTO transactions (type_operation, montant, id_compte_source, "
                      "date_operation, libelle, statut, solde_avant, solde_apres) "
                      "VALUES (?, ?, ?, ?, ?, 'valide', ?, ?)");
        query.addBindValue(typeOperation);
        query.addBindValue(montant);
        query.addBindValue(idCompteSource);
        query.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
        query.addBindValue(motif);
        query.addBindValue(soldeAvant);
        query.addBindValue(soldeApres);
    }

    if (!query.exec()) {
        qWarning() << "Erreur enregistrement transaction:" << query.lastError().text();
        return false;
    }

    return true;
}

CompteBancaire* GestionnaireTransactions::chargerCompteDepuisBD(const QString& numeroCompte)
{
    if (!m_creationBD.estOuverte()) {
        return nullptr;
    }

    QSqlQuery query(m_creationBD.getDatabase());
    query.prepare("SELECT * FROM comptes WHERE numero_compte = ? AND est_actif = 1");
    query.addBindValue(numeroCompte);

    if (!query.exec() || !query.next()) {
        return nullptr;
    }

    QString nomTitulaire = query.value("nom_titulaire").toString();
    double solde = query.value("solde").toDouble();
    QString typeCompte = query.value("type_compte").toString();
    int idUtilisateur = query.value("id_utilisateur").toInt();

    CompteBancaire* compte = nullptr;

    if (typeCompte == "courant") {
        double decouvert = query.value("decouvert_autorise").toDouble();
        compte = new CompteCourant(numeroCompte, nomTitulaire, solde, idUtilisateur, decouvert);
    } else if (typeCompte == "epargne") {
        double taux = query.value("taux_interet").toDouble();
        compte = new CompteEpargne(numeroCompte, nomTitulaire, solde, idUtilisateur, taux);
    }

    if (compte) {
        compte->setDateCreation(query.value("date_creation").toString());
        compte->setDerniereOperation(query.value("derniere_operation").toString());
    }

    return compte;
}

void GestionnaireTransactions::effectuerDepot(CompteBancaire* compte, double montant, const QString& motif)
{
    if (!compte) {
        QMessageBox::warning(nullptr, "Erreur", "Compte invalide!");
        return;
    }

    if (montant <= 0 || montant > 999999999.99) {
        QMessageBox::warning(nullptr, "Erreur", "Montant invalide! (0 < montant ≤ 999,999,999.99)");
        return;
    }

    double soldeAvant = compte->getSolde();

    compte->deposer(montant);
    double soldeApres = compte->getSolde();

    // Calcul des intérêts pour les comptes épargne
    if (CompteEpargne* compteEpargne = dynamic_cast<CompteEpargne*>(compte)) {
        compteEpargne->calculerInterets();
        soldeApres = compte->getSolde(); // Mise à jour après intérêts
    }

    // Mise à jour de la dernière opération
    QString timestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
    QString operation = QString("Dépôt: +%1 FCFA - %2 [%3]")
                            .arg(QString::number(montant, 'f', 2))
                            .arg(motif.isEmpty() ? "Sans motif" : motif)
                            .arg(timestamp);
    compte->setDerniereOperation(operation);

    // Enregistrement de la transaction
    if (!enregistrerTransaction("depot", montant, compte->getNumeroCompte(), QString(), motif, soldeAvant, soldeApres)) {
        qWarning() << "Erreur lors de l'enregistrement de la transaction de dépôt";
    }

    emit transactionEffectuee();
}

void GestionnaireTransactions::effectuerRetrait(CompteBancaire* compte, double montant, const QString& motif)
{
    if (!compte) {
        QMessageBox::warning(nullptr, "Erreur", "Compte invalide!");
        return;
    }

    if (montant <= 0 || montant > 999999999.99) {
        QMessageBox::warning(nullptr, "Erreur", "Montant invalide! (0 < montant ≤ 999,999,999.99)");
        return;
    }

    double soldeAvant = compte->getSolde();
    bool retraitReussi = false;
    QString messageErreur;

    if (CompteCourant* cc = dynamic_cast<CompteCourant*>(compte)) {
        if ((soldeAvant - montant) >= -cc->getDecouvertAutorise()) {
            retraitReussi = cc->retirer(montant);
        } else {
            messageErreur = QString("Découvert autorisé dépassé! Limite: %1 FCFA")
                                .arg(QString::number(cc->getDecouvertAutorise(), 'f', 2));
        }
    } else if (CompteEpargne* ce = dynamic_cast<CompteEpargne*>(compte)) {
        if (montant <= soldeAvant) {
            retraitReussi = ce->retirer(montant);
        } else {
            messageErreur = "Solde insuffisant pour un compte épargne!";
        }
    } else {
        messageErreur = "Type de compte non reconnu!";
    }

    if (retraitReussi) {
        double soldeApres = compte->getSolde();
        QString timestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
        QString operation = QString("Retrait: -%1 FCFA - %2 [%3]")
                                .arg(QString::number(montant, 'f', 2))
                                .arg(motif.isEmpty() ? "Sans motif" : motif)
                                .arg(timestamp);
        compte->setDerniereOperation(operation);

        if (!enregistrerTransaction("retrait", montant, compte->getNumeroCompte(), QString(), motif, soldeAvant, soldeApres)) {
            qWarning() << "Erreur lors de l'enregistrement de la transaction de retrait";
        }

        emit transactionEffectuee();
    } else {
        QMessageBox::warning(nullptr, "Erreur",
                             messageErreur.isEmpty() ? "Opération impossible!" : messageErreur);
    }
}

void GestionnaireTransactions::effectuerVirement(const QString& compteSource, const QString& compteDest, double montant, const QString& motif)
{
    if (compteSource.isEmpty() || compteDest.isEmpty()) {
        QMessageBox::warning(nullptr, "Erreur", "Numéros de compte manquants!");
        return;
    }

    if (compteSource == compteDest) {
        QMessageBox::warning(nullptr, "Erreur", "Impossible de virer vers le même compte!");
        return;
    }

    if (montant <= 0 || montant > 999999999.99) {
        QMessageBox::warning(nullptr, "Erreur", "Montant invalide!");
        return;
    }

    CompteBancaire* source = m_banque.trouverCompte(compteSource);
    if (!source) {
        source = chargerCompteDepuisBD(compteSource);
        if (source) {
            m_banque.ajouterCompte(source);
        }
    }

    CompteBancaire* dest = m_banque.trouverCompte(compteDest);
    if (!dest) {
        dest = chargerCompteDepuisBD(compteDest);
        if (dest) {
            m_banque.ajouterCompte(dest);
        }
    }

    if (!source || !dest) {
        QMessageBox::warning(nullptr, "Erreur", source ? "Compte bénéficiaire introuvable!" : "Compte source introuvable!");
        return;
    }

    double soldeAvantSource = source->getSolde();
    double soldeAvantDest = dest->getSolde();

    bool virementPossible = false;
    QString messageErreur;

    if (CompteCourant* cc = dynamic_cast<CompteCourant*>(source)) {
        if ((soldeAvantSource - montant) >= -cc->getDecouvertAutorise()) {
            virementPossible = true;
        } else {
            messageErreur = "Découvert autorisé dépassé!";
        }
    } else if (dynamic_cast<CompteEpargne*>(source)) {
        if (montant <= soldeAvantSource) {
            virementPossible = true;
        } else {
            messageErreur = "Solde insuffisant!";
        }
    }

    if (virementPossible && (soldeAvantDest + montant > 999999999.99)) {
        virementPossible = false;
        messageErreur = "Le virement dépasserait la limite du compte bénéficiaire!";
    }

    if (virementPossible) {
        if (source->retirer(montant)) {
            dest->deposer(montant);

            double soldeApresSource = source->getSolde();
            double soldeApresDest = dest->getSolde();

            QString timestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss");
            QString opSource = QString("Virement sortant: -%1 FCFA vers %2 - %3 [%4]")
                                   .arg(QString::number(montant, 'f', 2))
                                   .arg(compteDest)
                                   .arg(motif.isEmpty() ? "Sans motif" : motif)
                                   .arg(timestamp);
            QString opDest = QString("Virement entrant: +%1 FCFA de %2 - %3 [%4]")
                                 .arg(QString::number(montant, 'f', 2))
                                 .arg(compteSource)
                                 .arg(motif.isEmpty() ? "Sans motif" : motif)
                                 .arg(timestamp);

            source->setDerniereOperation(opSource);
            dest->setDerniereOperation(opDest);

            // Enregistrement des transactions
            enregistrerTransaction("virement", montant, compteSource, compteDest, motif, soldeAvantSource, soldeApresSource);
            enregistrerTransaction("virement", montant, compteDest, compteSource, motif, soldeAvantDest, soldeApresDest);

            emit transactionEffectuee();
            QMessageBox::information(nullptr, "Succès", "Virement effectué avec succès!");
        } else {
            QMessageBox::warning(nullptr, "Erreur", "Échec du virement!");
        }
    } else {
        QMessageBox::warning(nullptr, "Erreur",
                             messageErreur.isEmpty() ? "Virement impossible!" : messageErreur);
    }
}
