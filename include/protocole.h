//
// Created by alexis on 27/05/25.
//

#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#define CMD_CONNECT       "UID"
#define CMD_REGISTER      "INSCR"
#define CMD_LIST          "LIST"
#define CMD_PLAY          "PLAY"
#define CMD_JOIN          "JOIN"
#define CMD_QUIT          "QUIT"
#define CMD_PING          "PING"

// Réponses serveur → client (si besoin)
#define RSP_OK            "OK"
#define RSP_ERROR         "ERR"
#define RSP_UNKNOWN       "UNKNOWN"
#define RSP_PLAYER_FOUND  "FOUND"
#define RSP_PLAYER_NEW    "NEW"

#endif //PROTOCOLE_H
