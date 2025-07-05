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
        Erreur
    };

    explicit WidgetNotificationModerne(QWidget *parent = nullptr);
    ~WidgetNotificationModerne();

    // Méthodes pour afficher les notifications
    void afficherNotification(const QString &titre, const QString &message, TypeNotification type);
    void afficherNotification(const QString &message, TypeNotification type);

    // Méthodes statiques pour faciliter l'utilisation
    static void afficherInformation(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherConseil(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherRecommandation(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherRemarque(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherPreventif(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherAvertissement(const QString &titre, const QString &message, QWidget *parent = nullptr);
    static void afficherErreur(const QString &titre, const QString &message, QWidget *parent = nullptr);

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
    QLabel *labelIcone;
    QLabel *labelTitre;
    QLabel *labelMessage;
    QPushButton *boutonFermer;

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
};

#endif // WIDGETNOTIFICATIONMODERNE_H
