//
// Created by alexis on 05/06/25.
//
//
// Created by alexis on 03/12/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include "LG.h"
#include <pthread.h>
#include <signal.h>


#define SHM_NAME "/shared_key_memory_BL"
#define SEM_NAME "/shared_key_semaphore_BL"

//Mémoire partagé pour le mode chat
#define SHM_CHAT "/shared_memory_chat"
#define SEM_CHAT "/shared_semaphore_chat"







void Menu();
void _choisirPseudo(char* pseudo);
void* envoi_messages(void* arg);
void* ecoute_messages(void* arg);
void handle_tstp(int signum);



///////////////////////////////////////Création des threads pour les discussions (chat)///////////////////////////////
pthread_t thread_ecoute, thread_envoi; //Création des threads d'envoi et d'écoute pour la phase discussion libre

joueur joueur_globale;
int msgid_global;
sem_t sem_discussion;  // Sémaphore qui bloque les threads

int main() {
    struct sigaction sa;
    // Initialiser la structure sigaction à zéro
    sa.sa_handler = handle_tstp;  // Spécifie la fonction à appeler pour SIGTSTP
    sigemptyset(&sa.sa_mask);     // Aucun signal ne sera bloqué pendant le traitement du signal
    sa.sa_flags = 0;              // Pas de flags spéciaux

    // Attacher le gestionnaire au signal SIGTSTP
    sigaction(SIGTSTP, &sa, NULL);

    key_t key;
    joueur_globale.pid = getpid();
    int shm_fd;
    sem_t *sem;
    int *shared_key; // Pour récupérer la clé depuis la mémoire partagée

    pthread_create(&thread_ecoute, NULL, ecoute_messages, NULL);
    pthread_create(&thread_envoi, NULL, envoi_messages, NULL);


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////// Connexion au segment de mémoire partagée //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) { perror("Erreur lors de l'ouverture de la mémoire partagée"); exit(EXIT_FAILURE); }

    // Mapper la mémoire partagée
    shared_key = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_key == MAP_FAILED) { perror("Erreur lors du mapping de la mémoire partagée");
        close(shm_fd); // Détache la mémoire partagée en cas d'erreur
        exit(EXIT_FAILURE);
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////// Ouverture du sémaphore //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    sem = sem_open(SEM_NAME, 0); // 0 pour accéder au sémaphore existant
    if (sem == SEM_FAILED) { perror("Erreur lors de l'ouverture du sémaphore");
        munmap(shared_key, sizeof(int));
        close(shm_fd); // Détache la mémoire partagée en cas d'erreur
        exit(EXIT_FAILURE);
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////// Lecture de la memoire partagée //////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (sem_wait(sem) == -1) { perror("Erreur lors de sem_wait");
        munmap(shared_key, sizeof(int));
        close(shm_fd); // Détache la mémoire partagée en cas d'erreur
        sem_close(sem); // Ferme le sémaphore en cas d'erreur
        exit(EXIT_FAILURE);
    }

    // Lire la clé dans la mémoire partagée
    key = *shared_key;
    printf("Clé récupérée depuis la mémoire partagée : %d\n", key);

    // Libérer l'accès avec sem_post
    if (sem_post(sem) == -1) { perror("Erreur lors de sem_post");
        munmap(shared_key, sizeof(int));
        close(shm_fd);
        sem_close(sem);
        exit(EXIT_FAILURE);
    }

    if (key == -1) { perror("Erreur lors de la génération de la clé"); exit(EXIT_FAILURE); }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////// Connexion à la boite aux lettres /////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    msgid_global = msgget(key, 0666);
    if (msgid_global == -1) { perror("Erreur lors de la connexion à la boîte aux lettres"); exit(EXIT_FAILURE); }

    printf("Boîte aux lettres connectée avec succès ! (msgid: %d)\n", msgid_global);




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sleep(2); // Tempo pour visualisation des messages

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Variables de communication avec le serveur
    message SND = {0}; // message d'envoi SEND
    message RCV= {0}; // message de récéption

    char msg[256]; // Buffer message

    // Variables de gestion
    int choix; // Choix du joueur
    int PartieRejointe = 0 ; // Flag à 1 si le joueur a rejoins une partie (par défaut 0)




    /////// Connexion au serveur ///////
    int ConnexionOk = 0;
    joueur_globale.IDpartie = -1;

    while (!ConnexionOk) {
        system("clear");

        memset(&RCV, 0, sizeof(RCV)); // Initialise toute la structure RCV à 0
        memset(&SND, 0, sizeof(SND)); // Initialise toute la structure SND à 0

        // Choix du pseudo
        _choisirPseudo(joueur_globale.pseudo);
        printf("Veuillez patienter pendant votre connexion au serveur...\n");

        // Envoi du message de connexion au serveur
        SND.Type = 99;
        SND.Corp.requete = 1;
        SND.Corp.joueur.IDpartie = joueur_globale.IDpartie;
        SND.Corp.joueur.pid = joueur_globale.pid;
        strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);
        if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
            perror("Erreur lors de l'envoi du message");
            break;
        }

        // Attendre la réponse du serveur
        if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
            perror("Erreur lors de la réception du message");
            break;
        }

        // Afficher la réponse du serveur
        printf("Réponse du serveur : %d : %s\n\n",RCV.Corp.requete, RCV.Corp.msg);
        if(RCV.Corp.requete == 6) {
            ConnexionOk = 1;
        }
        else {
            printf("\n[Appuyez pour continuer]");
            getchar();
        }
    }




    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////// Gestion du menu joueur //////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while (1){

        while (PartieRejointe == 0) {
            printf("Debug: Entrée dans la boucle de Menu\n");
            Menu();
            scanf("%d", &choix);
            getchar(); // Pour vider le buffer après la lecture du choix
            //printf("%d",joueur_globale.IDpartie);
            memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
            memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
            switch (choix) {

                case 1: // Afficher les règles
                    regles();
                break;

                case 2: // Informations sur les rôles
                    Menu_Roles();
                break;

                case 3: // Jouer (rejoindre partie)
                    //Envoie de la requête liste parties
                    SND.Type = 99;
                    SND.Corp.requete = 10;
                    SND.Corp.joueur.pid = joueur_globale.pid;
                    strcpy(SND.Corp.msg,"0");
                    strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);
                    if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("Erreur lors de l'envoi du message");
                        break;
                    }

                    // Attendre la réponse du serveur
                    if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
                        perror("Erreur lors de la réception du message");
                        break;
                    }
                    printf(": %s\n\n", RCV.Corp.msg);
                    if(RCV.Corp.requete != 6) {
                        break;
                    }
                    memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
                    memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0

                    //ajouter demande et affichage de la liste des parties
                    printf("\nQuelles parties souhaitez vous rejoindre ? [0 1 2] : ");
                    scanf("%d", &choix);
                    //Envoie de la requête pour jouer
                    SND.Type = 99;
                    SND.Corp.requete = 3;
                    SND.Corp.joueur.pid = joueur_globale.pid;
                    sprintf(SND.Corp.msg,"%d",choix);
                    strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);
                    if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("Erreur lors de l'envoi du message");
                        break;
                    }

                    // Attendre la réponse du serveur
                    if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
                        perror("Erreur lors de la réception du message");
                        break;
                    }
                    printf("Réponse du serveur : %d : %s\n\n",RCV.Corp.requete, RCV.Corp.msg);
                    // Vérifier que la réponse est positive
                    if(RCV.Corp.requete == 6) {
                        PartieRejointe=1;
                        joueur_globale.IDpartie = RCV.Corp.joueur.IDpartie;
                    }
                    printf("\n\nbit partie rejointe : %d\n\n",PartieRejointe);
                    sleep(2);
                break;

                case 4: // Jouer (créer partie)
                    SND.Type = 99;
                    SND.Corp.requete = 2;
                    SND.Corp.joueur.pid = joueur_globale.pid;
                    if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("Erreur lors de l'envoi du message");
                    break;
                    }

                    // Attendre la réponse du serveur
                    if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
                        perror("Erreur lors de la réception du message");
                        break;
                    }
                    printf("Réponse du serveur : %d : %s\n\n",RCV.Corp.requete, RCV.Corp.msg);
                    // Vérifier que la réponse est positive
                    if(RCV.Corp.requete == 6) {
                        PartieRejointe=1;
                        joueur_globale.IDpartie = RCV.Corp.joueur.IDpartie;
                    }
                    printf("\n\nbit partie rejointe : %d\n\n",PartieRejointe);
                    sleep(2);
                break;

                case 5: // Quitter
                    system("clear");
                    printf("Vous allez être déconnecté...\n");

                    do {
                        memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
                        memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0

                        // Envoi du message de déconnexion au serveur
                        SND.Type = 99;
                        SND.Corp.requete = 4;
                        SND.Corp.joueur.pid = joueur_globale.pid;
                        strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);
                        strcpy(SND.Corp.msg, msg);
                        if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                            perror("Erreur lors de l'envoi du message");
                            break;
                        }

                        // Attendre la réponse du serveur
                        if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
                            perror("Erreur lors de la réception du message");
                            break;
                        }

                        // Afficher la réponse du serveur
                        printf("Réponse du serveur : %d : %s\n\n",RCV.Corp.requete, RCV.Corp.msg);
                        if(RCV.Corp.requete == 6) {
                            ConnexionOk = 0;
                        }
                    } while (ConnexionOk);

                    // Affichage du message de fin
                    AffichageLG();
                    printf("\n");
                    printf("**************************************************\n");
                    printf("*                                                *\n");
                    printf("*    MERCI D'AVOIR JOUE A NOTRE LOUP-GAROU !     *\n");
                    printf("*                   A BIENTÔT !                  *\n");
                    printf("*                                                *\n");
                    printf("**************************************************\n");
                    printf("\n");

                    // Fin
                    exit(0);
                break;

                default: // Option invalide
                    printf("Choix invalide. Veuillez entrer un nombre entre 1 et 5.\n");
                break;
            }
        }

        system("clear");

        while (PartieRejointe) {
            memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
            memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0

            // Attendre la réponse du serveur
            printf("\n\n");
            if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) == -1) {
                perror("Erreur lors de la réception du message");
                break;
            }

           //Traiter les requêtes reçu du serveur
            switch (RCV.Corp.requete) {

            case 11:
                 printf("%s",RCV.Corp.msg);
                 printf("👥 Joueurs connectés : %d / %d\n", RCV.Corp.partie.nbJoueurs, RCV.Corp.partie.nbJoueursMax);

            break;

            case 12:  // LANCEMENT DE LA PARTIE
                system("clear");
                printf("La partie Commence !!!\n");
                printf("%s",RCV.Corp.msg);
                printf("\n");
                snprintf(joueur_globale.role, sizeof(joueur_globale.role), "%s", SND.Corp.joueur.role);


                //printf("%d",joueur_globale.IDpartie);
                SND.Type = joueur_globale.IDpartie;
                SND.Corp.requete = 16;
                SND.Corp.joueur.IDpartie = joueur_globale.IDpartie;
                SND.Corp.joueur.pid = joueur_globale.pid;
                strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);
                if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("Erreur lors de l'envoi du message");
                    break;
                }
            break;

            case 13: // UPDATE STATUS PARTIE (ex tour voyante etc ...)
                printf("%s",RCV.Corp.msg);
            break;

            case 21:  // DEMANDE CHOIX DU JOUEUR (VOTE)
                //printf("LANCEMENT DU VOTRE \n A votre avis, qui est le loup ? \n");
                printf("%s",RCV.Corp.msg);
                int choixJoueur;
                scanf("%d", &choixJoueur);
                getchar();  // Nettoyer le buffer

                // Envoyer la réponse au serveur
                memset(&SND, 0, sizeof(SND));
                SND.Type = joueur_globale.IDpartie;
                SND.Corp.requete = 26;  // Réponse au choix du joueur
                SND.Corp.joueur.pid = joueur_globale.pid;
                snprintf(joueur_globale.role, sizeof(joueur_globale.role), "%s", SND.Corp.joueur.role);
                SND.Corp.joueur.choix = choixJoueur;
                if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("Erreur lors de l'envoi de la réponse");
                }
            break;

            case 22:  // LANCEMENT PHASE DE DIALOGUE LIBRE
                printf("💬 Début de la phase de discussion libre !\n");
                sem_post(&sem_discussion);
                sem_post(&sem_discussion);
            break;

            case 23:  // FIN PHASE DE DIALOGUE LIBRE
                //printf("🛑 Fin de la phase de discussion libre.\n");
                printf("%s",RCV.Corp.msg);
                sem_init(&sem_discussion, 0, 0);  // Bloque à nouveau les threads
            break;

            case 28:  // FIN DE PARTIE
                printf("%s",RCV.Corp.msg);
                printf("\n\nFin de partie !\n\n[Appuyez pour revenir au menu]");
                getchar();
                PartieRejointe = 0;
                joueur_globale.role[0] = '\0';
                joueur_globale.IDpartie = -1;
            break;

            case 31:  // MORT
                printf("%s",RCV.Corp.msg);
                while (getchar() != '\n');
                printf("\n\nVous êtes mort.\n\n[Appuyez pour revenir au menu]");
                getchar();
                PartieRejointe = 0;
                joueur_globale.role[0] = '\0';
                joueur_globale.IDpartie = -1;
            break;

            default:
                printf("⚠️ Requête inconnue reçue : %d\n", RCV.Corp.requete);
            break;
           }
        }
    }
}

