#ifndef COMPTECOURANT_H
#define COMPTECOURANT_H

#include "CompteBancaire.h"

class CompteCourant : public CompteBancaire
{
public:
    CompteCourant(const QString& numeroCompte,
                  const QString& nomTitulaire,
                  double solde,
                  int idUtilisateur,
                  double decouvertAutorise);

    bool retirer(double montant) override;

    double getDecouvertAutorise() const;
    void setDecouvertAutorise(double decouvert);

private:
    double m_decouvertAutorise;
};

#endif // COMPTECOURANT_H
