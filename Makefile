TARGET := klystrack
ECHO := echo
CFG := debug
MACHINE := 
NSIS := C:/program\ files\ \(x86\)/nsis/makensis.exe /V2 /NOCD
WGET := wget --quiet
MAKEBUNDLE := ../klystron/tools/bin/makebundle.exe
UPLOAD := cmd.exe /c upload.bat
DLLS := zip/data/SDL2_image.dll zip/data/SDL2.dll
DESTDIR ?= /usr
EXT := .c
CC := gcc -Werror 
CDEP := gcc -E -MM
ARCHIVE := klystrack
INSTALLER := klystrack.exe
SDL_VER := 2.0.3
SDL_IMAGEVER := 2.0.0
THEMES :=
REV := cp -f

CFLAGS := $(MACHINE) -ftree-vectorize -std=gnu99 --no-strict-aliasing

ifdef COMSPEC
	TARGET := $(TARGET).exe
	ARCHIVE := $(ARCHIVE).zip
	SDLFLAGS := -I c:/mingw/include/SDL2
	SDLLIBS :=  -lSDL2main -lSDL2 -lSDL2_image -lwinmm
	CFLAGS += -mthreads 
	ZIP := pkzipc -exclude=.* -zipdate=newest -path=relative -silent -rec -dir -add
	ZIPEXT := pkzipc -ext -silent
else
	ZIP := tar czf
	DLLS = 
	ARCHIVE := $(ARCHIVE).tar.gz
	SDLFLAGS := `sdl2-config --cflags` -U_FORTIFY_SOURCE
	SDLLIBS := `sdl2-config --libs` -lSDL2_image
endif

ifdef COMSPEC
	RES_PATH := .
	CFLAGS += -DRESOURCES_IN_BINARY_DIR
	CONFIG_PATH := ~/.klystrack
else
	RES_PATH ?= $(DESTDIR)/lib/klystrack
	CONFIG_PATH := ~/.klystrack
endif

EXTFLAGS := -DNOSDL_MIXER -DUSESDLMUTEXES -DENABLEAUDIODUMP -DSTEREOOUTPUT -DUSESDL_IMAGE $(EXTFLAGS)
LDFLAGS :=  -L ../klystron/bin.$(CFG) -lengine_gfx -lengine_util -lengine_snd -lengine_gui -lm $(SDLLIBS) 
INCLUDEFLAGS := -I src $(SDLFLAGS) -I ../klystron/src -L../klystron/bin.$(CFG) -DRES_PATH="$(RES_PATH)" -DCONFIG_PATH="$(CONFIG_PATH)" $(EXTFLAGS)

ifdef COMSPEC
	LDFLAGS := -lmingw32 $(LDFLAGS)
endif

