//
// Created by alexis on 05/06/25.
//

#ifndef GAMEPLAY_H
#define GAMEPLAY_H


#include <stddef.h>
#include <stdint.h>

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

typedef struct {
    char UID[8];     // UID du badge RFID
    char nom[15];       // nom de la carte
    int valeur;         // valeur de carte
    int element;        // element de la carte (FEU ¦ EAU ¦ GLACE)
    int couleur;        // couleur de la carte (BLEU ¦ VERT ¦ JAUNE ¦ ROSE ¦ ORANGE )
} carte;

carte* trouverCarte(char *uid, joueur *cartes, int nb_cartes);

joueur* trouverJoueur(char *uid, joueur *cartes, int nb_joueurs);

#endif //GAMEPLAY_H
