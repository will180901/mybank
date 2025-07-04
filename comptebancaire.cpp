#include "CompteBancaire.h"

CompteBancaire::CompteBancaire(const QString& numero, const QString& titulaire, double solde, int idUtilisateur)
    : m_numeroCompte(numero),
    m_nomTitulaire(titulaire),
    m_solde(solde),
    m_idUtilisateur(idUtilisateur)
{}

void CompteBancaire::deposer(double montant)
{
    if (montant > 0) {
        m_solde += montant;
    }
}

int CompteBancaire::getIdUtilisateur() const {
    return m_idUtilisateur;
}

bool CompteBancaire::retirer(double montant)
{
    Q_UNUSED(montant);
    return false;
}

QString CompteBancaire::getNumeroCompte() const { return m_numeroCompte; }
QString CompteBancaire::getNomTitulaire() const { return m_nomTitulaire; }
double CompteBancaire::getSolde() const { return m_solde; }
QString CompteBancaire::getDateCreation() const { return m_dateCreation; }
QString CompteBancaire::getDerniereOperation() const { return m_derniereOperation; }

void CompteBancaire::setDateCreation(const QString& date) { m_dateCreation = date; }
void CompteBancaire::setDerniereOperation(const QString& operation) { m_derniereOperation = operation; }
