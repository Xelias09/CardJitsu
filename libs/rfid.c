#include "rfid.h"
#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// Fonctions de base pour la communication SPI
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

// Initialisation du module RFID
int initRFID(void) {
    // Initialiser WiringPi
    if (wiringPiSetup() == -1) {
        return -1;
    }

    // Initialiser SPI
    if (wiringPiSPISetup(CHANNEL, SPEED) < 0) {
        return -2;
    }

    // Reset du module
    writeRegister(CommandReg, PCD_RESETPHASE);
    delay(50);

    // Configuration des registres
    writeRegister(TxModeReg, 0x00);
    writeRegister(RxModeReg, 0x00);
    writeRegister(ModWidthReg, 0x26);
    writeRegister(TModeReg, 0x80);
    writeRegister(TPrescalerReg, 0xA9);
    writeRegister(TReloadRegH, 0x03);
    writeRegister(TReloadRegL, 0xE8);
    writeRegister(TxASKReg, 0x40);
    writeRegister(ModeReg, 0x3D);

    // Activer l'antenne
    setBitMask(TxControlReg, 0x03);

    return 0;
}

// Communication avec les cartes PICC
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

// Détecter la présence d'une carte
int detectCard(void) {
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

// Lire l'UID d'une carte
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

// Comparer deux UIDs
int compareUID(uint8_t *uid1, uint8_t *uid2) {
    int i;
    for (i = 0; i < 4; i++) {
        if (uid1[i] != uid2[i]) {
            return 0; // Différents
        }
    }
    return 1; // Identiques
}

// Trouver une carte par son UID dans la base de données
CarteJitsu* trouverCarte(uint8_t *uid, CarteJitsu *cartes, int nb_cartes) {
    int i;

    for (i = 0; i < nb_cartes; i++) {
        if (compareUID(cartes[i].uid, uid)) {
            return &cartes[i];
        }
    }
    return NULL;
}

// Afficher une carte trouvée avec un joli format
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

// Afficher les informations d'une carte inconnue
void afficherUIDInconnu(uint8_t *uid) {
    printf("❓ Carte inconnue\n");
    printf("💡 Pour l'ajouter, copiez cette ligne dans le tableau 'cartes[]':\n");
    printf("   {{0x%02X, 0x%02X, 0x%02X, 0x%02X}, \"Nom de la carte\", \"Element\", valeur, \"Couleur\"},\n\n",
           uid[0], uid[1], uid[2], uid[3]);
}