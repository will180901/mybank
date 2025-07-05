#include "fenmain.h"
#include "ui_fenmain.h"
#include "utilitairesmotdepasse.h"
#include "highlightdelegate.h"
#include "widgetnotificationmoderne.h"

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
#include <QCloseEvent>
#include <QTimer>
#include <QVBoxLayout>
#include <QDesktopServices>  // Pour ouvrir le navigateur

fenMain::fenMain(CreationBD& m_BD, QWidget *parent, const QString &utilisateur_id)
    : QMainWindow(parent),
    ui(new Ui::fenMain),
    m_utilisateur_id(utilisateur_id),
    m_soldeVisibleCompteCourant(true),
    m_soldeVisibleCompteEpargne(true),
    m_soldeAnimation(new AnimationSolde(this)),
    m_banque("MyBank", m_BD),
    m_creationBD(m_BD),
    m_loader(nullptr),
    m_loaderCount(0)
{
    ui->setupUi(this);
    this->installEventFilter(this);

    // Configuration du timer de délai minimum
    m_minimumDisplayTimer.setSingleShot(true);
    connect(&m_minimumDisplayTimer, &QTimer::timeout, this, &fenMain::arreterLoaderApresDelai);

    // Création et configuration du loader
    m_loader = new SkeletonLoader(this);
    m_loader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_loader->setFixedHeight(4);
    m_loader->hide();
    m_loader->setHighlightColor(QColor(180, 220, 255)); // Bleu clair pâle

    // Insertion du loader dans l'interface
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        mainLayout->insertWidget(1, m_loader);
    } else {
        qWarning() << "Main layout not found, using fallback positioning";
        m_loader->setParent(ui->centralwidget);
        m_loader->setGeometry(0, ui->bar_nav->height(), width(), 4);
    }

    // Initialisation des gestionnaires
    m_gestionnaireComptes = new GestionnaireComptes(m_banque, m_creationBD, m_utilisateur_id, this);
    m_gestionnaireTransactions = new GestionnaireTransactions(m_banque, m_creationBD, this);
    m_gestionnaireHistorique = new GestionnaireHistorique(m_creationBD, m_utilisateur_id, this);
    m_gestionnaireInterface = new GestionnaireInterface(this);
    m_gestionnaireUtilisateur = new GestionnaireUtilisateur(m_creationBD, m_utilisateur_id, this);

    m_gestionnaireTheme = new GestionnaireTheme(ui->zone_bouton_bascule, this);

    // Connecter au gestionnaire global pour synchroniser
    GestionnaireTheme* gestionnaireGlobal = GestionnaireTheme::instance();
    connect(gestionnaireGlobal, &GestionnaireTheme::themeChangeGlobal,
            this, &fenMain::appliquerTheme);

    // S'assurer que le thème est synchronisé
    connect(m_gestionnaireTheme, &GestionnaireTheme::themeChange,
            [](bool themeSombre) {
                GestionnaireTheme::instance()->forcerTheme(themeSombre);
            });

    // Initialiser le thème
    m_gestionnaireTheme->initialiserThemeCouleur(this);
    if (m_loader) {
        m_loader->setTheme(m_gestionnaireTheme->estThemeSombreActif());
    }

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

fenMain::~fenMain()
{
    delete ui;
    emit fenetreDetruite();
}

// Gestion du loader
void fenMain::demarrerLoader()
{
    if (m_loaderCount == 0) {
        m_loaderTimer.start();
        m_loader->show();
        m_loader->demarrerAnimation();
        qApp->processEvents();

        // Démarrer le timer de délai minimum de 5 secondes
        m_minimumDisplayTimer.start(5000);
    }
    m_loaderCount++;
}

void fenMain::arreterLoader()
{
    if (m_loaderCount > 0) {
        m_loaderCount--;
    }

    // On ne cache pas immédiatement, on attend que le timer expire
    // La méthode arreterLoaderApresDelai s'occupera de la fermeture
}

