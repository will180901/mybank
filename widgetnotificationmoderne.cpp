#include "WidgetNotificationModerne.h"
#include <QApplication>
#include <QScreen>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QPainterPath>
#include <QGraphicsBlurEffect>
#include <QGuiApplication>

// Initialisation de la variable statique
int WidgetNotificationModerne::decalageY = 0;

WidgetNotificationModerne::WidgetNotificationModerne(QWidget *parent)
    : QWidget(parent)
    , layoutPrincipal(nullptr)
    , labelIcone(nullptr)
    , labelTitre(nullptr)
    , labelMessage(nullptr)
    , boutonFermer(nullptr)
    , animationEntree(nullptr)
    , animationSortie(nullptr)
    , animationOpacite(nullptr)
    , effetOpacite(nullptr)
    , typeActuel(Information)
    , timerFermeture(nullptr)
    , fermetureAutomatiqueActivee(true)
    , dureeAffichage(5000) // 5 secondes par défaut
    , animationEnCours(false)
{
    configurerInterface();
    configurerAnimations();

    // Configuration de base du widget
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(300);
    setMaximumWidth(600);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Timer pour fermeture automatique
    timerFermeture = new QTimer(this);
    timerFermeture->setSingleShot(true);
    connect(timerFermeture, &QTimer::timeout, this, &WidgetNotificationModerne::fermerAutomatiquement);

    // Installer un filtre d'événements pour capturer les clics
    installEventFilter(this);
}

WidgetNotificationModerne::~WidgetNotificationModerne()
{
    // Arrêter les animations pour éviter les callbacks sur objet détruit
    if (animationEntree) {
        animationEntree->stop();
        animationEntree->deleteLater();
    }
    if (animationSortie) {
        animationSortie->stop();
        animationSortie->deleteLater();
    }
    if (animationOpacite) {
        animationOpacite->stop();
        animationOpacite->deleteLater();
    }
    if (timerFermeture) {
        timerFermeture->stop();
        timerFermeture->deleteLater();
    }

    // Réinitialiser le décalage quand la dernière notification est fermée
    decalageY = 0;
}

void WidgetNotificationModerne::configurerInterface()
{
    // Layout principal
    layoutPrincipal = new QHBoxLayout(this);
    layoutPrincipal->setContentsMargins(15, 10, 15, 10);
    layoutPrincipal->setSpacing(12);

    // Label pour l'icône
    labelIcone = new QLabel(this);
    labelIcone->setFixedSize(24, 24);
    labelIcone->setAlignment(Qt::AlignCenter);

    // Label pour le titre
    labelTitre = new QLabel(this);
    labelTitre->setWordWrap(true);

    // Label pour le message
    labelMessage = new QLabel(this);
    labelMessage->setWordWrap(true);
    labelMessage->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Bouton fermer
    boutonFermer = new QPushButton("✕", this);
    boutonFermer->setFixedSize(24, 24);
    boutonFermer->setStyleSheet(
        "QPushButton {"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "    border: none;"
        "    border-radius: 12px;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 0.3);"
        "}"
        "QPushButton:pressed {"
        "    background-color: rgba(255, 255, 255, 0.1);"
        "}"
        );

    // Ajouter les composants au layout
    layoutPrincipal->addWidget(labelIcone);
    layoutPrincipal->addWidget(labelTitre);
    layoutPrincipal->addWidget(labelMessage);
    layoutPrincipal->addStretch();
    layoutPrincipal->addWidget(boutonFermer);

    // Connecter le bouton fermer
    connect(boutonFermer, &QPushButton::clicked, this, &WidgetNotificationModerne::fermerNotification);
}

void WidgetNotificationModerne::configurerAnimations()
{
    // Effet d'opacité
    effetOpacite = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effetOpacite);

    // Animation d'entrée (de droite à gauche)
    animationEntree = new QPropertyAnimation(this, "pos");
    animationEntree->setDuration(600);
    animationEntree->setEasingCurve(QEasingCurve::OutCubic);

    // Animation de sortie
    animationSortie = new QPropertyAnimation(this, "pos");
    animationSortie->setDuration(400);
    animationSortie->setEasingCurve(QEasingCurve::InCubic);

    // Animation d'opacité
    animationOpacite = new QPropertyAnimation(effetOpacite, "opacity");
    animationOpacite->setDuration(300);

    // Connecter les animations
    connect(animationEntree, &QPropertyAnimation::finished, this, &WidgetNotificationModerne::animationTerminee);
    connect(animationSortie, &QPropertyAnimation::finished, this, [this]() {
        decalageY -= height() + 10; // Réduire le décalage
        close();
    });
}

