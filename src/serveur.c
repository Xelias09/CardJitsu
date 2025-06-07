// serveur.c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/socket_utils.h"
#include "../include/gameplay.h"
#include "../include/data.h"
#include "../include/protocole.h"

#define PORT 50000

void *clientHandler(void *arg);
void *threadPartieHandler(void *arg);
int creerPartie(SharedInfos *infoServeur);
void afficher_joueurs_enregistres(SharedInfos *info);

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
    free(arg);  // important : libérer la mémoire allouée dans main()

    char buffer[MAX_BUFFER] = {0};
    int code = 0;
    int nb_data = 0;
    int index;
    char *data[MAX_PARTS] = {0};

    printf("[DEBUG] Thread client lancé, fd = %d\n", tempfd);

    while (1) {
        nb_data = 0;
        code = 0;
        memset(buffer, 0, sizeof(buffer));
        afficher_joueurs_enregistres(&infoServeur);
        int r = recv(tempfd, buffer, sizeof(buffer) - 1, 0);
        if (r <= 0) {
            printf("[INFO] Client déconnecté ou erreur recv (fd=%d)\n", tempfd);
            close(tempfd);
            return 0;
        }

        printf("[DEBUG] Reçu (%d octets) : %s\n", r, buffer);

        deserialiser_message(buffer, &code, data, &nb_data);
        int i;
        for ( i = 0; i < nb_data; i++) {
            printf("[DEBUG] data[%d] = '%s'\n", i, data[i]);
        }

        printf("[DEBUG] Code reçu : %d, nb_data : %d\n", code, nb_data);

        memset(buffer, 0, sizeof(buffer));

        switch (code) {
            case CMD_CONNECT:
                if (nb_data < 1) {
                    printf("[ERREUR] CMD_CONNECT : UID manquant\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                printf("[INFO] CMD_CONNECT reçu avec UID : %s\n", data[0]);
                pthread_mutex_lock(&infoServeur.mutex);
                index = trouverJoueur(data[0], infoServeur.joueurs, infoServeur.nb_joueurs_total);
                if (infoServeur.joueurs[index].status == ONLINE) {
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }
                pthread_mutex_unlock(&infoServeur.mutex);

                if (index == -1) {
                    printf("[INFO] Joueur inconnu, envoi RSP_PLAYER_NEW\n");
                    sprintf(buffer, "%d:", RSP_PLAYER_NEW);
                }
                else {
                    pthread_mutex_lock(&infoServeur.mutex);
                    infoServeur.joueurs[index].status = ONLINE;
                    infoServeur.joueurs[index].socket.fd = tempfd;
                    serialiser_joueur(buffer, RSP_CONNECTED, &infoServeur.joueurs[index]);
                    pthread_mutex_unlock(&infoServeur.mutex);
                    printf("[INFO] Joueur %s connecté (fd=%d)\n", infoServeur.joueurs[index].nom, tempfd);
                }

                send(tempfd, buffer, strlen(buffer), 0);
                printf("[DEBUG] Réponse envoyée : %s\n", buffer);
                break;

            case CMD_REGISTER:
                if (nb_data < 2) {
                    printf("[ERREUR] CMD_REGISTER : données insuffisantes.\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                printf("[INFO] CMD_REGISTER reçu pour UID : %s\n", data[0]);
                index = trouverJoueur(data[0], infoServeur.joueurs, infoServeur.nb_joueurs_total);

                if (index == -1) {
                    printf("[INFO] Nouveau joueur, ajout UID : %s, pseudo : %s\n", data[0], data[1]);
                    pthread_mutex_lock(&infoServeur.mutex);
                    // Ajout à la fin de la liste
                    int last = infoServeur.nb_joueurs_total;
                    if (last >= NB_JOUEURS_MAX) {
                        printf("[ERREUR] Trop de joueurs enregistrés !\n");
                        sprintf(buffer, "%d:", RSP_ERROR);
                        pthread_mutex_unlock(&infoServeur.mutex);
                        send(tempfd, buffer, strlen(buffer), 0);
                        break;
                    }
                    joueur *j = &infoServeur.joueurs[last];
                    j->exp = 0;
                    j->status = ONLINE;
                    j->rang = 0;
                    strncpy(j->UID, data[0], sizeof(j->UID) - 1);
                    j->UID[sizeof(j->UID) - 1] = '\0';
                    strncpy(j->nom, data[1], sizeof(j->nom) - 1);
                    j->nom[sizeof(j->nom) - 1] = '\0';
                    j->socket.fd = tempfd;

                    // Incrémenter le compteur
                    infoServeur.nb_joueurs_total++;

                    serialiser_joueur(buffer, RSP_CONNECTED, j);
                    pthread_mutex_unlock(&infoServeur.mutex);

                    printf("[DEBUG] Joueur enregistré : %s (%s)\n", j->nom, j->UID);
                }
                else {
                    pthread_mutex_lock(&infoServeur.mutex);
                    sprintf(buffer, "%d:", RSP_ERROR);
                    pthread_mutex_unlock(&infoServeur.mutex);

                    printf("[WARN] Joueur UID déjà présent : %s. Refus enregistrement.\n", data[0]);
                }

                send(tempfd, buffer, strlen(buffer), 0);
                printf("[DEBUG] Réponse envoyée : %s\n", buffer);
                break;

            case CMD_CREAT:
                if (nb_data < 1) {
                    printf("[ERREUR] CMD_CREAT : UID manquant\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                pthread_mutex_lock(&infoServeur.mutex);
                index = trouverJoueur(data[0], infoServeur.joueurs, infoServeur.nb_joueurs_total);
                pthread_mutex_unlock(&infoServeur.mutex);

                if (index == -1) {
                    printf("[ERREUR] CMD_CREAT : Joueur introuvable\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                pthread_mutex_lock(&infoServeur.mutex);
                int partieID = creerPartie(&infoServeur);
                pthread_mutex_unlock(&infoServeur.mutex);

                if (partieID == -1) {
                    printf("[ERREUR] CMD_CREAT : Plus de slots disponibles\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                // Ajouter le joueur à la partie
                pthread_mutex_lock(&infoServeur.mutex);
                partie *p = &infoServeur.parties[partieID];
                p->liste_joueur[0] = infoServeur.joueurs[index];
                p->nbJoueurs = 1;
                p->status = 1; // 1 = attente
                pthread_mutex_unlock(&infoServeur.mutex);

                printf("[INFO] Partie %d créée par %s (%s)\n", partieID, infoServeur.joueurs[index].nom, data[0]);

                sprintf(buffer, "%d:%d", RSP_OK, partieID);
                send(tempfd, buffer, strlen(buffer), 0);

                // Lancer un thread de gestion de partie (optionnel à ce stade)
                pthread_t threadPartie;
                int *argp = malloc(sizeof(int));
                *argp = partieID;
                pthread_create(&threadPartie, NULL, threadPartieHandler, argp);
                pthread_detach(threadPartie);
                break;

            default:
                printf("[WARN] Code inconnu reçu : %d\n", code);
                break;
        }
    }

    return 0;
}

void *threadPartieHandler(void *arg) {
    int idPartie = *((int*)arg);
    free(arg);  // libère l'argument passé au thread

    printf("[PARTIE %d] Thread démarré.\n", idPartie);

    while (1) {
        pthread_mutex_lock(&infoServeur.mutex);
        partie *p = &infoServeur.parties[idPartie];

        if (p->status == 3) { // 3 = partie terminée
            pthread_mutex_unlock(&infoServeur.mutex);
            printf("[PARTIE %d] Partie terminée. Thread se termine.\n", idPartie);
            break;
        }

        // si 2 joueurs, démarrer
        if (p->status == 1 && p->nbJoueurs == 2) {
            p->status = 2; // 2 = en cours
            printf("[PARTIE %d] Démarrage de la partie avec %s et %s\n",
                idPartie,
                p->liste_joueur[0].nom,
                p->liste_joueur[1].nom);
        }
        pthread_mutex_unlock(&infoServeur.mutex);

        sleep(1);
    }
    return NULL;
}

void afficher_joueurs_enregistres(SharedInfos *info) {
    pthread_mutex_lock(&info->mutex);

    // --- Affichage des joueurs ---
    printf("\n========== Liste des joueurs (%d) ==========\n", info->nb_joueurs_total);
    printf("| %-3s | %-15s | %-10s | %-7s | %-4s | %-5s | %-3s |\n",
           "ID", "Nom", "UID", "Statut", "Rang", "EXP", "FD");
    printf("---------------------------------------------------------------------\n");

    for (int i = 0; i < info->nb_joueurs_total; i++) {
        joueur *j = &info->joueurs[i];
        printf("| %-3d | %-15s | %-10s | %-7s | %-4d | %-5d | %-3d |\n",
               i + 1,
               j->nom,
               j->UID,
               j->status == ONLINE ? "ONLINE" : "OFFLINE",
               j->rang,
               j->exp,
               j->socket.fd);
    }

    printf("=====================================================================\n");

    // --- Affichage des parties ---
    printf("\n========== Liste des parties créées ==========\n");
    printf("| %-3s | %-7s | %-10s |\n", "ID", "Status", "Nb Joueurs");
    printf("---------------------------------------------\n");

    for (int i = 0; i < NB_PARTIES_MAX; i++) {
        if (info->parties[i].status != 0) {  // Partie active
            printf("| %-3d | %-7s | %-10d |\n",
                   info->parties[i].idPartie,
                   info->parties[i].status == 1 ? "EN ATT." :
                   info->parties[i].status == 2 ? "EN COURS" : "TERMINE",
                   info->parties[i].nbJoueurs);
        }
    }

    printf("=============================================\n\n");

    pthread_mutex_unlock(&info->mutex);
}





