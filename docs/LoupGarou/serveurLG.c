//
// Created by alexis on 05/06/25.
//
//
// Created by alexis on 03/12/24.
//

#include <ctype.h>
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

#include <pthread.h>
#include <signal.h>

#include "LG.h"


#define SHM_NAME_BL "/shared_key_memory_BL"
#define SEM_NAME_BL "/shared_key_semaphore_BL"

#define SHM_NAME_PARTIES "/shared_parties_memory"

#define MAX_PARTIES 3
#define MAX_JOUEURS 6

// Codes couleurs pour debug
// GREEN "\033[0;32m"
// RED "\033[0;31m"
// YELLOW "\033[0;33m"
// BLUE "\033[0;34m"
// RESET "\033[0m"
// PURPLE "\033[0;35m"

#define PAUSE(msg) printf("\033[0;33m%s [Appuyez sur entrée pour continuer]\033[0m", msg); getchar();

void AffichageMessage(message msg,int mode);
void attribuerRoleAleatoire(QteRoles *Role, char *RoleJoueur);
void vider_boite_aux_lettres(int msgid);
int estPseudoValide(const char *pseudo);
void* thread_function(void* arg);
void handle_tstp(int signum);
void findMax(int arr[], int size, int *maxValue, int *maxIndex);


typedef struct {
    char name[20]; // nom du role
    int dispo; // quantité disponible
} Role;

typedef struct {
    joueur joueurs[50];
    partie parties[MAX_PARTIES];
    int nb_joueurs;
    pthread_mutex_t mutex;
} SharedInfos;

typedef struct {
    int msgid;  // Boîte aux lettres
    SharedInfos *SharedInfos;  // Mémoire partagée
    int num_partie;
} ThreadData;

