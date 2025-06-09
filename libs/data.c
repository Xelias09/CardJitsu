//
// Created by alexis on 27/05/25.
//

#include "../include/data.h"

#include <pthread.h>
#include <string.h>

void initialiserSharedInfos(SharedInfos *info) {
    // Initialisation des compteurs
    info->nb_joueurs_total = 0;
    info->nb_joueurs_online = 0;

    // Vider le tableau des joueurs
    memset(info->joueurs, 0, sizeof(info->joueurs));

    // Vider le tableau des parties
    int i;
    for ( i = 0; i < NB_PARTIES_MAX; i++) {
        info->parties[i].idPartie = i;
        info->parties[i].nbJoueurs = 0;
        info->parties[i].status = LIBRE;  // inactive
        memset(info->parties[i].liste_joueur, 0, sizeof(info->parties[i].liste_joueur));
    }

    // Vider le tableau des cartes (si nécessaire, sinon à charger depuis un fichier)
    memset(info->cartes, 0, sizeof(info->cartes));
}

void resetPartie(SharedInfos *info, int idPartie) {
    if (idPartie < 0 || idPartie >= NB_PARTIES_MAX) {
        printf("[ERREUR] resetPartie : ID %d invalide\n", idPartie);
        return;
    }

    partie *p = &info->parties[idPartie];
    p->idPartie = idPartie;
    p->nbJoueurs = 0;
    p->status = LIBRE;  // 0 = inactive

    memset(p->liste_joueur, 0, sizeof(p->liste_joueur));

    printf("[INFO] Partie %d réinitialisée.\n", idPartie);
}


int creerPartie(SharedInfos *infoServeur) {
    int i;
    for (i = 0; i < NB_PARTIES_MAX; i++) {
        if (infoServeur->parties[i].status == LIBRE) {  // 0 = partie inactive
            // infoServeur->parties[i].idPartie = i;
            infoServeur->parties[i].nbJoueurs = 0;
            infoServeur->parties[i].status = ATTENTE; // 1 = partie en attente
            memset(infoServeur->parties[i].liste_joueur, 0, sizeof(infoServeur->parties[i].liste_joueur));
            printf("[INFO] Partie %d initialisée\n", i);
            return i;
        }
    }
    // Aucune place disponible
    return -1;
}
