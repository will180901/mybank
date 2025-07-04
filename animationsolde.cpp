#include "animationsolde.h"
#include <QDebug>

AnimationSolde::AnimationSolde(QObject *parent) : QObject(parent) {}

void AnimationSolde::appliquerAvecLabel(QLabel* label, bool masquer)
{
    if (!label) return;
    QGraphicsBlurEffect* effetFlou = creerEffetFlou(label);
    animerFlou(effetFlou, masquer);
    animerTransition(label, masquer);
}

QGraphicsBlurEffect* AnimationSolde::creerEffetFlou(QLabel* label)
{
    QGraphicsBlurEffect* effetFlou = qobject_cast<QGraphicsBlurEffect*>(label->graphicsEffect());

    if (!effetFlou) {
        effetFlou = new QGraphicsBlurEffect(label);
        effetFlou->setBlurRadius(0);
        label->setGraphicsEffect(effetFlou);
    }

    return effetFlou;
}

void AnimationSolde::animerFlou(QGraphicsBlurEffect* effet, bool masquer)
{
    if (!effet) return;

    QPropertyAnimation* animation = new QPropertyAnimation(effet, "blurRadius", this);
    animation->setDuration(300);
    animation->setEasingCurve(QEasingCurve::InOutQuad);

    if (masquer) {
        animation->setStartValue(0);
        animation->setEndValue(8);
    } else {
        animation->setStartValue(8);
        animation->setEndValue(0);
    }

    animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void AnimationSolde::animerTransition(QLabel* label, bool masquer=false)
{
    if (!label) return;

    // Q_UNUSED(masquer);
    // Sauvegarder le texte original dans une propriété dynamique
    static const char* PROP_ORIGINAL_TEXT = "originalText";

    if (masquer) {
        // Sauvegarde du texte original et masquage
        label->setProperty(PROP_ORIGINAL_TEXT, label->text());
        QString texteMasque = genererTexteMasque(label->text());
        label->setText(texteMasque);
        label->setProperty("class", "solde-label solde-masque");
    } else {
        // Restauration du texte original
        QVariant original = label->property(PROP_ORIGINAL_TEXT);
        if (original.isValid()) {
            label->setText(original.toString());
        }
        label->setProperty("class", "solde-label");
    }

    label->style()->unpolish(label);
    label->style()->polish(label);
    label->update();
}

QString AnimationSolde::genererTexteMasque(const QString& original)
{
    // Trouver le dernier espace pour séparer le montant et la devise
    int dernierEspace = original.lastIndexOf(' ');

    if (dernierEspace == -1) {
        // Si pas d'espace, masquer tout le texte
        return QString(original.length(), '*');
    }

    // Extraire le montant (avant le dernier espace) et la devise
    QString montant = original.left(dernierEspace);
    QString devise = original.mid(dernierEspace + 1);

    // Générer une chaîne d'étoiles de même longueur que le montant
    QString montantMasque = QString(montant.length(), '*');

    return montantMasque + ' ' + devise;
}