DIRS := $(notdir $(wildcard src/*)) 
THEMEDIRS := $(notdir $(wildcard themes/*)) 

ifeq ($(CFG),debug)
 CFLAGS += -g -Wall -DDEBUG -fno-inline 
else
 ifeq ($(CFG),profile)
  CFLAGS += -O3 -pg -Wall 
 else
  ifeq ($(CFG),release)
   CFLAGS += -O3 -Wall -s 
   ifdef COMSPEC
	 CFLAGS += -mwindows
   endif
  else
   @$(ECHO) "Invalid configuration "$(CFG)" specified."
   @$(ECHO) "Possible choices for configuration are "
   @$(ECHO) "'release', 'profile' and 'debug'"
   @exit 1
  endif
 endif
endif

# $(1) = subdir, $(2) = filename prefix (i.e. subdir without a slash)
define directory_defs
 SRC := $$(notdir $$(wildcard src/$(value 1)/*$(EXT))) 
 DEP := $$(patsubst %$(EXT),deps/$(CFG)_$(2)%.d,$$(SRC))
 OBJ := $$(patsubst %$(EXT),objs.$(CFG)/$(2)%.o,$$(SRC))
 
 OBJS := $(OBJS) $$(OBJ)
 DEPS := $(DEPS) $$(DEP)
 
objs.$(CFG)/$(2)%.o: src/$(1)%$(EXT)
	@$(ECHO) "Compiling $$(notdir $$<)..."
	@mkdir -p objs.$(CFG)
	@$(CC) $(INCLUDEFLAGS) -c $(CFLAGS) -o $$@ $$<

deps/$(CFG)_$(2)%.d: src/$(1)%$(EXT)
	@mkdir -p deps
	@$(ECHO) "Generating dependencies for $$(notdir $$<)..."
	@set -e ; $(CDEP) $(INCLUDEFLAGS) $$< > $$@.$$$$$$$$; \
	sed 's,\($$*\)\.o[ :]*,objs.$(CFG)\/$(2)\1.o $$@ : ,g' \
		< $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$
endef

define theme_defs	

 THEMES := $$(THEMES) res/$(1)

res/$(1): themes/$(1)/* #themes/$(1)/font/* themes/$(1)/font7x6/* themes/$(1)/tiny/* 
	@$(ECHO) "Building theme $(1)..."
	@mkdir -p res
	@mkdir -p themetemp
	@-cp -f themes/$(1)/colors.txt themetemp
	@-cp -f themes/$(1)/bevel.* themetemp
	@-cp -f themes/$(1)/vu.* themetemp
	@-cp -f themes/$(1)/analyzor.* themetemp
	@-cp -f themes/$(1)/logo.* themetemp
	@-cp -f themes/$(1)/catometer.* themetemp
	-@if test -d themes/$(1)/font; then $(MAKEBUNDLE) themetemp/8x8.fnt themes/$(1)/font ; fi
	-@if test -d themes/$(1)/font7x6; then $(MAKEBUNDLE) themetemp/7x6.fnt themes/$(1)/font7x6 ; fi
	-@if test -d themes/$(1)/tiny; then $(MAKEBUNDLE) themetemp/4x6.fnt themes/$(1)/tiny ; fi
	@-$(MAKEBUNDLE) $$@ themetemp
	@rm -rf themetemp
	
endef

build: Makefile src/version.h src/version_number.h
	@touch src/version
	@make -C ../klystron CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"
	@make all CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"

src/version.h: src/version
	@echo '#ifndef VERSION_H' > ./src/version.h
	@echo '#define VERSION_H' >> ./src/version.h
	@echo '#include "version_number.h"' >> ./src/version.h
	@echo -n '#define REVISION "' >> ./src/version.h
	@date +"%Y%m%d" | tr -d '\n' >> ./src/version.h
	@echo '"' >> ./src/version.h
	@echo '#define VERSION_STRING "klystrack " VERSION " " REVISION' >> ./src/version.h
	@echo '#endif' >> ./src/version.h
	
src/version_number.h: src/version
	@echo '#ifndef VERSION_NUMBER' > src/version_number.h
	@echo '#define VERSION_NUMBER' >> src/version_number.h
	@echo -n '#define VERSION "' >> src/version_number.h
	@echo -n `cat src/version` >> src/version_number.h
	@echo '"' >> src/version_number.h
	@echo '#endif' >> src/version_number.h

# root (i.e. src/*.c)
$(eval $(call directory_defs,,))
# subdirs (src/*/*.c)
$(foreach dir,$(DIRS),$(eval $(call directory_defs,$(dir)/,$(dir)_)))
# themes
$(foreach dir,$(THEMEDIRS),$(eval $(call theme_defs,$(dir))))

ifdef COMSPEC
  OBJS += objs.$(CFG)/windres.o
endif
  
.PHONY: zip all build nightly installer

all: bin.$(CFG)/$(TARGET) $(THEMES)
	
zip: doc/* $(THEMES) $(DLLS) examples/instruments/* examples/songs/* linux/Makefile $(DLLS)
	@make -C ../klystron CFG=release EXTFLAGS="$(EXTFLAGS)"
	@make build CFG=release
	@mkdir -p zip/data/res
	@mkdir -p zip/data/examples/songs
	@mkdir -p zip/data/examples/instruments
	@cp examples/songs/*.kt zip/data/examples/songs
	@cp examples/instruments/*.ki zip/data/examples/instruments
	@cp res/* zip/data/res
	@mkdir -p zip/data/key
	@cp key/* zip/data/key
	@cp doc/LICENSE zip/data/LICENSE
	@cp doc/SDL.txt zip/data/SDL.txt
	@cp doc/SDL_image.txt zip/data/SDL_image.txt
	@cp bin.release/$(TARGET) zip/data/$(TARGET)
ifdef COMSPEC
	@cd zip/data; rm -f ../$(ARCHIVE); $(ZIP) ../$(ARCHIVE) "*"
	@cp -f zip/klystrack.zip zip/klystrack-`cat src/version`-win32.zip
else
	-@rm -f zip/data/Makefile
	cd zip; cp -r data klystrack-`cat ../src/version` ; rm -f $(ARCHIVE); $(ZIP) klystrack-`cat ../src/version`.tar.gz klystrack-`cat ../src/version` ; rm -rf klystrack-`cat ../src/version`
	@cp -f linux/Makefile zip/data
endif
	
installer: zip installer/klystrack.nsi
ifdef COMSPEC
	@$(NSIS) /DVERSION=`cat src/version` installer/klystrack.nsi
endif
	
nightly: zip
	@cp zip/$(ARCHIVE) zip/klystrack-nightly-`date +"%Y%m%d" | tr -d '\n'`-win32.zip
	
clean:
	@rm -rf deps objs.debug objs.release objs.profile bin.release bin.debug bin.profile zip temp res

bin.$(CFG)/$(TARGET): $(OBJS)
	@$(ECHO) "Linking $(TARGET)..."
	@mkdir -p bin.$(CFG)
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

release: bin.release/$(TARGET)
	@$(ECHO) "Building release -->"
	
#bin.release/$(TARGET): 
#	@make CFG=release
	
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

zip/data/SDL2_image.dll:
	@$(ECHO) "Downloading "$@"..."
	@mkdir -p temp
	@cd temp ; $(WGET) http://www.libsdl.org/projects/SDL_image/release/SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip ; $(ZIPEXT) SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip SDL2_image.dll libpng16-16.dll zlib1.dll ; rm SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip
	@mkdir -p zip/data
	@mv temp/SDL2_image.dll zip/data/SDL2_image.dll
	@mv temp/libpng16-16.dll zip/data/libpng16-16.dll
	@mv temp/zlib1.dll zip/data/zlib1.dll

zip/data/SDL2.dll:
	@$(ECHO) "Downloading "$@"..."
	@mkdir -p temp
	@cd temp ; $(WGET) https://www.libsdl.org/release/SDL2-$(SDL_VER)-win32-x86.zip ; $(ZIPEXT) SDL2-$(SDL_VER)-win32-x86.zip SDL2.dll ; rm SDL2-$(SDL_VER)-win32-x86.zip
	@mkdir -p zip/data
	@mv temp/SDL2.dll zip/data/SDL2.dll
	
	
objs.$(CFG)/windres.o: windres/* icon/*
	windres -i windres/resource.rc -o $@
	