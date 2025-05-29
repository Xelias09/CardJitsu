#include <wiringPi.h>
#include <lcd.h>
#include <mcp23008.h>
#include <stdio.h>
#include <unistd.h>

#define ADRESSELCD 0x21
#define AF_BASE 100

#define AF_RS  (AF_BASE + 1)
#define AF_STRB (AF_BASE + 2)
#define AF_DB4 (AF_BASE + 3)
#define AF_DB5 (AF_BASE + 4)
#define AF_DB6 (AF_BASE + 5)
#define AF_DB7 (AF_BASE + 6)
#define AF_BACKLIGHT_PIN (AF_BASE + 7)

int main() {
    int lcd;

    printf("[DEBUG] Initialisation de WiringPi...\n");
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "[ERREUR] wiringPiSetup a échoué\n");
        return 1;
    }

    printf("[DEBUG] Mapping MCP23008 sur base %d, adresse I2C 0x%X...\n", AF_BASE, ADRESSELCD);
    mcp23008Setup(AF_BASE, ADRESSELCD);

    pinMode(AF_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(AF_BACKLIGHT_PIN, HIGH); // Allume le rétroéclairage

    printf("[DEBUG] Initialisation de l'écran LCD...\n");
    lcd = lcdInit(2, 16, 4,
                  AF_RS, AF_STRB,
                  AF_DB4, AF_DB5, AF_DB6, AF_DB7,
                  0, 0, 0, 0);
    if (lcd < 0) {
        fprintf(stderr, "[ERREUR] lcdInit a échoué\n");
        return 1;
    }

    lcdClear(lcd);
    lcdPosition(lcd, 0, 0);
    lcdPuts(lcd, "JoyPi LCD");
    lcdPosition(lcd, 0, 1);
    lcdPuts(lcd, "via MCP23008");

    printf("[INFO] Affichage terminé avec succès.\n");

    sleep(10);
    lcdClear(lcd);
    return 0;
}
