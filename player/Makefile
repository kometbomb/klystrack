ifdef COMSPEC
 SDL := -lSDL2main -lSDL2 -I /mingw/include/SDL2
 LIBS := -lmingw32 -lengine_snd 
else
 SDL := `sdl-config --libs` 
 LIBS := -lengine_snd 
endif

player.exe: player.c
	gcc -DUSESDLMUTEXES -DSTEREOOUTPUT -DENABLEAUDIODUMP -DNOSDL_MIXER -DDEBUG -o player.exe player.c -g -Wall $(LIBS) $(SDL) -I ../../klystron/src -L ../../klystron/bin.debug
