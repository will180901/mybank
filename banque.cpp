#include "Banque.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

Banque::Banque(const QString& nom, CreationBD& creationBD)
    : m_nom(nom), m_creationBD(creationBD), m_comptesCharges(false)
{
    chargerTousLesComptes() ;
}

Banque::~Banque() {
    viderComptes();
}

// =============================================================================
// 1. CHARGEMENT INITIAL - Tous les comptes depuis la BDD
// =============================================================================
void Banque::chargerTousLesComptes()
{
    if (!m_creationBD.estOuverte()) {
        qWarning() << "Base de données non ouverte! Impossible de charger les comptes.";
        return;
    }

    if (m_comptesCharges) {
        qDebug() << "Comptes déjà chargés, éviter le rechargement multiple.";
        return;
    }

    viderComptes();

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Charger TOUS les comptes actifs (pas de filtre par utilisateur)
    query.prepare("SELECT * FROM comptes WHERE est_actif = 1 ORDER BY date_creation");

    if (!query.exec()) {
        qWarning() << "Erreur récupération comptes:" << query.lastError().text();
        return;
    }

    int comptesCharges = 0;
    while (query.next()) {
        QString numeroCompte = query.value("numero_compte").toString();
        QString nomTitulaire = query.value("nom_titulaire").toString();
        double solde = query.value("solde").toDouble();
        QString typeCompte = query.value("type_compte").toString();
        QString dateCreation = query.value("date_creation").toString();
        QString derniereOperation = query.value("derniere_operation").toString();
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
            compte->setDateCreation(dateCreation);
            compte->setDerniereOperation(derniereOperation);
            ajouterCompte(compte);
            comptesCharges++;
        }
    }

    m_comptesCharges = true;
    qDebug() << "Chargement terminé:" << comptesCharges << "comptes chargés dans m_comptes.";
}

