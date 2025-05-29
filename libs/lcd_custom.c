#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include "../data/gpio/include/wiringPi.h"
#include "../data/gpio/include/lcd.h"
#include "../data/gpio/include/mcp23008.h"
#include "../include/lcd_custom.h"

static pthread_mutex_t lcd_mutex = PTHREAD_MUTEX_INITIALIZER;

#define AF_BASE 100
#define AF_RS  (AF_BASE + 1)
#define AF_STRB (AF_BASE + 2)
#define AF_DB4 (AF_BASE + 3)
#define AF_DB5 (AF_BASE + 4)
#define AF_DB6 (AF_BASE + 5)
#define AF_DB7 (AF_BASE + 6)
#define AF_BACKLIGHT_PIN (AF_BASE + 7)

typedef struct {
    int fd;
    char text[64];
    int row;
    int delay_ms;
} scroll_args_t;

int lcd_setup(int i2c_addr) {
    int lcd_fd;

    if (wiringPiSetup() == -1)
        return -1;

    mcp23008Setup(AF_BASE, i2c_addr);

    pinMode(AF_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(AF_BACKLIGHT_PIN, HIGH);

    lcd_fd = lcdInit(2, 16, 4,
                     AF_RS, AF_STRB,
                     AF_DB4, AF_DB5, AF_DB6, AF_DB7,
                     0, 0, 0, 0);

    return lcd_fd;
}

// Fonction interne : scroll seulement si texte > 16 caractères
static void _lcd_scroll_line(int lcd_fd, const char *text, int row, int delay_ms) {
    int len = strlen(text);
    char buffer[17];

    if (row < 0 || row > 1) return;

    if (len <= 16) {
        pthread_mutex_lock(&lcd_mutex);
        lcdPosition(lcd_fd, 0, row);
        lcdPuts(lcd_fd, text);
        pthread_mutex_unlock(&lcd_mutex);
    } else {
        int start;
        for (start = 0; start <= len - 16; start++) {
            strncpy(buffer, text + start, 16);
            buffer[16] = '\0';
            pthread_mutex_lock(&lcd_mutex);
            lcdPosition(lcd_fd, 0, row);
            lcdPuts(lcd_fd, buffer);
            pthread_mutex_unlock(&lcd_mutex);
            delay(delay_ms);
        }
    }

}

// Fonction exécutable par un thread : appelle _lcd_scroll_line
static void *scroll_thread(void *args_void) {
    scroll_args_t *args = (scroll_args_t *)args_void;
    _lcd_scroll_line(args->fd, args->text, args->row, args->delay_ms);
    free(args); // libération mémoire automatique
    return NULL;
}

// Affiche un message sur 2 lignes, avec scroll intelligent (en parallèle)
void lcd_display_message(int lcd_fd, const char *message) {
    char line1[64] = "", line2[64] = "";
    const char *newline = strchr(message, '\n');

    if (newline) {
        strncpy(line1, message, newline - message);
        line1[newline - message] = '\0';
        strncpy(line2, newline + 1, 63);
        line2[63] = '\0';
    } else {
        strncpy(line1, message, 63);
        line1[63] = '\0';
    }

    lcdClear(lcd_fd);

    // Préparation des threads
    scroll_args_t *args1 = malloc(sizeof(scroll_args_t));
    scroll_args_t *args2 = malloc(sizeof(scroll_args_t));

    if (!args1 || !args2) return;

    strncpy(args1->text, line1, 63); args1->text[63] = '\0';
    args1->fd = lcd_fd; args1->row = 0; args1->delay_ms = 300;

    strncpy(args2->text, line2, 63); args2->text[63] = '\0';
    args2->fd = lcd_fd; args2->row = 1; args2->delay_ms = 300;

    pthread_t t1, t2;
    pthread_create(&t1, NULL, scroll_thread, args1);
    pthread_create(&t2, NULL, scroll_thread, args2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

// Fonction publique : scroll forcé, même si le texte est court
void lcd_scroll_text(int lcd_fd, const char *text, int row, int delay_ms) {
    int len = strlen(text);
    char buffer[17];

    if (row < 0 || row > 1) return;

    int start;
    for (start = 0; start <= len - 16; start++) {
        strncpy(buffer, text + start, 16);
        buffer[16] = '\0';
        lcdPosition(lcd_fd, 0, row);
        lcdPuts(lcd_fd, buffer);
        delay(delay_ms);
    }
}

void lcd_clear(int lcd_fd) {
    lcdClear(lcd_fd);
}