int main() {
    struct sigaction sa;

    // Initialiser la structure sigaction à zéro
    sa.sa_handler = handle_tstp;  // Spécifie la fonction à appeler pour SIGTSTP
    sigemptyset(&sa.sa_mask);     // Aucun signal ne sera bloqué pendant le traitement du signal
    sa.sa_flags = 0;              // Pas de flags spéciaux

    // Attacher le gestionnaire au signal SIGTSTP
    sigaction(SIGTSTP, &sa, NULL);


    /////// Céation d'une clé ///////
    key_t key;
    char path[300];
    int project_id = 1;
    strcpy(path, "serveur"); // Chemin pour la clé
    key = ftok(path, project_id);
    if (key == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la génération de la clé\033[0m");
        exit(EXIT_FAILURE);
    }

    /////// Création de la boîte aux lettres d'acceptation des clients ///////
    int msgid;
    msgid = msgget(key,0666);
    if (msgid != -1) {
        vider_boite_aux_lettres(msgid);
    }
    else{
        printf("Aucune boîte aux lettres existante, création...\n");
    }

    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la création de la boîte aux lettres\033[0m");
        exit(EXIT_FAILURE);
    }
    printf("LOG-serveur : \033[0;32mBoîte aux lettres créée avec succès ! (msgid: %d)\n\033[0m", msgid);

    /////// Création du segement de mêmoire partagée pour la clé de boite aux lettre ///////
    int shmidBL;
    int *shared_key; // Pour récupérer la clé depuis la mémoire partagée
    shmidBL = shm_open(SHM_NAME_BL, O_CREAT | O_RDWR, 0666);
    if (shmidBL == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la création du segment de mémoire partagée : shmkey\033[0m");
        exit(EXIT_FAILURE);
    }

    // Spécifier la taille de la mémoire partagée
    if (ftruncate(shmidBL, sizeof(int)) == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la configuration de la taille du segment de mémoire : shmkey\033[0m");
        exit(EXIT_FAILURE);
    }
    printf("LOG-serveur : \033[0;32mSegment de mémoire partagé créé avec succès ! (shmid: %d)\n\033[0m", shmidBL);

    // Mappage de la mémoire partagée
    shared_key = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shmidBL, 0);
    if (shared_key == MAP_FAILED) {
        perror("LOG-serveur : \033[0;31mErreur lors du mapping de la mémoire partagée : shmkey\033[0m");
        close(shmidBL); // Détache la mémoire partagée en cas d'erreur
        exit(EXIT_FAILURE);
    }

    /////// Création d'un sémaphore pour gérer l'accès à la mêmoire partagée ///////
    sem_t *sem; // Pointeur vers le sémaphore
    sem_unlink(SEM_NAME_BL); // Supprimer tout sémaphore existant
    sem = sem_open(SEM_NAME_BL, O_CREAT | O_EXCL, 0666, 1); // Créer un sémaphore
    if (sem == SEM_FAILED) {
        perror("LOG-serveur : \033[0;31mErreur lors de la création du sémaphore\033[0m");
        exit(EXIT_FAILURE);
    }
    printf("LOG-serveur : \033[0;32mSémaphore initialisé : %d\n\033[0m", shmidBL);

    /////// Ecriture de la clé de la boite aux lettres dans la mêmoire partagée ///////
    if (sem_wait(sem) == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de sem_wait\033[0m");
        exit(EXIT_FAILURE);
    }  // reserve le sémaphore
    printf("LOG-serveur : \033[0;32mLOG-serveur : prise du sémaphore : sem_wait()\n\033[0m");
    *shared_key = key;  // Stockage de la clé de la boîte aux lettres
    printf("LOG-serveur : \033[0;32mLOG-serveur : Clé de la boîte aux lettres stockée dans la mémoire partagée (key: %d)\n\033[0m", *shared_key );
    if (sem_post(sem) == -1) {
        perror("LOG-serveur : \033[0;31mLOG-serveur : Erreur lors de sem_post\033[0m");
        exit(EXIT_FAILURE);
    } // Libère le sémaphore
    printf("LOG-serveur : \033[0;32mlibération du sémaphore : sem_post()\n\033[0m");




    /////// Initialisation de l'espace mêmoire partagé pour la gestion des parties ///////
    int shminfos = shm_open(SHM_NAME_PARTIES, O_CREAT | O_RDWR, 0666);
    if (shminfos == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la création du segment de mémoire partagée\033[0m");
        exit(1);
    }
    size_t taille_mem = sizeof(SharedInfos); // calcul taille de la zone mémoire
    if (ftruncate(shminfos, taille_mem) == -1) {
        perror("LOG-serveur : \033[0;31mErreur lors de la configuration de la taille du segment de mémoire : shmparties\033[0m");
        exit(1);
    } // Spécifier la taille de la mémoire partagée
    // Mappage de la mémoire partagée
    SharedInfos *sharedInfos = (SharedInfos *)mmap(NULL, taille_mem, PROT_READ | PROT_WRITE, MAP_SHARED, shminfos, 0);
    if (sharedInfos == MAP_FAILED) {
        perror("LOG-serveur : \033[0;31mErreur lors du mapping de la mémoire partagée : shmparties\033[0m");
        exit(1);
    }

    // Initialiser les structures de parties en mémoire partagée
    memset(sharedInfos, 0, sizeof(*sharedInfos));
    for (int x = 0; x < MAX_PARTIES; x++) {
        sharedInfos->parties[x].idPartie = -1;
        sharedInfos->joueurs[x].IDpartie = -1;
        sharedInfos->parties[x].nbJoueursMax = MAX_JOUEURS;
    }
    pthread_mutex_init(&sharedInfos->mutex, NULL);  // Initialisation des mutex




    /////// Début gestion des connexions ///////

    message RCV; // Buffer messages reçu
    message SND; //Buffer messeage SEND

    int nbr_joueurs_max = 20;
    pthread_t threads[MAX_PARTIES];

    int flag_joueurDejaConnecte;
    int flag_pseudoDouble;
    int flag_partieTrouvee;
    int flag_rejoindre_ok;

    while (1) {
        pthread_mutex_unlock(&sharedInfos->mutex);
        // Affichage du header du tableau
        printf("+--------+------------------+------------------+------------------+-------------------+-------+\n");
        printf("|  PID   |     ID Partie    |      Pseudo      |      Rôle        |      Statut       | Choix |\n");
        printf("+--------+------------------+------------------+------------------+-------------------+-------+\n");
        // Parcours du tableau des joueurs et affichage des informations sous forme de tableau
        for (int i = 0; i < sharedInfos->nb_joueurs; i++) {
            joueur *j = &sharedInfos->joueurs[i];
            printf("| %-6d | %-10d | %-16s | %-16s | %-17s | %-5d |\n",
                   j->pid,
                   j->IDpartie,
                   j->pseudo,
                   j->role,
                   j->status,
                   j->choix
            );
        }
        printf("+--------+------------------+------------------+------------------+-------------------+-------+\n");

        // initialisation des structure message à 0
        memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
        memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0

        // Attente message client
        printf("\nLOG-serveur : \033[0;33mAttente client ...\n\033[0m");
        if (msgrcv(msgid, &RCV, sizeof(RCV.Corp), 99, 0) == -1) {
            perror("LOG-serveur : \033[0;31mErreur lors de la réception d'un message\033[0m");
            break;
        }
        AffichageMessage(RCV,1);  // Afficher le message reçu

        switch (RCV.Corp.requete) {
            case 1: // Requête de connexion
                pthread_mutex_lock(&sharedInfos->mutex);
                flag_joueurDejaConnecte = 0;
                flag_pseudoDouble = 0;
                if (sharedInfos->nb_joueurs < nbr_joueurs_max ) { // Vérifier si le nombre de joueurs max n'est pas atteint
                    if (estPseudoValide(RCV.Corp.joueur.pseudo)) { // verifie que le pseudo du joueur est valide
                        for (int i = 0; i < sharedInfos->nb_joueurs; i++) {
                            if (sharedInfos->joueurs[i].pid == RCV.Corp.joueur.pid) {
                                flag_joueurDejaConnecte = 1;
                                break;
                            }
                            if (strcmp(sharedInfos->joueurs[i].pseudo, RCV.Corp.joueur.pseudo) == 0) {
                                flag_pseudoDouble = 1;
                                break;
                            }
                        } // Vérifier que le joueur n'est pas déjà connecté et que le pseudo est unique
                        if (!flag_joueurDejaConnecte && !flag_pseudoDouble) {
                            sharedInfos->joueurs[sharedInfos->nb_joueurs] = RCV.Corp.joueur;
                            strcpy(sharedInfos->joueurs[sharedInfos->nb_joueurs].status, "Vivant");
                            SND.Corp.requete = 6;
                            sprintf(SND.Corp.msg, "\033[0;32mConnexion établie !\033[0m");
                            sharedInfos->nb_joueurs++;
                        } // Preparation envoi connexion établie
                        else if (flag_joueurDejaConnecte) {
                            SND.Corp.requete = 99;
                            sprintf(SND.Corp.msg, "\033[0;31mJoueur déjà connecté !\033[0m");
                        } // Preparation envoi joueur déja connecté
                        else if (flag_pseudoDouble) {
                            SND.Corp.requete = 98;
                            sprintf(SND.Corp.msg, "\033[0;31mPseudo déjà utilisé, veuillez en choisir un autre.\033[0m");
                        } // Preparation envoi pseudo du joueur en double
                    } else {
                        SND.Corp.requete = 96;
                        sprintf(SND.Corp.msg, "\033[0;31mPseudo invalide !\nMinimum 4 caractères [A-Z][a-z].\033[0m");
                    } // Preparation envoi pseudo invalide
                } else {
                    SND.Corp.requete = 97;
                    sprintf(SND.Corp.msg, "\033[0;31mNombre maximum de joueurs atteint !\033[0m");
                } // Preparation envoi limite de joueur atteinte
                // Envoi du message au client
                SND.Type = RCV.Corp.joueur.pid;
                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                    break;
                }
                AffichageMessage(SND,0);  // Afficher le message envoyé
                pthread_mutex_unlock(&sharedInfos->mutex);
            break;

            case 2: // Créer une Partie
                printf("LOG-serveur : \033[0;33mCréation d'une partie demandée par (PID): \033[0;35m%d\033[0m)\n",RCV.Corp.joueur.pid);
                pthread_mutex_lock(&sharedInfos->mutex);
                flag_partieTrouvee = 0;
                for (int i = 0; i < MAX_PARTIES; i++) {
                    if (sharedInfos->parties[i].idPartie == -1) {  // Trouver une partie vide (idPartie == -1)

                        // Initialiser les données de la partie
                        printf("LOG-serveur : \033[0;32m Initialisation de la partie\033[0m)\n");
                        sharedInfos->parties[i].idPartie = i+1; // initialisation d'un ID temporaire
                        sharedInfos->parties[i].nbJoueursMax = MAX_JOUEURS;
                        sharedInfos->parties[i].status = 0; // Partie en attente
                        sharedInfos->parties[i].nbJoueurs = 0; // Initialisation à 0
                        SND.Corp.joueur.IDpartie = sharedInfos->parties[i].idPartie;

                        for (int j = 0; j < sharedInfos->nb_joueurs; j++) {
                            if (sharedInfos->joueurs[j].pid == RCV.Corp.joueur.pid) {
                                sharedInfos->parties[i].nbJoueurs++;
                                sharedInfos->joueurs[j].IDpartie = sharedInfos->parties[i].idPartie;
                            }
                        }

                        // Allouer de la mémoire pour les données du thread*
                        printf("LOG-serveur : \033[0;32m Allocation de la mémoire pour le thread de la partie\033[0m)\n");
                        ThreadData *thread_data = malloc(sizeof(ThreadData));
                        if (thread_data == NULL) {
                            sprintf(SND.Corp.msg, "\033[0;31mImpossible de créer une nouvelle partie.\033[0m");
                            perror("Erreur allocation mémoire pour ThreadData");
                            break; // Sortir si la mémoire n'a pas pu être allouée
                        }

                        printf("LOG-serveur : \033[0;32m Ecriture des informations dans thread_data\033[0m)\n");
                        thread_data->msgid = msgid;
                        thread_data->SharedInfos = sharedInfos;
                        thread_data->num_partie = i;

                        pthread_mutex_unlock(&sharedInfos->mutex);

                        // Créer le thread
                        printf("LOG-serveur : \033[0;32m Création du thread\033[0m)\n");
                        if (pthread_create(&threads[i], NULL, thread_function, thread_data) != 0) {
                            sprintf(SND.Corp.msg, "\033[0;31mImpossible de créer une nouvelle partie.\033[0m");
                            perror("Erreur création thread");
                            free(thread_data);  // Libérer la mémoire si la création du thread échoue
                            break;
                        }
                        else {
                            pthread_detach(threads[i]);  // Detacher le thread pour qu'il se nettoie tout seul
                        }

                        printf("LOG-serveur : \033[0;32m MaJ flag partie trouvée \033[0m)\n");
                        flag_partieTrouvee = 1;  // Partie créée avec succès
                        SND.Corp.requete = 6;

                    break;  // Sortir de la boucle après la création de la partie
                    } // trouve une partie vide et démarre un thread
                }

                if (!flag_partieTrouvee) {
                    // Gérer le cas où aucune partie n'a pu être créée (par exemple, s'il y a déjà MAX_PARTIES créées)
                    SND.Corp.requete = 95;
                    sprintf(SND.Corp.msg, "\033[0;31mImpossible de créer une nouvelle partie. erreur ????\033[0m");
                    SND.Type = RCV.Corp.joueur.pid;
                    if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("Erreur lors de l'envoi de la réponse");
                    }
                } // Erreur si aucune partie de libre
                else {
                    sprintf(SND.Corp.msg, "\033[0;32m Partie créée ! (vous avez était ajouté en tant qu joueur !\033[0m");
                    SND.Type = RCV.Corp.joueur.pid;
                    if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("Erreur lors de l'envoi de la réponse");
                    }
                    AffichageMessage(SND,0);
                } // envoi message partie trouvée
            break;

            case 3: // Rejoindre partie
                flag_rejoindre_ok = 0;
                printf("LOG-serveur : \033[0;33mDemande à rejoindre une partie (PID): \033[0;35m%d\033[0m)\n",RCV.Corp.joueur.pid);
                int num_partie = atoi(RCV.Corp.msg);
                if (sharedInfos->parties[num_partie].idPartie == -1) {
                    sprintf(SND.Corp.msg, "\033[0;31mImpossible de rejoindre la partie, La partie n'existe pas !.\033[0m");
                    SND.Corp.requete = 7;
                } // verifie que la partie existe
                else {
                    if (sharedInfos->parties[num_partie].nbJoueurs >= sharedInfos->parties[num_partie].nbJoueursMax) {
                        sprintf(SND.Corp.msg, "\033[0;31mImpossible de rejoindre la partie, limite de joueurs atteinte.\033[0m");
                        SND.Corp.requete = 7;
                    }
                    else {
                        for (int j = 0; j < sharedInfos->nb_joueurs; j++) {
                            if (sharedInfos->joueurs[j].pid == RCV.Corp.joueur.pid) {

                                sharedInfos->joueurs[j].IDpartie = sharedInfos->parties[num_partie].idPartie;
                                sharedInfos->parties[num_partie].nbJoueurs++;
                                SND.Corp.joueur.IDpartie = sharedInfos->parties[num_partie].idPartie;

                                flag_rejoindre_ok = 1;
                            }
                        }
                        for (int j = 0; j < sharedInfos->nb_joueurs; j++) {
                            if (sharedInfos->joueurs[j].IDpartie == sharedInfos->parties[num_partie].idPartie && sharedInfos->joueurs[j].pid != RCV.Corp.joueur.pid) {

                                sprintf(SND.Corp.msg, "\033[0;32m%s à rejoint la partie ! \033[0m",RCV.Corp.joueur.pseudo);
                                SND.Corp.partie.nbJoueurs = sharedInfos->parties[num_partie].nbJoueurs;
                                SND.Corp.partie.nbJoueursMax = sharedInfos->parties[num_partie].nbJoueursMax;
                                SND.Corp.requete = 11;
                                SND.Type = sharedInfos->joueurs[j].pid;
                                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                                    perror("Erreur lors de l'envoi de la réponse");
                                }
                                AffichageMessage(SND,0);
                            }
                        }
                        if (sharedInfos->parties[num_partie].nbJoueurs == sharedInfos->parties[num_partie].nbJoueursMax) {
                            printf("LOG-serveur : \033[0;32m Démarrage de la partie ! \033[0m)\n");
                            sharedInfos->parties[num_partie].status = 1;
                        } // joueurs max atteint
                    }
                }

                if (flag_rejoindre_ok) {
                    sprintf(SND.Corp.msg, "\033[0;32m Vous avez rejoins la partie ! \033[0m");
                    SND.Corp.requete = 6;
                }
                pthread_mutex_unlock(&sharedInfos->mutex);
                SND.Type = RCV.Corp.joueur.pid;
                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                            perror("Erreur lors de l'envoi de la réponse");
                        }
                AffichageMessage(SND,0);  // Afficher le message envoyé

            break;

            case 4: // Requête de déconnexion
                printf("LOG-serveur : \033[0;33mDéconnexion demandée par (PID): \033[0;35m%d\033[0m)\n",RCV.Corp.joueur.pid);
                int joueurTrouve = 0;
                pthread_mutex_lock(&sharedInfos->mutex);
                for (int i = 0; i < sharedInfos->nb_joueurs; i++) {
                    if (sharedInfos->joueurs[i].pid == RCV.Corp.joueur.pid) {
                        joueurTrouve = 1;
                        // Décalage des joueurs suivants pour combler l'espace laissé par le joueur supprimé
                        for (int j = i; j < sharedInfos->nb_joueurs - 1; j++) {
                            sharedInfos->joueurs[j] = sharedInfos->joueurs[j + 1];
                        }
                        // Réduction du nombre de joueurs
                        sharedInfos->nb_joueurs--;
                        break;
                    }
                } // trouve le joueur et le supprime de la liste
                if (joueurTrouve) {
                    printf("LOG-serveur : \033[0;32mJoueur (PID: %d) déconnecté avec succès.\n\033[0m",RCV.Corp.joueur.pid);
                    // Envoyer confirmation de déconnexion au client
                    SND.Corp.requete = 6 ;
                    sprintf(SND.Corp.msg, "\033[0;32mDéconnexion réussie !\033[0m");
                } // Préparation message si joueur trouvé
                else {
                    printf("LOG-serveur : \033[0;31mJoueur %s (PID: %d) introuvable pour déconnexion.\n\033[0m",
                    RCV.Corp.joueur.pseudo, RCV.Corp.joueur.pid);

                    // Envoyer un message d'erreur si le joueur n'était pas dans la liste
                    SND.Corp.requete = 94; // Code 95 = Joueur introuvable
                    sprintf(SND.Corp.msg, "\033[0;31mErreur : joueur introuvable.\033[0m");
                } // Préparation message si joueur non trouvé
                // Envoi du message au client
                SND.Type = RCV.Corp.joueur.pid;
                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la confirmation de déconnexion\033[0m");
                    break;
                }
                pthread_mutex_unlock(&sharedInfos->mutex);
                AffichageMessage(SND, 0);  // Afficher le message envoyé

            // AJOUTER GESTION JOUEUR QUITTE EN ETANT DANS UNE PARTIE

            break;

            case 10:
                printf("LOG-serveur : \033[0;33mDemande liste des parties (PID): \033[0;35m%d\033[0m\n", RCV.Corp.joueur.pid);

                char liste_parties[1024] = "";  // Buffer pour stocker la liste des parties

                for (int i = 0; i < MAX_PARTIES; i++) {
                    if (sharedInfos->parties[i].idPartie != -1) {  // Vérifie si la partie est active
                        char ligne[100];  // Stocker chaque ligne de partie temporairement
                        sprintf(ligne, "\033[0;36m%d\033[0m : \033[0;32mPartie %d\033[0m | \033[0;33m%d/%d joueurs\033[0m\n",
                                i,
                                sharedInfos->parties[i].idPartie,
                                sharedInfos->parties[i].nbJoueurs,
                                sharedInfos->parties[i].nbJoueursMax);
                        strcat(liste_parties, ligne);  // Ajoute la ligne au message final
                    }
                }

                // Remplissage de la structure SND avant envoi
                sprintf(SND.Corp.msg, "Liste des parties :\n%s",liste_parties);
                SND.Corp.partie.nbJoueurs = sharedInfos->parties[num_partie].nbJoueurs;
                SND.Corp.partie.nbJoueursMax = sharedInfos->parties[num_partie].nbJoueursMax;
                SND.Corp.requete = 6;
                SND.Type = RCV.Corp.joueur.pid;

                // Envoi du message avec la liste des parties
                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("Erreur lors de l'envoi de la réponse");
                }

                AffichageMessage(SND, 0);
            break;

            default :
                sprintf(SND.Corp.msg, "\033[0;31m Requête Inconnue \033[0m");
                SND.Type = RCV.Corp.joueur.pid;
                if (msgsnd(msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("LOG-serveur : \033[0;31mErreur lors de l'envoi du message\033[0m");
                    break;
                }
            break;
        }
    }

    // Suppression de la boîte aux lettres
    PAUSE("Attente avant supression de la boite aux lettres");
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("\033[0;31mErreur lors de la suppression de la boîte aux lettres\033[0m");
        exit(EXIT_FAILURE);
    }

    printf("LOG-serveur : \033[0;32mBoîte aux lettres supprimée.\n\033[0m");

    return 0;
}

