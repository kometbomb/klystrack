TARGET := klystrack
ECHO := echo
CFG := debug
MACHINE := -march=pentium2 
NSIS := C:/program\ files\ \(x86\)/nsis/makensis.exe /V2 /NOCD
ZIP := pkzipc -exclude=.* -zipdate=newest -path=relative -silent -rec -dir -add
ZIPEXT := pkzipc -ext -silent
WGET := wget --quiet
REV := SubWCRev.exe .
UPLOAD := cmd.exe /c upload.bat
MAKEBUNDLE := ../klystron/tools/bin/makebundle.exe
DLLS := zip/data/SDL.dll zip/data/SDL_mixer.dll
DESTDIR ?= /usr
EXT := .c
CC := gcc
CDEP := gcc -E -MM
ARCHIVE := klystrack.zip
INSTALLER := klystrack.exe
MIXERVER := 1.2.11
SDLVER := 1.2.14

ifdef COMSPEC
	TARGET := klystrack.exe
	SDLFLAGS := -I /mingw/include/sdl
	SDLLIBS :=  -lmingw32 -lSDLmain -lSDL -lSDL_mixer 
	CFLAGS += -mthreads 
else
	DLLS = 
	SDLFLAGS := `sdl-config --cflags` -U_FORTIFY_SOURCE
	SDLLIBS := `sdl-config --libs` -lSDL_mixer
	REV := cp -f
endif

ifdef COMSPEC
	RES_PATH := .
	CONFIG_PATH := ~/.klystrack
else
	RES_PATH := $(DESTDIR)/lib/klystrack
	CONFIG_PATH := ~/.klystrack
endif

CFLAGS := $(MACHINE) -ftree-vectorize -std=gnu99 --no-strict-aliasing
LDFLAGS :=  -lmingw32 -L ../klystron/bin.$(CFG) -lengine_gfx -lengine_util -lengine_snd -lengine_gui -lSDLmain -lSDL -lSDL_mixer 
INCLUDEFLAGS := -I src $(SDLFLAGS) -I ../klystron/src -L../klystron/bin.$(CFG) -DRES_PATH="$(RES_PATH)" -DCONFIG_PATH="$(CONFIG_PATH)" $(EXTFLAGS) -DUSESDLMUTEXES -DENABLEAUDIODUMP -DSTEREOOUTPUT

