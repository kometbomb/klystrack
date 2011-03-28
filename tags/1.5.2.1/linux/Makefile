# linux installer makefile

TARGET=klystrack
RES_PATH = $(DESTDIR)/usr/lib/klystrack

install: 
	@mkdir -p $(DESTDIR)/usr/bin
	@cp -f $(TARGET) $(DESTDIR)/usr/bin/$(TARGET)
	@mkdir -p $(RES_PATH)
	@cp -Rf res $(RES_PATH)
	@cp -Rf examples $(RES_PATH)
	@mkdir -p $(DESTDIR)/usr/share/doc/klystrack
	@cp LICENSE $(DESTDIR)/usr/share/doc/klystrack
	@cp SDL.txt $(DESTDIR)/usr/share/doc/klystrack
	@cp SDL_image.txt $(DESTDIR)/usr/share/doc/klystrack
	
clean:
