#include "gestionnairetheme.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>

GestionnaireTheme::GestionnaireTheme(QWidget* zoneBoutonBascule, QObject* parent)
    : QObject(parent), m_zoneBoutonBascule(zoneBoutonBascule), m_boutonBasculeChangertheme(nullptr)
{
    QSettings settings("MyBank", "Theme");
    m_modeSombreActive = settings.value("themeSombre", false).toBool();

    configurerBoutonBasculeThemeCouleur();
}

bool GestionnaireTheme::estThemeSombreActif() const
{
    return m_modeSombreActive;
}

void GestionnaireTheme::forcerTheme(bool themeSombre)
{
    if (m_boutonBasculeChangertheme) {
        m_boutonBasculeChangertheme->blockSignals(true);
        m_boutonBasculeChangertheme->definirEtatBascule(themeSombre);
        m_boutonBasculeChangertheme->blockSignals(false);
    }

    QSettings settings("MyBank", "Theme");
    settings.setValue("themeSombre", themeSombre);
    m_modeSombreActive = themeSombre;

    emit themeChange(themeSombre);

}

void GestionnaireTheme::configurerBoutonBasculeThemeCouleur()
{
    if (m_boutonBasculeChangertheme) {
        delete m_boutonBasculeChangertheme;
        m_boutonBasculeChangertheme = nullptr;
    }

    m_boutonBasculeChangertheme = new MonBoutonBascule();
    m_boutonBasculeChangertheme->definirEtatBascule(m_modeSombreActive);

    connect(m_boutonBasculeChangertheme, &MonBoutonBascule::aBascule,
            this, &GestionnaireTheme::gererBasculeThemeCouleur);

    // Ajouter le bouton à l'interface
    if (m_zoneBoutonBascule) {
        QVBoxLayout *layoutBouton = qobject_cast<QVBoxLayout*>(m_zoneBoutonBascule->layout());

        if (!layoutBouton) {
            layoutBouton = new QVBoxLayout(m_zoneBoutonBascule);
            layoutBouton->setContentsMargins(0, 0, 0, 0);
            m_zoneBoutonBascule->setLayout(layoutBouton);
        }

        layoutBouton->addWidget(m_boutonBasculeChangertheme);
        layoutBouton->setAlignment(m_boutonBasculeChangertheme, Qt::AlignCenter);
    } else {
        qWarning() << "Erreur: La zone pour le bouton de bascule de thème n'a pas été trouvée";
    }
}

void GestionnaireTheme::gererBasculeThemeCouleur(bool estActive)
{
    forcerTheme(estActive);
}

void GestionnaireTheme::initialiserThemeCouleur(QWidget* fenetrePrincipale)
{
    QSettings settings("MyBank", "Theme");
    bool themeSombreActif = settings.value("themeSombre", false).toBool();
    m_modeSombreActive = themeSombreActif;

    QString cheminTheme = themeSombreActif
                              ? ":/themes/theme_sombre.txt"
                              : ":/themes/theme_clair.txt";

    QFile fichierTheme(cheminTheme);
    if (fichierTheme.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fichierTheme);
        QString styleSheet = in.readAll();
        fenetrePrincipale->setStyleSheet(styleSheet);
        fichierTheme.close();
    } else {
        qWarning() << "Impossible de charger le fichier de thème:" << cheminTheme;
        fenetrePrincipale->setStyleSheet("");
    }

    if (m_boutonBasculeChangertheme) {
        m_boutonBasculeChangertheme->blockSignals(true);
        m_boutonBasculeChangertheme->definirEtatBascule(themeSombreActif);
        m_boutonBasculeChangertheme->blockSignals(false);
    }


    if (m_boutonBasculeChangertheme) {
        m_boutonBasculeChangertheme->blockSignals(true);
        m_boutonBasculeChangertheme->definirEtatBascule(themeSombreActif);

        // Forcez une mise à jour immédiate
        m_boutonBasculeChangertheme->update();

        m_boutonBasculeChangertheme->blockSignals(false);
    }
}
