#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/protocole.h"
#include "../include/socket_utils.h"
#include "../include/rfid.h"

#define SERVER_IP "172.20.10.2"
#define SERVER_PORT 50000

int main() {
    if (initRFID() != 0) {
        fprintf(stderr, "Erreur initialisation RFID\n");
        return 1;
    }

    socket_t sockClient = connecterClt2Srv(SERVER_IP, SERVER_PORT);
    printf("Connecté au serveur %s:%d\n", SERVER_IP, SERVER_PORT);

    while (1) {
        uint8_t uid[4];
        int f_rfid;
        while (!f_rfid) {
            printf("Attente détection carte");
            while (detectCard() != MI_OK);
            if (readUID(uid)==MI_OK) {
                f_rfid=1;
            }
            sleep(2);
        }
        char buffer[256];

        sprintf(buffer,"%s:%02X%02X%02X%02X", CMD_CONNECT, uid[0], uid[1], uid[2], uid[3]);
        printf("Carte détectée: %02X %02X %02X %02X\n", uid[0], uid[1], uid[2], uid[3]);

        // Envoyer UID au serveur
        send(sockClient.fd, buffer, sizeof(buffer)-1, 0);

        memset(buffer, 0, sizeof(buffer));
        // Recevoir réponse

        int recu = recv(sockClient.fd, buffer, sizeof(buffer) - 1, 0);
        if (recu > 0) {
            buffer[recu] = '\0';
            printf("Réponse serveur:\n%s\n", buffer);
        } else {
            printf("Erreur réception réponse serveur\n");
        }

        sleep(2);
    }
    fermerSocket(sockClient);
    return 0;
}
