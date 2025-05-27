# Répertoires
SRC_DIR = src
BIN_DIR = bin
INCLUDE_DIR = data/gpio/include
LIB_DIR = data/gpio/lib

# Compilateur croisé
CC = data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

# Options de compilation
CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR) -lwiringPi -lwiringPiDev

# Cibles spécifiques
SEGMENT_SRC = $(SRC_DIR)/test_7segments.c
SEGMENT_BIN = $(BIN_DIR)/test_7segments

LCD_SRC = $(SRC_DIR)/test_lcd.c
LCD_BIN = $(BIN_DIR)/test_lcd

RFID_SRC = $(SRC_DIR)/test_rfid.c
RFID_BIN = $(BIN_DIR)/test_rfid

# Règles principales
all: segment lcd rfid

segment: $(SEGMENT_BIN)
lcd: $(LCD_BIN)
rfid: $(RFID_BIN)

$(SEGMENT_BIN): $(SEGMENT_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(LCD_BIN): $(LCD_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(RFID_BIN): $(RFID_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Création des dossiers si absents

$(BIN_DIR):
	mkdir -p $@


# Envoi des fichiers binaires sur la Raspberry Pi
upload: all
	scp $(SEGMENT_BIN) $(LCD_BIN) $(RFID_BIN) pi@172.20.10.4:/home/pi/Desktop/tests

# Nettoyage
clean:
	rm -f $(BIN_DIR)/*

.PHONY: all clean
