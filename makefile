# Makefile du projet
# Definitions des chemins
SRC = ./src
BIN_DIR = ./bin
OBJDIR = ./build
LIB_DIR = ./data/gpio/lib

# ADRESSE IP DU RASPI
ADDRPIJOY = 172.20.10.6
ADDRPI = 172.20.10.2

# Compilateur croisé
CC = ./data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

# Options de compilation
CFLAGS = -I$(INCLUDE_DIR) -I/include
LDFLAGS = -L$(LIB_DIR) -l7seg -lwiringPi -lwiringPiDev -llcd_custom -lmatrice -lrfid -ldata -lsocket -lgameplay -lprotocole -lpthread

# Fichiers client / server
CLIENT_SRC = $(SRC)/client.c
CLIENT_BIN = $(BIN_DIR)/client

SERVER_SRC = $(SRC)/serveur.c
SERVER_BIN = $(BIN_DIR)/serveur

all: client serveur

client: $(CLIENT_BIN)
serveur: $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(SERVER_BIN): $(SERVER_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

SSHPASS1 = sshpass -p 'raspberry'
upload_joy :
	$(SSHPASS1) scp $(BIN_DIR)/* pi@$(ADDRPIJOY):/home/pi/Desktop/tests

SSHPASS2 = sshpass -p 'objetco'
upload_eti :
	$(SSHPASS2) scp $(BIN_DIR)/* etienne@$(ADDRPI):/home/etienne/Desktop/tests

clean:
	rm -f $(BIN_DIR)/test_*
	rm -f $(OBJDIR)/*
	rm -f $(LIB_DIR)/liblcd_custom.so $(LIB_DIR)/lib7seg.so $(LIB_DIR)/librfid.so $(LIB_DIR)/libmatrice.so