//
// Created by alexis on 27/05/25.
//

#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>

#define MAX_BUFFER 1024
#define MAX_CLIENTS 10

typedef struct {
    int fd;
    short mode;
    struct sockaddr_in adrLoc;
    struct sockaddr_in adrDist;
} socket_t;

typedef struct {
    char data[MAX_BUFFER];
    size_t size;
} buffer_t;

socket_t creerSocket(short mode);

socket_t creerSocketAdr(short mode, char *IP, unsigned short port);

socket_t creerSocketEcoute(char *IP, unsigned short port, short maxC);

socket_t creerSocketUDP(char *IP, unsigned short port);

socket_t accepterClt(socket_t sockEcoute);

void connecterSrv(socket_t socket, char *IP, unsigned short port);

socket_t connecterClt2Srv(char *IP, unsigned short port);

int _envoyerDGRAM(socket_t *socket, struct sockaddr_in adrDist, unsigned short port, char *msg);

int _envoyerSTREAM(socket_t *socket, char *msg);

int _recevoirDGRAM(socket_t *socket, buffer_t *buff);

int _recevoirSTREAM(socket_t *socket, buffer_t *buff);

socket_t fermerSocket(socket_t socket);

#endif //SOCKET_H
