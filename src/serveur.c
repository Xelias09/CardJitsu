// serveur.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/socket_utils.h"
#include "../include/gameplay.h"
#include "../include/data.h"
#include "../include/protocole.h"

#define PORT 50000

void formatCarte(carte *c, char *buffer, size_t size);
void *clientHandler(void *arg);

SharedInfos infoServeur;

int main() {
    //Créer socket écoute
    socket_t sockServ = creerSocketEcoute(NULL, PORT, 5);
    printf("Serveur démarré sur le port %d\n", PORT);

    while (1) {
        socket_t sockClient = accepterClt(sockServ);
        printf("Client connecté, fd=%d\n", sockClient.fd);

        int *arg = malloc(sizeof(int));
        if (!arg) {
            perror("malloc");
            continue;
        }
        *arg = sockClient.fd;

        pthread_t thread;
        pthread_create(&thread, NULL, clientHandler, arg);
        pthread_detach(thread);  // éviter les fuites de threads zombies
    }
    fermerSocket(sockServ);
    return 0;
}


void *clientHandler(void *arg) {
    int tempfd = *((int*)arg);
    char buffer[MAX_BUFFER] = {0};

    while (1) {

        recv(tempfd, buffer, sizeof(buffer) - 1, 0);

        // 1. Découpe du message en "code" et "données"
        int code = atoi(strtok(buffer, ":"));
        char *dataRaw = strtok(NULL, "\n");

        // 2. Découper dataRaw par virgule
        char *data[] = {0};
        int nb_data = 0;

        char *token = strtok(dataRaw, ",");
        while (token != NULL && nb_data < MAX_PARTS) {
            data[nb_data++] = token;
            token = strtok(NULL, ",");
        }

        switch (code) {
            case CMD_CONNECT: // ajout flag connecter
                printf("Demande de connexion Client de UID : %s\n",data[0]);
                int index = trouverJoueur(data[0],infoServeur.joueurs,infoServeur.nb_joueurs_total);
                if (index == -1) {
                    printf("Envoi demande de pseudo à UID : %s\n",data[0]);
                    sprintf(buffer, "%d:",RSP_PLAYER_NEW);
                }
                else {
                    sprintf(buffer, "%d:",RSP_PLAYER_FOUND);
                    infoServeur.joueurs[index].status = ONLINE;
                    infoServeur.joueurs[index].socket.fd = tempfd;

                } // Envoi de la réponse au client
                send(tempfd, buffer, sizeof(buffer)-1, 0);
                break;

            default :
                break;
        }
    }

    return 0;
}

// Fonction pour sérialiser la carte en texte
void formatCarte(carte *c, char *buffer, size_t size) {
    if (!c) {
        snprintf(buffer, size, "Carte inconnue");
        return;
    }
    snprintf(buffer, size,
        "Nom: %s\nElement: %d\nValeur: %d\nCouleur: %d\n",
        c->nom, c->element, c->valeur, c->couleur);
}

//Création d'une partie ITE
int creerPartie(SharedInfos *infoServeur) {
    pthread_mutex_lock(&infoServeur->mutex);
    int i;
    for (i = 0; i < NB_PARTIES_MAX; i++) {
        if (infoServeur->parties[i].status == 0) {  // status 0 = partie vide/inactive
            infoServeur->parties[i].idPartie = i;
            infoServeur->parties[i].nbJoueurs = 0;
            infoServeur->parties[i].status = 1;  // 1 = partie créée mais pas commencée
            // vider la liste des joueurs
            memset(infoServeur->parties[i].liste_joueur, 0, sizeof(infoServeur->parties[i].liste_joueur));
            pthread_mutex_unlock(&infoServeur->mutex);
            return i;
        }
    }

    pthread_mutex_unlock(&infoServeur->mutex);
    return -1; // aucune partie dispo
}