DIRS := $(notdir $(wildcard src/*)) 

ifeq ($(CFG),debug)
 CFLAGS += -O3 -g -Wall -DDEBUG -fno-inline 
else
 ifeq ($(CFG),profile)
  CFLAGS += -O3 -pg -Wall 
 else
  ifeq ($(CFG),release)
   CFLAGS += -O3 -Wall -s -mwindows
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
	@$(CC) -static $(INCLUDEFLAGS) -c $(CFLAGS) -o $$@ $$<

deps/$(CFG)_$(2)%.d: src/$(1)%$(EXT)
	@mkdir -p deps
	@$(ECHO) "Generating dependencies for $$(notdir $$<)..."
	@set -e ; $(CDEP) $(INCLUDEFLAGS) $$< > $$@.$$$$$$$$; \
	sed 's,\($$*\)\.o[ :]*,objs.$(CFG)\/$(2)\1.o $$@ : ,g' \
		< $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$
endef

# root (i.e. src/*.c)
$(eval $(call directory_defs,,))
# subdirs (src/*/*.c)
$(foreach dir,$(DIRS),$(eval $(call directory_defs,$(dir)/,$(dir)_)))

.PHONY: zip all build nightly installer

build: Makefile src/version
	@echo '#ifndef VERSION_NUMBER' > src/version_number.h
	@echo '#define VERSION_NUMBER' >> src/version_number.h
	@echo -n '#define VERSION "' >> src/version_number.h
	@echo -n `cat src/version` >> src/version_number.h
	@echo '"' >> src/version_number.h
	@echo '#endif' >> src/version_number.h
ifdef COMSPEC
	@$(REV) ./src/version.in ./src/version.h
else
	@echo '#ifndef VERSION_H' > ./src/version.h
	@echo '#define VERSION_H' >> ./src/version.h
	@echo '#include "version_number.h"' >> ./src/version.h
	@echo -n '#define REVISION "r' >> ./src/version.h
	@svnversion -n . >> ./src/version.h
	@echo '"' >> ./src/version.h
	@echo '#define VERSION_STRING "klystrack " VERSION " " REVISION' >> ./src/version.h
	@echo '#endif' >> ./src/version.h
endif
	@make -C ../klystron CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"
	@make all CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"

all: bin.$(CFG)/$(TARGET) res/Default
	
zip: doc/* res/Default $(DLLS) examples/* linux/Makefile
	@make -C ../klystron CFG=release EXTFLAGS="$(EXTFLAGS)"
	@make build CFG=release
	@mkdir -p zip/data/res
	@mkdir -p zip/data/examples/songs
	@mkdir -p zip/data/examples/instruments
	@cp examples/songs/*.kt zip/data/examples/songs
	@cp examples/instruments/*.ki zip/data/examples/instruments
	@cp res/Default zip/data/res/Default
	@cp doc/LICENSE zip/data/LICENSE
	@cp doc/SDL.txt zip/data/SDL.txt
	@cp bin.release/$(TARGET) zip/data/$(TARGET)
ifdef COMSPEC
	@cd zip/data; rm -f ../$(ARCHIVE); $(ZIP) ../$(ARCHIVE) "*"
	@cp -f zip/klystrack.zip zip/klystrack-`cat src/version`-win32.zip
else
	@cp -f linux/Makefile zip/data
endif
	
installer: zip installer/klystrack.nsi
ifdef COMSPEC
	@$(NSIS) /DVERSION=`cat src/version` installer/klystrack.nsi
endif
	
nightly: zip
	@$(REV) ver.in ver.txt
	@cp zip/$(ARCHIVE) zip/klystrack-nightly-`cat ver.txt`-win32.zip
ifneq ($(UPLOAD),)
	@$(UPLOAD) zip/klystrack-nightly-`cat ver.txt`-win32.zip
endif
	@rm -f ver.txt

clean:
	@rm -rf deps objs.$(CFG) bin.$(CFG)

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
	
res/Default: data/bevel.bmp temp/8x8.fnt temp/7x6.fnt data/colors.txt
	@mkdir -p res
	@mkdir -p temp
	@cp -f data/colors.txt temp
	@cp -f data/bevel.bmp temp
	@$(MAKEBUNDLE) $@ temp

temp/8x8.fnt: data/font/*
	@mkdir -p temp
	@$(MAKEBUNDLE) $@ data/font

temp/7x6.fnt: data/font7x6/*
	@mkdir -p temp
	@$(MAKEBUNDLE) $@ data/font7x6

zip/data/SDL.dll:
	@$(ECHO) "Downloading SDL.dll..."
	@cd temp ; $(WGET) http://www.libsdl.org/release/SDL-$(SDLVER)-win32.zip ; $(ZIPEXT) SDL-$(SDLVER)-win32.zip SDL.dll ; rm SDL-$(SDLVER)-win32.zip
	@mkdir -p zip/data
	@mv temp/SDL.dll zip/data/SDL.dll
		
zip/data/SDL_mixer.dll:
	@$(ECHO) "Downloading SDL_mixer.dll..."
	@cd temp ; $(WGET) http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-$(MIXERVER)-win32.zip ; $(ZIPEXT) SDL_mixer-$(MIXERVER)-win32.zip SDL_mixer.dll ; rm SDL_mixer-$(MIXERVER)-win32.zip
	@mkdir -p zip/data
	@mv temp/SDL_mixer.dll zip/data/SDL_mixer.dll
