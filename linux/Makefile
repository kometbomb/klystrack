# linux installer makefile

TARGET=klystrack
DESTDIR?=/usr/local
RES_PATH = $(DESTDIR)/lib/klystrack

install: 
	cp -f $(TARGET) $(DESTDIR)/bin/$(TARGET)
	mkdir -p $(RES_PATH)
	cp -Rf res $(RES_PATH)
	cp -Rf examples $(RES_PATH)
	mkdir -p $(DESTDIR)/share/klystrack
	cp LICENSE $(DESTDIR)/share/klystrack
	cp SDL.txt $(DESTDIR)/share/klystrack