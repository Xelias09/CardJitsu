# ADRESSE IP DU RASPI
ADDRPI = 192.168.1.3

# Compilateur croisé
CC = ../data/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc
CFLAGS = -fPIC -I../include -I../data/gpio/include
OBJDIR = ../build
LIBDIR = ../data/gpio/lib

# Sources originales
SRCS = 7seg.c rfid.c lcd.c

OBJS = $(SRCS:%.c=$(OBJDIR)/%.o)
TARGETS = $(SRCS:%.c=$(LIBDIR)/lib%.so)
LDFLAGS = -L../data/gpio/lib -lwiringPi -lwiringPiDev

# Cibles principales (ajout de libcardjitsu.so)
all: lib7seg.so librfid.so liblcd.so libcardjitsu.so

# Règles existantes
lib7seg.so: $(OBJDIR)/7seg.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/lib7seg.so $< $(LDFLAGS)

librfid.so: $(OBJDIR)/rfid.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/librfid.so $< $(LDFLAGS)

liblcd.so: $(OBJDIR)/lcd.o | $(LIBDIR)
	$(CC) -shared -o $(LIBDIR)/liblcd.so $< $(LDFLAGS)

# Compilation des objets
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Création des répertoires
$(OBJDIR):
	mkdir -p $@

$(LIBDIR):
	mkdir -p $@

# SSH pour upload
SSHPASS = sshpass -p 'raspberry'

# Upload
upload: all
	$(SSHPASS) scp ../data/gpio/lib/lib7seg.so pi@$(ADDRPI):/home/pi/Desktop/lib/
	$(SSHPASS) scp ../data/gpio/lib/librfid.so pi@$(ADDRPI):/home/pi/Desktop/lib/
	$(SSHPASS) scp ../data/gpio/lib/libcardjitsu.so pi@$(ADDRPI):/home/pi/Desktop/lib/

# Nettoyage étendu
clean:
	rm -f $(OBJS)
	rm -f $(LIBDIR)/liblcd.so $(LIBDIR)/lib7seg.so $(LIBDIR)/librfid.so $(LIBDIR)/libcardjitsu.so
	rm -f test_rfid test_cardjitsu test_quick