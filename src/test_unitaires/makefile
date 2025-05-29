# ADRESSE IP DU RASPI
ADDRPI = 172.20.10.4

# Répertoires
TEST_SRC_DIR = .
LIB_TEST_SRC_DIR = ../test_lib
BIN_DIR = ../../bin
INCLUDE_DIR = ../../data/gpio/include
LIB_DIR = ../../data/gpio/lib

# Compilateur croisé
CC = ../../data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

# Options de compilation
CFLAGS = -I$(INCLUDE_DIR) -I../../include
LDFLAGS = -L$(LIB_DIR) -l7seg -lwiringPi -lwiringPiDev -llcd_custom

# Fichiers sources de test
SEGMENT_SRC = $(TEST_SRC_DIR)/test_7segments.c
SEGMENT_BIN = $(BIN_DIR)/test_7segments

LCD_SRC = $(TEST_SRC_DIR)/test_lcd.c
LCD_BIN = $(BIN_DIR)/test_lcd

RFID_SRC = $(TEST_SRC_DIR)/test_rfid.c
RFID_BIN = $(BIN_DIR)/test_rfid

LIB7SEG_SRC = $(LIB_TEST_SRC_DIR)/test_lib_7seg.c
LIB7SEG_BIN = $(BIN_DIR)/test_lib_7seg

LIBLCD_SRC = $(LIB_TEST_SRC_DIR)/test_lib_lcd.c
LIBLCD_BIN = $(BIN_DIR)/test_lib_lcd

# Règles principales
all: segment lcd rfid lib7seg liblcd

segment: $(SEGMENT_BIN)
lcd: $(LCD_BIN)
rfid: $(RFID_BIN)
lib7seg: $(LIB7SEG_BIN)
liblcd: $(LIBLCD_BIN)

$(SEGMENT_BIN): $(SEGMENT_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(LCD_BIN): $(LCD_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(RFID_BIN): $(RFID_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(LIB7SEG_BIN): $(LIB7SEG_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(LIBLCD_BIN): $(LIBLCD_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $@

SSHPASS = sshpass -p 'raspberry'
upload: all
	$(SSHPASS) scp $(BIN_DIR)/test_* pi@$(ADDRPI):/home/pi/Desktop/tests

clean:
	rm -f $(BIN_DIR)/test_*

.PHONY: all clean upload segment lcd rfid lib7seg
