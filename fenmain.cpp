#include "fenmain.h"
#include "ui_fenmain.h"
#include "utilitairesmotdepasse.h"
#include "highlightdelegate.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>
#include <QStyle>

fenMain::fenMain(CreationBD& m_BD, QWidget *parent, const QString &utilisateur_id)
    : QMainWindow(parent),
    ui(new Ui::fenMain),
    m_utilisateur_id(utilisateur_id),
    m_soldeVisibleCompteCourant(true),
    m_soldeVisibleCompteEpargne(true),
    m_soldeAnimation(new AnimationSolde(this)),
    m_banque("MyBank", m_BD),
    m_creationBD(m_BD)
{
    ui->setupUi(this);
    this->installEventFilter(this);

    // Initialisation des gestionnaires
    m_gestionnaireComptes = new GestionnaireComptes(m_banque, m_creationBD, m_utilisateur_id, this);
    m_gestionnaireTransactions = new GestionnaireTransactions(m_banque, m_creationBD, this);
    m_gestionnaireHistorique = new GestionnaireHistorique(m_creationBD, m_utilisateur_id, this);
    m_gestionnaireTheme = new GestionnaireTheme(ui->zone_bouton_bascule, this);
    m_gestionnaireInterface = new GestionnaireInterface(this);
    m_gestionnaireUtilisateur = new GestionnaireUtilisateur(m_creationBD, m_utilisateur_id, this);

    // Initialiser le thème
    m_gestionnaireTheme->initialiserThemeCouleur(this);

    // Connexion des signaux des gestionnaires
    connect(m_gestionnaireComptes, &GestionnaireComptes::comptesModifies, this, [this]() {
        m_gestionnaireComptes->mettreAJourAffichageComptes(
            ui->label_solde_compte_courant,
            ui->label_decouvert_autorise_compte_courant,
            ui->label_numero_de_compte_courant,
            ui->label_date_creation_compte_courant,
            ui->label_derniere_transaction_compte_courant,
            ui->label_solde_compte_epargne,
            ui->label_taux_interet_compte_epargne,
            ui->label_numero_de_compte_epargne,
            ui->label_date_creation_compte_epargne,
            ui->label_derniere_transaction_compte_epargne
            );
    });

    connect(m_gestionnaireTransactions, &GestionnaireTransactions::transactionEffectuee, this, [this]() {
        m_gestionnaireComptes->mettreAJourAffichageComptes(
            ui->label_solde_compte_courant,
            ui->label_decouvert_autorise_compte_courant,
            ui->label_numero_de_compte_courant,
            ui->label_date_creation_compte_courant,
            ui->label_derniere_transaction_compte_courant,
            ui->label_solde_compte_epargne,
            ui->label_taux_interet_compte_epargne,
            ui->label_numero_de_compte_epargne,
            ui->label_date_creation_compte_epargne,
            ui->label_derniere_transaction_compte_epargne
            );
        afficherTop5DernieresTransactions();
        chargerHistoriqueTransactions();
    });

    connect(m_gestionnaireUtilisateur, &GestionnaireUtilisateur::utilisateurModifie, this, [this]() {
        m_gestionnaireUtilisateur->chargerInformationsUtilisateur(ui->labelBienvenue);
        m_gestionnaireComptes->mettreAJourAffichageComptes(
            ui->label_solde_compte_courant,
            ui->label_decouvert_autorise_compte_courant,
            ui->label_numero_de_compte_courant,
            ui->label_date_creation_compte_courant,
            ui->label_derniere_transaction_compte_courant,
            ui->label_solde_compte_epargne,
            ui->label_taux_interet_compte_epargne,
            ui->label_numero_de_compte_epargne,
            ui->label_date_creation_compte_epargne,
            ui->label_derniere_transaction_compte_epargne
            );
    });



    connect(m_gestionnaireTheme, &GestionnaireTheme::themeChange,
            this, &fenMain::appliquerTheme);

    // Initialisation de l'interface
    qApp->installEventFilter(this);
    this->showMaximized();

    // Création du delegate pour la recherche
    HighlightDelegate* m_highlightDelegate = new HighlightDelegate(ui->tableWidget_historique_transaction, this);
    ui->tableWidget_historique_transaction->setItemDelegate(m_highlightDelegate);
    connect(ui->ZoneRecherche, &QLineEdit::textChanged, m_highlightDelegate, &HighlightDelegate::handleSearchChanged);

    // Configuration initiale
    ui->comboBox_type_operation_onglet_depot_retrait->clear();
    ui->comboBox_type_operation_onglet_depot_retrait->addItem("Dépôt");
    ui->comboBox_type_operation_onglet_depot_retrait->addItem("Retrait");
    ui->comboBox_type_operation_onglet_depot_retrait->setCurrentIndex(0);

    ui->mes_pages->setCurrentWidget(ui->page_dashboard);
    ui->btn_masquer_solde_compte_courant->installEventFilter(this);
    ui->btn_masquer_solde_compte_epargne->installEventFilter(this);

    mettreAJourStyleBoutonsLateraux();
    m_gestionnaireInterface->appliquerStyleBoutonMasquage(ui->btn_masquer_solde_compte_courant, false);
    m_gestionnaireInterface->appliquerStyleBoutonMasquage(ui->btn_masquer_solde_compte_epargne, false);
    m_gestionnaireInterface->mettreAjourIcone(ui->btn_masquer_solde_compte_courant, m_soldeVisibleCompteCourant);
    m_gestionnaireInterface->mettreAjourIcone(ui->btn_masquer_solde_compte_epargne, m_soldeVisibleCompteEpargne);

    UtilitairesMotDePasse::configurerBoutonVisibilite(ui->lineEdit_mot_de_passe_parametre);
    UtilitairesMotDePasse::configurerBoutonVisibilite(ui->lineEdit_mot_de_passe_confirmation_parametre);

    chargerDonneesDepuisBD();
}