void AffichageMessage(message msg, int mode) {
    // Afficher le message
    char *affichageMode = "NULL";
    if (mode == 1) {
        affichageMode = "reçu de";
    }
    else if (mode == 0) {
        affichageMode = "envoyer à";
        msg.Corp.joueur.pid = msg.Type;
    }

    printf("\nLOG-serveur : Message %s PID \033[0;35m%d\033[0m :\n"
           "\tpseudo :\033[0;35m%s\033[0m\n"
           "\tIDparite :\033[0;35m%d\033[0m\n"
           "\trole :\033[0;35m%s\033[0m\n"
           "\trequête :\033[0;35m%d\033[0m\n"
           "\tmessage :\033[0;35m%s\033[0m\n",affichageMode, msg.Corp.joueur.pid, msg.Corp.joueur.pseudo,msg.Corp.joueur.IDpartie, msg.Corp.joueur.role,msg.Corp.requete, msg.Corp.msg);
}

void vider_boite_aux_lettres(int msgid) {
    message msg;
    int res;

    printf("Vidage de la boîte aux lettres...\n");
    while ((res = msgrcv(msgid, &msg, sizeof(msg.Corp), 0, IPC_NOWAIT)) != -1) {
        AffichageMessage(msg,1);
    }
    system("clear");  // Effacer l'écran
    printf("Boîte aux lettres vidée avec succès.\n");
}