// =============================================================================
// 2. CRÉATION/SUPPRESSION avec synchronisation BDD immédiate
// =============================================================================
bool Banque::creerCompteBD(CompteBancaire* compte)
{
    if (!compte || !m_creationBD.estOuverte()) {
        qWarning() << "Impossible de créer le compte: compte null ou BDD fermée.";
        return false;
    }

    // Vérifier si le compte existe déjà en mémoire
    if (trouverCompte(compte->getNumeroCompte())) {
        qWarning() << "Le compte" << compte->getNumeroCompte() << "existe déjà.";
        return false;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    QString typeCompte = "courant";
    double decouvert = 0.0;
    double taux = 0.0;

    // Déterminer le type et les paramètres spécifiques
    if (CompteCourant* cc = dynamic_cast<CompteCourant*>(compte)) {
        typeCompte = "courant";
        decouvert = cc->getDecouvertAutorise();
    } else if (CompteEpargne* ce = dynamic_cast<CompteEpargne*>(compte)) {
        typeCompte = "epargne";
        taux = ce->getTauxInteret();
    } else {
        qWarning() << "Type de compte non reconnu.";
        return false;
    }

    // Mise à jour des métadonnées
    compte->setDateCreation(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    compte->setDerniereOperation("Création du compte");

    // Insertion en BDD
    query.prepare("INSERT INTO comptes (numero_compte, nom_titulaire, solde, type_compte, "
                  "date_creation, derniere_operation, id_utilisateur, decouvert_autorise, taux_interet, id_banque, est_actif) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1, 1)");




    query.addBindValue(compte->getNumeroCompte());
    query.addBindValue(compte->getNomTitulaire());
    query.addBindValue(compte->getSolde());
    query.addBindValue(typeCompte);
    query.addBindValue(compte->getDateCreation());
    query.addBindValue(compte->getDerniereOperation());
    query.addBindValue(compte->getIdUtilisateur());
    query.addBindValue(decouvert);
    query.addBindValue(taux);

    if (query.exec()) {
        // AJOUT IMMÉDIAT à m_comptes après insertion BDD réussie
        ajouterCompte(compte);
        qDebug() << "Compte" << compte->getNumeroCompte() << "créé et ajouté à m_comptes.";
        return true;
    } else {
        qWarning() << "Erreur création compte en BDD:" << query.lastError().text();
        return false;
    }
}

bool Banque::supprimerCompteBD(const QString& numeroCompte)
{
    if (!m_creationBD.estOuverte()) {
        qWarning() << "Base de données non ouverte pour suppression.";
        return false;
    }

    // Vérifier que le compte existe en mémoire
    CompteBancaire* compte = trouverCompte(numeroCompte);
    if (!compte) {
        qWarning() << "Compte" << numeroCompte << "non trouvé en mémoire.";
        return false;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Suppression logique en BDD (est_actif = 0)
    query.prepare("UPDATE comptes SET est_actif = 0 WHERE numero_compte = ?");
    query.addBindValue(numeroCompte);

    if (query.exec()) {
        // RETRAIT IMMÉDIAT de m_comptes après suppression BDD réussie
        supprimerCompte(numeroCompte);
        qDebug() << "Compte" << numeroCompte << "supprimé de la BDD et retiré de m_comptes.";
        return true;
    } else {
        qWarning() << "Erreur suppression compte en BDD:" << query.lastError().text();
        return false;
    }
}

// =============================================================================
// 3. SAUVEGARDE FINALE - Toutes les modifications de m_comptes vers BDD
// =============================================================================
bool Banque::sauvegarderToutesLesModifications()
{
    if (!m_creationBD.estOuverte()) {
        qWarning() << "Base de données non ouverte pour sauvegarde.";
        return false;
    }

    QSqlDatabase db = m_creationBD.getDatabase();

    // Utiliser une transaction pour garantir la cohérence
    if (!db.transaction()) {
        qWarning() << "Impossible de démarrer la transaction de sauvegarde.";
        return false;
    }

    bool sauvegardeReussie = true;
    int comptesSauvegardes = 0;

    for (CompteBancaire* compte : m_comptes) {
        if (compte && !mettreAJourCompteBD(compte)) {
            qWarning() << "Erreur sauvegarde du compte:" << compte->getNumeroCompte();
            sauvegardeReussie = false;
            // Ne pas break pour tenter de sauvegarder tous les comptes
        }
        comptesSauvegardes++;
    }

    if (sauvegardeReussie && db.commit()) {
        qDebug() << "Sauvegarde réussie:" << comptesSauvegardes << "comptes mis à jour en BDD.";
        return true;
    } else {
        db.rollback();
        qWarning() << "Échec de la sauvegarde, rollback effectué.";
        return false;
    }
}

bool Banque::mettreAJourCompteBD(CompteBancaire* compte)
{
    if (!compte) return false;

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Préparer les valeurs spécifiques selon le type
    double decouvert = 0.0;
    double taux = 0.0;

    if (CompteCourant* cc = dynamic_cast<CompteCourant*>(compte)) {
        decouvert = cc->getDecouvertAutorise();
    } else if (CompteEpargne* ce = dynamic_cast<CompteEpargne*>(compte)) {
        taux = ce->getTauxInteret();
    }

    query.prepare("UPDATE comptes SET nom_titulaire = ?, solde = ?, derniere_operation = ?, "
                  "decouvert_autorise = ?, taux_interet = ? WHERE numero_compte = ? AND est_actif = 1");

    query.addBindValue(compte->getNomTitulaire());
    query.addBindValue(compte->getSolde());
    query.addBindValue(compte->getDerniereOperation());
    query.addBindValue(decouvert);
    query.addBindValue(taux);
    query.addBindValue(compte->getNumeroCompte());

    return query.exec();
}

// =============================================================================
// OPÉRATIONS MÉTIER - Exclusivement sur m_comptes
// =============================================================================

bool Banque::effectuerVirement(const QString& compteSource, const QString& compteDest, double montant)
{
    CompteBancaire* source = trouverCompte(compteSource);
    CompteBancaire* dest = trouverCompte(compteDest);

    if (!source || !dest) {
        qWarning() << "Compte source ou destination non trouvé pour le virement.";
        return false;
    }

    if (montant <= 0) {
        qWarning() << "Montant du virement invalide:" << montant;
        return false;
    }

    // Vérifier si le compte source est un compte épargne
    if (dynamic_cast<CompteEpargne*>(source)) {
        if (source->getSolde() - montant < 0) {
            qWarning() << "Virement échoué: solde insuffisant sur le compte épargne source.";
            return false;
        }
    }

    if (source->retirer(montant)) {
        dest->deposer(montant);

        // Mise à jour des métadonnées
        QString operation = QString("Virement de %1 vers %2").arg(montant).arg(compteDest);
        marquerDerniereOperation(source, operation);

        operation = QString("Virement reçu de %1 depuis %2").arg(montant).arg(compteSource);
        marquerDerniereOperation(dest, operation);

        qDebug() << "Virement réussi:" << montant << "de" << compteSource << "vers" << compteDest;
        return true;
    }

    qWarning() << "Virement échoué: solde insuffisant sur le compte source.";
    return false;
}



bool Banque::deposer(const QString& numeroCompte, double montant)
{
    CompteBancaire* compte = trouverCompte(numeroCompte);
    if (!compte || montant <= 0) {
        return false;
    }

    compte->deposer(montant);
    marquerDerniereOperation(compte, QString("Dépôt de %1").arg(montant));
    return true;
}

bool Banque::retirer(const QString& numeroCompte, double montant)
{
    CompteBancaire* compte = trouverCompte(numeroCompte);
    if (!compte || montant <= 0) {
        return false;
    }

    if (compte->retirer(montant)) {
        marquerDerniereOperation(compte, QString("Retrait de %1").arg(montant));
        return true;
    }
    return false;
}

void Banque::calculerInterets()
{
    for (CompteBancaire* compte : m_comptes) {
        if (CompteEpargne* ce = dynamic_cast<CompteEpargne*>(compte)) {
            double soldeAvant = ce->getSolde(); // Sauvegarde du solde initial
            ce->calculerInterets(); // Calcul des intérêts (modifie le solde)
            double interets = ce->getSolde() - soldeAvant; // Calcul de la différence

            if (interets > 0) {
                marquerDerniereOperation(ce, QString("Intérêts créditeurs: %1").arg(interets));
            }
        }
    }
}

// =============================================================================
// GESTION MÉMOIRE DES COMPTES
// =============================================================================
void Banque::ajouterCompte(CompteBancaire* compte)
{
    if (compte && !trouverCompte(compte->getNumeroCompte())) {
        m_comptes.append(compte);
        qDebug() << "Compte" << compte->getNumeroCompte() << "ajouté à m_comptes.";
    }
}

CompteBancaire* Banque::trouverCompte(const QString& numeroCompte) const
{
    for (CompteBancaire* compte : m_comptes) {
        if (compte && compte->getNumeroCompte() == numeroCompte) {
            return compte;
        }
    }
    return nullptr;
}

void Banque::supprimerCompte(const QString& numeroCompte)
{
    for (auto it = m_comptes.begin(); it != m_comptes.end(); ++it) {
        if (*it && (*it)->getNumeroCompte() == numeroCompte) {
            delete *it;
            m_comptes.erase(it);
            qDebug() << "Compte" << numeroCompte << "retiré de m_comptes.";
            break;
        }
    }
}

void Banque::viderComptes()
{
    for (CompteBancaire* compte : m_comptes) {
        delete compte;
    }
    m_comptes.clear();
    m_comptesCharges = false;
    qDebug() << "Tous les comptes supprimés de la mémoire.";
}

// =============================================================================
// GETTERS ET UTILITAIRES
// =============================================================================
QString Banque::getNom() const {
    return m_nom;
}

QList<CompteBancaire*> Banque::getComptes() const {
    return m_comptes;
}

int Banque::getNombreComptes() const {
    return m_comptes.size();
}

bool Banque::sontComptesCharges() const {
    return m_comptesCharges;
}

void Banque::marquerDerniereOperation(CompteBancaire* compte, const QString& operation)
{
    if (compte) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        compte->setDerniereOperation(QString("%1 - %2").arg(timestamp, operation));
    }
}
