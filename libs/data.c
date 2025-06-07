//
// Created by alexis on 27/05/25.
//

#include "../include/data.h"

#include <string.h>

int creerPartie(SharedInfos *infoServeur) {
    for (int i = 0; i < NB_PARTIES_MAX; i++) {
        if (infoServeur->parties[i].status == 0) {  // 0 = partie inactive
            infoServeur->parties[i].idPartie = i;
            infoServeur->parties[i].nbJoueurs = 0;
            infoServeur->parties[i].status = 1; // 1 = partie en attente
            memset(infoServeur->parties[i].liste_joueur, 0, sizeof(infoServeur->parties[i].liste_joueur));
            printf("[INFO] Partie %d initialisée\n", i);
            return i;
        }
    }
    // Aucune place disponible
    return -1;
}
