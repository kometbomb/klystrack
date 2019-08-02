TARGET := klystrack
KLYSTRON=klystron
ECHO := echo
CFG := debug
EXE := bin.$(CFG)/$(TARGET)
MACHINE :=
NSIS := C:/program\ files\ \(x86\)/nsis/makensis.exe -V2 -NOCD
CURL := curl
MAKEBUNDLE := $(KLYSTRON)/tools/bin/makebundle.exe
UPLOAD := cmd.exe /c upload.bat
DLLS := zip/data/SDL2_image.dll zip/data/SDL2.dll
EXT := .c
CC := gcc
CDEP := $(CC) -E -MM
ARCHIVE := klystrack
INSTALLER := klystrack.exe
SDL_VER := 2.0.10
SDL_IMAGEVER := 2.0.5
THEMES :=
REV := cp -f

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin
CFLAGS := $(MACHINE) -ftree-vectorize -std=gnu99 -Wno-strict-aliasing

ifdef COMSPEC
	# Compiling for Windows
	RES_PATH := .
	CFLAGS += -DRESOURCES_IN_BINARY_DIR
	CONFIG_PATH := ~/.klystrack
else
	# Not compiling for Windows
	RES_PATH = $(PREFIX)/lib/klystrack
	CONFIG_PATH := ~/.klystrack
	CFLAGS += -DCONFIG_DEFAULT_DISABLE_RENDER_TO_TEXTURE
endif

include klystron/common.mk

ifdef COMSPEC
	TARGET := $(TARGET).exe
	ARCHIVE := $(ARCHIVE).zip
	CFLAGS += -mthreads
	ZIP := zip -r ../$(ARCHIVE) .
	ZIPEXT := unzip
else
	ZIP := tar czf
	DLLS =
	ARCHIVE := $(ARCHIVE).tar.gz
endif

