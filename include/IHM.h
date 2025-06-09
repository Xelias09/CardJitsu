//
// Created by alexis on 05/06/25.
//

#ifndef IHM_H
#define IHM_

#define MAX_GAGNEES_PAR_TYPE 10

typedef struct {
    int couleurs_feu[MAX_GAGNEES_PAR_TYPE];
    int nb_feu;
    int couleurs_eau[MAX_GAGNEES_PAR_TYPE];
    int nb_eau;
    int couleurs_glace[MAX_GAGNEES_PAR_TYPE];
    int nb_glace;
    char nom[16];
} historique_affichage_t;

void regles();
void afficherMenu();
int lireChoixMenu();
void ajouter_carte_gagnee(historique_affichage_t* h, int element, int couleur);
const char* cercle_couleur(int couleur);
void afficher_historique(historique_affichage_t moi,historique_affichage_t adv);

#endif //IHM_H
