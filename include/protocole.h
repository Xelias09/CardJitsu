//
// Created by alexis on 27/05/25.
//

#ifndef PROTOCOLE_H
#define PROTOCOLE_H

#define CMD_CONNECT       51
#define CMD_REGISTER      52
#define CMD_LIST          53
#define CMD_PLAY          54
#define CMD_JOIN          55
#define CMD_QUIT          56

// Réponses serveur → client (si besoin)
#define RSP_OK            61
#define RSP_ERROR         62
#define RSP_UNKNOWN       63
#define RSP_PLAYER_FOUND  64
#define RSP_PLAYER_NEW    65

#endif //PROTOCOLE_H
