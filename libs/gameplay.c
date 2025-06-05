//
// Created by alexis on 05/06/25.
//

#include "../include/gameplay.h"
#include "../include/data.h"
#include <stddef.h>
#include <string.h>

int trouverCarte(char *uid, carte *cartes, int nb_cartes) {
    int i;
    for (i=0; i<nb_cartes; i++) {
        if (strcmp(uid, cartes->UID) == 0) {
            return i;
        }
    }
    return -1;
}

joueur* trouverJoueur(char *uid, joueur *joueurs, int nb_joueurs) {
    int i;
    for (i=0; i<nb_joueurs; i++) {
        if (strcmp(uid, joueurs->UID) == 0) {
            return joueurs[i];
        }
    }
    return NULL;
}