int estPseudoValide(const char *pseudo) {
    // Vérifier si le pseudo a au moins 4 caractères
    if (strlen(pseudo) < 4) {
        return 0; // Pseudo trop court
    }
    // Vérifier si tous les caractères du pseudo sont des lettres (A-Z, a-z)
    for (int i = 0; i < strlen(pseudo); i++) {
        if (!isalpha(pseudo[i])) {
            return 0; // Caractère invalide
        }
    }
    return 1; // Pseudo valide
}

void* thread_function(void* arg) {

    ThreadData* data = (ThreadData *)arg;

    // Initialise la structure d'attribution des roles
    Role roles[] = {
        {"LOUP-GAROU", 2},
        {"VILLAGEOIS", 2},
        {"VOYANTE", 1},
        {"MAIRE", 1}
    };
    int nb_roles = MAX_JOUEURS;
    int flag_LG_en_vie = 2;
    int flag_joueurs_en_vie = 6;
    int flag_voyante_en_vie = 1;

    pthread_mutex_lock(&data->SharedInfos->mutex);
    // Initialiser l'ID de la partie
    printf("LOG-PARTIE%d : \033[0;32mThread partie n°%d démarée !\n\033[0m",data->num_partie,data->SharedInfos->parties[data->num_partie].idPartie);
    pthread_mutex_unlock(&data->SharedInfos->mutex);


    // attente démarrage de la partie
    printf("LOG-PARTIE%d : \033[0;33mAttente démarrage partie...\n\033[0m", data->num_partie);
    while (1) {
        pthread_mutex_lock(&data->SharedInfos->mutex);
        if (data->SharedInfos->parties->status == 1) {
            break;
        }
        pthread_mutex_unlock(&data->SharedInfos->mutex);
        sleep(1);
    }
    pthread_mutex_unlock(&data->SharedInfos->mutex);

    printf("LOG-PARTIE%d : \033[0;32mDémarrage de la partie...\n\033[0m", data->num_partie);

    // Attribution des rôles pour chaque joueur de la partie
    printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
    pthread_mutex_lock(&data->SharedInfos->mutex);
    printf("LOG-PARTIE%d : \033[0;33mAttribution des rôles...\n\033[0m", data->num_partie);
    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        printf("LOG-PARTIE%d : \033[0;32mAttribution des rôles ...\n\033[0m", data->num_partie);
        // Vérifier si le joueur est bien dans cette partie en comparant l'IDpartie
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie) {
            do {
                // Initialiser le générateur de nombres aléatoires
                srand(time(0));
                int nombre_aleatoire = rand() % 4; // entre 0 et 3
                if (roles[nombre_aleatoire].dispo != 0) {
                    strcpy(data->SharedInfos->joueurs[i].role, roles[nombre_aleatoire].name);
                    roles[nombre_aleatoire].dispo--;
                    nb_roles--;
                    printf("LOG-PARTIE%d : \033[0;32mAttribution du rôle : %s au joueur :\n\tPID :%d\n\t%s\033[0m",
                        data->num_partie,
                        data->SharedInfos->joueurs[i].role,
                        data->SharedInfos->joueurs[i].pid,
                        data->SharedInfos->joueurs[i].pseudo
                    );
                }
            } while (strcmp(data->SharedInfos->joueurs[i].role, "") == 0);
        }
    }
    pthread_mutex_unlock(&data->SharedInfos->mutex);
    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);
    sleep(1);

    message RCV; // Buffer messages reçu
    message SND; //Buffer messeage SEND

    memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
    memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0

    int nb_ACK;

    pthread_mutex_lock(&data->SharedInfos->mutex);
    printf("LOG-PARTIE%d : \033[0;33mEnvoie des messages de démarrage de la partie...\n\033[0m", data->num_partie);
    printf("LOG-PARTIE%d : nb_joueurs = %d\n", data->num_partie, data->SharedInfos->nb_joueurs);

    // Envoi du rôle à tous les joueurs de la partie ( démarrage de la partie)
    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        printf("LOG-PARTIE%d : Vérification du joueur %d (IDpartie=%d, pid=%d)\n",
               data->num_partie, i, data->SharedInfos->joueurs[i].IDpartie, data->SharedInfos->joueurs[i].pid);
        // Si le joueur est dans la partie
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie) {
            printf("LOG-PARTIE%d : Envoi du rôle à PID %d...\n", data->num_partie, data->SharedInfos->joueurs[i].pid);

            memset(&SND, 0, sizeof(SND));
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, "Vivant");
            SND.Corp.requete = 12;
            sprintf(SND.Corp.msg, "**************************************************\n\t\tVOTRE ROLE : %s\n**************************************************\n", data->SharedInfos->joueurs[i].role);

            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : Erreur lors de l'envoi de la réponse");
                break;
            }
            printf("LOG-PARTIE%d : Message envoyé à PID %d !\n", data->SharedInfos->parties[data->num_partie].idPartie, data->SharedInfos->joueurs[i].pid);
            AffichageMessage(SND,0);  // Afficher le message envoyé
        }
    }

    pthread_mutex_unlock(&data->SharedInfos->mutex);

    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

    sleep(1);
    printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
    pthread_mutex_lock(&data->SharedInfos->mutex);
    nb_ACK = 0;

    // Attente de l'aquittement du démarrage de la partie
    while (nb_ACK < data->SharedInfos->parties[data->num_partie].nbJoueurs) {
        memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
        printf("LOG-PARTIE%d : \033[0;33mAttente ACK démarrage...\n\033[0m", data->num_partie);
        if (msgrcv(data->msgid, &RCV, sizeof(RCV.Corp), data->SharedInfos->parties[data->num_partie].idPartie, 0) == -1) {
            printf("LOG-PARTIE%d ", data->num_partie);
            perror("\033[0;31mErreur lors de la réception d'un message\033[0m");
            break;
        }
        AffichageMessage(RCV,1);  // Afficher le message reçu
        if (RCV.Corp.requete == 16) {
            nb_ACK++;
        }
    }
    pthread_mutex_unlock(&data->SharedInfos->mutex);
    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

    sleep(1);
    int partie_en_cours = 1;

    // Déroulement de la partie
    while (partie_en_cours) {

        printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
        pthread_mutex_lock(&data->SharedInfos->mutex);
        printf("LOG-PARTIE%d : \033[0;33mEnvoi UPDATE tour voyante [no ack]\n\033[0m", data->num_partie);
        printf("\n\nflag voyante en vie : %d\n\n",flag_voyante_en_vie);
        for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
            // Vérifier si le joueur est bien dans cette partie en comparant l'IDpartie
            if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie) {
                memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
                SND.Type = data->SharedInfos->joueurs[i].pid;
                strcpy(SND.Corp.joueur.role,data->SharedInfos->joueurs[i].role);
                strcpy(SND.Corp.joueur.status,data->SharedInfos->joueurs[i].status);
                SND.Corp.requete = 13;
                sprintf(SND.Corp.msg, "\033[0;34mLa nuit tombe sur le village ...\n*****C'est au tour de la voyante de se réveiller...****\033[0m");
                if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                    break;
                }
                AffichageMessage(SND,0);  // Afficher le message envoyé
            }
        }
        pthread_mutex_unlock(&data->SharedInfos->mutex);
        printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

        sleep(1);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////           VOYANTE           //////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        if (flag_voyante_en_vie) {
            printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
            pthread_mutex_lock(&data->SharedInfos->mutex);
            printf("LOG-PARTIE%d : \033[0;33mEnvoi demande de choix à la voyante\n\033[0m", data->num_partie);
            int flag_choix = 0;
            char* cibles[100];  // Tableau pour stocker les pseudos des cibles
            int index = 0;      // Index pour ajouter des cibles

            for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
                // Récupérer la liste des cibles, ignorer les "VOYANTE"
                if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].role, "VOYANTE") != 0 && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
                    cibles[index] = data->SharedInfos->joueurs[i].pseudo;  // Ajouter le pseudo dans le tableau cibles
                    index++;  // Incrémenter l'index pour ajouter une nouvelle cible
                }
            }
            pthread_mutex_unlock(&data->SharedInfos->mutex);
            printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

            // Créer une chaîne de caractères pour afficher les pseudos des cibles
            char l_cibles[1024] = "";  // Tableau pour la chaîne finale
            l_cibles[0] = '\0';  // Vider la chaîne avant d'ajouter les nouvelles valeurs
            for (int i = 0; i < index; i++) {
                // Formater chaque ligne pour afficher l'index et le pseudo de chaque cible
                char line[100];
                sprintf(line, "%d : %s\n", i, cibles[i]);
                strcat(l_cibles, line);  // Ajouter cette ligne à la chaîne result
            }
            printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
            pthread_mutex_lock(&data->SharedInfos->mutex);
            for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
                // ENVOI UN MESSAGE UUNIQUEMENT A LA "VOYANTE"
                if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].role, "VOYANTE") == 0) {
                    memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
                    SND.Type = data->SharedInfos->joueurs[i].pid;
                    strcpy(SND.Corp.joueur.role,data->SharedInfos->joueurs[i].role);
                    strcpy(SND.Corp.joueur.status,"Vivant");
                    SND.Corp.requete = 21;
                    sprintf(SND.Corp.msg, "\033[0;34mChoisissez un joueur pour révéler son rôle : \n%s  \033[0m",l_cibles);
                    if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                        break;
                    }                system("clear");
                    AffichageMessage(SND,0);  // Afficher le message envoyé
                }
            }
            pthread_mutex_unlock(&data->SharedInfos->mutex);
            printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

            do {
                memset(&RCV, 0, sizeof(RCV));  // Initialise toute la structure RCV à 0
                printf("LOG-PARTIE%d : \033[0;33mAttente choix voyante\n\033[0m", data->num_partie);
                if (msgrcv(data->msgid, &RCV, sizeof(RCV.Corp), data->SharedInfos->parties[data->num_partie].idPartie, 0) == -1) {
                    printf("LOG-PARTIE%d ", data->num_partie);
                    perror("\033[0;31mErreur lors de la réception d'un message\033[0m");
                    break;
                }
                AffichageMessage(RCV,1);  // Afficher le message reçu
                if (RCV.Corp.requete == 26) {
                    flag_choix = 1;
                }

            }while (!flag_choix);
            printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
            pthread_mutex_lock(&data->SharedInfos->mutex);

            for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
                // ENVOI UN MESSAGE UNIQUEMENT A LA "VOYANTE"
                if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(cibles[RCV.Corp.joueur.choix], data->SharedInfos->joueurs[i].pseudo) == 0) {
                    memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
                    SND.Type = RCV.Corp.joueur.pid;
                    strcpy(SND.Corp.joueur.role,RCV.Corp.joueur.role);
                    strcpy(SND.Corp.joueur.status,"Vivant");
                    SND.Corp.requete = 13;
                    sprintf(SND.Corp.msg, "\033[0;34mle role de %s est : %s\033[0m",cibles[RCV.Corp.joueur.choix],data->SharedInfos->joueurs[i].role);
                    if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                        perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                        break;
                    }
                    AffichageMessage(SND,0);  // Afficher le message envoyé
                }
            }
        }
        pthread_mutex_unlock(&data->SharedInfos->mutex);
        printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

        sleep(1);




        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////           LOUPS             //////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Envoi du message de réveil des loups-garous
        printf("LOG-PARTIE%d : \033[0;33mEnvoi UPDATE tour loups [no ack]\n\033[0m", data->num_partie);
        for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
            if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie) {
                memset(&SND, 0, sizeof(SND));
                SND.Type = data->SharedInfos->joueurs[i].pid;
                strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
                strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
                SND.Corp.requete = 13;
                sprintf(SND.Corp.msg, "\033[0;34m***** C'est au tour des loups-garous de se réveiller... *****\033[0m");
                if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                    perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                    break;
                }
                AffichageMessage(SND, 0);
            }
        }

        // Attente du mutex
        pthread_mutex_lock(&data->SharedInfos->mutex);
        printf("LOG-PARTIE%d : \033[0;33mEnvoi demande de choix aux loups\n\033[0m", data->num_partie);
        sleep(1);

        // Construction de la liste des cibles
        char* cibles[100] = {NULL};
        int index = 0;
        for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
            if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie &&
                strcmp(data->SharedInfos->joueurs[i].role, "LOUP-GAROU") != 0 &&
                strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
                cibles[index] = data->SharedInfos->joueurs[i].pseudo;
            index++;
        }
    }
    pthread_mutex_unlock(&data->SharedInfos->mutex);

    // Création du message de sélection des cibles
    char l_cibles[1024] = "";
    for (int i = 0; i < index; i++) {
        char line[100];
        sprintf(line, "%d : %s\n", i, cibles[i]);
        strcat(l_cibles, line);
    }

    // Envoi du message aux loups-garous
    pthread_mutex_lock(&data->SharedInfos->mutex);
    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie
            && strcmp(data->SharedInfos->joueurs[i].role, "LOUP-GAROU") == 0
            && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {

            memset(&SND, 0, sizeof(SND));
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
            SND.Corp.requete = 21;
            sprintf(SND.Corp.msg, "\033[0;34mChoisissez un joueur à éliminer : %s\033[0m", l_cibles);
            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                break;
            }
            AffichageMessage(SND, 0);
        }
    }
    pthread_mutex_unlock(&data->SharedInfos->mutex);

    // Réception des votes
    int votes_lg[flag_LG_en_vie];
    memset(votes_lg, -1, sizeof(votes_lg));
    int vote_count = 0;
    while (vote_count < flag_LG_en_vie) {
        memset(&RCV, 0, sizeof(RCV));
        printf("LOG-PARTIE%d : \033[0;33mAttente choix Loups-Garous\n\033[0m", data->num_partie);
        if (msgrcv(data->msgid, &RCV, sizeof(RCV.Corp), data->SharedInfos->parties[data->num_partie].idPartie, 0) == -1) {
            perror("LOG-PARTIE : \033[0;31mErreur réception choix Loups-Garous\033[0m");
            break;
        }
        if (RCV.Corp.requete == 26) {
            votes_lg[vote_count] = RCV.Corp.joueur.choix;
            vote_count++;
        }
    }

    // Sélection de la victime
    if (vote_count == 0) {
        printf("LOG-PARTIE%d : Aucun vote reçu, aucun joueur ne sera éliminé.\n", data->num_partie);
    }

    int choix_final = votes_lg[0];
    for (int i = 1; i < vote_count; i++) {
        if (votes_lg[i] != choix_final) {
            choix_final = votes_lg[rand() % vote_count];
            break;
        }
    }

    printf("LOG-PARTIE%d : Victime finale choisie : %d (%s)\n", data->num_partie, choix_final, cibles[choix_final]);

    sleep(1);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////           JOUR              //////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
