//
// Created by alexis on 27/05/25.
//

#include "../include/protocole.h"

#include <string.h>

void serialiser_joueur(char *buffer, int code, joueur *j) {
    // On formate les données du joueur dans le buffer
    sprintf(buffer, "%d:%s,%s,%d,%d,%d,",
            code,
            j->nom,
            j->UID,
            j->status,
            j->rang,
            j->exp);
}

int deserialiser_message(char *buffer, int *code, char *data[], int *nb_data) {
    *nb_data = 0;

    // Découpe le code
    char *code_str = strtok(buffer, ":");
    if (code_str == NULL) return -1;

    *code = atoi(code_str);

    // Récupère les données
    char *token = strtok(NULL, ",");
    while (token != NULL && *nb_data < MAX_PARTS) {
        data[*nb_data] = strdup(token);
        (*nb_data)++;
        token = strtok(NULL, ",");
    }

    return 0;
}
