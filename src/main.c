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
#include "gui/mouse.h"
#include "gui/bevel.h"
#include "gui/menu.h"
#include "shortcutdefs.h"
#include "version.h"
#include "mused.h"
#include "config.h"
#include "bevdefs.h"

//#define DUMPKEYS

Mused mused;

/*---*/

int stat_song_position;
int stat_pattern_position[MUS_MAX_CHANNELS];
MusPattern *stat_pattern[MUS_MAX_CHANNELS];
int stat_pattern_number[MUS_MAX_CHANNELS];
GfxDomain *domain;

extern const Menu mainmenu[];

#define INST_LIST (6*8 + 3*2)
#define INFO 13
#define INST_VIEW2 38

void change_pixel_scale(void *, void*, void*);

static const View instrument_view_tab[] =
{
	{{0, 0, 164 + 32 + 4 + 8, 14}, bevel_view, (void*)BEV_BACKGROUND, EDITINSTRUMENT },
	{{2, 2, 164 + 32 + 8, 10}, instrument_name_view, (void*)1, -1 },
	{{164 + 32 + 4 + 8, 0, SCREEN_WIDTH - 164 - 32 - 2 - 2 - 8, 14}, instrument_disk_view, NULL, -1 },
	{{0, 14, 154, SCREEN_HEIGHT-14-INFO }, instrument_view, NULL, EDITINSTRUMENT },
	{{154, 14 + INST_LIST, SCREEN_WIDTH - 154, INST_VIEW2 }, instrument_view2, NULL, EDITINSTRUMENT },
	{{154, 14, SCREEN_WIDTH - 154 - SCROLLBAR, INST_LIST }, instrument_list, NULL, -1},
	{{154, 14 + INST_LIST + INST_VIEW2, SCREEN_WIDTH - 154 - SCROLLBAR, SCREEN_HEIGHT-(14 + INST_LIST)-INFO-INST_VIEW2 }, program_view, NULL, EDITPROG },
	{{SCREEN_WIDTH - SCROLLBAR, 14 + INST_LIST + INST_VIEW2, SCROLLBAR, SCREEN_HEIGHT-(14 + INST_LIST)-INFO-INST_VIEW2 }, slider, &mused.program_slider_param, EDITPROG },
	{{SCREEN_WIDTH - SCROLLBAR, 14, SCROLLBAR, INST_LIST }, slider, &mused.instrument_list_slider_param, EDITINSTRUMENT },
	{{0, SCREEN_HEIGHT - INFO, SCREEN_WIDTH, INFO }, info_line, NULL, -1 },
	{{0, 0, 0, 0}, NULL}
};

