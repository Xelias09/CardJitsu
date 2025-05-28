#include "../data/gpio/include/wiringPiI2C.h"
#include "../include/7seg.h"

#define HT16K33_ADDR 0x70

// Table des segments pour les chiffres 0 à 9
static const unsigned char digits[10] = {
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

// Mapping des adresses utilisées par les digits (en évitant les deux-points)
static const int digit_addresses[4] = {0x00, 0x02, 0x06, 0x08};

int init_display(void) {
    int fd = wiringPiI2CSetup(HT16K33_ADDR);
    if (fd == -1)
        return -1;

    wiringPiI2CWrite(fd, 0x21); // Oscillateur ON
    wiringPiI2CWrite(fd, 0x81); // Affichage ON, blink OFF
    wiringPiI2CWrite(fd, 0xE7); // Luminosité ~50%

    clear_display(fd);
    return fd;
}

void clear_display(int fd) {
    int i;
    for (i = 0; i < 16; i++) {
        wiringPiI2CWriteReg8(fd, i, 0x00);
    }
}

void display_digit(int fd, int pos, int digit) {
    if (digit < 0 || digit > 9 || pos < 0 || pos > 3) return;
    wiringPiI2CWriteReg8(fd, digit_addresses[pos], digits[digit]);
}

void display_number(int fd, int number) {
    int i;
    if (number < 0 || number > 9999) return;

    for (i = 3; i >= 0; i--) {
        int digit = number % 10;
        display_digit(fd, i, digit);
        number /= 10;
    }
}
