//
// Created by alexis on 27/05/25.
//
#ifndef DATA_H
#define DATA_H

#include <stdint.h>

// Types de cartes
#define FEU   21
#define EAU   22
#define GLACE 23

// Couleurs
#define BLEU    31
#define VERT    32
#define JAUNE   33
#define ROSE    34
#define ORANGE  35

// Check
#define CHECK(sts, msg) if ((sts) == -1){ perror(msg); exit(-1); }

typedef struct {
    uint8_t UID[5];     // UID du badge RFID
} badge;

typedef struct {
    char nom[15];       // nom de la carte
    int valeur;         // valeur de carte
    int element;        // element de la carte (FEU ¦ EAU ¦ GLACE)
    int couleur;        // couleur de la carte (BLEU ¦ VERT ¦ JAUNE ¦ ROSE ¦ ORANGE )
} carte;

typedef struct {
    badge badge_id;     // badge
    carte data;         // données de la carte
} carte_jeu;

typedef struct {
    badge ID;           // badge
    char nom[15];       // nom du joueur
    int rang;           // rang / ceinture du joueur
    int exp;            // expérience du joueur
} joueur ;

#endif // DATA_H