QScreen* WidgetNotificationModerne::obtenirEcranActuel()
{
    if (parentWidget()) {
        return parentWidget()->screen();
    }

    // Obtenir l'écran sous le curseur
    QPoint cursorPos = QCursor::pos();
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(cursorPos)) {
            return screen;
        }
    }

    // Fallback à l'écran principal
    return QApplication::primaryScreen();
}

void WidgetNotificationModerne::afficherNotification(const QString &titre, const QString &message, TypeNotification type)
{
    typeActuel = type;

    // Arrêter le timer existant
    if (timerFermeture && timerFermeture->isActive()) {
        timerFermeture->stop();
    }

    // Configurer le contenu
    labelTitre->setText(titre.isEmpty() ? obtenirTitreParDefaut(type) : titre);
    labelMessage->setText(message);

    // Afficher/masquer le titre selon s'il y a un message
    if (message.isEmpty()) {
        labelTitre->hide();
        labelMessage->setText(titre);
    } else {
        labelTitre->show();
    }

    // Configurer le style selon le type
    configurerStyle(type);

    // Ajuster la taille au contenu
    adjustSize();

    // Positionner le widget
    positionnerWidget();

    // Afficher et animer
    show();
    raise();  // S'assurer qu'elle est au premier plan
    demarrerAnimationEntree();

    // Démarrer le timer de fermeture automatique si activé
    if (fermetureAutomatiqueActivee) {
        timerFermeture->start(dureeAffichage);
    }
}

void WidgetNotificationModerne::afficherNotification(const QString &message, TypeNotification type)
{
    afficherNotification(QString(), message, type);
}

void WidgetNotificationModerne::configurerStyle(TypeNotification type)
{
    couleurFond = obtenirCouleurFond(type);
    couleurTexte = obtenirCouleurTexte(type);
    iconeUnicode = obtenirIconeUnicode(type);

    // Appliquer les couleurs dynamiquement
    QString styleBouton = QString(
                              "QPushButton {"
                              "    background-color: rgba(255, 255, 255, 0.2);"
                              "    color: %1;"
                              "    border: none;"
                              "    border-radius: 12px;"
                              "    font-size: 12px;"
                              "    font-weight: bold;"
                              "}"
                              "QPushButton:hover {"
                              "    background-color: rgba(255, 255, 255, 0.3);"
                              "}"
                              "QPushButton:pressed {"
                              "    background-color: rgba(255, 255, 255, 0.1);"
                              "}").arg(couleurTexte);

    labelIcone->setStyleSheet(QString("QLabel { color: %1; font-size: 18px; font-weight: bold; }").arg(couleurTexte));
    labelTitre->setStyleSheet(QString("QLabel { color: %1; font-size: 14px; font-weight: bold; }").arg(couleurTexte));
    labelMessage->setStyleSheet(QString("QLabel { color: %1; font-size: 12px; }").arg(couleurTexte));
    boutonFermer->setStyleSheet(styleBouton);
    labelIcone->setText(iconeUnicode);

    // Forcer une mise à jour du style
    update();
}

void WidgetNotificationModerne::positionnerWidget()
{
    QScreen *screen = obtenirEcranActuel();
    if (!screen) {
        // Fallback : utiliser l'écran principal
        screen = QApplication::primaryScreen();
        if (!screen) {
            // Fallback ultime : utiliser le premier écran
            QList<QScreen*> screens = QApplication::screens();
            if (!screens.isEmpty()) {
                screen = screens.first();
            } else {
                return; // Aucun écran disponible
            }
        }
    }

    QRect screenGeometry = screen->availableGeometry();

    // Position finale : bas à droite
    int x = screenGeometry.right() - width() - 20;  // 20px marge droite
    int y = screenGeometry.bottom() - height() - 20 - decalageY; // 20px marge bas + décalage

    // Position de départ (hors écran à droite)
    int startX = screenGeometry.right() + width();

    // Définir les positions pour l'animation
    animationEntree->setStartValue(QPoint(startX, y));
    animationEntree->setEndValue(QPoint(x, y));
    animationSortie->setStartValue(QPoint(x, y));
    animationSortie->setEndValue(QPoint(startX, y));

    // Positionner initialement hors écran
    move(startX, y);

    // Augmenter le décalage pour la prochaine notification
    decalageY += height() + 10;
}

