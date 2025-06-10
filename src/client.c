#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/7seg.h"
#include "../include/protocole.h"
#include "../include/socket_utils.h"
#include "../include/rfid.h"
#include "../include/data.h"
#include "../include/IHM.h"
#include "../include/matrice.h"
#include "../include/lcd_custom.h"


#define SERVER_IP "172.20.10.2"
#define SERVER_PORT 50000
#define LCD_I2C_ADDR 0x21

int Menu();

joueur client;
historique_affichage_t moi = {0}, adv = {0};

int main() {
    matrice_init();
    matrice_clear();
    //printf("[DEBUG] Initialisation du lecteur RFID...\n");

    int lcd_fd = lcd_setup(LCD_I2C_ADDR);
    int seg_fd = init_display();

    if (initRFID() != 0) {
        fprintf(stderr, "Erreur initialisation RFID\n");
        return 1;
    }

    //printf("[DEBUG] Connexion au serveur...\n");
    socket_t sockClient = connecterClt2Srv(SERVER_IP, SERVER_PORT);
    printf("Connecté au serveur %s:%d\n", SERVER_IP, SERVER_PORT);

    uint8_t uid[4];
    int f_rfid = 0;

    // Initialisation communication
    char buffer[MAX_BUFFER] = {0};
    int code = 0;
    int nb_data = 0;
    char *data[MAX_PARTS] = {0};

    int ConnexionOk = 0;


    while (!ConnexionOk) {
        nb_data = 0;
        code = 0;
        //printf("[DEBUG] Attente détection RFID...\n");
        while (!f_rfid) {
            printf("En attente de carte RFID...\n");
            while (detectCard() != MI_OK);
            if (readUID(uid) == MI_OK) {
                f_rfid = 1;
                //printf("[DEBUG] UID détecté : %02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3]);
            }
            sleep(2);
        }

        sprintf(client.UID,"%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
        //printf("[DEBUG] UID enregistré dans structure client : %s\n", client.UID);

        // Envoi UID au serveur
        //printf("[DEBUG] UID = '%s' | NOM = '%s'\n", client.UID, client.nom);
        sprintf(buffer, "%d:%s,", CMD_CONNECT, client.UID);
        //printf("[DEBUG] Envoi CMD_CONNECT : %s\n", buffer);
        send(sockClient.fd, buffer, strlen(buffer), 0);
        memset(buffer, 0, sizeof(buffer));

        //printf("[DEBUG] Attente de réponse serveur...\n");
        recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
        //printf("[DEBUG] Réponse brute : %s\n", buffer);

        deserialiser_message(buffer, &code, data, &nb_data);
        //printf("[DEBUG] Code reçu : %d | nb_data = %d\n", code, nb_data);

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

                //printf("[DEBUG] UID = '%s' | NOM = '%s'\n", client.UID, client.nom);
                sprintf(buffer, "%d:%s,%s", CMD_REGISTER, client.UID, client.nom);
                //printf("[DEBUG] Envoi CMD_REGISTER : %s\n", buffer);
                send(sockClient.fd, buffer, strlen(buffer), 0);

                // 💡 Recevoir la réponse à CMD_REGISTER ici directement
                memset(buffer, 0, sizeof(buffer));
                recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
                deserialiser_message(buffer, &code, data, &nb_data);
                if (code == RSP_CONNECTED) {
                    printf("Inscription réussie. Bienvenue %s !\n", data[0]);
                    strcpy(client.nom, data[0]);
                    ConnexionOk = 1;
                } else {
                    printf("[ERREUR] Échec de l'inscription.\n");
                    f_rfid = 0;
                }
                break;


            default:
                printf("[WARN] Code inconnu reçu : %d\n", code);
                f_rfid =0;
                break;
        }

        //printf("[DEBUG] Contenu buffer post-switch : %s\n", buffer);
    }

    while (ConnexionOk) {
        int PartieRejointe = 0;
        int choix;

        while (!PartieRejointe && ConnexionOk) {

            choix = Menu(); // Appel de la fonction Menu et récupération du choix

            switch (choix) {

                case 1: // affichage des règles
                    regles();
                    printf("\n(Appuyez sur Entrée pour revenir au menu…) ");
                    getchar();
                break;

                case 2: // rejoindre une partie
                    sprintf(buffer, "%d:%s,", CMD_LIST, client.UID);
                    send(sockClient.fd, buffer, strlen(buffer), 0);

                    memset(buffer, 0, sizeof(buffer));
                    int r = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);

                    if (r <= 0) {
                        printf("[ERREUR] Impossible de recevoir la liste des parties\n");
                        break;
                    }
                    buffer[r] = '\0';

                    //printf("[DEBUG] Réponse brute CMD_LIST : %s\n", buffer);

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

                    printf("Entrez l'ID de la partie à rejoindre (10 pour annuler) : ");

                    int choixPartie;
                    scanf("%d", &choixPartie);

                    if (choixPartie == 10) {
                        printf("Retour au menu.\n");
                        break;
                    }

                    sprintf(buffer, "%d:%s,%d", CMD_JOIN, client.UID, choixPartie);
                    //printf("[DEBUG] Contenu buffer avant envoi : %s\n", buffer);
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
                    buffer[r] = '\0';
                    //printf("[DEBUG] Reçu (%d octets) : %s\n", r, buffer);
                    code = 0;
                    nb_data = 0;
                    int i;
                    for (i = 0; i < MAX_PARTS; i++) data[i] = NULL;
                    // Désérialiser la réponse
                    deserialiser_message(buffer, &code, data, &nb_data);
                    //printf("[DEBUG] Reçu (%d octets) : %s\n", r, buffer);

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

        system("clear");
        lcd_clear(lcd_fd);

        //################################ Déroulement du jeu ###############################################
        while (PartieRejointe && ConnexionOk){
            lcd_clear(lcd_fd);
            clear_display(seg_fd);
            matrice_clear();

            memset(buffer, 0, sizeof(buffer));
            memset(data,0,sizeof(data));
            //printf("[DEBUG] attente message\n");
            int msg_recu = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
            //printf("[DEBUG] message reçu : %s\n",buffer);
            int i;
            if (msg_recu <= 0) break;
            buffer[msg_recu] = '\0';
            deserialiser_message(buffer, &code, data, &nb_data);

            switch (code) {
                case CMD_GAME_START:
                    //printf("[DEBUG] DEBUT CMD GAME START\n");
                    lcd_clear(lcd_fd);
                    lcd_display_message(lcd_fd,"La partie demarre !");
                    for (i = 0; i < 2; i++) {
                        const uint8_t signal[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                        matrice_display(signal);
                        sleep(1);
                        matrice_clear();
                    }
                    sprintf(buffer, "%d:%s", RSP_GAME_OK, client.UID);
                    send(sockClient.fd, buffer, strlen(buffer), 0);
                    //printf("[DEBUG] FIN CMD GAME START\n");
                    break;

                case CMD_GAME_SCAN:
                    //printf("[DEBUG] DEBUT CMD_GAME_SCAN\n");
                    //printf("\t[DEBUG] CLEAR LCD\n");
                    lcd_clear(lcd_fd);
                    //printf("\t[DEBUG] DISPLAY LCD\n");
                    lcd_display_message(lcd_fd,"scanner votre\ncarte...");
                    //printf("\t[DEBUG] ATTENTE CARTE\n");
                    while (detectCard() != MI_OK);
                    //printf("\t[DEBUG] READ UID\n");
                    if (readUID(uid) == MI_OK) {
                        char uidStr[9];
                        sprintf(uidStr, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
                        sprintf(buffer, "%d:%s,%s", RSP_GAME_CARD, client.UID, uidStr);
                        send(sockClient.fd, buffer, strlen(buffer), 0);
                        //printf("\t[DEBUG] CLEAR LCD\n");
                        lcd_clear(lcd_fd);
                        //printf("\t[DEBUG] DISPLAY LCD ENVOI CARTE\n");
                        lcd_display_message(lcd_fd,"Envoie de\nvotre carte...");
                    }
                    //printf("[DEBUG] FIN CMD_GAME_SCAN\n");
                    break;

                case CMD_GAME_WINNER:
                    lcd_clear(lcd_fd);
                    lcd_display_message(lcd_fd,"Vous avez gagne\nla partie !");
                    sleep(5);
                    lcd_clear(lcd_fd);
                    PartieRejointe = 0;
                    break;

                case CMD_GAME_LOSE:
                    lcd_clear(lcd_fd);
                    lcd_display_message(lcd_fd,"Vous avez perdu\nla partie !");
                    sleep(5);
                    lcd_clear(lcd_fd);
                    PartieRejointe = 0;
                    break;

                case CMD_GAME_STOP:
                    printf("\n[INFO] Partie interrompue par le serveur.\n");
                    PartieRejointe = 0;
                    break;

                case CMD_GAME_CARD: // réponse serveur avec détails
                    //printf("[DEBUG] DEBUT CMD_GAME_CARD\n");
                    if (nb_data >= 9) {
                        // Lecture des données du message
                        char *nom1 = data[0];
                        int elem1 = atoi(data[1]);
                        int val1  = atoi(data[2]);
                        int coul1 = atoi(data[3]);

                        char *nom2 = data[4];
                        int elem2 = atoi(data[5]);
                        int val2  = atoi(data[6]);
                        int coul2 = atoi(data[7]);

                        char gagnant[20];
                        sprintf(gagnant,data[8]);

                        strncpy(moi.nom, nom1, sizeof(moi.nom) - 1);
                        moi.nom[sizeof(moi.nom) - 1] = '\0';

                        strncpy(adv.nom, nom2, sizeof(adv.nom) - 1);
                        adv.nom[sizeof(adv.nom) - 1] = '\0';

                        //printf("\n🃏 %s [%d|%d|%d] vs %s [%d|%d|%d]\n",
                        //       moi.nom, elem1, val1, coul1,
                        //       adv.nom, elem2, val2, coul2);
                        //printf("ganant : %s | client : %s\n\n",gagnant,client.nom);

                        val1 = val1 % 100;
                        val2 = val2 % 100;
                        int val_affiche = val1 * 100 + val2;
                        //printf("\t[DEBUG] DISPLAY 7 SEG\n");
                        display_number(seg_fd, val_affiche);
                        //printf("\t[DEBUG] DISPLAY MATRICE\n");
                        afficher_face_a_face(elem1,elem2);

                        sleep(2);

                        //printf("\t[DEBUG] CALCUL STATUS [WIN | LOSE | EGALITE]\n");
                        if (strcmp(gagnant, client.nom) == 0) {
                            lcd_clear(lcd_fd);
                            lcd_display_message(lcd_fd,"Vous avez gagne\nce tour !");
                            ajouter_carte_gagnee(&moi, elem1, coul1);
                            printf("Gagne");

                        } else if (strcmp(gagnant,"EGALITE") == 0) {
                            lcd_clear(lcd_fd);
                            lcd_display_message(lcd_fd,"Egalite !");
                            printf("Egalite");

                        } else {
                            lcd_clear(lcd_fd);
                            lcd_display_message(lcd_fd,"Vous avez perdu\nce tour !");
                            printf("Perdu");
                            ajouter_carte_gagnee(&adv, elem2, coul2);
                        }
                    }
                    printf("AFFICHAGE HISTORIQUE \n");
                    system("clear");
                    afficher_historique(moi, adv);
                    sprintf(buffer, "%d:%s", RSP_GAME_OK, client.UID);
                    send(sockClient.fd, buffer, strlen(buffer), 0);
                    //printf("[DEBUG] FIN CMD_GAME_CARD\n");
                    break;


                default:
                    printf("[WARN] Code inconnu : %d\n", code);
                    break;
            }
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