void Menu() { // Afficher le menu principal
    system("clear");  // Effacer l'écran
    printf("\n========================================\n");
    printf("   Bienvenue dans le jeu du Loup Garou  \n");
    printf("========================================\n");
    printf("1. Voir les règles du jeu\n");
    printf("2. Informations sur les rôles\n");
    printf("3. Rejoindre partie\n");
    printf("4. Créer partie\n");
    printf("5. Quitter\n");
    printf("Veuillez entrer votre choix (1-4) : ");
}

void _choisirPseudo(char* pseudo) {
    system("clear");  // Effacer l'écran
    /////// Message de bienvnu ///////
    // Affichage du message de bienvenue
    printf("**************************************************\n");
    printf("*                                                *\n");
    printf("*    BIENVENUE DANS NOTRE PARTIE DE LOUP-GAROU   *\n");
    printf("*       PRÉPAREZ-VOUS À VIVRE UNE AVENTURE       *\n");
    printf("*                                                *\n");
    printf("**************************************************\n");
    printf("\n");
    AffichageLG();
    // Changer le pseudo
    printf("Entrez votre nouveau pseudo : ");
    fgets(pseudo, 50, stdin);
    pseudo[strcspn(pseudo, "\n")] = 0;  // Enlever le '\n' de la fin du pseudo
}

