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
void afficher_joueurs_enregistres(SharedInfos *info);
void initialiser_cartes_test(SharedInfos* info) {
    pthread_mutex_lock(&info->mutex);

    info->cartes[0] = (carte){ .UID = "C71A8AC9", .nom = "Feu Rouge",  .valeur = 5, .element = FEU,   .couleur = ROSE };
    info->cartes[1] = (carte){ .UID = "F661AA21", .nom = "Glace Bleue", .valeur = 3, .element = GLACE, .couleur = BLEU };
    info->cartes[2] = (carte){ .UID = "D4187021", .nom = "Eau Verte",   .valeur = 7, .element = EAU,   .couleur = VERT };
    info->cartes[3] = (carte){ .UID = "70105F60", .nom = "Feu Jaune",   .valeur = 6, .element = FEU,   .couleur = JAUNE };
    info->cartes[4] = (carte){ .UID = "F0769960", .nom = "Glace Rose",  .valeur = 2, .element = GLACE, .couleur = ROSE };
    info->cartes[5] = (carte){ .UID = "446189C9", .nom = "Eau Orange",  .valeur = 4, .element = EAU,   .couleur = ORANGE };

    pthread_mutex_unlock(&info->mutex);

    printf("[INIT] Cartes de test initialisées.\n");

}
SharedInfos infoServeur;

