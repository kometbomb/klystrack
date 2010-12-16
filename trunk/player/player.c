/*

Example klystrack command line replayer. Use as you like.
Usage: player.exe <song>

*/

/* SDL stuff */

#include "SDL.h"
#include "SDL_mixer.h"

/* klystron stuff */

#include "snd/cyd.h"
#include "snd/music.h"

#undef main

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <song>\n", argv[0]);
		return 1;
	}
	
	MusSong song;
	CydEngine cyd;
	MusEngine mus;
	
	memset(&song, 0, sizeof(song));
	
	/* To be sure, let's init enough channels */
	
	cyd_init(&cyd, 44100, MUS_MAX_CHANNELS);
	
	if (!mus_load_song(argv[1], &song, cyd.wavetable_entries))
	{
		fprintf(stderr, "Could not open %s\n", argv[1]);
		cyd_deinit(&cyd);
		return 2;
	}

	SDL_Init(SDL_INIT_AUDIO);
	Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
		
	/* We need only one channel since Cyd does the mixing */
	
	Mix_AllocateChannels(1);
			
	/* Notify the music engine about Cyd */
	
	mus_init_engine(&mus, &cyd);
	
	/* Add Cyd in SDL_Mixer audio output queue */ 
	
	cyd_register(&cyd);     
	
	/* Start updating the music engine at the rate set in the song */
	
	cyd_set_callback(&cyd, mus_advance_tick, &mus, song.song_rate);
	
	/* Start playing from position 0 */
	
	mus_set_song(&mus, &song, 0);
	
	printf("Playing %s...\n\nPress any key to exit.\n", song.title);
	
	int done = 0;
	
	while (!done)
	{
		SDL_Event e;
		
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
				case SDL_KEYDOWN:
					done = 1;
					break;
			}
		}
		
		SDL_Delay(10);
	}
	
	printf("Quit.\n");
	
	cyd_unregister(&cyd); 
	
	cyd_deinit(&cyd);
	
	Mix_CloseAudio();
	
	cyd_unregister(&cyd);
	cyd_deinit(&cyd);
	
	mus_free_song(&song);
	
	SDL_Quit();
	
	return 0;
}