void WidgetNotificationModerne::demarrerAnimationEntree()
{
    animationEnCours = true;

    // Animation d'opacité
    effetOpacite->setOpacity(0.0);
    animationOpacite->setStartValue(0.0);
    animationOpacite->setEndValue(1.0);

    // Démarrer les animations
    animationEntree->start();
    animationOpacite->start();
}

void WidgetNotificationModerne::demarrerAnimationSortie()
{
    animationEnCours = true;

    // Arrêter le timer de fermeture
    if (timerFermeture && timerFermeture->isActive()) {
        timerFermeture->stop();
    }

    // Animation d'opacité pour la sortie
    animationOpacite->setStartValue(1.0);
    animationOpacite->setEndValue(0.0);

    // Démarrer les animations
    animationSortie->start();
    animationOpacite->start();
}

void WidgetNotificationModerne::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Créer un chemin arrondi
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);

    // Couleur de fond avec transparence
    QColor bgColor(couleurFond);
    bgColor.setAlpha(180); // Transparence

    // Dessiner le fond
    painter.fillPath(path, bgColor);

    // Effet de bordure subtile
    QPen pen(QColor(255, 255, 255, 50));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

void WidgetNotificationModerne::fermerNotification()
{
    if (!animationEnCours) {
        demarrerAnimationSortie();
    }
}

void WidgetNotificationModerne::fermerAutomatiquement()
{
    if (!animationEnCours) {
        demarrerAnimationSortie();
    }
}

void WidgetNotificationModerne::animationTerminee()
{
    animationEnCours = false;
}

bool WidgetNotificationModerne::eventFilter(QObject *obj, QEvent *event)
{
    // Ne pas capturer les événements du bouton fermer
    if (obj == boutonFermer) {
        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            fermerNotification();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void WidgetNotificationModerne::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        fermerNotification();
    }
    QWidget::mousePressEvent(event);
}

QString WidgetNotificationModerne::obtenirIconeUnicode(TypeNotification type)
{
    switch (type) {
    case Information:     return "ℹ";
    case Conseil:         return "💡";
    case Recommandation:  return "⭐";
    case Remarque:        return "📝";
    case Preventif:       return "🛡";
    case Avertissement:   return "⚠";
    case Erreur:          return "❌";
    default:              return "ℹ";
    }
}

QString WidgetNotificationModerne::obtenirCouleurFond(TypeNotification type)
{
    switch (type) {
    case Information:     return "#3498db"; // Bleu
    case Conseil:         return "#2ecc71"; // Vert
    case Recommandation:  return "#9b59b6"; // Violet
    case Remarque:        return "#34495e"; // Gris foncé
    case Preventif:       return "#f39c12"; // Orange
    case Avertissement:   return "#e67e22"; // Orange foncé
    case Erreur:          return "#e74c3c"; // Rouge
    default:              return "#3498db";
    }
}

QString WidgetNotificationModerne::obtenirCouleurTexte(TypeNotification type)
{
    Q_UNUSED(type)
    return "#ffffff"; // Blanc pour tous les types
}

QString WidgetNotificationModerne::obtenirTitreParDefaut(TypeNotification type)
{
    switch (type) {
    case Information:     return "Information";
    case Conseil:         return "Conseil";
    case Recommandation:  return "Recommandation";
    case Remarque:        return "Remarque";
    case Preventif:       return "Préventif";
    case Avertissement:   return "Avertissement";
    case Erreur:          return "Erreur";
    default:              return "Notification";
    }
}

void WidgetNotificationModerne::configurerDureeAffichage(int millisecondes)
{
    dureeAffichage = millisecondes;
}

void WidgetNotificationModerne::configurerFermetureAutomatique(bool activer)
{
    fermetureAutomatiqueActivee = activer;
}

// Méthodes statiques pour faciliter l'utilisation
void WidgetNotificationModerne::afficherInformation(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Information);
}

void WidgetNotificationModerne::afficherConseil(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Conseil);
}

void WidgetNotificationModerne::afficherRecommandation(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Recommandation);
}

void WidgetNotificationModerne::afficherRemarque(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Remarque);
}

void WidgetNotificationModerne::afficherPreventif(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Preventif);
}

void WidgetNotificationModerne::afficherAvertissement(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Avertissement);
}

void WidgetNotificationModerne::afficherErreur(const QString &titre, const QString &message, QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherNotification(titre, message, Erreur);
}
