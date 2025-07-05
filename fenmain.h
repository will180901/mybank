#ifndef FENMAIN_H
#define FENMAIN_H

#include "gestionnairecomptes.h"
#include "gestionnairetransactions.h"
#include "gestionnairehistorique.h"
#include "gestionnairetheme.h"
#include "gestionnaireinterface.h"
#include "gestionnaireutilisateur.h"
#include "banque.h"
#include "creationbd.h"
#include "animationsolde.h"
#include "monboutonbascule.h"
#include "skeletonloader.h"
#include "ui_fenmain.h"

#include <QMainWindow>
#include <QEvent>
#include <QCloseEvent>
#include <QElapsedTimer>
#include <QTimer>
#include <QDesktopServices>  // Ajout pour ouvrir le navigateur

class fenMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit fenMain(CreationBD& m_BD, QWidget *parent = nullptr, const QString &utilisateur_id = QString());
    ~fenMain();

    QWidget* getZoneBoutonBascule() const { return ui->zone_bouton_bascule; }
    AnimationSolde* getAnimationSolde() const { return m_soldeAnimation; }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_btn_valider_transaction_clicked();
    void on_btn_supprimer_compte_courant_clicked();
    void on_btn_supprimer_compte_epargne_clicked();
    void on_btn_masquer_solde_compte_courant_clicked();
    void on_btn_masquer_solde_compte_epargne_clicked();
    void on_btn_dashboard_barre_latterale_clicked();
    void on_btn_historique_barre_latterale_clicked();
    void on_btn_parametres_barre_latterale_clicked();
    void on_btn_effectuer_transaction_compte_clicked();
    void on_btn_consulter_compte_clicked();
    void on_btn_voir_liste_complete_transaction_clicked();
    void on_btn_ajouter_compte_courant_clicked();
    void on_btn_ajouter_compte_epargne_clicked();
    void on_btn_modifier_info_tutilaire_parametre_clicked();
    void on_btn_deconnexion_barre_latterale_clicked();
    void on_btn_aide_et_supports_barre_latterale_clicked();  // Nouveau slot
    void appliquerTheme(bool themeSombre);

signals:
    void demandeDeconnexion();
    void fenetreDetruite();
    void themeChange(bool themeSombre);

private:
    Ui::fenMain *ui;

    // Gestionnaires
    GestionnaireComptes* m_gestionnaireComptes;
    GestionnaireTransactions* m_gestionnaireTransactions;
    GestionnaireHistorique* m_gestionnaireHistorique;
    GestionnaireTheme* m_gestionnaireTheme;
    GestionnaireInterface* m_gestionnaireInterface;
    GestionnaireUtilisateur* m_gestionnaireUtilisateur;

    // Loader et gestion des opérations
    SkeletonLoader* m_loader;
    int m_loaderCount;
    QElapsedTimer m_loaderTimer;
    QTimer m_minimumDisplayTimer; // Nouveau timer pour le délai minimum

    // Variables membres
    QString m_utilisateur_id;
    bool m_soldeVisibleCompteCourant;
    bool m_soldeVisibleCompteEpargne;
    AnimationSolde* m_soldeAnimation;
    Banque m_banque;
    CreationBD& m_creationBD;
    QString m_boutonActif;

    // Méthodes internes
    void chargerDonneesDepuisBD();
    void afficherTop5DernieresTransactions();
    void chargerHistoriqueTransactions();
    void mettreAJourStyleBoutonsLateraux();
    void sauvegarderDonnees();

    // Gestion du loader
    void demarrerLoader();
    void arreterLoader();
    void arreterLoaderApresDelai(); // Nouvelle méthode

    // Guard RAII pour la gestion automatique du loader
    class LoaderGuard {
    public:
        LoaderGuard(fenMain* fen) : m_fen(fen) {
            if(m_fen) m_fen->demarrerLoader();
        }
        ~LoaderGuard() {
            if(m_fen) m_fen->arreterLoader();
        }

        LoaderGuard(const LoaderGuard&) = delete;
        LoaderGuard& operator=(const LoaderGuard&) = delete;

    private:
        fenMain* m_fen;
    };
};

#endif // FENMAIN_H