void fenMain::appliquerTheme(bool themeSombre)
{
    QString cheminTheme = themeSombre
                              ? ":/themes/theme_sombre.txt"
                              : ":/themes/theme_clair.txt";

    QFile fichierTheme(cheminTheme);
    if (fichierTheme.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fichierTheme);
        QString styleSheet = in.readAll();
        this->setStyleSheet(styleSheet);
        fichierTheme.close();
    } else {
        qWarning() << "Impossible de charger le fichier de thème:" << cheminTheme;
        this->setStyleSheet("");
    }
}


fenMain::~fenMain()
{
    // Sauvegarde des données
    QSqlDatabase db = m_creationBD.getDatabase();
    if (db.isOpen()) {
        db.transaction();
        for (CompteBancaire* compte : m_banque.getComptes()) {
            QSqlQuery query(db);
            query.prepare("UPDATE comptes SET solde = ?, derniere_operation = ? WHERE numero_compte = ?");
            query.addBindValue(compte->getSolde());
            query.addBindValue(compte->getDerniereOperation());
            query.addBindValue(compte->getNumeroCompte());
            query.exec();
        }
        db.commit();
    }

    delete ui;
    delete m_soldeAnimation;

    // Suppression des gestionnaires
    delete m_gestionnaireComptes;
    delete m_gestionnaireTransactions;
    delete m_gestionnaireHistorique;
    delete m_gestionnaireTheme;
    delete m_gestionnaireInterface;
    delete m_gestionnaireUtilisateur;

    emit fenetreDetruite();
}

void fenMain::chargerDonneesDepuisBD()
{
    m_gestionnaireUtilisateur->chargerInformationsUtilisateur(ui->labelBienvenue);
    m_banque.chargerTousLesComptes();
    m_gestionnaireComptes->mettreAJourAffichageComptes(
        ui->label_solde_compte_courant,
        ui->label_decouvert_autorise_compte_courant,
        ui->label_numero_de_compte_courant,
        ui->label_date_creation_compte_courant,
        ui->label_derniere_transaction_compte_courant,
        ui->label_solde_compte_epargne,
        ui->label_taux_interet_compte_epargne,
        ui->label_numero_de_compte_epargne,
        ui->label_date_creation_compte_epargne,
        ui->label_derniere_transaction_compte_epargne
        );
    afficherTop5DernieresTransactions();
    chargerHistoriqueTransactions();
}

