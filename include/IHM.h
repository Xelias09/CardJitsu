//
// Created by alexis on 05/06/25.
//

#ifndef IHM_H
#define IHM_H

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



#endif //IHM_H
