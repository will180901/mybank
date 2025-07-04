#include "authentification.h"
#include "fenmain.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Authentification *auth = new Authentification();
    fenMain *mainWindow = nullptr;

    // Connexion pour l'authentification réussie
    QObject::connect(auth, &Authentification::authentificationReussie,
                     [auth, &mainWindow](const QString& userId, CreationBD & m_BD) {
                         auth->hide();
                         mainWindow = new fenMain(m_BD, nullptr, userId);

                         // Nouvelle connexion pour la déconnexion
                         QObject::connect(mainWindow, &fenMain::demandeDeconnexion, [auth, &mainWindow]() {
                             // Émettre signal de destruction avant suppression
                             QObject::connect(mainWindow, &fenMain::fenetreDetruite, [auth]() {
                                 auth->show();
                             });
                             mainWindow->deleteLater();
                             mainWindow = nullptr;
                         });

                         mainWindow->show();
                     });

    // Connexion pour la fermeture de l'application
    QObject::connect(auth, &QDialog::rejected, [&a]() {
        a.quit();
    });

    auth->show();
    return a.exec();
}
