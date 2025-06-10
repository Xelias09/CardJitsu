//
// Created by alexis on 05/06/25.
//

#include <stdio.h>
#include <stdlib.h>
#include "../include/IHM.h"
#include "../include/data.h"

// Règles du jeu :
void regles() {
    system("clear");  // Effacer l'écran
    printf("=====================================\n");
    printf("         RÈGLES DU CARD-JITSU        \n");
    printf("=====================================\n");
    printf("1. Chaque joueur possède des cartes :\n");
    printf("   🔥 Feu   🌊 Eau   ❄️ Neige\n");
    printf("\n");
    printf("2. Les forces élémentaires sont :\n");
    printf("   - 🔥 Feu bat ❄️ Neige\n");
    printf("   - ❄️ Neige bat 🌊 Eau\n");
    printf("   - 🌊 Eau bat 🔥 Feu\n");
    printf("\n");
    printf("3. Conditions de victoire :\n");
    printf("   ➤ Posséder une carte de chaque élément (Feu, Eau, Neige) \n");
    printf("     avec des COULEURS différentes\n");
    printf("      ou\n");
    printf("   ➤ Posséder 3 cartes du MÊME élément mais de 3 couleurs différentes\n");
    printf("\n");
    printf("4. À chaque tour :\n");
    printf("   - Chaque joueur joue une carte face cachée\n");
    printf("   - Les cartes sont révélées et comparées selon les règles ci-dessus\n");
    printf("\n");
    printf("5. En cas d'égalité :\n");
    printf("   - Les cartes sont rejetées et le tour est rejoué\n");
    printf("-------------------------------------\n");
    printf("         Bonne chance, Pingouin !       \n");
    printf("=====================================\n");

}

void afficherMenu(void) {
    system("clear");  // Nettoie l'écran (optionnel)
    printf("\n========================================\n");
    printf("       Menu principal du jeu            \n");
    printf("========================================\n");
    printf("1. Afficher les règles\n");
    printf("2. Rejoindre une partie\n");
    printf("3. Créer une partie\n");
    printf("10. Quitter\n");
    printf("----------------------------------------\n");
}


int lireChoixMenu(void) {
    int choix;
    printf("Veuillez entrer votre choix (1-10) : ");

    if (scanf("%d", &choix) != 1) {
        // Saisie invalide : on vide stdin
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { }
        return 0; // 0 = choix invalide
    }

    getchar(); // Retirer le '\n' restant
    return choix;
}

const char* couleur_ansi(int couleur) {
    switch (couleur) {
        case ORANGE: return "\033[31m#\033[0m"; // ORANGE
        case VERT: return "\033[32m#\033[0m"; // Vert
        case BLEU: return "\033[34m#\033[0m"; // Bleu
        case JAUNE: return "\033[33m#\033[0m"; // Jaune
        case ROSE: return "\033[35m#\033[0m"; // ROSE
        default: return "#";
    }
}

void ajouter_carte_gagnee(historique_affichage_t* h, int element, int couleur) {
    switch (element) {
        case FEU:
            if (h->nb_feu < MAX_GAGNEES_PAR_TYPE)
                h->couleurs_feu[h->nb_feu++] = couleur;
            break;
        case EAU:
            if (h->nb_eau < MAX_GAGNEES_PAR_TYPE)
                h->couleurs_eau[h->nb_eau++] = couleur;
            break;
        case GLACE:
            if (h->nb_glace < MAX_GAGNEES_PAR_TYPE)
                h->couleurs_glace[h->nb_glace++] = couleur;
            break;
    }
}

void afficher_historique(historique_affichage_t moi, historique_affichage_t adv) {
    int i;
    printf("\nMOI\n");
    printf("  Feu   : ");
    for (i = 0; i < moi.nb_feu; i++) {
        printf("%s ", couleur_ansi(moi.couleurs_feu[i]));
    }
    printf("\n");

    printf("  Glace : ");
    for (i = 0; i < moi.nb_glace; i++) {
        printf("%s ", couleur_ansi(moi.couleurs_glace[i]));
    }
    printf("\n");

    printf("  Eau   : ");
    for (i=0; i < moi.nb_eau; i++) {
        printf("%s ", couleur_ansi(moi.couleurs_eau[i]));
    }
    printf("\n");

    printf("\nADVERSAIRE\n");
    printf("  Feu   : ");
    for (i = 0; i < adv.nb_feu; i++) {
        printf("%s ", couleur_ansi(adv.couleurs_feu[i]));
    }
    printf("\n");

    printf("  Glace : ");
    for (i = 0; i < adv.nb_glace; i++) {
        printf("%s ", couleur_ansi(adv.couleurs_glace[i]));
    }
    printf("\n");

    printf("  Eau   : ");
    for (i = 0; i < adv.nb_eau; i++) {
        printf("%s ", couleur_ansi(adv.couleurs_eau[i]));
    }
    printf("\n\n");
}



