#ifndef GESTIONNAIRETHEME_H
#define GESTIONNAIRETHEME_H

#include "monboutonbascule.h"
#include <QObject>
#include <QSettings>
#include <QWidget>

class GestionnaireTheme : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireTheme(QWidget* zoneBoutonBascule, QObject* parent = nullptr);

    bool estThemeSombreActif() const;
    void forcerTheme(bool themeSombre);
    void initialiserThemeCouleur(QWidget* fenetrePrincipale);

signals:
    void themeChange(bool themeSombre);

private slots:
    void gererBasculeThemeCouleur(bool estActive);

private:
    void configurerBoutonBasculeThemeCouleur();

    QWidget* m_zoneBoutonBascule;
    MonBoutonBascule* m_boutonBasculeChangertheme;
    bool m_modeSombreActive;
};

#endif // GESTIONNAIRETHEME_H
