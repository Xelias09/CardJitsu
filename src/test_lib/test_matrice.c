//
// Created by inas on 29/05/25.
//
#include <stdio.h>
#include "../../data/gpio/include/wiringPi.h"

#include "../../include/matrice.h"

int main() {
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "Erreur d'initialisation de WiringPi.\n");
        return 1;
    }

    matrice_init();

    const uint8_t feu[8] = {
        0x18, 0x18, 0x3C, 0x7E,
        0x7E, 0x3C, 0x28, 0x00
    };

    const uint8_t neige[8] = {
        0x42, 0xC3, 0x24, 0x18,
        0x18, 0x24, 0xC3, 0x42
    };

    const uint8_t eau[8] = {
        0x18, 0x18, 0x3C, 0x3C,
        0x7E, 0x7E, 0x3C, 0x18
    };

    matrice_display(feu);
    delay(2000);

    matrice_display(neige);
    delay(2000);

    matrice_display(eau);
    delay(2000);

    matrice_clear();
    return 0;
}