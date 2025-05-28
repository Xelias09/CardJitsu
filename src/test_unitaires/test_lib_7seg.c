//
// Created by alexis on 28/05/25.
//

#include <stdio.h>
#include <unistd.h>
#include "../../include/7seg.h"

int main() {
    int fd = init_display();
    if (fd == -1) {
        perror("Erreur lors de l'initialisation de l'afficheur");
        return 1;
    }

    printf("Test : Affichage des nombres de 0 à 9...\n");

    int i;
    for (i = 0; i <= 9; i++) {
        display_number(fd, i);
        printf("Affichage : %d\n", i);
        sleep(1);
    }

    printf("Test : Affichage d'un nombre à 4 chiffres (2025)...\n");
    display_number(fd, 2025);
    sleep(2);

    printf("Effacement de l'affichage.\n");
    clear_display(fd);

    return 0;
}
