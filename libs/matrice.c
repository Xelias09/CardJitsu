//
// Created by inas on 29/05/25.
//

#include "../include/matrice.h"
#include "../include/data.h"

static const uint8_t SPI_CHANNEL = 1;
static const uint32_t SPI_SPEED = 1000000;

static void max7219_send(uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = { reg, data };
    wiringPiSPIDataRW(SPI_CHANNEL, buffer, 2);
}

void matrice_init() {
    wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED);
    max7219_send(0x0B, 0x07); // REG_SCAN_LIMIT
    max7219_send(0x09, 0x00); // REG_DECODE_MODE
    max7219_send(0x0C, 0x01); // REG_SHUTDOWN
    max7219_send(0x0F, 0x00); // REG_DISPLAYTEST
    max7219_send(0x0A, 0x07); // REG_INTENSITY (luminosité moyenne)
}

int i;
void matrice_clear() {
    for (i = 0; i < 8; i++) {
        max7219_send(0x01 + i, 0x00); // Efface chaque ligne
    }
}

void matrice_display(const uint8_t pattern[8]) {
    for (i = 0; i < 8; i++) {
        max7219_send(0x01 + i, pattern[i]); // Affiche chaque ligne du motif
    }
}

void afficher_face_a_face(int elem1, int elem2) {
    const uint8_t* motif = NULL;

    if (elem1 == FEU && elem2 == GLACE) {
        motif = FEU_NEIGE;
    } else if (elem1 == FEU && elem2 == FEU) {
        motif = FEU_FEU;
    } else if (elem1 == FEU && elem2 == EAU) {
        motif = FEU_EAU;
    } else if (elem1 == EAU && elem2 == FEU) {
        motif = EAU_FEU;
    } else if (elem1 == EAU && elem2 == EAU) {
        motif = EAU_EAU;
    } else if (elem1 == EAU && elem2 == GLACE) {
        motif = EAU_NEIGE;
    } else if (elem1 == GLACE && elem2 == FEU) {
        motif = NEIGE_FEU;
    } else if (elem1 == GLACE && elem2 == EAU) {
        motif = NEIGE_EAU;
    } else if (elem1 == GLACE && elem2 == GLACE) {
        motif = NEIGE_NEIGE;
    }

    if (motif != NULL) {
        matrice_display(motif);
    } else {
        matrice_clear();  // Optionnel : si combinaison invalide
    }
}