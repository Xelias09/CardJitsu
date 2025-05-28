//
// Created by alexis on 27/05/25.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../include/socket_utils.h"
#include "../../include/app/data.h"

#ifdef _DEBUG_
    #define DEBUG_PRINT(fmt, ...) fprintf(stderr, "DEBUG: " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

socket_t creerSocket(short mode) {
    socket_t socket_test;
    DEBUG_PRINT("Création d'une socket en mode %d", mode);
    CHECK(socket_test.fd = socket(AF_INET, mode, 0), "--socket()--");
    socket_test.mode = mode;
    return socket_test;
}

socket_t creerSocketAdr(short mode, char *IP, unsigned short port) {
    socket_t sock;
    struct sockaddr_in addr;

    DEBUG_PRINT("Création socket avec IP=%s, port=%d", IP ? IP : "INADDR_ANY", port);
    CHECK((sock.fd = socket(AF_INET, mode, 0)), "--socket()--");
    sock.mode = mode;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (IP == NULL)
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        CHECK(inet_pton(AF_INET, IP, &addr.sin_addr), "--inet_pton()--");

    CHECK(bind(sock.fd, (struct sockaddr*)&addr, sizeof(addr)), "--bind()--");
    sock.adrLoc = addr;

    return sock;
}

socket_t creerSocketEcoute(char *IP, unsigned short port, short maxC) {
    socket_t sd = creerSocketAdr(SOCK_STREAM, IP, port);
    DEBUG_PRINT("Mise en écoute sur %s:%d", IP ? IP : "INADDR_ANY", port);
    CHECK(listen(sd.fd, maxC), "--listen()--");
    return sd;
}

socket_t creerSocketUDP(char *IP, unsigned short port) {
    return creerSocketAdr(SOCK_DGRAM, IP, port);
}

socket_t accepterClt(socket_t sockEcoute){
    socket_t sd;
    struct sockaddr_in adrClient;
    socklen_t lgAdrClient = sizeof(adrClient);
    DEBUG_PRINT("Attente client sur socket %d", sockEcoute.fd);
    CHECK(sd.fd = accept(sockEcoute.fd, (struct sockaddr *)&adrClient, &lgAdrClient), "--accept()--");
    return sd;
}

void connecterSrv(socket_t socket, char *IP, unsigned short port) {
    struct sockaddr_in adrSrv;
    adrSrv.sin_family = AF_INET;
    adrSrv.sin_addr.s_addr = inet_addr(IP);
    adrSrv.sin_port = htons(port);
    memset(&adrSrv.sin_zero, 0, 8);
    DEBUG_PRINT("Connexion serveur %s:%d", IP, port);
    CHECK(connect(socket.fd, (struct sockaddr *)&adrSrv, sizeof(adrSrv)), "--connect()--");
}

socket_t connecterClt2Srv(char *IP, unsigned short port) {
    socket_t socket = creerSocket(SOCK_STREAM);
    connecterSrv(socket, IP, port);
    return socket;
}

int _envoyerDGRAM(socket_t *socket, struct sockaddr_in adrDist, unsigned short port, char *msg) {
    socklen_t addrLen = sizeof(struct sockaddr_in);
    size_t bytesRead;

    if (adrDist.sin_family != AF_INET)
        adrDist.sin_family = AF_INET;

    if (port != 0)
        adrDist.sin_port = htons(port);

    DEBUG_PRINT("Envoi DGRAM à %s:%d", inet_ntoa(adrDist.sin_addr), ntohs(adrDist.sin_port));
    CHECK(bytesRead = sendto(socket->fd, msg, strlen(msg), 0, (struct sockaddr*)&adrDist, addrLen), "--sendto()--");

    socket->adrDist = adrDist;
    return bytesRead;
}

int _envoyerSTREAM(socket_t *socket, char *msg) {
    size_t len = strlen(msg) + 1;
    size_t bytesRead;

    DEBUG_PRINT("Envoi STREAM : %s", msg);
    CHECK(bytesRead = write(socket->fd, msg, len), "--write()--");

    return bytesRead;
}

int _recevoirDGRAM(socket_t *socket, buffer_t *buff) {
    socklen_t addrLen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
    ssize_t bytesRead;

    memset(&clientAddr, 0, sizeof(clientAddr));
    DEBUG_PRINT("Réception DGRAM sur socket %d", socket->fd);

    CHECK(bytesRead = recvfrom(socket->fd, buff->data, MAX_BUFF - 1, 0,
                               (struct sockaddr*)&clientAddr, &addrLen), "--recvfrom()--");

    socket->adrDist = clientAddr;
    buff->data[bytesRead] = '\0';
    buff->size = bytesRead;

    return bytesRead;
}

int _recevoirSTREAM(socket_t *socket, buffer_t *buff) {
    int recu;

    if (!socket || !buff) return -1;

    recu = recv(socket->fd, buff->data, MAX_BUFF - 1, 0);

    if (recu >= 0)
        buff->data[recu] = '\0';

    return recu;
}

socket_t fermerSocket(socket_t socket){
    DEBUG_PRINT("Fermeture de socket %d", socket.fd);
    CHECK(close(socket.fd), "--close()--");
    return socket;
}
