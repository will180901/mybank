#include "gestionnairehistorique.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QLabel>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QRegularExpression>

GestionnaireHistorique::GestionnaireHistorique(CreationBD& creationBD, const QString& utilisateurId, QObject* parent)
    : QObject(parent), m_creationBD(creationBD), m_utilisateurId(utilisateurId)
{

     chargerComptesUtilisateur();
}



void GestionnaireHistorique::chargerComptesUtilisateur()
{
    m_mesComptes.clear();
    if (!m_creationBD.estOuverte()) {
        qWarning() << "Base de données non ouverte!";
        return;
    }

    QSqlQuery query(m_creationBD.getDatabase());
    query.prepare("SELECT numero_compte FROM comptes WHERE id_utilisateur = :id");
    query.bindValue(":id", m_utilisateurId.toInt());

    if (!query.exec()) {
        qWarning() << "Erreur de chargement des comptes:" << query.lastError();
        return;
    }

    while (query.next()) {
        m_mesComptes << query.value("numero_compte").toString();
    }
}


void GestionnaireHistorique::configurerTableHistorique(QTableWidget* table)
{
    table->setUpdatesEnabled(false);
    table->setSortingEnabled(false);
    table->clearContents();
    table->setRowCount(0);

    // Configuration des colonnes
    QStringList headers;
    headers << "Nom expéditeur"
            << "Nom bénéficiaire"
            << "Compte source"
            << "Compte bénéficiaire"
            << "Type"
            << "Montant (FCFA)"
            << "Solde avant"
            << "Solde après"
            << "Date"
            << "Motif";

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);

    // Configuration de l'apparence
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setColumnWidth(0, 120); // Nom expéditeur
    table->setColumnWidth(1, 120); // Nom bénéficiaire
    table->setColumnWidth(2, 130); // Compte source
    table->setColumnWidth(3, 130); // Compte bénéficiaire
    table->setColumnWidth(4, 80);  // Type
    table->setColumnWidth(5, 120); // Montant
    table->setColumnWidth(6, 100); // Solde avant
    table->setColumnWidth(7, 100); // Solde après
    table->setColumnWidth(8, 100); // Date
}

QString GestionnaireHistorique::determinerClasseBadgeMontant(const QString& typeOp, const QString& compteDest, const QStringList& mesComptes) const
{
    if (typeOp == "depot") {
        return "MontantBadgePositif";
    } else if (typeOp == "retrait") {
        return "MontantBadgeNegatif";
    } else if (typeOp == "virement") {
        if (mesComptes.contains(compteDest)) {
            return "MontantBadgePositif";
        } else {
            return "MontantBadgeNegatif";
        }
    }
    return "MontantBadgeNeutre";
}

void GestionnaireHistorique::peuplerLigneHistorique(QTableWidget* table, int row, const QSqlQuery& query)
{
    // Extraction des données
    QString dateStr = query.value("date_operation").toString();
    QString motif = query.value("motif").toString();
    QString typeOp = query.value("type_operation").toString();
    double montant = query.value("montant").toDouble();
    QString compteSource = query.value("compte_source").toString();
    QString nomExpediteur = query.value("nom_expediteur").toString();
    QString compteDest = query.value("compte_dest").toString();
    QString nomBeneficiaire = query.value("nom_beneficiaire").toString();
    double soldeAvant = query.value("solde_avant").toDouble();
    double soldeApres = query.value("solde_apres").toDouble();

    // Formatage de la date
    QDateTime dateOp = QDateTime::fromString(dateStr, Qt::ISODate);
    if (!dateOp.isValid()) {
        dateOp = QDateTime::fromString(dateStr, "yyyy-MM-dd hh:mm:ss");
    }

    // Traduction du type d'opération
    QString typeAffichage;
    if (typeOp == "depot") typeAffichage = "Dépôt";
    else if (typeOp == "retrait") typeAffichage = "Retrait";
    else if (typeOp == "virement") typeAffichage = "Virement";
    else typeAffichage = typeOp;

    // Détermination du type de montant pour le badge
    QString montantText;
    QString badgeClass;

    if (typeOp == "depot") {
        montantText = "+" + QString::number(montant, 'f', 2);
        badgeClass = "MontantBadgePositif";
    } else if (typeOp == "retrait") {
        montantText = "-" + QString::number(montant, 'f', 2);
        badgeClass = "MontantBadgeNegatif";
    }else if (typeOp == "virement") {
        // Remplacer 'mesComptes' par 'm_mesComptes'
        if (compteDest.isEmpty() || !m_mesComptes.contains(compteDest)) {
            montantText = "-" + QString::number(montant, 'f', 2);
            badgeClass = "MontantBadgeNegatif";
        } else {
            montantText = "+" + QString::number(montant, 'f', 2);
            badgeClass = "MontantBadgePositif";
        }
    } else {
        montantText = QString::number(montant, 'f', 2);
        badgeClass = "MontantBadgeNeutre";
    }

    // Ajout des données dans le tableau
    table->setItem(row, 0, new QTableWidgetItem(nomExpediteur));
    table->setItem(row, 1, new QTableWidgetItem(nomBeneficiaire.isEmpty() ? "N/A" : nomBeneficiaire));
    table->setItem(row, 2, new QTableWidgetItem(compteSource));
    table->setItem(row, 3, new QTableWidgetItem(compteDest.isEmpty() ? "N/A" : compteDest));
    table->setItem(row, 4, new QTableWidgetItem(typeAffichage));

    // Colonne Montant (avec badge)
    QLabel *montantLabel = new QLabel();
    montantLabel->setText(montantText + " FCFA");
    montantLabel->setAlignment(Qt::AlignCenter);
    montantLabel->setProperty("class", badgeClass);

    QWidget *montantCellWidget = new QWidget();
    QHBoxLayout *montantLayout = new QHBoxLayout(montantCellWidget);
    montantLayout->addWidget(montantLabel);
    montantLayout->setAlignment(Qt::AlignCenter);
    montantLayout->setContentsMargins(0, 0, 0, 0);
    montantCellWidget->setLayout(montantLayout);
    table->setCellWidget(row, 5, montantCellWidget);

    // Colonnes Solde avant et après
    table->setItem(row, 6, new QTableWidgetItem(QString::number(soldeAvant, 'f', 2) + " FCFA"));
    table->setItem(row, 7, new QTableWidgetItem(QString::number(soldeApres, 'f', 2) + " FCFA"));
    table->setItem(row, 8, new QTableWidgetItem(dateOp.toString("dd/MM/yyyy")));
    table->setItem(row, 9, new QTableWidgetItem(motif.isEmpty() ? "Sans motif" : motif));
}