int main() {
    initialiserSharedInfos(&infoServeur);
    initialiser_cartes_test(&infoServeur);
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
    int f_regitered = 0;
    char *data[MAX_PARTS] = {0};
    partie *p;

    printf("[DEBUG] Thread client lancé, fd = %d\n", tempfd);

    while (1) {
        nb_data = 0;
        code = 0;
        memset(buffer, 0, sizeof(buffer));
        if (f_regitered) {
            while (infoServeur.joueurs[index].mode == 1);
        }
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
                    printf("[INFO] Joueur déjà connecté avec l'UID : %s\n", data[0]);
                    pthread_mutex_unlock(&infoServeur.mutex);
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }
                pthread_mutex_unlock(&infoServeur.mutex);

                if (index == -1) {
                    printf("[INFO] Joueur inconnu, envoi RSP_PLAYER_NEW\n");
                    sprintf(buffer, "%d:", RSP_PLAYER_NEW);
                    f_regitered = 1;
                }
                else {
                    pthread_mutex_lock(&infoServeur.mutex);
                    infoServeur.joueurs[index].status = ONLINE;
                    infoServeur.joueurs[index].socket.fd = tempfd;
                    serialiser_joueur(buffer, RSP_CONNECTED, &infoServeur.joueurs[index]);
                    infoServeur.nb_joueurs_online++;
                    pthread_mutex_unlock(&infoServeur.mutex);
                    printf("[INFO] Joueur %s connecté (fd=%d)\n", infoServeur.joueurs[index].nom, tempfd);
                    f_regitered = 1;
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
                    infoServeur.nb_joueurs_online++;

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
                infoServeur.joueurs[index].mode = 1;
                p = &infoServeur.parties[partieID];
                p->liste_joueur[0] = infoServeur.joueurs[index];
                p->nbJoueurs = 1;
                p->status = ATTENTE; // 1 = attente
                pthread_mutex_unlock(&infoServeur.mutex);

                printf("[INFO] Partie %d créée par %s (%s)\n", partieID, infoServeur.joueurs[index].nom, data[0]);

                sprintf(buffer, "%d:%d,", RSP_OK, partieID);
                send(tempfd, buffer, strlen(buffer), 0);

                // Lancer un thread de gestion de partie
                pthread_t threadPartie;
                int *argp = malloc(sizeof(int));
                *argp = partieID;
                pthread_create(&threadPartie, NULL, threadPartieHandler, argp);
                pthread_detach(threadPartie);
                printf("\n\nBREAK\n");
                break;

            case CMD_LIST:
                if (nb_data < 1) {
                    printf("[ERREUR] CMD_LIST : UID manquant\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                pthread_mutex_lock(&infoServeur.mutex);
                index = trouverJoueur(data[0], infoServeur.joueurs, infoServeur.nb_joueurs_total);
                pthread_mutex_unlock(&infoServeur.mutex);

                if (index == -1) {
                    printf("[ERREUR] CMD_LIST : UID invalide\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer, "%d:", RSP_LIST);  // début du message

                pthread_mutex_lock(&infoServeur.mutex);
                for (i = 0; i < NB_PARTIES_MAX; i++) {

                    p = &infoServeur.parties[i];
                    if (p->status != 0) {
                        char ligne[128] = {0};

                        const char *status_str =
                            (p->status == ATTENTE) ? "EN_ATTENTE" :
                            (p->status == ENCOURS) ? "EN_COURS" :
                            (p->status == LIBRE)   ? "LIBRE" :
                            (p->status == TERMINE) ? "TERMINE" : "INVALIDE";

                        // Commence la ligne avec id, statut, nb joueurs
                        sprintf(ligne, "%d,%s,%d,", p->idPartie, status_str, p->nbJoueurs);

                        // Ajoute les noms des joueurs séparés par |
                        int j;
                        for (j = 0; j < p->nbJoueurs; j++) {
                            strcat(ligne, p->liste_joueur[j].nom);
                            if (j < p->nbJoueurs - 1)
                                strcat(ligne, "|");
                        }

                        strcat(ligne, ";"); // fin de description de la partie
                        strcat(buffer, ligne); // ajoute à la réponse
                    }
                }
                pthread_mutex_unlock(&infoServeur.mutex);

                send(tempfd, buffer, strlen(buffer), 0);
                break;

            case CMD_JOIN:
                if (nb_data < 2) {
                    printf("[ERREUR] CMD_JOIN : arguments manquants (UID et ID_PARTIE)\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }
                int id_partie = atoi(data[1]);

                pthread_mutex_lock(&infoServeur.mutex);

                // Vérification du joueur
                int indexJoueur = trouverJoueur(data[0], infoServeur.joueurs, infoServeur.nb_joueurs_total);
                if (indexJoueur == -1) {
                    pthread_mutex_unlock(&infoServeur.mutex);
                    printf("[ERREUR] CMD_JOIN : UID inconnu\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                // Vérification de la partie
                if (id_partie < 0 || id_partie >= NB_PARTIES_MAX ||
                    infoServeur.parties[id_partie].status != ATTENTE) {
                    pthread_mutex_unlock(&infoServeur.mutex);
                    printf("[ERREUR] CMD_JOIN : Partie invalide ou non disponible\n");
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                    }

                p = &infoServeur.parties[id_partie];

                if (p->nbJoueurs >= 2) {
                    pthread_mutex_unlock(&infoServeur.mutex);
                    printf("[ERREUR] CMD_JOIN : Partie %d déjà pleine\n", id_partie);
                    sprintf(buffer, "%d:", RSP_ERROR);
                    send(tempfd, buffer, strlen(buffer), 0);
                    break;
                }

                // Ajouter le joueur
                infoServeur.joueurs[index].mode = 1;
                p->liste_joueur[p->nbJoueurs] = infoServeur.joueurs[indexJoueur];
                p->nbJoueurs++;

                pthread_mutex_unlock(&infoServeur.mutex);

                printf("[INFO] Joueur %s a rejoint la partie %d\n", infoServeur.joueurs[indexJoueur].nom, id_partie);

                // Répondre au client
                sprintf(buffer, "%d:%d", RSP_JOINED, id_partie);
                send(tempfd, buffer, strlen(buffer), 0);
                break;

            default:
                printf("[WARN] Code inconnu reçu : %d\n", code);
                break;
        }
        printf("\n\n[DEBUG] Envoi à fd %d : %s\n\n", tempfd, buffer);
    }
    return 0;
}

void* threadPartieHandler(void* arg) {
    int idPartie = *((int*)arg);
    free(arg);

    printf("\t[PARTIE %d] Thread démarré.\n", idPartie);

    partie* p = &infoServeur.parties[idPartie];

    char buffer[MAX_BUFFER];
    int code = 0, nb_data = 0;
    int i;
    char* data[MAX_PARTS] = {0};
    char *gagnant = malloc(32);
    if (gagnant == NULL) { perror("malloc"); exit(1); }

    while (infoServeur.parties[idPartie].status == ATTENTE && infoServeur.parties[idPartie].nbJoueurs != 2);

    infoServeur.parties[idPartie].status = ENCOURS;

    // Notifier les deux joueurs que la partie démarre
    sprintf(buffer, "%d:%d,", CMD_GAME_START, idPartie);
    for (i = 0; i < 2; i++) {
        send(p->liste_joueur[i].socket.fd, buffer, strlen(buffer), 0);
    }

    // 1. Attendre RSP_GAME_OK de chaque joueur
    for (i = 0; i < 2; i++) {
        memset(buffer, 0, sizeof(buffer));
        recv(p->liste_joueur[i].socket.fd, buffer, sizeof(buffer) - 1, 0);
        deserialiser_message(buffer, &code, data, &nb_data);
        if (code != RSP_GAME_OK) {
            printf("[PARTIE %d] Erreur: joueur %d n'a pas répondu OK\n", idPartie, i);
            return NULL;
        }
        printf("[PARTIE %d] %s est prêt.\n", idPartie, p->liste_joueur[i].nom);
    }

    printf("[PARTIE %d] Tous les joueurs sont prêts. Début du jeu.\n", idPartie);

    int fin = 0;
    while (!fin) {
            sleep(5);
            // === PHASE 1 : DEMANDE DE CARTE AUX JOUEURS ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 1 : Demande de carte aux joueurs ===\033[0m\n", idPartie);

            sprintf(buffer, "%d:%d,", CMD_GAME_SCAN, idPartie);
            for (i = 0; i < 2; i++) {
                printf("[PARTIE %d] 📤 [SEND] -> %s : \"%s\"\n", idPartie, p->liste_joueur[i].nom, buffer);
                send(p->liste_joueur[i].socket.fd, buffer, strlen(buffer), 0);
            }


            // === PHASE 2 : RÉCEPTION DES CARTES ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 2 : Réception des cartes des joueurs ===\033[0m\n", idPartie);

            char uid_cartes[2][16] = {{0}};
            carte cartes[2];

            for (i = 0; i < 2; i++) {
                memset(buffer, 0, sizeof(buffer));
                recv(p->liste_joueur[i].socket.fd, buffer, sizeof(buffer) - 1, 0);
                printf("[PARTIE %d] 📥 [RECV] <- %s : \"%s\"\n", idPartie, p->liste_joueur[i].nom, buffer);

                deserialiser_message(buffer, &code, data, &nb_data);
                if (code != RSP_GAME_CARD || nb_data < 2) {
                    printf("[PARTIE %d] ❌ Erreur réception carte de %s\n", idPartie, p->liste_joueur[i].nom);
                    return NULL;
                }

                strncpy(uid_cartes[i], data[1], sizeof(uid_cartes[i]) - 1);
                int idx = trouverCarte(uid_cartes[i], infoServeur.cartes, sizeof(infoServeur.cartes));
                if (idx == -1) {
                    printf("[PARTIE %d] ❌ UID inconnu reçu de %s : %s\n", idPartie, p->liste_joueur[i].nom, uid_cartes[i]);
                    return NULL;
                }

                cartes[i] = infoServeur.cartes[idx];
                printf("[PARTIE %d] ✅ %s a joué : %s [élément=%d | valeur=%d | couleur=%d]\n",
                       idPartie, p->liste_joueur[i].nom, cartes[i].nom,
                       cartes[i].element, cartes[i].valeur, cartes[i].couleur);
            }

            // === PHASE 3 : COMPARAISON DES CARTES ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 3 : Comparaison des cartes ===\033[0m\n", idPartie);

            int vainqueur = -1;
            if (cartes[0].element == cartes[1].element) {
                if (cartes[0].valeur > cartes[1].valeur) vainqueur = 0;
                else if (cartes[1].valeur > cartes[0].valeur) vainqueur = 1;
            } else if ((cartes[0].element == FEU   && cartes[1].element == GLACE) ||
                       (cartes[0].element == GLACE && cartes[1].element == EAU)   ||
                       (cartes[0].element == EAU   && cartes[1].element == FEU)) {
                vainqueur = 0;
                sprintf(gagnant,"%s",p->liste_joueur[0].nom);
            } else {
                vainqueur = 1;
                sprintf(gagnant,"%s",p->liste_joueur[1].nom);
            }

            if (vainqueur != -1) {
                printf("[PARTIE %d] 🏆 Manche gagnée par : %s\n", idPartie, p->liste_joueur[vainqueur].nom);
            } else {
                sprintf(gagnant,"EGALITE");
                printf("[PARTIE %d] ⚔️ Égalité !\n", idPartie);
            }

            // === PHASE 4 : ENREGISTREMENT DE LA CARTE ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 4 : Enregistrement carte gagnante ===\033[0m\n", idPartie);

            if (vainqueur != -1) {
                historique_joueur_t* h = &p->historique[vainqueur];
                if (h->nb_cartes < MAX_CARTES_HISTO) {
                    h->cartes[h->nb_cartes++] = cartes[vainqueur];
                    printf("[PARTIE %d] 🗃️ Carte ajoutée à l’historique de %s\n", idPartie, p->liste_joueur[vainqueur].nom);
                }
            }

            // === PHASE 5 : ENVOI DES RÉSULTATS AUX JOUEURS ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 5 : Envoi des résultats aux joueurs ===\033[0m\n", idPartie);

            for (i = 0; i < 2; i++) {
                int adv = (i + 1) % 2;
                sprintf(buffer, "%d:%s,%d,%d,%d,%s,%d,%d,%d,%s", CMD_GAME_CARD,
                        cartes[i].nom, cartes[i].element, cartes[i].valeur, cartes[i].couleur,
                        cartes[adv].nom, cartes[adv].element, cartes[adv].valeur, cartes[adv].couleur,
                        gagnant);

                printf("[PARTIE %d] 📤 [SEND] -> %s : Résultat de la manche | buffer : %s\n", idPartie, p->liste_joueur[i].nom,buffer);
                send(p->liste_joueur[i].socket.fd, buffer, strlen(buffer), 0);
            }
            // Attendre RSP_GAME_OK de chaque joueur
            for (i = 0; i < 2; i++) {
                memset(buffer, 0, sizeof(buffer));
                recv(p->liste_joueur[i].socket.fd, buffer, sizeof(buffer) - 1, 0);
                deserialiser_message(buffer, &code, data, &nb_data);
                if (code != RSP_GAME_OK) {
                    printf("[PARTIE %d] Erreur: joueur %d n'a pas répondu OK\n", idPartie, i);
                    return NULL;
                }
                printf("[PARTIE %d] %s est prêt.\n", idPartie, p->liste_joueur[i].nom);
            }

            // === PHASE 6 : VÉRIFICATION DE LA FIN ===
            printf("\n\033[1;34m[PARTIE %d] === PHASE 6 : Vérification condition de victoire ===\033[0m\n", idPartie);

            for (i = 0; i < 2; i++) {
                if (verifier_victoire(&p->historique[i])) {
                    int perdant = (i + 1) % 2;

                    sprintf(buffer, "%d:%s", CMD_GAME_WINNER, p->liste_joueur[i].nom);
                    printf("[PARTIE %d] 📤 [SEND] -> %s : CMD_GAME_WINNER\n", idPartie, p->liste_joueur[i].nom);
                    send(p->liste_joueur[i].socket.fd, buffer, strlen(buffer), 0);

                    sprintf(buffer, "%d:%s", CMD_GAME_LOSE, p->liste_joueur[perdant].nom);
                    printf("[PARTIE %d] 📤 [SEND] -> %s : CMD_GAME_LOSE\n", idPartie, p->liste_joueur[perdant].nom);
                    send(p->liste_joueur[perdant].socket.fd, buffer, strlen(buffer), 0);
                    sleep(5);

                    printf("\n\033[1;32m[PARTIE %d] 🎉 %s remporte la partie !\033[0m\n", idPartie, p->liste_joueur[i].nom);

                    fin = 1;
                    p->status = TERMINE;
                    p->liste_joueur[0].mode = 0;
                    p->liste_joueur[1].mode = 0;
                    break;
                }
            }
        }
    printf("[PARTIE %d] Thread terminé.\n", idPartie);
    free(gagnant);
    return NULL;

}


void afficher_joueurs_enregistres(SharedInfos *info) {
    int i;
    pthread_mutex_lock(&info->mutex);
    printf("\nJoueurs online : %d/%d",info->nb_joueurs_online,info->nb_joueurs_total);
    // --- Affichage des joueurs ---
    printf("\n========== Liste des joueurs (%d) ==========\n", info->nb_joueurs_total);
    printf("| %-3s | %-15s | %-10s | %-7s | %-4s | %-5s | %-3s |\n",
           "ID", "Nom", "UID", "Statut", "Rang", "EXP", "FD");
    printf("---------------------------------------------------------------------\n");

    for (i = 0; i < info->nb_joueurs_total; i++) {
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

    for (i = 0; i < NB_PARTIES_MAX; i++) {
        if (info->parties[i].status != 0) {  // Partie active
            printf("| %-3d | %-7s | %-10d |\n",
                   info->parties[i].idPartie,
                   info->parties[i].status == ATTENTE ? "EN ATT." :
                   info->parties[i].status == ENCOURS ? "EN COURS" :
                   info->parties[i].status == LIBRE ? "LIBRE" : "TERMINE",
                   info->parties[i].nbJoueurs);
        }
    }

    printf("=============================================\n\n");

    pthread_mutex_unlock(&info->mutex);
}