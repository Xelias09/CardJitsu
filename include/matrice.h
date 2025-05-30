//
// Created by inas on 29/05/25.
//

#ifndef MATRICE_H
#define MATRICE_H

#include <stdint.h>
void wiringPiSPISetup(uint8_t uint8, uint32_t uint32);
void wiringPiSPIDataRW(uint8_t uint8, uint8_t * str, int i);

void matrice_init();
void matrice_clear();

void matrice_display();

#endif //MATRICE_H
