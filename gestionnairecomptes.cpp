#include "gestionnairecomptes.h"
#include "comptecourant.h"
#include "compteepargne.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QUuid>
#include <QDateTime>

GestionnaireComptes::GestionnaireComptes(Banque& banque, CreationBD& creationBD, const QString& utilisateurId, QObject* parent)
    : QObject(parent), m_banque(banque), m_creationBD(creationBD), m_utilisateurId(utilisateurId)
{
}

CompteCourant* GestionnaireComptes::getCompteCourant() const
{
    for (CompteBancaire* compte : m_banque.getComptes()) {
        if (compte->getIdUtilisateur() == m_utilisateurId.toInt()) {
            if (CompteCourant* cc = dynamic_cast<CompteCourant*>(compte)) {
                return cc;
            }
        }
    }
    return nullptr;
}

CompteEpargne* GestionnaireComptes::getCompteEpargne() const
{
    for (CompteBancaire* compte : m_banque.getComptes()) {
        if (compte->getIdUtilisateur() == m_utilisateurId.toInt()) {
            if (CompteEpargne* ce = dynamic_cast<CompteEpargne*>(compte)) {
                return ce;
            }
        }
    }
    return nullptr;
}

QString GestionnaireComptes::genererNumeroCompte(const QString& typeCompte)
{
    QString prefixe = (typeCompte == "courant") ? "CC" : "CE";
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).remove('-').left(10);
    return prefixe + uuid.toUpper();
}

void GestionnaireComptes::creerCompteCourantEnBD()
{
    QString numeroCompte = genererNumeroCompte("courant");
    QString dateCreation = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");
    QString derniereOperation = "Création compte";
    double decouvertAutorise = 500.0;

    QSqlQuery query(m_creationBD.getDatabase());
    query.prepare("SELECT nom_complet FROM utilisateurs WHERE id = ?");
    query.addBindValue(m_utilisateurId.toInt());
    QString nomTitulaire = "Utilisateur";
    if (query.exec() && query.next()) {
        nomTitulaire = query.value("nom_complet").toString();
    }

    CompteCourant* nouveauCompte = new CompteCourant(
        numeroCompte,
        nomTitulaire,
        0.0,
        m_utilisateurId.toInt(),
        decouvertAutorise
        );
    nouveauCompte->setDateCreation(dateCreation);
    nouveauCompte->setDerniereOperation(derniereOperation);

    if (m_banque.creerCompteBD(nouveauCompte)) {
        emit comptesModifies();
        QMessageBox::information(nullptr, "Succès", "Compte courant créé!");
    } else {
        delete nouveauCompte;
        QMessageBox::critical(nullptr, "Erreur", "Échec création compte");
    }
}

void GestionnaireComptes::creerCompteEpargneEnBD()
{
    QString numeroCompte = genererNumeroCompte("epargne");
    QString dateCreation = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");
    QString derniereOperation = "Création compte";
    double tauxInteret = 2.5;

    QSqlQuery query(m_creationBD.getDatabase());
    query.prepare("SELECT nom_complet FROM utilisateurs WHERE id = ?");
    query.addBindValue(m_utilisateurId.toInt());
    QString nomTitulaire = "Utilisateur";
    if (query.exec() && query.next()) {
        nomTitulaire = query.value("nom_complet").toString();
    }

    CompteEpargne* nouveauCompte = new CompteEpargne(
        numeroCompte,
        nomTitulaire,
        0.0,
        m_utilisateurId.toInt(),
        tauxInteret
        );
    nouveauCompte->setDateCreation(dateCreation);
    nouveauCompte->setDerniereOperation(derniereOperation);

    if (m_banque.creerCompteBD(nouveauCompte)) {
        emit comptesModifies();
        QMessageBox::information(nullptr, "Succès", "Compte épargne créé!");
    } else {
        delete nouveauCompte;
        QMessageBox::critical(nullptr, "Erreur", "Échec création compte");
    }
}

void GestionnaireComptes::creerCompteCourant()
{
    if (getCompteCourant() != nullptr) {
        QMessageBox::information(nullptr, "Information", "Un compte courant existe déjà!");
        return;
    }
    creerCompteCourantEnBD();
}

void GestionnaireComptes::creerCompteEpargne()
{
    if (getCompteEpargne() != nullptr) {
        QMessageBox::information(nullptr, "Information", "Un compte épargne existe déjà!");
        return;
    }
    creerCompteEpargneEnBD();
}

void GestionnaireComptes::supprimerCompteCourant()
{
    CompteCourant* compte = getCompteCourant();
    if (!compte) return;

    if (m_banque.supprimerCompteBD(compte->getNumeroCompte())) {
        emit comptesModifies();
        QMessageBox::information(nullptr, "Succès", "Compte supprimé!");
    } else {
        QMessageBox::critical(nullptr, "Erreur", "Échec suppression");
    }
}

void GestionnaireComptes::supprimerCompteEpargne()
{
    CompteEpargne* compte = getCompteEpargne();
    if (!compte) return;

    if (m_banque.supprimerCompteBD(compte->getNumeroCompte())) {
        emit comptesModifies();
        QMessageBox::information(nullptr, "Succès", "Compte supprimé!");
    } else {
        QMessageBox::critical(nullptr, "Erreur", "Échec suppression");
    }
}

void GestionnaireComptes::mettreAJourAffichageComptes(
    QLabel* labelSoldeCourant, QLabel* labelDecouvert, QLabel* labelNumeroCourant,
    QLabel* labelDateCreationCourant, QLabel* labelDerniereOperationCourant,
    QLabel* labelSoldeEpargne, QLabel* labelTauxInteret, QLabel* labelNumeroEpargne,
    QLabel* labelDateCreationEpargne, QLabel* labelDerniereOperationEpargne)
{
    // Compte Courant
    if (CompteCourant* compteCourant = getCompteCourant()) {
        labelSoldeCourant->setText(QString::number(compteCourant->getSolde(), 'f', 2) + " FCFA");
        labelDecouvert->setText(QString::number(compteCourant->getDecouvertAutorise(), 'f', 2) + " FCFA");
        labelNumeroCourant->setText(compteCourant->getNumeroCompte());
        labelDateCreationCourant->setText(compteCourant->getDateCreation());
        labelDerniereOperationCourant->setText(compteCourant->getDerniereOperation());
    } else {
        labelSoldeCourant->setText("0.00 FCFA");
        labelDecouvert->setText("0.00 FCFA");
        labelNumeroCourant->setText("N/A");
        labelDateCreationCourant->setText("N/A");
        labelDerniereOperationCourant->setText("N/A");
    }

    // Compte Épargne
    if (CompteEpargne* compteEpargne = getCompteEpargne()) {
        labelSoldeEpargne->setText(QString::number(compteEpargne->getSolde(), 'f', 2) + " FCFA");
        labelTauxInteret->setText(QString::number(compteEpargne->getTauxInteret(), 'f', 2) + "%");
        labelNumeroEpargne->setText(compteEpargne->getNumeroCompte());
        labelDateCreationEpargne->setText(compteEpargne->getDateCreation());
        labelDerniereOperationEpargne->setText(compteEpargne->getDerniereOperation());
    } else {
        labelSoldeEpargne->setText("0.00 FCFA");
        labelTauxInteret->setText("0.00%");
        labelNumeroEpargne->setText("N/A");
        labelDateCreationEpargne->setText("N/A");
        labelDerniereOperationEpargne->setText("N/A");
    }
}
