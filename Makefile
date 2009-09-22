TARGET=mused.exe
VPATH=src:src
ECHO = echo
CFG = debug
MACHINE = -march=pentium4 -mfpmath=sse -msse3 

# The directories containing the source files, separated by ':'


# The source files: regardless of where they reside in 
# the source tree, VPATH will locate them...
Group0_SRC = $(notdir ${wildcard src/*.c})

# Build a Dependency list and an Object list, by replacing 
# the .cpp extension to .d for dependency files, and .o for 
# object files.
Group0_DEP = $(patsubst %.c, deps/Group0_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.c, objs.$(CFG)/Group0_%.o, ${Group0_SRC}) ../klystron/objs.$(CFG)/Group0_music.o ../klystron/objs.$(CFG)/Group0_cyd.o ../klystron/objs.$(CFG)/Group0_cydflt.o ../klystron/objs.$(CFG)/Group2_rnd.o\
../klystron/objs.$(CFG)/Group2_bundle.o ../klystron/objs.$(CFG)/Group1_font.o ../klystron/objs.$(CFG)/Group1_gfx.o


	
# What compiler to use for generating dependencies: 
# it will be invoked with -MM
CXX = gcc -std=gnu99 --no-strict-aliasing
CXXDEP = gcc -E -std=gnu99

# What include flags to pass to the compiler
INCLUDEFLAGS= -I ../Common -I src -I /mingw/include/sdl -I ../klystron/src 

CXXFLAGS = $(MACHINE) -mthreads -ftree-vectorize

# Separate compile options per configuration
ifeq ($(CFG),debug)
CXXFLAGS += -O3 -g -Wall ${INCLUDEFLAGS} -DDEBUG -fno-inline 
else
ifeq ($(CFG),profile)
CXXFLAGS += -O3 -g -pg -Wall ${INCLUDEFLAGS}
else
ifeq ($(CFG),release)
CXXFLAGS += -O3 -Wall ${INCLUDEFLAGS} -s
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

# A common link flag for all configurations
LDFLAGS = -lmingw32 -lSDLmain -lSDL -lSDL_mixer -lcomdlg32

all:	inform bin.$(CFG)/$(TARGET)

inform:
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/${TARGET}: $(Group0_OBJ) | inform
	@mkdir -p bin.$(CFG)
	$(CXX) $(CXXFLAGS) -o $@ $^ ${LDFLAGS}

objs.$(CFG)/Group0_%.o: %.c
	@mkdir -p objs.$(CFG)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

deps/Group0_%.d: %.c
	@mkdir -p deps
	@$(ECHO) "Generating dependencies for $<"
	@set -e ; $(CXXDEP) -MM $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/Group0_\1.o $@ : ,g' \
		< $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -rf deps objs.$(CFG) bin.$(CFG)

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first 
# invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif
