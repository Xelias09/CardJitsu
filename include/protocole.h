//
// Created by alexis on 27/05/25.
//

#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#include "data.h"

// Commandes client → serveur
#define CMD_CONNECT             51
#define CMD_REGISTER            52
#define CMD_LIST                53
#define CMD_CREAT               54
#define CMD_JOIN                55
#define CMD_QUIT                56

// Réponses serveur → client
#define RSP_OK                  61
#define RSP_ERROR               62
#define RSP_LIST                63
#define RSP_CONNECTED           64
#define RSP_PLAYER_NEW          65          //demander à rentrer le pseudo
#define RSP_JOINED              66

// Echange Gameplay serveur → client
#define CMD_GAME_START          71
#define CMD_GAME_STOP           72
#define CMD_GAME_WINNER         73
#define CMD_GAME_LOSE           74
#define CMD_GAME_CARD           75          // + envoyer l'état de la partie
#define CMD_GAME_SCAN           76

// Echange Gameplay client → serveur
#define RSP_GAME_OK             81
#define RSP_GAME_CARD           82

void serialiser_joueur(char* buffer, int code, joueur *j);

int deserialiser_message(char *buffer, int *code, char *data[], int *nb_data);

#endif //PROTOCOLE_H
