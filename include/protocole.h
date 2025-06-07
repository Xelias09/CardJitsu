//
// Created by alexis on 27/05/25.
//

#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#include <string.h>

#include "data.h"

#define CMD_CONNECT       51
#define CMD_REGISTER      52
#define CMD_LIST          53
#define CMD_CREAT         54
#define CMD_JOIN          55
#define CMD_QUIT          56

// Réponses serveur → client (si besoin)
#define RSP_OK            61
#define RSP_ERROR         62
#define RSP_UNKNOWN       63
#define RSP_CONNECTED     64
#define RSP_PLAYER_NEW    65 //demander à rentrer le pseudo


void serialiser_joueur(char* buffer, int code, joueur *j);

int deserialiser_message(char *buffer, int *code, char *data[], int *nb_data);

#endif //PROTOCOLE_H
