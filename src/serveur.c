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

SharedInfos infoServeur = {
    .nb_joueurs_online = 0,
    .joueurs = {{0}}, // vide
    .parties = {{0}},
    .cartes = {
        {{0x4F, 0x16, 0x89, 0xC9}, "Dragon de Feu", "Feu", 12, JAUNE},
        {{0x12, 0x34, 0x56, 0x78}, "Ninja d'Eau", "Eau", 8, BLEU},
        {{0xAB, 0xCD, 0xEF, 0x01}, "Guerrier Neige", "Neige", 10, VERT},
        {{0x25, 0x47, 0x69, 0x8B}, "Loup des Glaces", "Neige", 6, ROSE},
        {{0xFE, 0xDC, 0xBA, 0x98}, "Phoenix Écarlate", "Feu", 11, ORANGE},
    }
};

int main() {
    //Créer socket écoute
    socket_t sockServ = creerSocketEcoute(NULL, PORT, 5);
    printf("Serveur démarré sur le port %d\n", PORT);

    while (1) {
        socket_t sockClient = accepterClt(sockServ);
        printf("Client connecté, fd=%d\n", sockClient.fd);

        char buffer[MAX_BUFFER] = {0};
        recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);

        // Découpe du message en "code" et "données"
        char *code = strtok(buffer, ":");
        char *dataRaw = strtok(NULL, "\n");

        // 2. Découper dataRaw par virgule
        char *data[MAX_PARTS] = {0};
        int nb_data = 0;

        char *token = strtok(dataRaw, ",");
        while (token != NULL && nb_data < MAX_PARTS) {
            data[nb_data++] = token;
            token = strtok(NULL, ",");
        }

        switch (code) {
            case CMD_CONNECT:
                printf("Demande de connexion Client UID : %s",data[0]);
                trouverJoueur
                break;
            case CMD_JOIN:

                break;

            default :
        }

    }

    fermerSocket(sockServ);
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

//verifier l'enregistrement des id dans un fichier txt
/*int uid_registered(const char *uid_str, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    char line[32];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; // enlever saut de ligne
        if (strcmp(line, uid_str) == 0) {
            fclose(file);
            return 1; // trouvé
        }
    }
    fclose(file);
    return 0; // pas trouvé
}*/

