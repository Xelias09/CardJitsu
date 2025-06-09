//
// Created by alexis on 05/06/25.
//

#include "../include/gameplay.h"

#include <stdbool.h>

#include "../include/data.h"
#include <string.h>

int trouverCarte(char *uid, carte *cartes, int nb_cartes) {
    int i;
    for (i = 0; i<nb_cartes; i++) {
        if (strcmp(uid, cartes[i].UID) == 0) {
            return i;
        }
    }
    return -1;
}

int trouverJoueur(char *uid, joueur *joueurs, int nb_joueurs) {
    int i;
    for (i = 0; i < nb_joueurs; i++) {
        if (strcmp(uid, joueurs[i].UID) == 0) {
            return i;
        }
    }
    return -1;
}

int verifier_victoire(historique_joueur_t* h) {
    if (h->nb_cartes < 3) return 0;
    int i;
    int j;
    int k;
    for (i = 0; i < h->nb_cartes; i++) {
        for (j = i + 1; j < h->nb_cartes; j++) {
            for (k = j + 1; k < h->nb_cartes; k++) {
                carte c1 = h->cartes[i];
                carte c2 = h->cartes[j];
                carte c3 = h->cartes[k];

                // Vérifie si les 3 éléments sont différents
                bool elements_differents = (c1.element != c2.element &&
                                            c1.element != c3.element &&
                                            c2.element != c3.element);

                // Vérifie si les 3 couleurs sont différentes
                bool couleurs_differentes = (c1.couleur != c2.couleur &&
                                             c1.couleur != c3.couleur &&
                                             c2.couleur != c3.couleur);

                // ✅ Condition 1
                if (elements_differents && couleurs_differentes) {
                    return 1;
                }

                // ✅ Condition 2 : même élément + 3 couleurs différentes
                bool meme_element = (c1.element == c2.element &&
                                     c1.element == c3.element);

                if (meme_element && couleurs_differentes) {
                    return 1;
                }
            }
        }
    }

    return 0; // pas de victoire

}