void GestionnaireHistorique::afficherTop5DernieresTransactions(QTableWidget* table)
{
    configurerTableHistorique(table);

    // Vérification de la base de données
    if (!m_creationBD.estOuverte()) {
        qCritical() << "Base de données non ouverte!";
        return;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Requête pour récupérer les 5 dernières transactions
    query.prepare(
        "SELECT "
        "   t.date_operation, "
        "   t.libelle AS motif, "
        "   t.type_operation, "
        "   t.montant, "
        "   c1.numero_compte AS compte_source, "
        "   c1.nom_titulaire AS nom_expediteur, "
        "   c2.numero_compte AS compte_dest, "
        "   c2.nom_titulaire AS nom_beneficiaire, "
        "   t.solde_avant, "
        "   t.solde_apres "
        "FROM transactions t "
        "INNER JOIN comptes c1 ON t.id_compte_source = c1.id "
        "LEFT JOIN comptes c2 ON t.id_compte_beneficiaire = c2.id "
        "WHERE c1.id_utilisateur = :user_id OR c2.id_utilisateur = :user_id "
        "ORDER BY t.date_operation DESC "
        "LIMIT 5"
        );

    query.bindValue(":user_id", m_utilisateurId.toInt());

    if (!query.exec()) {
        qCritical() << "Erreur SQL:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        peuplerLigneHistorique(table, row, query);
        row++;
    }

    table->setUpdatesEnabled(true);
}

void GestionnaireHistorique::chargerHistoriqueComplet(QTableWidget* table)
{
    qDebug() << "Chargement de l'historique complet des transactions...";
    configurerTableHistorique(table);

    // Vérification de la base de données
    if (!m_creationBD.estOuverte()) {
        qCritical() << "Base de données non ouverte!";
        return;
    }

    QSqlDatabase db = m_creationBD.getDatabase();
    QSqlQuery query(db);

    // Requête SQL
    query.prepare(
        "SELECT "
        "   t.date_operation, "
        "   t.libelle AS motif, "
        "   t.type_operation, "
        "   t.montant, "
        "   c1.numero_compte AS compte_source, "
        "   c1.nom_titulaire AS nom_expediteur, "
        "   c2.numero_compte AS compte_dest, "
        "   c2.nom_titulaire AS nom_beneficiaire, "
        "   t.solde_avant, "
        "   t.solde_apres "
        "FROM transactions t "
        "INNER JOIN comptes c1 ON t.id_compte_source = c1.id "
        "LEFT JOIN comptes c2 ON t.id_compte_beneficiaire = c2.id "
        "WHERE c1.id_utilisateur = :user_id OR c2.id_utilisateur = :user_id "
        "ORDER BY t.date_operation DESC"
        );

    query.bindValue(":user_id", m_utilisateurId.toInt());
    qDebug() << "ID utilisateur:" << m_utilisateurId.toInt();

    if (!query.exec()) {
        qCritical() << "Erreur SQL:" << query.lastError().text();
        qCritical() << "Erreur détaillée:" << query.lastError().databaseText();
        return;
    }

    // Préparation du tableau
    int rowCount = 0;
    while (query.next()) rowCount++;
    table->setRowCount(rowCount);
    qDebug() << "Nombre de transactions trouvées:" << rowCount;

    // Réexécution de la requête pour parcourir les résultats
    query.exec();
    int row = 0;
    while (query.next()) {
        peuplerLigneHistorique(table, row, query);
        row++;
    }

    // Configuration finale du tableau
    table->setSortingEnabled(true);
    table->sortByColumn(8, Qt::DescendingOrder); // Trier par la colonne 'Date'
    table->resizeColumnsToContents();
    table->horizontalHeader()->setSectionResizeMode(9, QHeaderView::Stretch); // Étirer la colonne 'Motif'
    table->setUpdatesEnabled(true);

    qDebug() << "Historique des transactions chargé avec succès";
}
