TARGET=klystrack
ARCHIVE = klystrack.zip
VPATH=src:src
ECHO = echo
CFG = debug
MACHINE = -march=pentium4 -mfpmath=sse -msse3 
ZIP = pkzipc -zipdate=newest -path=relative -silent -rec -dir -add
ZIPEXT = pkzipc -ext -silent
WGET = wget --quiet
REV = SubWCRev.exe .
UPLOAD = cmd.exe /c upload.bat
MAKEBUNDLE = ../klystron/tools/bin/makebundle.exe
DLLS = zip/data/SDL.dll zip/data/SDL_mixer.dll
DESTDIR?=/usr

ifdef COMSPEC
RES_PATH = .
else
RES_PATH = $(DESTDIR)/lib/klystrack
endif

# The directories containing the source files, separated by ':'


# The source files: regardless of where they reside in 
# the source tree, VPATH will locate them...
Group0_SRC = $(notdir ${wildcard src/*.c})

# Build a Dependency list and an Object list, by replacing 
# the .cpp extension to .d for dependency files, and .o for 
# object files.
Group0_DEP = $(patsubst %.c, deps/Group0_$(CFG)_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.c, objs.$(CFG)/Group0_%.o, ${Group0_SRC}) 

ifdef COMSPEC
TARGET =klystrack.exe
SDLFLAGS = -I /mingw/include/sdl
SDLLIBS =  -lmingw32 -lSDLmain -lSDL -lSDL_mixer -mthreads 
else
DLLS = 
SDLFLAGS = `sdl-config --cflags` -U_FORTIFY_SOURCE
SDLLIBS = `sdl-config --libs` -lSDL_mixer
REV = cp -f
endif

INCLUDEFLAGS= -I src $(SDLFLAGS) -I ../klystron/src -L../klystron/bin.$(CFG) -DRES_PATH="$(RES_PATH)"
	
# What compiler to use for generating dependencies: 
# it will be invoked with -MM
CC = gcc -std=gnu99 --no-strict-aliasing
CDEP = gcc -E -std=gnu99

ifndef CFLAGS
CFLAGS = $(MACHINE) -ftree-vectorize
endif

# Separate compile options per configuration
ifeq ($(CFG),debug)
CFLAGS += -O3 -g -Wall ${INCLUDEFLAGS} -DDEBUG -fno-inline $(DEBUGPARAMS)
else
ifeq ($(CFG),profile)
CFLAGS += -O3 -g -pg -Wall ${INCLUDEFLAGS}
else
ifeq ($(CFG),release)
CFLAGS += -O3 -Wall ${INCLUDEFLAGS} -s
ifdef COMSPEC
CFLAGS += -mwindows
endif
else
@$(ECHO) "Invalid configuration "$(CFG)" specified."
@$(ECHO) "You must specify a configuration when "
@$(ECHO) "running make, e.g. make CFG=debug"
@$(ECHO) "Possible choices for configuration are "
@$(ECHO) "'release', 'profile' and 'debug'"
@exit 1
exit
endif
endif
endif

build: Makefile src/version
	@echo '#ifndef VERSION_NUMBER' > src/version_number.h
	@echo '#define VERSION_NUMBER' >> src/version_number.h
	@echo -n '#define VERSION "' >> src/version_number.h
	@echo -n `cat src/version` >> src/version_number.h
	@echo '"' >> src/version_number.h
	@echo '#endif' >> src/version_number.h
ifdef COMSPEC
	$(REV) ./src/version.in ./src/version.h
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
	make -C ../klystron CFG=$(CFG)
	make all CFG=$(CFG)

all:	inform bin.$(CFG)/$(TARGET) res/data

.PHONY: zip all build nightly

zip: doc/* res/data $(DLLS) examples/* linux/Makefile
	make -C ../klystron CFG=release
	make CFG=release
	@mkdir -p zip/data/res
	@mkdir -p zip/data/examples/songs
	cp -R examples/songs/*.sng zip/data/examples/songs
	cp res/data zip/data/res/data
	cp doc/LICENSE zip/data/LICENSE
	cp doc/SDL.txt zip/data/SDL.txt
	cp bin.release/$(TARGET) zip/data/$(TARGET)
ifdef COMSPEC
	cd zip/data; $(ZIP) ../$(ARCHIVE) "*"
	cp -f zip/klystrack.zip zip/klystrack-`cat src/version`-win32.zip
else
	cp -f linux/Makefile zip/data
endif
	
nightly: zip
	$(REV) ver.in ver.txt
	cp zip/$(ARCHIVE) zip/klystrack-nightly-`cat ver.txt`-win32.zip
ifneq ($(UPLOAD),)
	$(UPLOAD) zip/klystrack-nightly-`cat ver.txt`-win32.zip
endif
	rm -f ver.txt

inform:
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/${TARGET}: $(Group0_OBJ) | inform
	@mkdir -p bin.$(CFG)
	$(CC) $(CFLAGS) -o $@ $^ -lengine_gfx -lengine_snd -lengine_util ${SDLLIBS}

objs.$(CFG)/Group0_%.o: %.c
	@mkdir -p objs.$(CFG)
	$(CC) $(CFLAGS) -c -o $@ $<

deps/Group0_$(CFG)_%.d: %.c
	@mkdir -p deps
	@$(ECHO) "Generating dependencies for $<"
	@set -e ; $(CDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/Group0_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$
	
res/data: data/bevel.bmp temp/8x8.fnt temp/7x6.fnt data/colors.txt
	@mkdir -p res
	@mkdir -p temp
	cp -f data/colors.txt temp
	cp -f data/bevel.bmp temp
	$(MAKEBUNDLE) res/data temp

temp/8x8.fnt: data/font/*
	@mkdir -p temp
	$(MAKEBUNDLE) temp/8x8.fnt data/font

temp/7x6.fnt: data/font7x6/*
	@mkdir -p temp
	$(MAKEBUNDLE) temp/7x6.fnt data/font7x6

zip/data/SDL.dll:
	cd temp ; $(WGET) http://www.libsdl.org/release/SDL-1.2.14-win32.zip ; $(ZIPEXT) SDL-1.2.14-win32.zip SDL.dll ; rm SDL-1.2.14-win32.zip
	@mkdir -p zip/data
	mv temp/SDL.dll zip/data/SDL.dll
		
zip/data/SDL_mixer.dll:
	cd temp ; $(WGET) http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.8-win32.zip ; $(ZIPEXT) SDL_mixer-1.2.8-win32.zip SDL_mixer.dll ; rm SDL_mixer-1.2.8-win32.zip
	@mkdir -p zip/data
	mv temp/SDL_mixer.dll zip/data/SDL_mixer.dll
		
clean:
	@rm -rf deps objs.release objs.debug objs.profile bin.release bin.debug bin.profile res temp zip ver.txt

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first 
# invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif
