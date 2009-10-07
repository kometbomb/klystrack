/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "SDL.h"
#include "SDL_mixer.h"
#include "gfx/gfx.h"
#include "snd/music.h"
#include "toolutil.h"
#include "copypaste.h"
#include "toolutil.h"
#include "diskop.h"
#include "event.h"
#include "view.h"
#include "slider.h"
#include "action.h"
#include "mouse.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#include "mused.h"
Mused mused;

/*---*/

int stat_song_position;
int stat_pattern_position[MUS_CHANNELS];
MusPattern *stat_pattern[MUS_CHANNELS];
int stat_pattern_number[MUS_CHANNELS];


static const View instrument_view_tab[] =
{
	{{4, 0, 162, 10}, instrument_name_view},
	{{0, 10, 158, SCREEN_HEIGHT-10}, instrument_view},
	{{158, 10, SCREEN_WIDTH - 158 - SCROLLBAR, 5*8 }, instrument_list},
	{{158, 10 + 9*11, SCREEN_WIDTH - 158 - SCROLLBAR, SCREEN_HEIGHT-(10 + 5*8)}, program_view},
	{{SCREEN_WIDTH - SCROLLBAR, 10 + 5*8, SCROLLBAR, SCREEN_HEIGHT-(10 + 5*8)}, slider, &mused.program_slider_param},
	{{SCREEN_WIDTH - SCROLLBAR, 10, SCROLLBAR, 5*8 }, slider, &mused.instrument_list_slider_param},
	{{0, 0, 0, 0}, NULL}
};

static const View pattern_view_tab[] =
{
	{{0, 0, SCREEN_WIDTH-SCROLLBAR, SCREEN_HEIGHT}, pattern_view},
	{{SCREEN_WIDTH-SCROLLBAR, 0, SCROLLBAR, SCREEN_HEIGHT}, slider, &mused.pattern_slider_param},
	{{0, 0, 0, 0}, NULL}
};

static const View sequence_view_tab[] =
{
	{{0, 0, SCREEN_WIDTH-SCROLLBAR, SCREEN_HEIGHT}, sequence_view},
	{{SCREEN_WIDTH-SCROLLBAR, 0, SCROLLBAR, SCREEN_HEIGHT}, slider, &mused.sequence_slider_param},
	{{0, 0, 0, 0}, NULL}
};

static const View reverb_view_tab[] =
{
	{{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, reverb_view},
	{{0, 0, 0, 0}, NULL}
};

static const View *tab[] = 
{ 
	instrument_view_tab,
	pattern_view_tab,
	sequence_view_tab,
	reverb_view_tab
};

static const struct { int mod, key; void (*action)(void*,void*,void*); int p1, p2, p3; } shortcuts[] =
{
	{ 0, SDLK_ESCAPE, quit_action, 0, 0, 0 },
	{ KMOD_ALT, SDLK_F4, quit_action, 0, 0, 0 },
	{ 0, SDLK_F2, change_mode_action, EDITPATTERN, 0, 0},
	{ 0, SDLK_F3, change_mode_action, EDITINSTRUMENT, 0, 0},
	{ KMOD_SHIFT, SDLK_F3, change_mode_action, EDITREVERB, 0, 0},
	{ 0, SDLK_F4, change_mode_action, EDITSEQUENCE, 0, 0},
	{ 0, SDLK_F5, play, 0, 0, 0 },
	{ 0, SDLK_F6, play, 1, 0, 0 },
	{ 0, SDLK_F8, stop, 0, 0, 0 },
	{ 0, SDLK_F9, change_octave, -1, 0, 0 },
	{ KMOD_SHIFT, SDLK_F9, change_song_rate, -1, 0, 0 },
	{ KMOD_SHIFT|KMOD_CTRL, SDLK_F9, change_time_signature, -1, 0, 0 },
	{ 0, SDLK_F10, change_octave, +1, 0, 0 },
	{ KMOD_SHIFT, SDLK_F10, change_song_rate, +1, 0, 0 },
	{ KMOD_SHIFT|KMOD_CTRL, SDLK_F10, change_time_signature, +1, 0, 0 },
	{ 0, SDLK_KP_PLUS, select_instrument, +1, 1, 0 },
	{ KMOD_CTRL, SDLK_KP_PLUS, change_song_speed, 0, +1, 0 },
	{ KMOD_ALT, SDLK_KP_PLUS, change_song_speed, 1, +1, 0 },
	{ 0, SDLK_KP_MINUS, select_instrument, -1, 1, 0 },
	{ KMOD_CTRL, SDLK_KP_MINUS, change_song_speed, 0, -1, 0 },
	{ KMOD_ALT, SDLK_KP_MINUS, change_song_speed, 1, -1, 0 },
	{ KMOD_CTRL, SDLK_n, new_song_action, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_s, save_song_action, 0, 0, 0 },
	{ KMOD_CTRL,  SDLK_o, open_song_action, 0, 0, 0 },
	{ KMOD_CTRL,  SDLK_c, generic_action, (int)copy, 0, 0 },
	{ KMOD_CTRL, SDLK_v, generic_action, (int)paste, 0, 0 },
	{ KMOD_CTRL, SDLK_x, generic_action, (int)cut, 0, 0 },
	{ KMOD_SHIFT, SDLK_DELETE, generic_action, (int)delete, 0, 0 },
	{ KMOD_SHIFT, SDLK_INSERT, generic_action, (int)paste, 0, 0 },
	{ KMOD_CTRL, SDLK_INSERT, generic_action, (int)copy, 0, 0 },

	/* Null terminated */
	{ 0, 0, NULL, 0, 0, 0 }
};

// mingw kludge for console output
#ifdef DEBUG
#undef main
#endif

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE|SDL_INIT_TIMER);
	atexit(SDL_Quit);

	GfxDomain *domain = gfx_create_domain();
	domain->screen_w = SCREEN_WIDTH;
	domain->screen_h = SCREEN_HEIGHT;
	domain->fps = 10;
	gfx_domain_update(domain);
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1);
	
	MusInstrument instrument[NUM_INSTRUMENTS];
	MusPattern pattern[NUM_PATTERNS];
	MusSeqPattern sequence[MUS_CHANNELS][NUM_SEQUENCES];
	
	init(instrument, pattern, sequence, gfx_domain_get_surface(domain));
	
	Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 2048);
	Mix_AllocateChannels(1);
	
	cyd_init(&mused.cyd, 44100, MUS_CHANNELS);
	mus_init_engine(&mused.mus, &mused.cyd);
	
	cyd_register(&mused.cyd);
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
	
	while (1)
	{
		SDL_Event e = { 0 };
		int got_event = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
				quit_action(0,0,0);
				break;
				
				case SDL_MOUSEBUTTONUP:
				
				mouse_released(&e);
				
				break;
				
				case SDL_KEYDOWN:
				{
					// key events should go only to the edited text field
									
					if (mused.mode != EDITBUFFER) 
					{
						for (int i = 0 ; shortcuts[i].action ; ++i)
						{
							if (e.key.keysym.sym == shortcuts[i].key
								&& (!(e.key.keysym.mod & KMOD_SHIFT) == !(shortcuts[i].mod & KMOD_SHIFT))
								&& (!(e.key.keysym.mod & KMOD_CTRL) == !(shortcuts[i].mod & KMOD_CTRL))
								&& (!(e.key.keysym.mod & KMOD_ALT) == !(shortcuts[i].mod & KMOD_ALT))
							)
							{
								cyd_lock(&mused.cyd, 1);
								shortcuts[i].action((void*)shortcuts[i].p1, (void*)shortcuts[i].p2, (void*)shortcuts[i].p3);
								cyd_lock(&mused.cyd, 0);
								e.key.keysym.sym = 0;
								break;
							}
						}
					}
					
					if (e.key.keysym.sym != 0)
					{
						cyd_lock(&mused.cyd, 1);
						
						switch (mused.mode)
						{
							case EDITBUFFER:
							edit_text(&e);
							break;
							
							case EDITPROG:
							edit_program_event(&e);
							break;
							
							case EDITINSTRUMENT:
							edit_instrument_event(&e);
							break;
							
							case EDITPATTERN:
							pattern_event(&e);
							break;
							
							case EDITSEQUENCE:
							sequence_event(&e);
							break;
							
							case EDITREVERB:
							reverb_event(&e);
							break;
						}
						
						cyd_lock(&mused.cyd, 0);
					}
				}
				break;
			}
			
			if (e.type != SDL_MOUSEMOTION || (e.motion.state)) ++got_event;
			
			// ensure the last event is a mouse click so it gets passed to the draw/event code
			
			if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && e.motion.state)) break; 
		}
		
		if (got_event || gfx_domain_is_next_frame(domain))
		{
			mus_poll_status(&mused.mus, &mused.stat_song_position, mused.stat_pattern_position, mused.stat_pattern);
		
			for (int i = 0 ; i < MUS_CHANNELS ; ++i)
			{
				stat_pattern_number[i] = (stat_pattern[i] - &mused.song.pattern[0])/sizeof(mused.song.pattern[0]);
			}
			
			int m = (mused.mode == EDITBUFFER || mused.mode == EDITPROG) ? mused.prev_mode : mused.mode;
		
			draw_view(tab[m], &e);
			
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(1);
		
		if (mused.done) 
		{
			int r = confirm_ync("Save song?");
			
			if (r == 0) mused.done = 0;
			if (r == -1) break;
			if (r == 1) { change_mode(EDITSEQUENCE); if (!save_data()) mused.done = 0; else break; }
		}
	}
	
	Mix_CloseAudio();
	
	cyd_unregister(&mused.cyd);
	cyd_deinit(&mused.cyd);
	
	gfx_domain_free(domain);
	
	deinit();
	
	return 0;
}
