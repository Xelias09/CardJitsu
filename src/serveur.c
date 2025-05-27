//
// Created by alexis on 27/05/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/data.h"
#include "../include/protocole.h"
#include "../include/socket_utils.h"

joueur data_joueur[MAX_CLIENTS];
int index_data_joueur = 0;

int main(int argc, char *argv[]) {
    socket_t socketEcoute, socketClient;
    pthread_t thread;

    int port = 5000; // Valeur par défaut

    if (argc >= 2) {
        port = atoi(argv[1]); // Convertit argv[1] en entier
    }

    // Création du socket d'écoute
    socketEcoute = creerSocketEcoute(NULL, port, MAX_CLIENTS);
    printf("Serveur démarré sur le port %d\n", port);

    // Boucle principale d'acceptation des clients
    while (1) {
        // Accepter un nouveau client
        socketClient = accepterClt(socketEcoute);

    }
    return 0;
}