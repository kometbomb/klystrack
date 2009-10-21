# linux installer makefile

TARGET=klystrack
RES_PATH = $(DESTDIR)/usr/local/lib/klystrack

install: 
	@mkdir -p $(DESTDIR)/usr/local/bin
	@cp -f $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
	@mkdir -p $(RES_PATH)
	@cp -Rf res $(RES_PATH)
	@cp -Rf examples $(RES_PATH)
	@mkdir -p $(DESTDIR)/usr/local/share/klystrack
	@cp LICENSE $(DESTDIR)/usr/local/share/klystrack
	@cp SDL.txt $(DESTDIR)/usr/local/share/klystrack
	
clean:
