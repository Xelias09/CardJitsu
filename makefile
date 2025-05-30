# Makefile du projet
# Definitions des chemins
BIN_DIR = ./bin
OBJDIR = ./build
LIBDIR = ./data/gpio/lib

clean:
	rm -f $(BIN_DIR)/test_*
	rm -f $(OBJDIR)/*
	rm -f $(LIBDIR)/liblcd_custom.so $(LIBDIR)/lib7seg.so $(LIBDIR)/librfid.so $(LIBDIR)/libmatrice.so