RESOURCES = $(subst themes,res,$(sort $(wildcard themes/*)))
KEYS = $(sort $(wildcard key/*))

EXTFLAGS := -DNOSDL_MIXER -DUSESDLMUTEXES -DENABLEAUDIODUMP -DSTEREOOUTPUT -DUSESDL_IMAGE -DUSESDL_RWOPS $(EXTFLAGS) $(CFLAGS)
LDFLAGS :=  -L $(KLYSTRON)/bin.$(CFG) -lengine_gfx -lengine_util -lengine_snd -lengine_gui -lm $(SDLLIBS)
INCLUDEFLAGS := -I src $(SDLFLAGS) -I $(KLYSTRON)/src -L$(KLYSTRON)/bin.$(CFG) -DRES_PATH="$(RES_PATH)" -DCONFIG_PATH="$(CONFIG_PATH)" $(EXTFLAGS) -DKLYSTRON=$(KLYSTRON)

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

CFLAGS += -Wno-strict-aliasing

# $(1) = subdir, $(2) = filename prefix (i.e. subdir without a slash)
define directory_defs
 SRC := $$(notdir $$(wildcard src/$(value 1)/*$(EXT)))
 DEP := $$(patsubst %$(EXT),deps/$(CFG)_$(2)%.d,$$(SRC))
 OBJ := $$(patsubst %$(EXT),objs.$(CFG)/$(2)%.o,$$(SRC))

 OBJS := $(OBJS) $$(OBJ)
 DEPS := $(DEPS) $$(DEP)

objs.$(CFG)/$(2)%.o: src/$(1)%$(EXT) src/version.h src/version_number.h
	$(MSG) "Compiling $$(notdir $$<)..."
	$(Q)mkdir -p objs.$(CFG)
	$(Q)$(CC) $(INCLUDEFLAGS) -c $(CFLAGS) -o $$@ $$<

deps/$(CFG)_$(2)%.d: src/$(1)%$(EXT) src/version.h src/version_number.h
	$(Q)mkdir -p deps
	$(MSG) "Generating dependencies for $$(notdir $$<)..."
	$(Q)set -e ; $(CDEP) $(INCLUDEFLAGS) $$< > $$@.$$$$$$$$; \
	sed 's,\($$*\)\.o[ :]*,objs.$(CFG)\/$(2)\1.o $$@ : ,g' \
		< $$@.$$$$$$$$ > $$@; \
	rm -f $$@.$$$$$$$$
endef

define theme_defs

 THEMES := $$(THEMES) res/$(1)

res/$(1): themes/$(1)/* #themes/$(1)/font/* themes/$(1)/font7x6/* themes/$(1)/tiny/*
	@$(ECHO) "Building theme $(1)..."
	$(Q)mkdir -p res
	$(Q)mkdir -p themetemp.$(1)
	-$(Q)if test -e themes/$(1)/colors.txt; then cp -f themes/$(1)/colors.txt themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/bevel.*; then cp -f themes/$(1)/bevel.* themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/vu.*; then cp -f themes/$(1)/vu.* themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/analyzor.*; then cp -f themes/$(1)/analyzor.* themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/logo.*; then cp -f themes/$(1)/logo.* themetemp ; fi
	-$(Q)if test -e themes/$(1)/catometer.*; then cp -f themes/$(1)/catometer.* themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/cursor.*; then cp -f themes/$(1)/cursor.* themetemp.$(1) ; fi
	-$(Q)if test -e themes/$(1)/icon.*; then cp -f themes/$(1)/icon.* themetemp.$(1) ; fi
	-$(Q)if test -d themes/$(1)/font; then $(MAKEBUNDLE) themetemp.$(1)/8x8.fnt themes/$(1)/font ; fi
	-$(Q)if test -d themes/$(1)/font7x6; then $(MAKEBUNDLE) themetemp.$(1)/7x6.fnt themes/$(1)/font7x6 ; fi
	-$(Q)if test -d themes/$(1)/tiny; then $(MAKEBUNDLE) themetemp.$(1)/4x6.fnt themes/$(1)/tiny ; fi
	$(Q)-$(MAKEBUNDLE) $$@ themetemp.$(1)
	$(Q)rm -rf themetemp.$(1)

endef

build: Makefile src/version.h src/version_number.h
	$(Q)touch src/version
	$(Q)$(MAKE) -C $(KLYSTRON) CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"
	$(Q)$(MAKE) all CFG=$(CFG) EXTFLAGS="$(EXTFLAGS)"

src/version.h: src/version
	$(Q)echo '#ifndef VERSION_H' > ./src/version.h
	$(Q)echo '#define VERSION_H' >> ./src/version.h
	$(Q)echo '#include "version_number.h"' >> ./src/version.h
	$(Q)echo '#define REVISION "' | tr -d '\n'  >> ./src/version.h
	$(Q)date +"%Y%m%d" | tr -d '\n' >> ./src/version.h
	$(Q)echo '"' >> ./src/version.h
	$(Q)echo '#define VERSION_STRING "klystrack " VERSION " " REVISION' >> ./src/version.h
	$(Q)echo '#endif' >> ./src/version.h

src/version_number.h: src/version
	$(Q)echo '#ifndef VERSION_NUMBER' > src/version_number.h
	$(Q)echo '#define VERSION_NUMBER' >> src/version_number.h
	$(Q)echo '#define VERSION "' | tr -d '\n' >> src/version_number.h
	$(Q)cat src/version | tr -d '\r\n' | tr -d '\n' >> src/version_number.h
	$(Q)echo '"' >> src/version_number.h
	$(Q)echo '#endif' >> src/version_number.h

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

all: $(EXE) $(THEMES)

zip: doc/* $(THEMES) $(DLLS) examples/instruments/* examples/songs/* $(DLLS)
	$(Q)$(MAKE) -C $(KLYSTRON) CFG=release EXTFLAGS="$(EXTFLAGS)"
	$(Q)$(MAKE) build CFG=release
	$(Q)mkdir -p zip/data/res
	$(Q)mkdir -p zip/data/examples/songs
	$(Q)mkdir -p zip/data/examples/songs/n00bstar-examples
	$(Q)mkdir -p zip/data/examples/instruments
	$(Q)mkdir -p zip/data/examples/instruments/n00bstar-instruments
	$(Q)cp examples/songs/*.kt zip/data/examples/songs
	$(Q)cp examples/songs/n00bstar-examples/*.kt zip/data/examples/songs/n00bstar-examples
	$(Q)cp examples/instruments/*.ki zip/data/examples/instruments
	$(Q)cp examples/instruments/n00bstar-instruments/*.ki zip/data/examples/instruments/n00bstar-instruments
	$(Q)cp res/* zip/data/res
	$(Q)mkdir -p zip/data/key
	$(Q)cp key/* zip/data/key
	$(Q)cp LICENSE zip/data/LICENSE
	$(Q)cp doc/SDL.txt zip/data/SDL.txt
	$(Q)cp doc/SDL_image.txt zip/data/SDL_image.txt
	$(Q)cp doc/Default.kt zip/data/Default.kt
	$(Q)cp bin.release/$(TARGET) zip/data/$(TARGET)
ifdef COMSPEC
	cd zip/data; rm -f ../$(ARCHIVE); $(ZIP) zip
	$(Q)cp -f zip/klystrack.zip zip/klystrack-`cat src/version | tr -d '\r\n'`-win32.zip
else
	-$(Q)rm -f zip/data/Makefile
	cd zip; cp -r data klystrack-`cat ../src/version | tr -d '\r\n'` ; rm -f $(ARCHIVE); $(ZIP) klystrack-`cat ../src/version`.tar.gz klystrack-`cat ../src/version` ; rm -rf klystrack-`cat ../src/version`
	$(Q)cp -f linux/Makefile zip/data
endif

installer: zip installer/klystrack.nsi
ifdef COMSPEC
	$(NSIS) -DVERSION=`cat src/version | tr -d '\r\n'` installer/klystrack.nsi
endif

nightly: zip
	$(Q)cp zip/$(ARCHIVE) zip/klystrack-nightly-`date +"%Y%m%d" | tr -d '\n'`-win32.zip

clean:
	$(MAKE) -C klystron clean
	$(Q)rm -rf deps objs.debug objs.release objs.profile bin.release bin.debug bin.profile zip temp res

$(EXE): $(OBJS)
	@$(ECHO) "Linking $(TARGET)..."
	$(Q)mkdir -p bin.$(CFG)
	$(Q)$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

release: bin.release/$(TARGET)
	@$(ECHO) "Building release -->"

$(DESTDIR)$(BINDIR)/$(TARGET): $(EXE)
	install -D -m 755 $< $@

$(DESTDIR)$(RES_PATH)/res/%: res/%
	install -D -m 644 $< $@

$(DESTDIR)$(RES_PATH)/key/%: key/%
	install -D -m 644 $< $@

install: $(EXE:bin.$(CFG)/%=$(DESTDIR)$(BINDIR)/%) $(RESOURCES:res/%=$(DESTDIR)$(RES_PATH)/res/%) $(KEYS:key/%=$(DESTDIR)$(RES_PATH)/key/%)

#bin.release/$(TARGET):
#	$(Q)make CFG=release

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

zip/data/SDL2_image.dll:
	@$(ECHO) "Downloading "$@"..."
	$(Q)mkdir -p temp
	$(Q)cd temp ; $(CURL) -O http://www.libsdl.org/projects/SDL_image/release/SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip ; $(ZIPEXT) SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip SDL2_image.dll libpng16-16.dll zlib1.dll ; rm SDL2_image-$(SDL_IMAGEVER)-win32-x86.zip
	$(Q)mkdir -p zip/data
	$(Q)mv temp/SDL2_image.dll zip/data/SDL2_image.dll
	$(Q)mv temp/libpng16-16.dll zip/data/libpng16-16.dll
	$(Q)mv temp/zlib1.dll zip/data/zlib1.dll

zip/data/SDL2.dll:
	@$(ECHO) "Downloading "$@"..."
	$(Q)mkdir -p temp
	$(Q)cd temp ; $(CURL) -O https://www.libsdl.org/release/SDL2-$(SDL_VER)-win32-x86.zip ; $(ZIPEXT) SDL2-$(SDL_VER)-win32-x86.zip SDL2.dll ; rm SDL2-$(SDL_VER)-win32-x86.zip
	$(Q)mkdir -p zip/data
	$(Q)mv temp/SDL2.dll zip/data/SDL2.dll


objs.$(CFG)/windres.o: windres/* icon/*
	windres -i windres/resource.rc -o $@
