ifdef COMSPEC
 SDL := -lSDLmain -lSDL -lSDL_mixer -I /mingw/include/sdl 
 LIBS := -lmingw32 -lengine_snd 
else
 SDL := `sdl-config --libs` -lSDL_mixer
 LIBS := -lengine_snd 
endif

player.exe: player.c
	gcc -DUSESDLMUTEXES -DSTEREOOUTPUT -DENABLEAUDIODUMP -o player.exe player.c -g -Wall $(LIBS) $(SDL) -I ../../klystron/src -L ../../klystron/bin.debug
