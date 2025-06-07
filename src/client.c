#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/protocole.h"
#include "../include/socket_utils.h"
#include "../include/rfid.h"
#include "../include/data.h"
#include "../include/IHM.h"

#define SERVER_IP "172.20.10.2"
#define SERVER_PORT 50000

int Menu();

joueur client;

int main() {
    printf("[DEBUG] Initialisation du lecteur RFID...\n");
    if (initRFID() != 0) {
        fprintf(stderr, "Erreur initialisation RFID\n");
        return 1;
    }

    printf("[DEBUG] Connexion au serveur...\n");
    socket_t sockClient = connecterClt2Srv(SERVER_IP, SERVER_PORT);
    printf("Connecté au serveur %s:%d\n", SERVER_IP, SERVER_PORT);

    uint8_t uid[4];
    int f_rfid = 0;

    printf("[DEBUG] Attente détection RFID...\n");
    while (!f_rfid) {
        printf("En attente de carte RFID...\n");
        while (detectCard() != MI_OK);
        if (readUID(uid) == MI_OK) {
            f_rfid = 1;
            printf("[DEBUG] UID détecté : %02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3]);
        }
        sleep(2);
    }

    sprintf(client.UID,"%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
    printf("[DEBUG] UID enregistré dans structure client : %s\n", client.UID);

    // Initialisation communication
    char buffer[MAX_BUFFER] = {0};
    int code = 0;
    int nb_data = 0;
    char *data[MAX_PARTS] = {0};

    // Envoi UID au serveur
    printf("[DEBUG] UID = '%s' | NOM = '%s'\n", client.UID, client.nom);
    sprintf(buffer, "%d:%s,", CMD_CONNECT, client.UID);
    printf("[DEBUG] Envoi CMD_CONNECT : %s\n", buffer);
    send(sockClient.fd, buffer, strlen(buffer), 0);  // <-- corrigé avec strlen

    int ConnexionOk = 0;


    while (!ConnexionOk) {
        nb_data = 0;
        code = 0;
        memset(buffer, 0, sizeof(buffer));

        printf("[DEBUG] Attente de réponse serveur...\n");
        recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
        printf("[DEBUG] Réponse brute : %s\n", buffer);

        deserialiser_message(buffer, &code, data, &nb_data);
        printf("[DEBUG] Code reçu : %d | nb_data = %d\n", code, nb_data);

        memset(buffer, 0, sizeof(buffer));

        switch (code) {
            case RSP_CONNECTED:
                printf("Connexion réussie ! Bienvenue %s !\n", data[0]);
                strcpy(client.nom, data[0]);
                ConnexionOk = 1;
                break;

            case RSP_PLAYER_NEW:
                printf("Nouvelle carte détectée. Entrez votre pseudo : ");
                fgets(client.nom, sizeof(client.nom), stdin);
                client.nom[strcspn(client.nom, "\n")] = 0;

                printf("[DEBUG] UID = '%s' | NOM = '%s'\n", client.UID, client.nom);
                sprintf(buffer, "%d:%s,%s", CMD_REGISTER, client.UID, client.nom);
                printf("[DEBUG] Envoi CMD_REGISTER : %s\n", buffer);
                send(sockClient.fd, buffer, strlen(buffer), 0);
                break;

            default:
                printf("[WARN] Code inconnu reçu : %d\n", code);
                break;
        }
        printf("[DEBUG] Contenu buffer post-switch : %s\n", buffer);
    }

    while (ConnexionOk) {
        int PartieRejointe = 0;
        int choix = Menu();  // Appel de la fonction Menu et récupération du choix

        while (PartieRejointe == 0) {

            switch (choix) {

                case 1:
                    regles();
                    printf("\n(Appuyez sur Entrée pour revenir au menu…) ");
                    getchar();
                break;

                case 2:
                    sprintf(buffer, "%d:%s,", CMD_LIST, client.UID);
                    send(sockClient.fd, buffer, strlen(buffer), 0);

                    memset(buffer, 0, sizeof(buffer));
                    int r = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
                    if (r <= 0) {
                        printf("[ERREUR] Impossible de recevoir la liste des parties\n");
                        break;
                    }
                    buffer[r] = '\0';

                    printf("[DEBUG] Réponse brute CMD_LIST : %s\n", buffer);

                    int code = 0;
                    char *token = strtok(buffer, ":");
                    if (token == NULL) break;
                    code = atoi(token);

                    if (code == RSP_ERROR) {
                        printf("[ERREUR] Réponse du serveur (code %d)\n", code);
                        break;
                    }

                    char *parties = strtok(NULL, "");
                    if (parties == NULL || strlen(parties) == 0) {
                        printf("Aucune partie disponible pour le moment.\n");
                        break;
                    }

                    printf("\nListe des parties disponibles :\n");

                    char *ligne = strtok(parties, ";");
                    while (ligne != NULL) {
                        int idPartie = -1, nbJoueurs = 0;
                        char status[32] = {0};
                        char joueurs[128] = {0};

                        sscanf(ligne, "%d,%[^,],%d,%127[^\n]", &idPartie, status, &nbJoueurs, joueurs);

                        printf("Partie %d - Status : %s - Nombre de joueurs : %d\n", idPartie, status, nbJoueurs);

                        if (nbJoueurs > 0) {
                            char *nom = strtok(joueurs, "|");
                            int j = 1;
                            while (nom != NULL && j <= nbJoueurs) {
                                printf("  Joueur %d : %s\n", j++, nom);
                                nom = strtok(NULL, "|");
                            }
                        }

                        printf("\n");
                        ligne = strtok(NULL, ";");
                    }

                    printf("Entrez l'ID de la partie à rejoindre (0 pour annuler) : ");
                    int choixPartie = 10;
                    scanf("%d", &choixPartie);

                    if (choixPartie == 10) {
                        printf("Retour au menu.\n");
                        break;
                    }

                    sprintf(buffer, "%d:%s,%d", CMD_JOIN, client.UID, choixPartie);
                    printf("[DEBUG] Contenu buffer avant envoi : %s\n", buffer);
                    send(sockClient.fd, buffer, strlen(buffer), 0);

                    memset(buffer, 0, sizeof(buffer));
                    r = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
                    if (r <= 0) {
                        printf("[ERREUR] Pas de réponse du serveur après requête rejoindre partie\n");
                        break;
                    }
                    buffer[r] = '\0';

                    deserialiser_message(buffer, &code, data, &nb_data);
                    if (code == RSP_JOINED) {
                        printf("Vous avez rejoint la partie %d avec succès !\n", choixPartie);
                        PartieRejointe = 1;
                    } else {
                        printf("Erreur lors de la tentative de rejoindre la partie %d (code %d)\n", choixPartie, code);
                    }
                     break;

                case 3: //Créer une partie
                    // Préparation du message pour créer la partie
                    sprintf(buffer, "%d:%s,", CMD_CREAT, client.UID);
                    send(sockClient.fd, buffer, strlen(buffer), 0);

                    // Réinitialiser buffer avant réception
                    memset(buffer, 0, sizeof(buffer));

                     r = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);

                    // Désérialiser la réponse
                    deserialiser_message(buffer, &code, data, &nb_data);
                    printf("[DEBUG] Reçu (%d octets) : %s\n", r, buffer);

                    if (code == RSP_OK) {
                        printf("Partie créée avec succès !\n");
                        PartieRejointe = 1;
                    } else {
                        printf("Erreur lors de la création de la partie : code %d\n", code);
                    }
                break;

                case 10:
                    printf("Au revoir !\n");
                    ConnexionOk = 0 ;
                break;

                default:
                    printf("Choix invalide. Veuillez réessayer.\n");
                    sleep(1);
                break;

             }
         }

         while (PartieRejointe == 1 ) {


             printf ("Bienvenue dans la partie");
             getchar();
         }
     }



    fermerSocket(sockClient);
    return 0;
}

int Menu() {
    int choix = 0;

    afficherMenu();// Affiche le menu
    choix = lireChoixMenu(); // Lit le choix

    return choix;
}
