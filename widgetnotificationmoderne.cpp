#include "WidgetNotificationModerne.h"
#include <QApplication>
#include <QScreen>
#include <QEasingCurve>
#include <QMouseEvent>
#include <QPainterPath>
#include <QGraphicsBlurEffect>
#include <QGuiApplication>
#include <QVBoxLayout>

int WidgetNotificationModerne::decalageY = 0;

WidgetNotificationModerne::WidgetNotificationModerne(QWidget *parent)
    : QWidget(parent)
    , layoutPrincipal(nullptr)
    , layoutVertical(nullptr)
    , labelIcone(nullptr)
    , labelTitre(nullptr)
    , labelMessage(nullptr)
    , boutonFermer(nullptr)
    , widgetActions(nullptr)
    , layoutActions(nullptr)
    , boutonConfirmer(nullptr)
    , boutonAnnuler(nullptr)
    , animationEntree(nullptr)
    , animationSortie(nullptr)
    , animationOpacite(nullptr)
    , effetOpacite(nullptr)
    , typeActuel(Information)
    , timerFermeture(nullptr)
    , fermetureAutomatiqueActivee(true)
    , dureeAffichage(5000)
    , animationEnCours(false)
    , callbackConfirmation(nullptr)
{
    configurerInterface();
    configurerAnimations();

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumWidth(300);
    setMaximumWidth(600);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    timerFermeture = new QTimer(this);
    timerFermeture->setSingleShot(true);
    connect(timerFermeture, &QTimer::timeout, this, &WidgetNotificationModerne::fermerAutomatiquement);

    installEventFilter(this);
}

WidgetNotificationModerne::~WidgetNotificationModerne()
{
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

    decalageY = 0;
}

void WidgetNotificationModerne::configurerInterface()
{
    layoutVertical = new QVBoxLayout(this);
    layoutVertical->setContentsMargins(0, 0, 0, 0);
    layoutVertical->setSpacing(0);

    layoutPrincipal = new QHBoxLayout();
    layoutPrincipal->setContentsMargins(15, 10, 15, 10);
    layoutPrincipal->setSpacing(12);

    labelIcone = new QLabel(this);
    labelIcone->setFixedSize(24, 24);
    labelIcone->setAlignment(Qt::AlignCenter);

    labelTitre = new QLabel(this);
    labelTitre->setWordWrap(true);

    labelMessage = new QLabel(this);
    labelMessage->setWordWrap(true);
    labelMessage->setTextInteractionFlags(Qt::TextSelectableByMouse);

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

    layoutPrincipal->addWidget(labelIcone);
    layoutPrincipal->addWidget(labelTitre);
    layoutPrincipal->addWidget(labelMessage);
    layoutPrincipal->addStretch();
    layoutPrincipal->addWidget(boutonFermer);

    widgetActions = new QWidget(this);
    layoutActions = new QHBoxLayout(widgetActions);
    layoutActions->setContentsMargins(15, 5, 15, 10);
    layoutActions->setSpacing(10);
    widgetActions->setVisible(false);

    boutonAnnuler = new QPushButton("Annuler", widgetActions);
    boutonAnnuler->setFixedHeight(30);
    boutonAnnuler->setStyleSheet(
        "QPushButton {"
        "    background-color: #e0e0e0;"
        "    color: #333333;"
        "    border-radius: 5px;"
        "    padding: 5px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #d0d0d0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #c0c0c0;"
        "}"
        );

    boutonConfirmer = new QPushButton("Confirmer", widgetActions);
    boutonConfirmer->setFixedHeight(30);
    boutonConfirmer->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 5px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #2471a3;"
        "}"
        );

    layoutActions->addStretch();
    layoutActions->addWidget(boutonAnnuler);
    layoutActions->addWidget(boutonConfirmer);

    layoutVertical->addLayout(layoutPrincipal);
    layoutVertical->addWidget(widgetActions);

    connect(boutonFermer, &QPushButton::clicked, this, &WidgetNotificationModerne::fermerNotification);
    connect(boutonAnnuler, &QPushButton::clicked, this, &WidgetNotificationModerne::onAnnulerClicked);
    connect(boutonConfirmer, &QPushButton::clicked, this, &WidgetNotificationModerne::onConfirmerClicked);
}

void WidgetNotificationModerne::configurerAnimations()
{
    effetOpacite = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effetOpacite);

    animationEntree = new QPropertyAnimation(this, "pos");
    animationEntree->setDuration(600);
    animationEntree->setEasingCurve(QEasingCurve::OutCubic);

    animationSortie = new QPropertyAnimation(this, "pos");
    animationSortie->setDuration(400);
    animationSortie->setEasingCurve(QEasingCurve::InCubic);

    animationOpacite = new QPropertyAnimation(effetOpacite, "opacity");
    animationOpacite->setDuration(300);

    connect(animationEntree, &QPropertyAnimation::finished, this, &WidgetNotificationModerne::animationTerminee);
    connect(animationSortie, &QPropertyAnimation::finished, this, [this]() {
        decalageY -= height() + 10;
        close();
    });
}

