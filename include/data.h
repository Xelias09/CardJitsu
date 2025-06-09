//
// Created by alexis on 27/05/25.
//

#ifndef DATA_H
#define DATA_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <bits/pthreadtypes.h>

#include "socket_utils.h"

// Check
#define CHECK(sts, msg) if ((sts) == -1){ perror(msg); exit(-1); }

#define OFFLINE 88
#define ONLINE 89
#define NB_JOUEURS_MAX 10
#define MAX_PARTS 10
#define MAX_BUFFER 1024
#define CARTES_MAX 100
#define NB_PARTIES_MAX 5

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

// Status parties
#define ATTENTE     36
#define ENCOURS     37
#define TERMINE     38
#define LIBRE       39

#define MAX_CARTES_HISTO 50

typedef struct {
    char UID[9];        // UID du badge RFID
    char nom[15];       // nom de la carte
    int valeur;         // valeur de carte
    int element;        // element de la carte (FEU ¦ EAU ¦ GLACE)
    int couleur;        // couleur de la carte (BLEU ¦ VERT ¦ JAUNE ¦ ROSE ¦ ORANGE )
} carte;

typedef struct {
    carte cartes[MAX_CARTES_HISTO];
    int nb_cartes;
} historique_joueur_t;

typedef struct {
    char UID[9];        // UID du badge RFID
    char nom[15];       // nom du joueur
    int mode;           // mode jeu ou menu
    int status;         // status du joueur online / offline
    int rang;           // rang / ceinture du joueur
    int exp;            // expérience du joueur
    socket_t socket;    // socket
} joueur;

// Structure des parties
typedef struct {
    int idPartie;            // ID de la partie
    int nbJoueurs;           // Nombre de joueurs dans la partie
    int status;              // Status de la partie
    joueur liste_joueur[2];  // Liste des joueurs dans la partie
    historique_joueur_t historique[2];
} partie;

typedef struct {
    int nb_joueurs_online;
    int nb_joueurs_total;
    joueur joueurs[NB_JOUEURS_MAX];
    partie parties[NB_PARTIES_MAX];
    carte cartes[CARTES_MAX];
    pthread_mutex_t mutex;
} SharedInfos ;

int creerPartie(SharedInfos *infoServeur);
void initialiserSharedInfos(SharedInfos *info);
void resetPartie(SharedInfos *info, int idPartie);

#endif // DATA_H

