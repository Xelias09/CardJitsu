#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define CHANNEL 0
#define SPEED 1000000

// MFRC522 Commands
#define PCD_IDLE       0x00
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F

// MFRC522 Registers (seulement les essentiels)
#define CommandReg      0x01
#define CommIEnReg      0x02
#define CommIrqReg      0x04
#define ErrorReg        0x06
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define ModeReg         0x11
#define TxModeReg       0x12
#define RxModeReg       0x13
#define TxControlReg    0x14
#define TxASKReg        0x15
#define ModWidthReg     0x24
#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReloadRegH     0x2C
#define TReloadRegL     0x2D

// PICC Commands (seulement pour détection et UID)
#define PICC_REQIDL    0x26
#define PICC_ANTICOLL  0x93

// Return codes
#define MI_OK          0
#define MI_ERR         2

// Structure pour les cartes CardJitsu
typedef struct {
    uint8_t uid[4];
    char nom[30];
    char element[10];
    int valeur;
    char couleur[10];
} CarteJitsu;

// Base de données des cartes (ajoutez toutes vos cartes ici)
CarteJitsu cartes[] = {
    {{0x4F, 0x16, 0x89, 0xC9}, "Dragon de Feu", "Feu", 12, "Rouge"},
    {{0x12, 0x34, 0x56, 0x78}, "Ninja d'Eau", "Eau", 8, "Bleu"},
    {{0xAB, 0xCD, 0xEF, 0x01}, "Guerrier Neige", "Neige", 10, "Blanc"},
    {{0x25, 0x47, 0x69, 0x8B}, "Loup des Glaces", "Neige", 6, "Blanc"},
    {{0xFE, 0xDC, 0xBA, 0x98}, "Phoenix Écarlate", "Feu", 11, "Rouge"},

};

int nb_cartes = sizeof(cartes) / sizeof(CarteJitsu);
int continue_reading = 1;

void writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buffer[2];
    buffer[0] = (reg << 1) & 0x7E;
    buffer[1] = value;
    wiringPiSPIDataRW(CHANNEL, buffer, 2);
}

uint8_t readRegister(uint8_t reg) {
    uint8_t buffer[2];
    buffer[0] = ((reg << 1) & 0x7E) | 0x80;
    buffer[1] = 0x00;
    wiringPiSPIDataRW(CHANNEL, buffer, 2);
    return buffer[1];
}

void setBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = readRegister(reg);
    writeRegister(reg, tmp | mask);
}

void clearBitMask(uint8_t reg, uint8_t mask) {
    uint8_t tmp = readRegister(reg);
    writeRegister(reg, tmp & (~mask));
}

void initRFID() {
    // Initialisation simple
    writeRegister(CommandReg, PCD_RESETPHASE);
    delay(50);

    writeRegister(TxModeReg, 0x00);
    writeRegister(RxModeReg, 0x00);
    writeRegister(ModWidthReg, 0x26);
    writeRegister(TModeReg, 0x80);
    writeRegister(TPrescalerReg, 0xA9);
    writeRegister(TReloadRegH, 0x03);
    writeRegister(TReloadRegL, 0xE8);
    writeRegister(TxASKReg, 0x40);
    writeRegister(ModeReg, 0x3D);

    // Activer antenne
    setBitMask(TxControlReg, 0x03);
}

