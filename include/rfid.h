#ifndef MFRC522_H
#define MFRC522_H

#include <stdint.h>

// Configuration SPI
#define CHANNEL 0
#define SPEED 1000000

// MFRC522 Commands
#define PCD_IDLE       0x00
#define PCD_TRANSCEIVE 0x0C
#define PCD_RESETPHASE 0x0F

// MFRC522 Registers
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

// PICC Commands
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

// Fonctions de base RFID
int initRFID(void);
void writeRegister(uint8_t reg, uint8_t value);
uint8_t readRegister(uint8_t reg);
void setBitMask(uint8_t reg, uint8_t mask);
void clearBitMask(uint8_t reg, uint8_t mask);

// Fonctions de communication avec les cartes
int toCard(uint8_t command, uint8_t *sendData, int sendLen, uint8_t *backData, int *backLen);
int detectCard(void);
int readUID(uint8_t *uid);

// Fonctions de gestion des cartes CardJitsu
CarteJitsu* trouverCarte(uint8_t *uid, CarteJitsu *cartes, int nb_cartes);
void afficherCarte(CarteJitsu *carte);
void afficherUIDInconnu(uint8_t *uid);

// Utilitaires
int compareUID(uint8_t *uid1, uint8_t *uid2);

#endif // MFRC522_H