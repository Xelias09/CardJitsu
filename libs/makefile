# ADRESSE IP DU RASPI
ADDRPI = 172.20.10.4
ADDRPISRV = 172.20.10.2
# Compilateur croisé
CC = ../data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

CFLAGS = -Wall -fPIC -I../include -I../data/gpio/include
OBJDIR = ../build
LIBDIR = ../data/gpio/lib

SRCS = 7seg.c rfid.c lcd_custom.c matrice.c data.c socket_utils.c gameplay.c protocole.c  IHM.c# ajouter ici d'autres fichiers sources au besoin
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
TARGETS = $(SRCS:%.c=$(LIBDIR)/lib%.so)

LDFLAGS = -L../data/gpio/lib -lwiringPi -lwiringPiDev

all: libdata.so lib7seg.so liblcd_custom.so libmatrice.so librfid.so libsocket.so libgameplay.so libprotocole.so libihm.so

# Règles existantes
libdata.so: $(OBJDIR)/data.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libdata.so $< $(LDFLAGS)

lib7seg.so: $(OBJDIR)/7seg.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/lib7seg.so $< $(LDFLAGS)

librfid.so: $(OBJDIR)/rfid.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/librfid.so $< $(LDFLAGS)

liblcd_custom.so: $(OBJDIR)/lcd_custom.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/liblcd_custom.so $< $(LDFLAGS)

libmatrice.so: $(OBJDIR)/matrice.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libmatrice.so $< $(LDFLAGS)

libsocket.so: $(OBJDIR)/socket_utils.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libsocket.so $< $(LDFLAGS)

libgameplay.so: $(OBJDIR)/gameplay.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libgameplay.so $< $(LDFLAGS)

libprotocole.so: $(OBJDIR)/protocole.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libprotocole.so $< $(LDFLAGS)

libihm.so: $(OBJDIR)/IHM.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libihm.so $< $(LDFLAGS)

# Compilation des objets
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

$(LIBDIR):
	mkdir -p $@

SSHPASS1 = sshpass -p 'raspberry'
uploadcli: all
	$(SSHPASS1) scp ../data/gpio/lib/* pi@$(ADDRPI):/home/pi/Desktop/lib/

SSHPASS2 = sshpass -p 'objetco'
uploadsrv: all
	$(SSHPASS2) scp ../data/gpio/lib/* etienne@$(ADDRPISRV):/home/etienne/Desktop/lib/

clean:
	rm -f $(OBJS)
	rm -f $(LIBDIR)/liblcd_custom.so $(LIBDIR)/lib7seg.so $(LIBDIR)/librfid.so $(LIBDIR)/libmatrice.so
	rm -f $(LIBDIR)/libcardjitsu.so $(LIBDIR)/libprotocole.so $(LIBDIR)/libsocket.so $(LIBDIR)/libihm.so