QScreen* WidgetNotificationModerne::obtenirEcranActuel()
{
    if (parentWidget()) {
        return parentWidget()->screen();
    }

    QPoint cursorPos = QCursor::pos();
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(cursorPos)) {
            return screen;
        }
    }

    return QApplication::primaryScreen();
}

void WidgetNotificationModerne::afficherNotification(const QString &titre, const QString &message, TypeNotification type)
{
    typeActuel = type;

    if (timerFermeture && timerFermeture->isActive()) {
        timerFermeture->stop();
    }

    labelTitre->setText(titre.isEmpty() ? obtenirTitreParDefaut(type) : titre);
    labelMessage->setText(message);

    if (message.isEmpty()) {
        labelTitre->hide();
        labelMessage->setText(titre);
    } else {
        labelTitre->show();
    }

    widgetActions->setVisible(false);
    configurerStyle(type);
    adjustSize();
    positionnerWidget();
    show();
    raise();
    demarrerAnimationEntree();

    if (fermetureAutomatiqueActivee && type != Critique) {
        timerFermeture->start(dureeAffichage);
    }
}

// Renommé pour éviter le conflit
void WidgetNotificationModerne::afficherConfirmationCritiqueImpl(const QString &titre, const QString &message,
                                                                 std::function<void()> callbackConfirmation)
{
    this->callbackConfirmation = callbackConfirmation;
    afficherNotification(titre, message, Critique);
    widgetActions->setVisible(true);
    fermetureAutomatiqueActivee = false;

    if (timerFermeture && timerFermeture->isActive()) {
        timerFermeture->stop();
    }

    adjustSize();
    positionnerWidget();
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
    update();
}

void WidgetNotificationModerne::positionnerWidget()
{
    QScreen *screen = obtenirEcranActuel();
    if (!screen) {
        screen = QApplication::primaryScreen();
        if (!screen) {
            QList<QScreen*> screens = QApplication::screens();
            if (!screens.isEmpty()) {
                screen = screens.first();
            } else {
                return;
            }
        }
    }

    QRect screenGeometry = screen->availableGeometry();
    int x = screenGeometry.right() - width() - 20;
    int y = screenGeometry.bottom() - height() - 20 - decalageY;
    int startX = screenGeometry.right() + width();

    animationEntree->setStartValue(QPoint(startX, y));
    animationEntree->setEndValue(QPoint(x, y));
    animationSortie->setStartValue(QPoint(x, y));
    animationSortie->setEndValue(QPoint(startX, y));
    move(startX, y);
    decalageY += height() + 10;
}

void WidgetNotificationModerne::demarrerAnimationEntree()
{
    animationEnCours = true;
    effetOpacite->setOpacity(0.0);
    animationOpacite->setStartValue(0.0);
    animationOpacite->setEndValue(1.0);
    animationEntree->start();
    animationOpacite->start();
}

void WidgetNotificationModerne::demarrerAnimationSortie()
{
    animationEnCours = true;
    if (timerFermeture && timerFermeture->isActive()) {
        timerFermeture->stop();
    }
    animationOpacite->setStartValue(1.0);
    animationOpacite->setEndValue(0.0);
    animationSortie->start();
    animationOpacite->start();
}

void WidgetNotificationModerne::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);
    QColor bgColor(couleurFond);
    bgColor.setAlpha(180);
    painter.fillPath(path, bgColor);
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

void WidgetNotificationModerne::onConfirmerClicked()
{
    if (callbackConfirmation) {
        callbackConfirmation();
    }
    fermerNotification();
}

void WidgetNotificationModerne::onAnnulerClicked()
{
    fermerNotification();
}

bool WidgetNotificationModerne::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == boutonFermer || obj == boutonConfirmer || obj == boutonAnnuler) {
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
    case Critique:        return "❗";
    default:              return "ℹ";
    }
}

QString WidgetNotificationModerne::obtenirCouleurFond(TypeNotification type)
{
    switch (type) {
    case Information:     return "#3498db";
    case Conseil:         return "#2ecc71";
    case Recommandation:  return "#9b59b6";
    case Remarque:        return "#34495e";
    case Preventif:       return "#f39c12";
    case Avertissement:   return "#e67e22";
    case Erreur:          return "#e74c3c";
    case Critique:        return "#d35400";
    default:              return "#3498db";
    }
}

QString WidgetNotificationModerne::obtenirCouleurTexte(TypeNotification type)
{
    return "#ffffff";
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
    case Critique:        return "Confirmation requise";
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

// Méthodes statiques
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

void WidgetNotificationModerne::afficherConfirmationCritique(const QString &titre, const QString &message,
                                                             std::function<void()> callbackConfirmation,
                                                             QWidget *parent)
{
    WidgetNotificationModerne *widget = new WidgetNotificationModerne(parent);
    widget->afficherConfirmationCritiqueImpl(titre, message, callbackConfirmation);
}