void fenMain::afficherTop5DernieresTransactions()
{
    m_gestionnaireHistorique->afficherTop5DernieresTransactions(ui->tableWidget_top_5_derniere_transaction);
}

void fenMain::chargerHistoriqueTransactions()
{
    m_gestionnaireHistorique->chargerHistoriqueComplet(ui->tableWidget_historique_transaction);
}

void fenMain::mettreAJourStyleBoutonsLateraux()
{
    m_gestionnaireInterface->mettreAJourStyleBoutonsLateraux(
        ui->btn_dashboard_barre_latterale,
        ui->btn_historique_barre_latterale,
        ui->btn_parametres_barre_latterale,
        ui->mes_pages->currentWidget(),
        ui->page_dashboard,
        ui->page_historique,
        ui->page_parametres
        );
}

bool fenMain::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->btn_masquer_solde_compte_courant || obj == ui->btn_masquer_solde_compte_epargne) {
        QToolButton* button = qobject_cast<QToolButton*>(obj);
        if (!button) return QMainWindow::eventFilter(obj, event);

        if (event->type() == QEvent::Enter) {
            m_gestionnaireInterface->appliquerStyleBoutonMasquage(button, true);
            bool visible = (button == ui->btn_masquer_solde_compte_courant) ? m_soldeVisibleCompteCourant : m_soldeVisibleCompteEpargne;
            m_gestionnaireInterface->mettreAjourIcone(button, visible);
            return true;
        }
        else if (event->type() == QEvent::Leave) {
            m_gestionnaireInterface->appliquerStyleBoutonMasquage(button, false);
            bool visible = (button == ui->btn_masquer_solde_compte_courant) ? m_soldeVisibleCompteCourant : m_soldeVisibleCompteEpargne;
            m_gestionnaireInterface->mettreAjourIcone(button, visible);
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// SLOTS

void fenMain::on_btn_valider_transaction_clicked()
{
    int indexOnglet = ui->mes_onglets_page_transaction->currentIndex();

    if (indexOnglet == 0) { // Onglet dépôt/retrait
        QString numeroCompte = ui->sai_numero_compte_onglet_depot_retrait->text().trimmed();
        double montant = ui->doubleSpinBox_montant_onglet_depot_retrait->value();
        QString motif = ui->textEdit_motif_onglet_depot_retrait->toPlainText().trimmed();
        QString typeOperation = ui->comboBox_type_operation_onglet_depot_retrait->currentText();

        if (numeroCompte.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un numéro de compte!");
            return;
        }

        if (montant <= 0) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un montant valide!");
            return;
        }

        CompteBancaire* compte = m_banque.trouverCompte(numeroCompte);
        if (!compte) {
            QMessageBox::warning(this, "Erreur", "Compte introuvable!");
            return;
        }

        if (typeOperation == "Dépôt") {
            m_gestionnaireTransactions->effectuerDepot(compte, montant, motif);
        } else if (typeOperation == "Retrait") {
            m_gestionnaireTransactions->effectuerRetrait(compte, montant, motif);
        }
    } else if (indexOnglet == 1) { // Onglet virement
        QString compteSource = ui->sai_numero_compte_source_onglet_virement->text().trimmed();
        QString compteDest = ui->sai_numero_compte_beneficiaire_onglet_virement->text().trimmed();
        double montant = ui->doubleSpinBox_montant_onglet_virement->value();
        QString motif = ui->textEdit_motif_onglet_virement->toPlainText().trimmed();

        if (compteSource.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir le compte source!");
            return;
        }

        if (compteDest.isEmpty()) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir le compte bénéficiaire!");
            return;
        }

        if (montant <= 0) {
            QMessageBox::warning(this, "Erreur", "Veuillez saisir un montant valide!");
            return;
        }

        m_gestionnaireTransactions->effectuerVirement(compteSource, compteDest, montant, motif);
    }

    // Réinitialiser les champs
    ui->sai_numero_compte_onglet_depot_retrait->clear();
    ui->doubleSpinBox_montant_onglet_depot_retrait->setValue(0);
    ui->textEdit_motif_onglet_depot_retrait->clear();
    ui->sai_numero_compte_source_onglet_virement->clear();
    ui->sai_numero_compte_beneficiaire_onglet_virement->clear();
    ui->doubleSpinBox_montant_onglet_virement->setValue(0);
    ui->textEdit_motif_onglet_virement->clear();
}

