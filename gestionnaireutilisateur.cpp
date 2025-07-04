#include "gestionnaireutilisateur.h"
#include <QSqlQuery>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QRegularExpression>

GestionnaireUtilisateur::GestionnaireUtilisateur(CreationBD& creationBD, const QString& utilisateurId, QObject* parent)
    : QObject(parent), m_creationBD(creationBD), m_utilisateurId(utilisateurId)
{
}

void GestionnaireUtilisateur::chargerInformationsUtilisateur(QLabel* labelBienvenue)
{
    if (!m_creationBD.estOuverte()) {
        qWarning() << "Base de données non ouverte!";
        return;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT nom_complet FROM utilisateurs WHERE id = ?");
    query.addBindValue(m_utilisateurId.toInt());

    if (query.exec() && query.next()) {
        QString nomComplet = query.value("nom_complet").toString();
        labelBienvenue->setText("Bienvenue, " + nomComplet + " !");
    } else {
        qWarning() << "Erreur récupération nom utilisateur:" << query.lastError().text();
        labelBienvenue->setText("Bienvenue !");
    }
}

void GestionnaireUtilisateur::mettreAJourInformationsUtilisateur(
    const QString& nouveauNom,
    const QString& nouvelEmail,
    const QString& nouveauMotDePasse,
    const QString& confirmationMotDePasse,
    QWidget* parentWindow)
{
    // Vérification que la base de données est accessible
    if (!m_creationBD.estOuverte()) {
        QMessageBox::critical(parentWindow, "Erreur", "Base de données non accessible!");
        return;
    }

    QSqlDatabase db = m_creationBD.getDatabase();

    // RÉCUPÉRATION DES VALEURS ACTUELLES
    QSqlQuery queryActuel(db);
    queryActuel.prepare("SELECT nom_complet, email, mot_de_passe FROM utilisateurs WHERE id = ?");
    queryActuel.addBindValue(m_utilisateurId.toInt());

    if (!queryActuel.exec() || !queryActuel.next()) {
        QMessageBox::critical(parentWindow, "Erreur", "Impossible de récupérer les informations actuelles de l'utilisateur.");
        return;
    }

    QString nomActuel = queryActuel.value("nom_complet").toString();
    QString emailActuel = queryActuel.value("email").toString();
    QString motDePasseActuel = queryActuel.value("mot_de_passe").toString();

    // DÉTERMINATION DES VALEURS FINALES
    QString nomFinal = nouveauNom.isEmpty() ? nomActuel : nouveauNom;
    QString emailFinal = nouvelEmail.isEmpty() ? emailActuel : nouvelEmail;

    // Pour le mot de passe, on ne le change que si les deux champs sont renseignés
    bool modifierMotDePasse = !nouveauMotDePasse.isEmpty() && !confirmationMotDePasse.isEmpty();
    QString motDePasseFinal = motDePasseActuel; // Par défaut, on garde l'ancien

    // VALIDATION DES DONNÉES
    // Validation email
    if (!nouvelEmail.isEmpty()) {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        if (!emailRegex.match(nouvelEmail).hasMatch()) {
            QMessageBox::warning(parentWindow, "Erreur de saisie", "Format d'email invalide.");
            return;
        }
    }

    // Validation mot de passe
    if (modifierMotDePasse) {
        if (nouveauMotDePasse.length() < 6) {
            QMessageBox::warning(parentWindow, "Erreur de saisie", "Le mot de passe doit contenir au moins 6 caractères.");
            return;
        }

        if (nouveauMotDePasse != confirmationMotDePasse) {
            QMessageBox::warning(parentWindow, "Erreur de saisie", "Les mots de passe ne correspondent pas.");
            return;
        }

        // Hachage du nouveau mot de passe
        motDePasseFinal = QString(QCryptographicHash::hash(nouveauMotDePasse.toUtf8(), QCryptographicHash::Sha256).toHex());
    }

    // Vérification email unique
    if (emailFinal != emailActuel) {
        QSqlQuery queryVerifEmail(db);
        queryVerifEmail.prepare("SELECT id FROM utilisateurs WHERE email = ? AND id != ?");
        queryVerifEmail.addBindValue(emailFinal);
        queryVerifEmail.addBindValue(m_utilisateurId.toInt());

        if (!queryVerifEmail.exec()) {
            QMessageBox::critical(parentWindow, "Erreur", "Erreur lors de la vérification de l'email : " + queryVerifEmail.lastError().text());
            return;
        }

        if (queryVerifEmail.next()) {
            QMessageBox::warning(parentWindow, "Erreur", "Cet email est déjà utilisé par un autre compte.");
            return;
        }
    }

    // MISE À JOUR DANS LA BASE DE DONNÉES
    QSqlQuery queryUpdate(db);
    queryUpdate.prepare("UPDATE utilisateurs SET nom_complet = ?, email = ?, mot_de_passe = ? WHERE id = ?");
    queryUpdate.addBindValue(nomFinal);
    queryUpdate.addBindValue(emailFinal);
    queryUpdate.addBindValue(motDePasseFinal);
    queryUpdate.addBindValue(m_utilisateurId.toInt());

    if (!queryUpdate.exec()) {
        QMessageBox::critical(parentWindow, "Erreur", "Erreur lors de la mise à jour : " + queryUpdate.lastError().text());
        return;
    }

    // Mise à jour du nom dans les comptes
    if (nomFinal != nomActuel) {
        QSqlQuery queryUpdateComptes(db);
        queryUpdateComptes.prepare("UPDATE comptes SET nom_titulaire = ? WHERE id_utilisateur = ?");
        queryUpdateComptes.addBindValue(nomFinal);
        queryUpdateComptes.addBindValue(m_utilisateurId.toInt());

        if (!queryUpdateComptes.exec()) {
            qWarning() << "Erreur mise à jour nom dans les comptes:" << queryUpdateComptes.lastError().text();
        }
    }

    // CONFIRMATION ET MISE À JOUR DE L'INTERFACE
    QString messageSucces = "Vos informations ont été mises à jour avec succès !\n\nModifications effectuées :\n";
    if (nomFinal != nomActuel) messageSucces += "- Nom modifié\n";
    if (emailFinal != emailActuel) messageSucces += "- Email modifié\n";
    if (modifierMotDePasse) messageSucces += "- Mot de passe modifié\n";

    QMessageBox::information(parentWindow, "Succès", messageSucces);
    emit utilisateurModifie();
}
