#ifndef GESTIONNAIREUTILISATEUR_H
#define GESTIONNAIREUTILISATEUR_H

#include "creationbd.h"
#include <QObject>
#include <QLabel>

class GestionnaireUtilisateur : public QObject
{
    Q_OBJECT
public:
    explicit GestionnaireUtilisateur(CreationBD& creationBD, const QString& utilisateurId, QObject* parent = nullptr);

    void chargerInformationsUtilisateur(QLabel* labelBienvenue);
    void mettreAJourInformationsUtilisateur(
        const QString& nouveauNom,
        const QString& nouvelEmail,
        const QString& nouveauMotDePasse,
        const QString& confirmationMotDePasse,
        QWidget* parentWindow
        );

signals:
    void utilisateurModifie();

private:
    CreationBD& m_creationBD;
    QString m_utilisateurId;
};

#endif // GESTIONNAIREUTILISATEUR_H
