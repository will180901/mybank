#include "gestionnairetheme.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QApplication>

// Instance singleton
GestionnaireTheme* GestionnaireTheme::s_instance = nullptr;

GestionnaireTheme::GestionnaireTheme(QWidget* zoneBoutonBascule, QObject* parent)
    : QObject(parent), m_zoneBoutonBascule(zoneBoutonBascule), m_boutonBasculeChangertheme(nullptr)
{
    m_modeSombreActive = chargerTheme();

    if (m_zoneBoutonBascule) {
        configurerBoutonBasculeThemeCouleur();
    }
}

GestionnaireTheme* GestionnaireTheme::instance()
{
    if (!s_instance) {
        s_instance = new GestionnaireTheme();
    }
    return s_instance;
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

    sauvegarderTheme(themeSombre);
    m_modeSombreActive = themeSombre;

    emit themeChange(themeSombre);
    emit themeChangeGlobal(themeSombre);
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
    bool themeSombreActif = chargerTheme();
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
        m_boutonBasculeChangertheme->update();
        m_boutonBasculeChangertheme->blockSignals(false);
    }
}

void GestionnaireTheme::appliquerThemeAuthentification(QWidget* fenetreAuth)
{
    if (!fenetreAuth) return;

    QString cheminTheme = m_modeSombreActive
                              ? ":/themes_authen/theme_sombre.txt"
                              : ":/themes_authen/theme_clair.txt";

    QFile fichierTheme(cheminTheme);
    if (fichierTheme.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fichierTheme);
        QString styleSheet = in.readAll();
        fenetreAuth->setStyleSheet(styleSheet);
        fichierTheme.close();
    } else {
        qWarning() << "Impossible de charger le fichier de thème d'authentification:" << cheminTheme;
        fenetreAuth->setStyleSheet("");
    }
}

void GestionnaireTheme::sauvegarderTheme(bool themeSombre)
{
    QSettings settings("MyBank", "Theme");
    settings.setValue("themeSombre", themeSombre);
}

bool GestionnaireTheme::chargerTheme()
{
    QSettings settings("MyBank", "Theme");
    return settings.value("themeSombre", false).toBool();
}
