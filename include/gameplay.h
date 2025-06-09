//
// Created by alexis on 05/06/25.
//

#ifndef GAMEPLAY_H
#define GAMEPLAY_H


#include <stddef.h>
#include <stdint.h>

#include "data.h"

int trouverCarte(char *uid, carte *cartes, int nb_cartes);

int trouverJoueur(char *uid, joueur *cartes, int nb_joueurs);

int verifier_victoire(historique_joueur_t* h);

#endif //GAMEPLAY_H
