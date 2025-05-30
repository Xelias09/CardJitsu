# ADRESSE IP DU RASPI
ADDRPI = 172.20.10.4

# Compilateur croisé
CC = ../data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

CFLAGS = -Wall -fPIC -I../include -I../data/gpio/include
OBJDIR = ../build
LIBDIR = ../data/gpio/lib

SRCS = 7seg.c rfid.c lcd_custom.c matrice.c  # ajouter ici d'autres fichiers sources au besoin
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
TARGETS = $(SRCS:%.c=$(LIBDIR)/lib%.so)

LDFLAGS = -L../data/gpio/lib -lwiringPi -lwiringPiDev

all: lib7seg.so liblcd_custom.so libmatrice.so librfid.so

# Règles existantes
lib7seg.so: $(OBJDIR)/7seg.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/lib7seg.so $< $(LDFLAGS)

librfid.so: $(OBJDIR)/rfid.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/librfid.so $< $(LDFLAGS)

liblcd_custom.so: $(OBJDIR)/lcd_custom.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/liblcd_custom.so $< $(LDFLAGS)

libmatrice.so: $(OBJDIR)/matrice.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libmatrice.so $< $(LDFLAGS)

# Compilation des objets
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

$(LIBDIR):
	mkdir -p $@

SSHPASS = sshpass -p 'raspberry'
upload: all
	$(SSHPASS) scp ../data/gpio/lib/* pi@$(ADDRPI):/home/pi/Desktop/lib/


clean:
	rm -f $(OBJS)
	rm -f $(LIBDIR)/liblcd_custom.so $(LIBDIR)/lib7seg.so $(LIBDIR)/librfid.so $(LIBDIR)/libmatrice.so $(LIBDIR)/libcardjitsu.so

