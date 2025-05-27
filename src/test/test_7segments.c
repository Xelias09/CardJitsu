//
// Created by alexis on 26/05/25.
//
#include <wiringPiI2C.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define HT16K33_ADDR 0x70

// Table des segments pour les chiffres 0 à 9
unsigned char digits[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

int main(int argc, char *argv[]) {
    int i;

    if (argc != 2) {
        printf("Utilisation : %s <chiffre 0-9>\n", argv[0]);
        return 1;
    }

    int digit = atoi(argv[1]);
    if (digit < 0 || digit > 9) {
        printf("Erreur : veuillez entrer un chiffre entre 0 et 9.\n");
        return 1;
    }

    int fd = wiringPiI2CSetup(HT16K33_ADDR);
    if (fd == -1) {
        perror("Erreur init I2C");
        return 1;
    }

    // Initialisation du HT16K33
    wiringPiI2CWrite(fd, 0x21); // Oscillateur ON
    wiringPiI2CWrite(fd, 0x81); // Affichage ON, blink OFF
    wiringPiI2CWrite(fd, 0xEF); // Luminosité max

    // Efface l'affichage
    for (i = 0; i < 6; i++) {
        wiringPiI2CWriteReg8(fd, i * 2, 0x00);
    }

    // Affiche le chiffre à la première position (gauche)
    wiringPiI2CWriteReg8(fd, 0x00, digits[digit]);
    usleep(100000);

    return 0;
}
