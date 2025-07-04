QT       += core gui sql widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    animationsolde.cpp \
    authentification.cpp \
    banque.cpp \
    comptebancaire.cpp \
    comptecourant.cpp \
    compteepargne.cpp \
    creationbd.cpp \
    gestionnairecomptes.cpp \
    gestionnairehistorique.cpp \
    gestionnaireinterface.cpp \
    gestionnairetheme.cpp \
    gestionnairetransactions.cpp \
    gestionnaireutilisateur.cpp \
    highlightdelegate.cpp \
    main.cpp \
    fenmain.cpp \
    monboutonbascule.cpp \
    skeletonloader.cpp \
    utilitairesmotdepasse.cpp \
    widgetnotificationmoderne.cpp

HEADERS += \
    animationsolde.h \
    authentification.h \
    banque.h \
    comptebancaire.h \
    comptecourant.h \
    compteepargne.h \
    creationbd.h \
    fenmain.h \
    gestionnairecomptes.h \
    gestionnairehistorique.h \
    gestionnaireinterface.h \
    gestionnairetheme.h \
    gestionnairetransactions.h \
    gestionnaireutilisateur.h \
    highlightdelegate.h \
    monboutonbascule.h \
    skeletonloader.h \
    utilitairesmotdepasse.h \
    widgetnotificationmoderne.h

FORMS += \
    authentification.ui \
    fenmain.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