void fenMain::on_btn_supprimer_compte_courant_clicked()
{
    if (QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer votre compte courant?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_gestionnaireComptes->supprimerCompteCourant();
    }
}

void fenMain::on_btn_supprimer_compte_epargne_clicked()
{
    if (QMessageBox::question(this, "Confirmation", "Voulez-vous vraiment supprimer votre compte épargne?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_gestionnaireComptes->supprimerCompteEpargne();
    }
}

void fenMain::on_btn_masquer_solde_compte_courant_clicked()
{
    m_soldeVisibleCompteCourant = !m_soldeVisibleCompteCourant;
    m_gestionnaireInterface->appliquerEffetFlou(
        ui->label_solde_compte_courant,
        !m_soldeVisibleCompteCourant,
        m_soldeAnimation
        );
    m_gestionnaireInterface->mettreAjourIcone(
        ui->btn_masquer_solde_compte_courant,
        m_soldeVisibleCompteCourant
        );
}

void fenMain::on_btn_masquer_solde_compte_epargne_clicked()
{
    m_soldeVisibleCompteEpargne = !m_soldeVisibleCompteEpargne;
    m_gestionnaireInterface->appliquerEffetFlou(
        ui->label_solde_compte_epargne,
        !m_soldeVisibleCompteEpargne,
        m_soldeAnimation
        );
    m_gestionnaireInterface->mettreAjourIcone(
        ui->btn_masquer_solde_compte_epargne,
        m_soldeVisibleCompteEpargne
        );
}

void fenMain::on_btn_dashboard_barre_latterale_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_dashboard);
    mettreAJourStyleBoutonsLateraux();
    afficherTop5DernieresTransactions();
}

void fenMain::on_btn_historique_barre_latterale_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
    chargerHistoriqueTransactions();
}

void fenMain::on_btn_parametres_barre_latterale_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_parametres);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_consulter_compte_epargne_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_consulter_compte_courant_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_effectuer_transaction_compte_courant_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_transaction);
    ui->mes_onglets_page_transaction->setCurrentWidget(ui->onglet_depot_retrait);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_effectuer_transaction_compte_epargne_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_transaction);
    ui->mes_onglets_page_transaction->setCurrentWidget(ui->onglet_depot_retrait);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_voir_liste_complete_transaction_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_ajouter_compte_courant_clicked()
{
    m_gestionnaireComptes->creerCompteCourant();
}

void fenMain::on_btn_ajouter_compte_epargne_clicked()
{
    m_gestionnaireComptes->creerCompteEpargne();
}

void fenMain::on_btn_modifier_info_tutilaire_parametre_clicked()
{
    QString nouveauNom = ui->lineEdit_nom_titulaire_parametre->text().trimmed();
    QString nouvelEmail = ui->lineEdit_email_titulaire_parametre->text().trimmed();
    QString nouveauMotDePasse = ui->lineEdit_mot_de_passe_parametre->text();
    QString confirmationMotDePasse = ui->lineEdit_mot_de_passe_confirmation_parametre->text();

    m_gestionnaireUtilisateur->mettreAJourInformationsUtilisateur(
        nouveauNom, nouvelEmail, nouveauMotDePasse, confirmationMotDePasse, this
        );
}

void fenMain::on_btn_deconnexion_barre_latterale_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Déconnexion",
                                  "Êtes-vous sûr de vouloir vous déconnecter ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Sauvegarder avant de déconnecter
        QSqlDatabase db = m_creationBD.getDatabase();
        if (db.isOpen()) {
            db.transaction();
            for (CompteBancaire* compte : m_banque.getComptes()) {
                QSqlQuery query(db);
                query.prepare("UPDATE comptes SET solde = ?, derniere_operation = ? WHERE numero_compte = ?");
                query.addBindValue(compte->getSolde());
                query.addBindValue(compte->getDerniereOperation());
                query.addBindValue(compte->getNumeroCompte());
                query.exec();
            }
            db.commit();
        }

        emit demandeDeconnexion();
    }
}
