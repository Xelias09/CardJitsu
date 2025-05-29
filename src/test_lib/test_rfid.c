#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "rfid.h"

// Base de données des cartes CardJitsu
CarteJitsu cartes[] = {
    {{0x4F, 0x16, 0x89, 0xC9}, "Dragon de Feu", "Feu", 12, "Rouge"},
    {{0x12, 0x34, 0x56, 0x78}, "Ninja d'Eau", "Eau", 8, "Bleu"},
    {{0xAB, 0xCD, 0xEF, 0x01}, "Guerrier Neige", "Neige", 10, "Blanc"},
    {{0x25, 0x47, 0x69, 0x8B}, "Loup des Glaces", "Neige", 6, "Blanc"},
    {{0xFE, 0xDC, 0xBA, 0x98}, "Phoenix Écarlate", "Feu", 11, "Rouge"},
};

int nb_cartes = sizeof(cartes) / sizeof(CarteJitsu);
int continue_reading = 1;

// Fonction pour jouer une carte (logique de jeu à implémenter)
void jouerCarte(CarteJitsu *carte) {
    printf("\n=== CARTE JOUÉE ===\n");
    afficherCarte(carte);
    
    // TODO: Implémenter la logique de jeu Card Jitsu
    // - Système de règles (Feu > Neige > Eau > Feu)
    // - Gestion des tours
    // - Score et victoires
    // - Intelligence artificielle pour l'adversaire
    
    printf("Carte ajoutée au jeu!\n");
}

// Gestionnaire de signal pour arrêter proprement
void stopReading(int sig) {
    continue_reading = 0;
    printf("\n🛑 Fin de la partie CardJitsu!\n");
}

// Afficher l'introduction du jeu
void afficherIntroduction(void) {
    printf("🎴 ═══════════════════════════════════════\n");
    printf("          🥋 CARD JITSU 🥋\n");
    printf("     Lecteur RFID Ultra-Simple\n");
    printf("   ═══════════════════════════════════════\n");
}

// Boucle principale de lecture des cartes
void boucleJeu(void) {
    uint8_t uid[4];
    uint8_t lastUID[4] = {0};
    CarteJitsu *carte;
    int i;

    printf("🃏 Approchez une carte... (Ctrl+C pour arrêter)\n\n");

    while (continue_reading) {
        if (detectCard() == MI_OK) {
            if (readUID(uid) == MI_OK) {
                // Éviter les lectures multiples de la même carte
                if (!compareUID(uid, lastUID)) {
                    printf("📖 UID: %02X,%02X,%02X,%02X\n",
                           uid[0], uid[1], uid[2], uid[3]);

                    // Chercher la carte dans la base de données
                    carte = trouverCarte(uid, cartes, nb_cartes);

                    if (carte != NULL) {
                        printf("✅ Carte reconnue!");
                        jouerCarte(carte);
                    } else {
                        afficherUIDInconnu(uid);
                    }

                    // Mémoriser l'UID pour éviter les lectures répétées
                    for (i = 0; i < 4; i++) {
                        lastUID[i] = uid[i];
                    }

                    delay(2000); // Anti-rebond
                }
            }
        }
        delay(100);
    }
}

int main(void) {
    int init_result;

    // Installer le gestionnaire de signal pour Ctrl+C
    signal(SIGINT, stopReading);

    // Afficher l'introduction
    afficherIntroduction();

    // Initialiser le module RFID
    init_result = initRFID();
    if (init_result != 0) {
        if (init_result == -1) {
            printf("❌ Erreur WiringPi\n");
        } else if (init_result == -2) {
            printf("❌ Erreur SPI\n");
        }
        return 1;
    }

    printf("📡 Lecteur initialisé\n");
    printf("🎯 %d cartes dans la base de données\n", nb_cartes);

    // Lancer la boucle de jeu
    boucleJeu();

    printf("👋 Merci d'avoir joué!\n");
    return 0;
}