index = 0;
int flag_joueur_mort;
l_cibles[0] = '\0';  // Vider la chaîne avant d'ajouter les nouvelles valeurs

printf("LOG-PARTIE%d : \033[0;33mAttente mutex...\n\033[0m", data->num_partie);
pthread_mutex_lock(&data->SharedInfos->mutex);

    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        if (strcmp(cibles[choix_final], data->SharedInfos->joueurs[i].pseudo) == 0) {
            // Marquer le joueur comme mort
            sprintf(data->SharedInfos->joueurs[i].status, "Mort");
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
            data->SharedInfos->joueurs[i].IDpartie = -1;
            data->SharedInfos->parties[data->num_partie].nbJoueurs--;
            SND.Corp.requete = 31;
            sprintf(SND.Corp.msg, "\033[0;34mLes loups vous ont dévoré pendant la nuit ! \033[0m");

            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                break;
            }

            AffichageMessage(SND, 0);  // Afficher le message envoyé
            flag_joueur_mort = i;
            // Vérification si le joueur est voyante
            if (strcmp(data->SharedInfos->joueurs[i].role, "VOYANTE") == 0) {
                flag_voyante_en_vie = 0;
            }
        }
    }

    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
            cibles[index] = data->SharedInfos->joueurs[i].pseudo;  // Ajouter le pseudo dans le tableau cibles
            index++;  // Incrémenter l'index pour ajouter une nouvelle cible
        }
    }

    pthread_mutex_unlock(&data->SharedInfos->mutex);
    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

    // Créer une chaîne de caractères pour afficher les pseudos des cibles
    l_cibles[0] = '\0';  // Vider la chaîne avant d'ajouter les nouvelles valeurs
    for (int i = 0; i < index; i++) {
        // Formater chaque ligne pour afficher l'index et le pseudo de chaque cible
        char line[100];
        sprintf(line, "%d : %s\n", i, cibles[i]);
        strcat(l_cibles, line);  // Ajouter cette ligne à la chaîne result
    }

    pthread_mutex_unlock(&data->SharedInfos->mutex);
    sleep(1);
    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

    // Afficher les cibles
    printf("l_cibles = %s", l_cibles);

    pthread_mutex_lock(&data->SharedInfos->mutex);
    sleep(1);
    printf("nombre de joueurs : %d", data->SharedInfos->parties[data->num_partie].nbJoueurs);

    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        // ENVOI UN MESSAGE A TOUS
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
            memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
            SND.Corp.requete = 21;
            sprintf(SND.Corp.msg, "\033[0;33mLe jour se lève sans %s qui était : %s \nChoisissez un joueur pour l'éliminer \n\033[0m: \n%s  \033[0m",
                    data->SharedInfos->joueurs[flag_joueur_mort].pseudo,
                    data->SharedInfos->joueurs[flag_joueur_mort].role,
                    l_cibles);

            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                break;
            }

            AffichageMessage(SND, 0);  // Afficher le message envoyé
        }
    }

    int VotesVillage[data->SharedInfos->parties[data->num_partie].nbJoueurs];
    memset(VotesVillage, 0, sizeof(VotesVillage));  // Initialisation des votes à 0
    vote_count = 0;

    while (vote_count < data->SharedInfos->parties[data->num_partie].nbJoueurs) {
        pthread_mutex_unlock(&data->SharedInfos->mutex);
        printf("LOG-PARTIE%d : \033[0;33mAttente choix villageois...\n\033[0m", data->num_partie);

        if (msgrcv(data->msgid, &RCV, sizeof(RCV.Corp), data->SharedInfos->parties[data->num_partie].idPartie, 0) == -1) {
            perror("LOG-PARTIE : \033[0;31mErreur réception choix Loups-Garous\033[0m");
            break;
        }

        AffichageMessage(RCV, 1);

        if (RCV.Corp.requete == 26) {
            VotesVillage[RCV.Corp.joueur.choix]++;
            if (strcmp(RCV.Corp.joueur.role, "MAIRE") == 0) {
                VotesVillage[RCV.Corp.joueur.choix]++;
            }
            vote_count++;
        }

        pthread_mutex_lock(&data->SharedInfos->mutex);
}

    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);
    pthread_mutex_unlock(&data->SharedInfos->mutex);

    sleep(1);
    printf("init size");

    int size = sizeof(VotesVillage) / sizeof(VotesVillage[0]);
    int maxValue, maxIndex;
    findMax(VotesVillage, size, &maxValue, &maxIndex);
    printf("cibles : %s", cibles[maxIndex]);
    pthread_mutex_lock(&data->SharedInfos->mutex);
    // Traitement du joueur éliminé
    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        if (strcmp(cibles[maxIndex], data->SharedInfos->joueurs[i].pseudo) == 0) {
            // Si c'est la voyante, la marquer comme morte et changer son statut
            printf("role du mort : %s",data->SharedInfos->joueurs[i].role);
            sprintf(data->SharedInfos->joueurs[i].status, "Mort");
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
            data->SharedInfos->joueurs[i].IDpartie = -1;
            data->SharedInfos->parties[data->num_partie].nbJoueurs--;
            SND.Corp.requete = 31;
            sprintf(SND.Corp.msg, "\033[0;34mLe village a décidé de vous éliminer ! \033[0m");

            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                break;
            }

            AffichageMessage(SND, 0);  // Afficher le message envoyé
            flag_joueur_mort = i;

            if (strcmp(data->SharedInfos->joueurs[i].role, "VOYANTE") == 0) {
                flag_voyante_en_vie = 0;
                data->SharedInfos->joueurs[i].role[0] = '\0';
            }
            // Si c'est un loup, le marquer comme morte et changer son statut
            if (strcmp(data->SharedInfos->joueurs[i].role, "LOUP-GAROU") == 0) {
                flag_LG_en_vie--;
            }
        }
    }

    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);
    pthread_mutex_unlock(&data->SharedInfos->mutex);
    sleep(1);
    pthread_mutex_lock(&data->SharedInfos->mutex);

    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {

        // ENVOI UN MESSAGE A TOUS
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
            memset(&SND, 0, sizeof(SND));  // Initialise toute la structure SND à 0
            SND.Type = data->SharedInfos->joueurs[i].pid;
            strcpy(SND.Corp.joueur.role, data->SharedInfos->joueurs[i].role);
            strcpy(SND.Corp.joueur.status, data->SharedInfos->joueurs[i].status);
            SND.Corp.requete = 13;
            sprintf(SND.Corp.msg, "\033[0;33m%s a été éliminé par le village, il était : %s \n\033[0m",
                    data->SharedInfos->joueurs[flag_joueur_mort].pseudo,
                    data->SharedInfos->joueurs[flag_joueur_mort].role);

            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse\033[0m");
                break;
            }

            AffichageMessage(SND, 0);  // Afficher le message envoyé
        }
    }

    pthread_mutex_unlock(&data->SharedInfos->mutex);
    sleep(1);

    // verif fin de partie
    if (flag_LG_en_vie == 0) {
        sprintf(SND.Corp.msg, "\033[0;32mLa partie est terminée ! ");
        sprintf(SND.Corp.msg + strlen(SND.Corp.msg), "Les villageois ont gagné !\033[0m");
        partie_en_cours = 0;
    } else if(flag_LG_en_vie >=data->SharedInfos->parties[data->num_partie].nbJoueurs-flag_LG_en_vie) {
        sprintf(SND.Corp.msg, "\033[0;32mLa partie est terminée ! ");
        sprintf(SND.Corp.msg + strlen(SND.Corp.msg), "Les loups ont gagné !\033[0m");
        partie_en_cours = 0;
    }

    pthread_mutex_unlock(&data->SharedInfos->mutex);
    }

    // Envoyer un message à tous les joueurs pour annoncer la fin de la partie
    SND.Corp.requete = 28;  // Message de fin de partie
    for (int i = 0; i < data->SharedInfos->nb_joueurs; i++) {
        if (data->SharedInfos->joueurs[i].IDpartie == data->SharedInfos->parties[data->num_partie].idPartie && strcmp(data->SharedInfos->joueurs[i].status, "Vivant") == 0) {
            SND.Type = data->SharedInfos->joueurs[i].pid;
            data->SharedInfos->joueurs[i].IDpartie = -1;
            data->SharedInfos->joueurs[i].role[0] = '\0';
            sprintf(data->SharedInfos->joueurs[i].status, "Vivant");
            if (msgsnd(data->msgid, &SND, sizeof(SND.Corp), 0) == -1) {
                perror("LOG-serveur : \033[0;31mErreur lors de l'envoi de la réponse de fin de partie\033[0m");
                break;
            }
            AffichageMessage(SND, 0);  // Afficher le message envoyé
        }
    }
    printf("LOG-PARTIE%d : \033[0;33mNettoyage de la partie ...\n\033[0m", data->num_partie);
    data->SharedInfos->parties[data->num_partie].idPartie = -1;
    data->SharedInfos->parties[data->num_partie].status = 0;
    data->SharedInfos->parties[data->num_partie].nbJoueurs = 0;
    data->SharedInfos->parties[data->num_partie].nbJoueursMax = MAX_JOUEURS;

    printf("LOG-PARTIE%d : \033[0;33mDépose du mutex\n\033[0m", data->num_partie);

    printf("LOG-PARTIE%d : \033[0;33mFin du thread\n\033[0m", data->num_partie);
    free(data); // Libérer la mémoire
    return NULL;
}

void handle_tstp(int signum) {
    printf("Le signal SIGTSTP (Ctrl+Z) a été intercepté et ignoré !\n");
}

void findMax(int arr[], int size, int *maxValue, int *maxIndex) {
    *maxValue = arr[0];
    *maxIndex = 0;

    for (int i = 1; i < size; i++) {
        if (arr[i] > *maxValue) {
            *maxValue = arr[i];
            *maxIndex = i;
        }
    }
}