int toCard(uint8_t command, uint8_t *sendData, int sendLen, uint8_t *backData, int *backLen) {
    int status = MI_ERR;
    uint8_t irqEn = 0x77;
    uint8_t waitIRq = 0x30;
    uint8_t lastBits;
    uint8_t n;
    int i;

    writeRegister(CommIEnReg, irqEn | 0x80);
    clearBitMask(CommIrqReg, 0x80);
    setBitMask(FIFOLevelReg, 0x80);
    writeRegister(CommandReg, PCD_IDLE);

    // Écrire données dans FIFO
    for (i = 0; i < sendLen; i++) {
        writeRegister(FIFODataReg, sendData[i]);
    }

    // Exécuter commande
    writeRegister(CommandReg, command);
    setBitMask(BitFramingReg, 0x80);

    // Attendre completion
    i = 2000;
    do {
        n = readRegister(CommIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    clearBitMask(BitFramingReg, 0x80);

    if (i != 0) {
        if (!(readRegister(ErrorReg) & 0x1B)) {
            status = MI_OK;

            n = readRegister(FIFOLevelReg);
            lastBits = readRegister(ControlReg) & 0x07;
            if (lastBits) {
                *backLen = (n - 1) * 8 + lastBits;
            } else {
                *backLen = n * 8;
            }

            if (n == 0) n = 1;
            if (n > 16) n = 16;

            // Lire données du FIFO
            for (i = 0; i < n; i++) {
                backData[i] = readRegister(FIFODataReg);
            }
        }
    }

    return status;
}

int detectCard() {
    uint8_t reqData = PICC_REQIDL;
    uint8_t backData[16];
    int backLen;
    int status;

    writeRegister(BitFramingReg, 0x07);
    status = toCard(PCD_TRANSCEIVE, &reqData, 1, backData, &backLen);

    if ((status != MI_OK) || (backLen != 0x10)) {
        status = MI_ERR;
    }

    return status;
}

int readUID(uint8_t *uid) {
    uint8_t anticollData[2] = {PICC_ANTICOLL, 0x20};
    uint8_t backData[16];
    int backLen;
    int status;
    int i;

    writeRegister(BitFramingReg, 0x00);
    status = toCard(PCD_TRANSCEIVE, anticollData, 2, backData, &backLen);

    if (status == MI_OK && backLen == 0x28) {
        for (i = 0; i < 4; i++) {
            uid[i] = backData[i];
        }
        return MI_OK;
    }

    return MI_ERR;
}

// Trouver une carte par son UID
CarteJitsu* trouverCarte(uint8_t *uid) {
    int i;

    for (i = 0; i < nb_cartes; i++) {
        if (cartes[i].uid[0] == uid[0] &&
            cartes[i].uid[1] == uid[1] &&
            cartes[i].uid[2] == uid[2] &&
            cartes[i].uid[3] == uid[3]) {
            return &cartes[i];
        }
    }
    return NULL;
}

// Afficher une carte trouvée
void afficherCarte(CarteJitsu *carte) {
    printf("\n");
    printf("╔══════════════════════════════╗\n");
    printf("║        CARD JITSU            ║\n");
    printf("║══════════════════════════════║\n");
    printf("║ %-28s ║\n", carte->nom);
    printf("║                              ║\n");
    printf("║ Élément: %-19s ║\n", carte->element);
    printf("║ Valeur:  %-19d ║\n", carte->valeur);
    printf("║ Couleur: %-19s ║\n", carte->couleur);
    printf("║                              ║\n");
    printf("║ UID: %02X-%02X-%02X-%02X            ║\n",
           carte->uid[0], carte->uid[1], carte->uid[2], carte->uid[3]);
    printf("╚══════════════════════════════╝\n");
    printf("\n");
}

// Jouer la carte (logique de jeu à ajouter)
void jouerCarte(CarteJitsu *carte) {
    printf("\n=== CARTE JOUÉE ===\n");
    afficherCarte(carte);

    //TODO : implémenter la logique de jeu


    printf("Carte ajoutée au jeu!\n");
}

void stopReading(int sig) {
    continue_reading = 0;
    printf("\n🛑 Fin de la partie CardJitsu!\n");
}

int main() {
    uint8_t uid[4];
    uint8_t lastUID[4] = {0};
    CarteJitsu *carte;
    int i;

    signal(SIGINT, stopReading);

    if (wiringPiSetup() == -1) {
        printf("❌ Erreur WiringPi\n");
        return 1;
    }

    if (wiringPiSPISetup(CHANNEL, SPEED) < 0) {
        printf("❌ Erreur SPI\n");
        return 1;
    }

    initRFID();

    printf("🎴 ═══════════════════════════════════════\n");
    printf("          🥋 CARD JITSU 🥋\n");
    printf("     Lecteur RFID Ultra-Simple\n");
    printf("   ═══════════════════════════════════════\n");
    printf("📡 Lecteur initialisé\n");
    printf("🎯 %d cartes dans la base de données\n", nb_cartes);
    printf("🃏 Approchez une carte... (Ctrl+C pour arrêter)\n\n");

    while (continue_reading) {
        if (detectCard() == MI_OK) {
            if (readUID(uid) == MI_OK) {
                // Éviter les lectures multiples
                if (uid[0] != lastUID[0] || uid[1] != lastUID[1] ||
                    uid[2] != lastUID[2] || uid[3] != lastUID[3]) {

                    printf("📖 UID: %02X,%02X,%02X,%02X\n",
                           uid[0], uid[1], uid[2], uid[3]);

                    // Chercher la carte dans la base
                    carte = trouverCarte(uid);

                    if (carte != NULL) {
                        printf("✅ Carte reconnue!");
                        jouerCarte(carte);
                    } else {
                        printf("❓ Carte inconnue\n");
                        printf("💡 Pour l'ajouter, copiez cette ligne dans le tableau 'cartes[]':\n");
                        printf("   {{0x%02X, 0x%02X, 0x%02X, 0x%02X}, \"Nom de la carte\", \"Element\", valeur, \"Couleur\"},\n\n",
                               uid[0], uid[1], uid[2], uid[3]);
                    }

                    // Mémoriser l'UID
                    for (i = 0; i < 4; i++) {
                        lastUID[i] = uid[i];
                    }

                    delay(2000); // Anti-rebond
                }
            }
        }
        delay(100);
    }

    printf("👋 Merci d'avoir joué!\n");
    return 0;
}