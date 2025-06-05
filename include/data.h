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
#define NB_JOUEURS_MAX 50
#define MAX_PARTS 10
#define MAX_BUFFER 1024
#define CARTES_MAX 100
#define NB_PARTIES_MAX 5

typedef struct {
    char UID[8];     // UID du badge RFID
    char nom[15];       // nom de la carte
    int valeur;         // valeur de carte
    int element;        // element de la carte (FEU ¦ EAU ¦ GLACE)
    int couleur;        // couleur de la carte (BLEU ¦ VERT ¦ JAUNE ¦ ROSE ¦ ORANGE )
} carte;

typedef struct {
    char UID[8];        // UID du badge RFID
    char nom[15];       // nom du joueur
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
} partie;

// Structure des messages
typedef struct {
    joueur joueur;           // Joueur (PID / PSEUDO / ROLE)
    partie partie;
    int requete;             // Requête
    char msg[2048];           // Zone de message Texte
} corp;

typedef struct {
    long num_socket;         // Numéro de la socket
    corp Corp;               // Corps du message
} message;

typedef struct {
    int nb_joueurs_online;
    int nb_joueurs_total;
    joueur joueurs[NB_JOUEURS_MAX];
    partie parties[NB_PARTIES_MAX];
    carte cartes[CARTES_MAX];
    pthread_mutex_t mutex;
} SharedInfos ;

#endif // DATA_H

