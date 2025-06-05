//
// Created by alexis on 05/06/25.
//

#ifndef GAMEPLAY_H
#define GAMEPLAY_H


#include <stddef.h>
#include <stdint.h>

#include "data.h"

// Types de cartes
#define FEU     21
#define EAU     22
#define GLACE   23

// Couleurs
#define BLEU    31
#define VERT    32
#define JAUNE   33
#define ROSE    34
#define ORANGE  35


int trouverCarte(char *uid, carte *cartes, int nb_cartes);

int trouverJoueur(char *uid, joueur *cartes, int nb_joueurs);

#endif //GAMEPLAY_H