static const View pattern_view_tab[] =
{
	{{0, 0, SCREEN_WIDTH, 14}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{2, 2, 164 + 32, 10}, instrument_name_view, (void*)1, -1},
	{{0, 14, SCREEN_WIDTH-SCROLLBAR, SCREEN_HEIGHT - INFO - 14}, pattern_view, NULL, -1},
	{{SCREEN_WIDTH-SCROLLBAR, 14, SCROLLBAR, SCREEN_HEIGHT - INFO -14}, slider, &mused.pattern_slider_param, -1},
	{{0, SCREEN_HEIGHT - INFO, SCREEN_WIDTH, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

#define CLASSIC_PAT (SCREEN_HEIGHT / 2 + 20 - 2 - 7)
#define CLASSIC_SONG_INFO (94)

static const View classic_view_tab[] =
{
	{{0,0,CLASSIC_SONG_INFO,SCREEN_HEIGHT - INFO - CLASSIC_PAT}, info_view, NULL, -1},
	{{CLASSIC_SONG_INFO, 0, SCREEN_WIDTH-SCROLLBAR-CLASSIC_SONG_INFO, SCREEN_HEIGHT - CLASSIC_PAT - INFO - 10 - 10 - 5}, sequence_view, NULL, EDITSEQUENCE},
	{{SCREEN_WIDTH-SCROLLBAR, 0, SCROLLBAR, SCREEN_HEIGHT - CLASSIC_PAT - INFO - 10 - 10 - 5}, slider, &mused.sequence_slider_param, EDITSEQUENCE},
	{{CLASSIC_SONG_INFO, SCREEN_HEIGHT - CLASSIC_PAT - INFO - 10 - 10 - 5, SCREEN_WIDTH - CLASSIC_SONG_INFO, 25}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{CLASSIC_SONG_INFO + 2, SCREEN_HEIGHT - CLASSIC_PAT - INFO - 20 - 2 - 1, 164 + 32, 10}, song_name_view, NULL, -1},
	{{CLASSIC_SONG_INFO + 2, SCREEN_HEIGHT - CLASSIC_PAT - INFO - 10 - 2, 164 + 32, 10}, instrument_name_view, (void*)1, -1},
	{{0, SCREEN_HEIGHT - INFO - CLASSIC_PAT, SCREEN_WIDTH-SCROLLBAR, CLASSIC_PAT}, pattern_view, NULL, EDITPATTERN},
	{{SCREEN_WIDTH - SCROLLBAR, SCREEN_HEIGHT - INFO - CLASSIC_PAT, SCROLLBAR, CLASSIC_PAT}, slider, &mused.pattern_slider_param, EDITPATTERN},
	{{0, SCREEN_HEIGHT - INFO, SCREEN_WIDTH, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

static const View sequence_view_tab[] =
{
	{{0,0,SCREEN_WIDTH,33+4}, info_view, NULL, -1},
	{{0, 33+4, 164 + 32 + 4 + 8, 14}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{2, 33+4+2, 164 + 32 + 8, 10}, song_name_view, NULL, -1},
	{{164 + 32 + 4 + 8, 33+4, SCREEN_WIDTH - 164 - 32 - 2 - 2 - 8, 14}, instrument_disk_view, NULL, -1},
	{{0, 33+4+14, SCREEN_WIDTH-SCROLLBAR, SCREEN_HEIGHT-(33+4+14)}, sequence_view, NULL, -1},
	{{SCREEN_WIDTH-SCROLLBAR, 33+4+14, SCROLLBAR, SCREEN_HEIGHT-(33+4+14)}, slider, &mused.sequence_slider_param, -1},
	{{0, 0, 0, 0}, NULL}
};

static const View reverb_view_tab[] =
{
	{{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, reverb_view, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

const View *tab[] = 
{ 
	instrument_view_tab,
	pattern_view_tab,
	sequence_view_tab,
	reverb_view_tab,
	classic_view_tab
};


static void menu_close_hook(void)
{
	change_mode(mused.prev_mode);
}


// mingw kludge for console output
#ifdef DEBUG
#undef main
#endif

int main(int argc, char **argv)
{
	debug("Starting %s", VERSION_STRING);

#ifdef DEBUG
	SDL_putenv("SDL_DEBUG=1");
#endif

	SDL_putenv("SDL_VIDEO_CENTERED=1");

	SDL_Init(SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE|SDL_INIT_TIMER);
	atexit(SDL_Quit);
	
	domain = gfx_create_domain();
	domain->screen_w = SCREEN_WIDTH;
	domain->screen_h = SCREEN_HEIGHT;
	domain->fps = 20;
	domain->scale = 1;
	gfx_domain_update(domain);
	
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1);
	SDL_WM_SetCaption(VERSION_STRING, NULL);
	
	MusInstrument instrument[NUM_INSTRUMENTS];
	MusPattern pattern[NUM_PATTERNS];
	MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES];
	MusChannel channel[CYD_MAX_CHANNELS];
	
	init(instrument, pattern, sequence, channel, gfx_domain_get_surface(domain));
	
	load_config(TOSTRING(CONFIG_PATH));
	
	if (Mix_OpenAudio(mused.mix_rate, AUDIO_S16SYS, 2, mused.mix_buffer)) warning("Mix_OpenAudio failed: %s", Mix_GetError());
	Mix_AllocateChannels(1);
	
	cyd_init(&mused.cyd, mused.mix_rate, MUS_MAX_CHANNELS);
	mus_init_engine(&mused.mus, &mused.cyd);
	
#ifdef DEBUG
	/* This is because I couldn't get my soundcard to record stereo mix for demo videos */
	if (argc > 1 && strcmp(argv[1], "-dump") == 0) cyd_enable_audio_dump(&mused.cyd);
#endif
	
	cyd_register(&mused.cyd);
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
	
	int active = 1;
	
	while (1)
	{
		SDL_Event e = { 0 };
		int got_event = 0, menu_closed = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
				quit_action(0,0,0);
				break;
				
				case SDL_ACTIVEEVENT:
				if (e.active.state & SDL_APPACTIVE)
				{	
					active = e.active.gain;
					debug("Window %s focus", active ? "gained" : "lost");
				}
				break;
				
				case SDL_USEREVENT:
					e.type = SDL_MOUSEBUTTONDOWN;
				break;
				
				case SDL_MOUSEMOTION:
					e.motion.xrel /= domain->scale;
					e.motion.yrel /= domain->scale;
					e.button.x /= domain->scale;
					e.button.y /= domain->scale;
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					e.button.x /= domain->scale;
					e.button.y /= domain->scale;
					if (e.button.button == SDL_BUTTON_RIGHT)
					{
						change_mode(MENU);
						open_menu(mainmenu, menu_close_hook, shortcuts, &mused.largefont, &mused.smallfont, mused.slider_bevel, BEV_MENUBAR, BEV_MENU, BEV_MENU_SELECTED, BEV_SEPARATOR, '§', '^');
					}
				break;
				
				case SDL_MOUSEBUTTONUP:
				{
					if (e.button.button == SDL_BUTTON_LEFT)
						mouse_released(&e);
					else if (e.button.button == SDL_BUTTON_RIGHT)	
						menu_closed = 1;
				}
				break;
				
				case SDL_KEYDOWN:
				{
#ifdef DUMPKEYS
					debug("SDL_KEYDOWN: time = %.1f sym = %x mod = %x unicode = %x scancode = %x", (double)SDL_GetTicks() / 1000.0, e.key.keysym.sym, e.key.keysym.mod, e.key.keysym.unicode, e.key.keysym.scancode);
#endif

					// Special multimedia keys look like a-z keypresses but the unicode value is zero
					// We don't care about the special keys and don't want fake keypresses either
					if (e.key.keysym.unicode == 0 && e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z)
						break;
						
					// key events should go only to the edited text field
									
					if (mused.mode != EDITBUFFER) 
					{
						cyd_lock(&mused.cyd, 1);
						do_shortcuts(&e.key, shortcuts);
						cyd_lock(&mused.cyd, 0);
					}
					
					if (e.key.keysym.sym != 0)
					{
						cyd_lock(&mused.cyd, 1);
						
						switch (mused.focus)
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
			
			if (mused.mode == EDITBUFFER) e.type = SDL_USEREVENT;
			
			if (e.type != SDL_MOUSEMOTION || (e.motion.state)) ++got_event;
			
			// ensure the last event is a mouse click so it gets passed to the draw/event code
			
			if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && e.motion.state)) break; 
		}
		
		if (active && (got_event || gfx_domain_is_next_frame(domain)))
		{
			mus_poll_status(&mused.mus, &mused.stat_song_position, mused.stat_pattern_position, mused.stat_pattern, channel);
		
			for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			{
				stat_pattern_number[i] = (stat_pattern[i] - &mused.song.pattern[0])/sizeof(mused.song.pattern[0]);
			}
			
			int m = mused.mode >= VIRTUAL_MODE ? mused.prev_mode : mused.mode;
		
			int prev_mode;
		
			do
			{
				prev_mode = mused.mode;
				
				if (mused.mode == MENU) 
				{
					SDL_Event foo = {0};
					draw_view(tab[m], &foo);
					draw_menu(mused.screen, &e);
					if (menu_closed) close_menu();
				}
				else
				{
					draw_view(tab[m], &e);
				}
				
				e.type = 0;
			}
			while (mused.mode != prev_mode); // Eliminates the one-frame long black screen
			
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(5);
		
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
	
	save_config(TOSTRING(CONFIG_PATH));
	
	deinit();
	
	return 0;
}
