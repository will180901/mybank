#ifndef GESTIONNAIRETHEME_H
#define GESTIONNAIRETHEME_H

#include <QObject>
#include <QSettings>
#include <QWidget>
#include <QApplication>
#include "monboutonbascule.h"

class GestionnaireTheme : public QObject
{
    Q_OBJECT

public:
    explicit GestionnaireTheme(QWidget* zoneBoutonBascule = nullptr, QObject* parent = nullptr);

    // Instance singleton pour synchronisation globale
    static GestionnaireTheme* instance();

    bool estThemeSombreActif() const; // Méthode correcte
    void forcerTheme(bool themeSombre);
    void configurerBoutonBasculeThemeCouleur();
    void initialiserThemeCouleur(QWidget* fenetrePrincipale);
    void appliquerThemeAuthentification(QWidget* fenetreAuth);

signals:
    void themeChange(bool themeSombre);
    void themeChangeGlobal(bool themeSombre);

private slots:
    void gererBasculeThemeCouleur(bool estActive);

private:
    static GestionnaireTheme* s_instance;
    QWidget* m_zoneBoutonBascule;
    MonBoutonBascule* m_boutonBasculeChangertheme;
    bool m_modeSombreActive;

    void sauvegarderTheme(bool themeSombre);
    bool chargerTheme();
};

#endif // GESTIONNAIRETHEME_H
