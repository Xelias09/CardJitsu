# Makefile du projet
# Definitions des chemins
SRC = ./src
BIN_DIR = ./bin
OBJDIR = ./build
LIB_DIR = ./data/gpio/lib

# ADRESSE IP DU RASPI
ADDRPI1 = 172.20.10.4
ADDRPI2 = 172.20.10.6
ADDRPISRV = 172.20.10.2

# Compilateur croisé
CC = ./data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

# Options de compilation
CFLAGS = -I$(INCLUDE_DIR) -I/include
LDFLAGS = -L$(LIB_DIR) -l7seg -lwiringPi -lwiringPiDev -llcd_custom -lmatrice -lrfid -ldata -lsocket -lgameplay -lprotocole -lpthread -lihm

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

SSHPASSCLI1 = sshpass -p 'raspberry'
uploadcli1 : all
	$(SSHPASSCLI1) scp $(BIN_DIR)/* pi@$(ADDRPI1):/home/pi/Desktop/tests

SSHPASSCLI2 = sshpass -p 'raspberry'
uploadcli2 : all
	$(SSHPASSCLI2) scp $(BIN_DIR)/* pi@$(ADDRPI2):/home/pi/Desktop/tests

SSHPASSSRV = sshpass -p 'objetco'
upload_eti : all
	$(SSHPASSSRV) scp $(BIN_DIR)/* etienne@$(ADDRPISRV):/home/etienne/Desktop/tests

clean:
	rm -f $(BIN_DIR)/*
	rm -f $(OBJDIR)/*