void fenMain::arreterLoaderApresDelai()
{
    // Si le timer est encore actif, ne rien faire
    if (m_minimumDisplayTimer.isActive()) return;

    if (m_loaderCount <= 0) {
        m_loaderCount = 0;
        m_loader->arreterAnimation();
        m_loader->hide();
    }
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

    if (m_loader) {
        m_loader->setTheme(themeSombre);
    }
}

void fenMain::closeEvent(QCloseEvent *event)
{
    // Arrêter le timer pour éviter les callbacks après fermeture
    m_minimumDisplayTimer.stop();
    sauvegarderDonnees();
    event->accept();
}

void fenMain::sauvegarderDonnees()
{
    LoaderGuard guard(this);

    QSqlDatabase db = m_creationBD.getDatabase();
    if (!db.isOpen()) {
        qWarning() << "La base de données n'est pas ouverte lors de la sauvegarde.";
        return;
    }

    if (!db.transaction()) {
        qWarning() << "Échec de la transaction:" << db.lastError().text();
        return;
    }

    for (CompteBancaire* compte : m_banque.getComptes()) {
        QSqlQuery query(db);
        query.prepare("UPDATE comptes SET solde = ?, derniere_operation = ? WHERE numero_compte = ?");
        query.addBindValue(compte->getSolde());
        query.addBindValue(compte->getDerniereOperation());
        query.addBindValue(compte->getNumeroCompte());

        if (!query.exec()) {
            qWarning() << "Échec de la sauvegarde du compte"
                       << compte->getNumeroCompte()
                       << ":" << query.lastError().text();
            db.rollback();
            return;
        }
    }

    if (!db.commit()) {
        qWarning() << "Échec du commit:" << db.lastError().text();
    }
}

void fenMain::chargerDonneesDepuisBD()
{
    LoaderGuard guard(this);

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
    LoaderGuard guard(this);
    m_gestionnaireHistorique->afficherTop5DernieresTransactions(ui->tableWidget_top_5_derniere_transaction);
}

void fenMain::chargerHistoriqueTransactions()
{
    LoaderGuard guard(this);
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
    LoaderGuard guard(this);

    int indexOnglet = ui->mes_onglets_page_transaction->currentIndex();

    if (indexOnglet == 0) {
        QString numeroCompte = ui->sai_numero_compte_onglet_depot_retrait->text().trimmed();
        double montant = ui->doubleSpinBox_montant_onglet_depot_retrait->value();
        QString motif = ui->textEdit_motif_onglet_depot_retrait->toPlainText().trimmed();
        QString typeOperation = ui->comboBox_type_operation_onglet_depot_retrait->currentText();

        if (numeroCompte.isEmpty()) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Veuillez saisir un numéro de compte!", this);
            return;
        }

        if (montant <= 0) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Veuillez saisir un montant valide!", this);
            return;
        }

        CompteBancaire* compte = m_banque.trouverCompte(numeroCompte);
        if (!compte) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Compte introuvable!", this);
            return;
        }

        if (typeOperation == "Dépôt") {
            m_gestionnaireTransactions->effectuerDepot(compte, montant, motif);
            WidgetNotificationModerne::afficherInformation("Succès", "Dépôt effectué avec succès", this);
        } else if (typeOperation == "Retrait") {
            m_gestionnaireTransactions->effectuerRetrait(compte, montant, motif);
            WidgetNotificationModerne::afficherInformation("Succès", "Retrait effectué avec succès", this);
        }
    } else if (indexOnglet == 1) {
        QString compteSource = ui->sai_numero_compte_source_onglet_virement->text().trimmed();
        QString compteDest = ui->sai_numero_compte_beneficiaire_onglet_virement->text().trimmed();
        double montant = ui->doubleSpinBox_montant_onglet_virement->value();
        QString motif = ui->textEdit_motif_onglet_virement->toPlainText().trimmed();

        if (compteSource.isEmpty()) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Veuillez saisir le compte source!", this);
            return;
        }

        if (compteDest.isEmpty()) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Veuillez saisir le compte bénéficiaire!", this);
            return;
        }

        if (montant <= 0) {
            WidgetNotificationModerne::afficherErreur("Erreur", "Veuillez saisir un montant valide!", this);
            return;
        }

        m_gestionnaireTransactions->effectuerVirement(compteSource, compteDest, montant, motif);
        WidgetNotificationModerne::afficherInformation("Succès", "Virement effectué avec succès", this);
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
    LoaderGuard guard(this);

    WidgetNotificationModerne::afficherConfirmationCritique(
        "Confirmation",
        "Voulez-vous vraiment supprimer votre compte courant?",
        [this]() {
            m_gestionnaireComptes->supprimerCompteCourant();
            WidgetNotificationModerne::afficherInformation("Succès", "Compte courant supprimé", this);
        },
        this
        );
}

