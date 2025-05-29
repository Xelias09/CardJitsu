//
// Created by alexis on 29/05/25.
//
#include <stdio.h>
#include <unistd.h>
#include "../../include/lcd_custom.h"

#define LCD_I2C_ADDR 0x21

int main() {
    int fd = lcd_setup(LCD_I2C_ADDR);
    if (fd < 0) {
        fprintf(stderr, "[ERREUR] Impossible d'initialiser l'écran LCD\n");
        return 1;
    }

    printf("[INFO] Test lcd_display_message avec texte court\n");
    lcd_display_message(fd, "Bonjour\nRaspberry");

    sleep(3);
    lcd_clear(fd);

    printf("[INFO] Test lcd_display_message avec texte long + scroll\n");
    lcd_display_message(fd, "Ceci est une ligne trop longue pour tenir sur une seule ligne ! \nCelle-ci egalement:)");

    sleep(3);
    lcd_clear(fd);

    printf("[INFO] Test scroll manuel court\n");
    lcd_scroll_text(fd, "Defilement force court", 1, 300);

    sleep(3);
    lcd_clear(fd);
    return 0;
}
