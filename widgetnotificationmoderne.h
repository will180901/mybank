#ifndef WIDGETNOTIFICATIONMODERNE_H
#define WIDGETNOTIFICATIONMODERNE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <functional>

class WidgetNotificationModerne : public QWidget
{
    Q_OBJECT

public:
    enum TypeNotification {
        Information,
        Conseil,
        Recommandation,
        Remarque,
        Preventif,
        Avertissement,
        Erreur,
        Critique
    };

    explicit WidgetNotificationModerne(QWidget *parent = nullptr);
    ~WidgetNotificationModerne();

    // Méthodes pour afficher les notifications
    void afficherNotification(const QString &titre, const QString &message, TypeNotification type);
    void afficherNotification(const QString &message, TypeNotification type);

    // Renommé pour éviter le conflit
    void afficherConfirmationCritiqueImpl(const QString &titre, const QString &message,
                                          std::function<void()> callbackConfirmation);

    // Méthodes statiques pour faciliter l'utilisation
    static void afficherInformation(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherConseil(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherRecommandation(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherRemarque(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherPreventif(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherAvertissement(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherErreur(const QString &titre, const QString &message, QWidget *parent = nullptr);

    // Méthode statique pour les confirmations critiques
    static void afficherConfirmationCritique(const QString &titre, const QString &message,
                                             std::function<void()> callbackConfirmation,
                                             QWidget *parent = nullptr);

    // Configuration
    void configurerDureeAffichage(int millisecondes);
    void configurerFermetureAutomatique(bool activer);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void fermerNotification();
    void fermerAutomatiquement();
    void animationTerminee();
    void onConfirmerClicked();
    void onAnnulerClicked();

private:
    void configurerInterface();
    void configurerStyle(TypeNotification type);
    void configurerAnimations();
    void demarrerAnimationEntree();
    void demarrerAnimationSortie();
    void positionnerWidget();
    QString obtenirIconeUnicode(TypeNotification type);
    QString obtenirCouleurFond(TypeNotification type);
    QString obtenirCouleurTexte(TypeNotification type);
    QString obtenirTitreParDefaut(TypeNotification type);
    QScreen* obtenirEcranActuel();

    // Composants UI
    QHBoxLayout *layoutPrincipal;
    QVBoxLayout *layoutVertical;
    QLabel *labelIcone;
    QLabel *labelTitre;
    QLabel *labelMessage;
    QPushButton *boutonFermer;

    QWidget *widgetActions;
    QHBoxLayout *layoutActions;
    QPushButton *boutonConfirmer;
    QPushButton *boutonAnnuler;

    // Animations
    QPropertyAnimation *animationEntree;
    QPropertyAnimation *animationSortie;
    QPropertyAnimation *animationOpacite;
    QGraphicsOpacityEffect *effetOpacite;

    // Configuration
    TypeNotification typeActuel;
    QTimer *timerFermeture;
    bool fermetureAutomatiqueActivee;
    int dureeAffichage;
    bool animationEnCours;

    // Style
    QString couleurFond;
    QString couleurTexte;
    QString iconeUnicode;

    // Empilement
    static int decalageY;

    // Callback pour les confirmations critiques
    std::function<void()> callbackConfirmation;
};

#endif // WIDGETNOTIFICATIONMODERNE_H