void fenMain::on_btn_supprimer_compte_epargne_clicked()
{
    LoaderGuard guard(this);

    WidgetNotificationModerne::afficherConfirmationCritique(
        "Confirmation",
        "Voulez-vous vraiment supprimer votre compte épargne?",
        [this]() {
            m_gestionnaireComptes->supprimerCompteEpargne();
            WidgetNotificationModerne::afficherInformation("Succès", "Compte épargne supprimé", this);
        },
        this
        );
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

void fenMain::on_btn_effectuer_transaction_compte_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_transaction);
    ui->mes_onglets_page_transaction->setCurrentWidget(ui->onglet_depot_retrait);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_consulter_compte_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_voir_liste_complete_transaction_clicked()
{
    ui->mes_pages->setCurrentWidget(ui->page_historique);
    mettreAJourStyleBoutonsLateraux();
}

void fenMain::on_btn_ajouter_compte_courant_clicked()
{
    LoaderGuard guard(this);

    m_gestionnaireComptes->creerCompteCourant();
    WidgetNotificationModerne::afficherInformation("Succès", "Compte courant créé", this);
}

void fenMain::on_btn_ajouter_compte_epargne_clicked()
{
    LoaderGuard guard(this);

    m_gestionnaireComptes->creerCompteEpargne();
    WidgetNotificationModerne::afficherInformation("Succès", "Compte épargne créé", this);
}

void fenMain::on_btn_modifier_info_tutilaire_parametre_clicked()
{
    LoaderGuard guard(this);

    QString nouveauNom = ui->lineEdit_nom_titulaire_parametre->text().trimmed();
    QString nouvelEmail = ui->lineEdit_email_titulaire_parametre->text().trimmed();
    QString nouveauMotDePasse = ui->lineEdit_mot_de_passe_parametre->text();
    QString confirmationMotDePasse = ui->lineEdit_mot_de_passe_confirmation_parametre->text();

    m_gestionnaireUtilisateur->mettreAJourInformationsUtilisateur(
        nouveauNom, nouvelEmail, nouveauMotDePasse, confirmationMotDePasse, this);
    WidgetNotificationModerne::afficherInformation("Succès", "Informations mises à jour", this);
}

void fenMain::on_btn_deconnexion_barre_latterale_clicked()
{
    WidgetNotificationModerne::afficherConfirmationCritique(
        "Déconnexion",
        "Êtes-vous sûr de vouloir vous déconnecter ?",
        [this]() {
            sauvegarderDonnees();
            emit demandeDeconnexion();
        },
        this
        );
}

void fenMain::on_btn_aide_et_supports_barre_latterale_clicked()
{
    // Chemin absolu vers le fichier de rapport
    QString cheminRapport = "D:/projets/MyBank/rapport/Rapport_projet_application_de_gestion_bancaire.html";

    // Vérifier si le fichier existe
    if (QFile::exists(cheminRapport)) {
        // Ouvrir dans le navigateur par défaut
        QDesktopServices::openUrl(QUrl::fromLocalFile(cheminRapport));
    } else {
        // Afficher une erreur si le fichier n'est pas trouvé
        WidgetNotificationModerne::afficherErreur(
            "Fichier introuvable",
            "Le fichier de rapport n'a pas été trouvé à l'emplacement :\n" + cheminRapport,
            this
            );
    }
}

void fenMain::on_btn_notification_clicked()
{
    WidgetNotificationModerne::afficherInformation("Notification", "Aucune nouvelle notification", this);
}