//Fonction thread écoute
void* ecoute_messages(void* arg) {
    message RCV;

    while (1) {
        sem_wait(&sem_discussion);  // Attente que la discussion soit activée
        if (msgrcv(msgid_global, &RCV, sizeof(RCV.Corp), joueur_globale.pid, 0) != -1) {
            printf("[💬] %s : %s\n", RCV.Corp.joueur.pseudo, RCV.Corp.msg);
        } else {
            perror("Erreur lors de la réception du message");
        }
    }
    return NULL;
}

//Fonction thread d'envoi
void* envoi_messages(void* arg) {
    message SND;

    while (1) {
        sem_wait(&sem_discussion);  // Attente que la discussion soit activée

        printf("Entrez Votre message : ");
        fgets(SND.Corp.msg, sizeof(SND.Corp.msg), stdin);
        SND.Corp.msg[strcspn(SND.Corp.msg, "\n")] = 0;  // Supprime le retour à la ligne

        // Préparer l'envoi
        SND.Type = joueur_globale.pid;  // ID de la partie pour l'envoi
        SND.Corp.requete = 27;  // Requête d'envoi de message
        SND.Corp.joueur.pid = joueur_globale.pid;
        strcpy(SND.Corp.joueur.pseudo, joueur_globale.pseudo);

        if (msgsnd(msgid_global, &SND, sizeof(SND.Corp), 0) == -1) {
            perror("Erreur lors de l'envoi du message");
        }
    }
    return NULL;
}

void handle_tstp(int signum) {
    printf("Le signal SIGTSTP (Ctrl+Z) a été intercepté et ignoré !\n");
}