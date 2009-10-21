# linux installer makefile

TARGET=klystrack
DESTDIR?=/usr/local
RES_PATH = $(DESTDIR)/lib/klystrack

install: 
	@mkdir -p $(DESTDIR)/usr/bin
	@cp -f $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
	@mkdir -p $(RES_PATH)
	@cp -Rf res $(RES_PATH)
	@cp -Rf examples $(RES_PATH)
	@mkdir -p $(DESTDIR)/usr/share/klystrack
	@cp LICENSE $(DESTDIR)/usr/share/klystrack
	@cp SDL.txt $(DESTDIR)/usr/share/klystrack
	
clean:
