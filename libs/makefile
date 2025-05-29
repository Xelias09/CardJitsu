# ADRESSE IP DU RASPI
ADDRPI = 172.20.10.12

# Compilateur croisé
CC = ../data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

CFLAGS = -Wall -fPIC -I../include -I../data/gpio/include
OBJDIR = ../build
LIBDIR = ../data/gpio/lib

SRCS = 7seg.c rfid.c lcd.c matrice.c  # ajouter ici d'autres fichiers sources au besoin
OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
TARGETS = $(SRCS:%.c=$(LIBDIR)/lib%.so)

LDFLAGS = -L../data/gpio/lib -lwiringPi -lwiringPiDev

all: libmatrice.so

lib7seg.so: $(OBJDIR)/7seg.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/lib7seg.so $< $(LDFLAGS)

librfid.so: $(OBJDIR)/rfid.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/librfid.so $< $(LDFLAGS)

liblcd.so: $(OBJDIR)/lcd.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/liblcd.so $< $(LDFLAGS)

libmatrice.so: $(OBJDIR)/matrice.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/libmatrice.so $< $(LDFLAGS)

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $@

$(LIBDIR):
	mkdir -p $@

SSHPASS = sshpass -p 'raspberry'
upload:
	$(SSHPASS) scp ../data/gpio/lib/* pi@$(ADDRPI):/home/pi/Desktop/lib/
clean:
	rm -f $(OBJS)
	rm -f $(LIBDIR)/liblcd.so $(LIBDIR)/lib7seg.so $(LIBDIR)/librfid.so $(LIBDIR)/libmatrice.so

