/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

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
#include "gfx/gfx.h"
#include "snd/music.h"
#include "gui/toolutil.h"
#include "copypaste.h"
#include "diskop.h"
#include "event.h"
#include "view.h"
#include "gui/slider.h"
#include "action.h"
#include "gui/mouse.h"
#include "gui/bevel.h"
#include "gui/menu.h"
#include "shortcutdefs.h"
#include "version.h"
#include "mused.h"
#include "config.h"
#include "mybevdefs.h"
#include <time.h>
#include "util/rnd.h"
#include "view/visu.h"
#include "view/pattern.h"
#include "view/sequence.h"
#include "view/wavetableview.h"
#include "view/timer.h"
#include "mymsg.h"
#include "key.h"
#include "nostalgy.h"
#include "theme.h"

#ifdef MIDI

#include "midi.h"

#endif

//#define DUMPKEYS

Mused mused;

/*---*/

int stat_song_position;
int stat_pattern_position[MUS_MAX_CHANNELS];
MusPattern *stat_pattern[MUS_MAX_CHANNELS];
int stat_pattern_number[MUS_MAX_CHANNELS];
GfxDomain *domain;

extern const Menu mainmenu[];

#define SCROLLBAR 10
#define INST_LIST (6*8 + 3*2)
#define INFO 13
#define INST_VIEW2 (38+10+10+10+52)

void change_pixel_scale(void *, void*, void*);

static const View instrument_view_tab[] =
{
	{{0, 0, -130, 14}, bevel_view, (void*)BEV_BACKGROUND, EDITINSTRUMENT },
	{{2, 2, -130-2, 10}, instrument_name_view, (void*)1, EDITINSTRUMENT},
	{{-130, 0, 130, 14}, instrument_disk_view, MAKEPTR(OD_T_INSTRUMENT), EDITINSTRUMENT},
	{{0, 14, 154, -INFO }, instrument_view, NULL, EDITINSTRUMENT },
	{{154, 14 + INST_LIST, 0, INST_VIEW2 }, instrument_view2, NULL, EDITINSTRUMENT },
	{{154, 14, - SCROLLBAR, INST_LIST }, instrument_list, NULL, EDITINSTRUMENT},
	{{154, 14 + INST_LIST + INST_VIEW2, 0 - SCROLLBAR, -INFO }, program_view, NULL, EDITPROG },
	{{0 - SCROLLBAR, 14 + INST_LIST + INST_VIEW2, SCROLLBAR, -INFO }, slider, &mused.program_slider_param, EDITPROG },
	{{0 - SCROLLBAR, 14, SCROLLBAR, INST_LIST }, slider, &mused.instrument_list_slider_param, EDITINSTRUMENT },
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1 },
	{{0, 0, 0, 0}, NULL}
};

static const View pattern_view_tab[] =
{
	{{0, 0, 0, 14}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{2, 2, 164 + 32, 10}, instrument_name_view, (void*)1, EDITINSTRUMENT},
	{{0, 14, 0-SCROLLBAR, 0 - INFO}, pattern_view2, NULL, EDITPATTERN},
	{{0-SCROLLBAR, 14, SCROLLBAR, 0 - INFO}, slider, &mused.pattern_slider_param, EDITPATTERN},
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

#define CLASSIC_PAT (0 / 2 + 20 - 2 - 7)
#define CLASSIC_SONG_INFO (94)
#define SONG_INFO1_H (24+10)
#define SONG_INFO2_H (24+10)
#define SONG_INFO3_H (15+10)
#define TOOLBAR_H 12
#define CLASSIC_SONG_INFO_H (SONG_INFO1_H+SONG_INFO2_H+SONG_INFO3_H+TOOLBAR_H)
#define TIMER_W (5*7+4)
#define SEQ_VIEW_INFO_H (24+10)

static const View classic_view_tab[] =
{
	{{0,0,CLASSIC_SONG_INFO,TOOLBAR_H}, toolbar_view, NULL, -1},
	{{0,TOOLBAR_H,CLASSIC_SONG_INFO,SONG_INFO1_H}, songinfo1_view, NULL, EDITSONGINFO},
	{{0,TOOLBAR_H + SONG_INFO1_H,CLASSIC_SONG_INFO,SONG_INFO2_H}, songinfo2_view, NULL, EDITSONGINFO},
	{{0,TOOLBAR_H + SONG_INFO1_H+SONG_INFO2_H,CLASSIC_SONG_INFO,SONG_INFO3_H}, songinfo3_view, NULL, EDITSONGINFO},
	{{- PLAYSTOP_INFO_W, CLASSIC_SONG_INFO_H - 25,PLAYSTOP_INFO_W,25}, playstop_view, NULL, EDITSONGINFO},
	{{0-SCROLLBAR, 0, SCROLLBAR, CLASSIC_SONG_INFO_H - 25}, slider, &mused.sequence_slider_param, EDITSEQUENCE},
	{{CLASSIC_SONG_INFO, 0, 0-SCROLLBAR, CLASSIC_SONG_INFO_H - 25}, sequence_spectrum_view, NULL, EDITSEQUENCE},
	{{CLASSIC_SONG_INFO, CLASSIC_SONG_INFO_H-25, - PLAYSTOP_INFO_W, 25}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{CLASSIC_SONG_INFO + 2, CLASSIC_SONG_INFO_H - 25 + 2, -2 - PLAYSTOP_INFO_W - TIMER_W - 1, 10}, song_name_view, NULL, -1},
	{{-2 - PLAYSTOP_INFO_W - TIMER_W, CLASSIC_SONG_INFO_H - 25 + 2, TIMER_W, 12}, timer_view, NULL, -1},
	{{CLASSIC_SONG_INFO + 2, CLASSIC_SONG_INFO_H - 25 + 2 + 10 + 1, -2 - PLAYSTOP_INFO_W, 10}, instrument_name_view, (void*)1, -1},
	{{0, CLASSIC_SONG_INFO_H, 0-SCROLLBAR, -INFO}, pattern_view2, NULL, EDITPATTERN},
	{{0 - SCROLLBAR, CLASSIC_SONG_INFO_H, SCROLLBAR, -INFO}, slider, &mused.pattern_slider_param, EDITPATTERN},
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

static const View sequence_view_tab[] =
{
	{{0,0,CLASSIC_SONG_INFO,SEQ_VIEW_INFO_H}, songinfo1_view, NULL, EDITSONGINFO},
	{{CLASSIC_SONG_INFO,0,CLASSIC_SONG_INFO,SEQ_VIEW_INFO_H}, songinfo2_view, NULL, EDITSONGINFO},
	{{CLASSIC_SONG_INFO*2,0,CLASSIC_SONG_INFO,SEQ_VIEW_INFO_H}, songinfo3_view, NULL, EDITSONGINFO},
	{{CLASSIC_SONG_INFO*3,0,0,SEQ_VIEW_INFO_H}, playstop_view, NULL, EDITSONGINFO},
	{{0, SEQ_VIEW_INFO_H, -130, 14}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{2, SEQ_VIEW_INFO_H+2, -130-2-(TIMER_W+1), 10}, song_name_view, NULL, -1},
	{{-130-2-TIMER_W, SEQ_VIEW_INFO_H+2, TIMER_W, 10}, timer_view, NULL, -1},
	{{-130, SEQ_VIEW_INFO_H, 130, 14}, instrument_disk_view, MAKEPTR(OD_T_SONG), -1},
	{{0, SEQ_VIEW_INFO_H+14, 0-SCROLLBAR, -INFO}, sequence_view2, NULL, EDITSEQUENCE},
	{{0-SCROLLBAR, SEQ_VIEW_INFO_H+14, SCROLLBAR, -INFO}, slider, &mused.sequence_slider_param, EDITSEQUENCE},
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

static const View fx_view_tab[] =
{
	{{0, 0, 0, 14}, fx_global_view, NULL, -1},
	{{0, 14, 0, 14}, bevel_view, (void*)BEV_BACKGROUND, -1},
	{{2, 16, -1, 10}, fx_name_view, NULL, -1},
	{{0, 28, 0, -INFO}, fx_view, NULL, -1},
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

#define SAMPLEVIEW 128

static const View wavetable_view_tab[] =
{
	{{0, 0, 204, -INFO-SAMPLEVIEW}, wavetable_view, NULL, -1},
	{{204, 0, -SCROLLBAR, -INFO-SAMPLEVIEW}, wavetablelist_view, NULL, -1},
	{{0 - SCROLLBAR, 0, SCROLLBAR, -INFO-SAMPLEVIEW }, slider, &mused.wavetable_list_slider_param, EDITWAVETABLE },
	{{0, -INFO-SAMPLEVIEW, -128, SAMPLEVIEW}, wavetable_sample_area, NULL, -1},
	{{-128, -INFO-SAMPLEVIEW, 128, SAMPLEVIEW}, wavetable_edit_area, NULL, -1},
	{{0, 0 - INFO, 0, INFO }, info_line, NULL, -1},
	{{0, 0, 0, 0}, NULL}
};

const View *tab[] = 
{ 
	pattern_view_tab,
	sequence_view_tab,
	classic_view_tab,
	instrument_view_tab,
	fx_view_tab,
	wavetable_view_tab,
};


static void menu_close_hook(void)
{
	change_mode(mused.prev_mode);
}


void my_open_menu(const Menu *menu, const Menu *action)
{
	debug("Menu opened");
	
	change_mode(MENU);
	open_menu(menu, action, menu_close_hook, shortcuts, &mused.headerfont, &mused.headerfont_selected, &mused.menufont, &mused.menufont_selected, &mused.shortcutfont, &mused.shortcutfont_selected, mused.slider_bevel);
}


#ifdef WIN32

#include "SDL_syswm.h"
#include <windows.h>
#include "../windres/resource.h"

/*

Found at <URL:http://forums.indiegamer.com/showthread.php?t=2138> and then gutted

*/

static HICON icon;

void init_icon()
{
	/*HWND hwnd;
	HINSTANCE handle = GetModuleHandle(NULL);
	icon = LoadIcon(handle, MAKEINTRESOURCE(IDI_MAINICON));

	SDL_SysWMinfo wminfo;
	
	SDL_VERSION(&wminfo.version)
	
	if (SDL_GetWMInfo(&wminfo) != 1)
	{
		return;
	}

	hwnd = wminfo.window;

	SetClassLong(hwnd, GCL_HICON, (LONG) icon);*/
}

void deinit_icon()
{
	DestroyIcon(icon);
}

#endif


// mingw kludge for console output
#if defined(DEBUG) && defined(WIN32)
#undef main
#endif

int main(int argc, char **argv)
{
	init_genrand(time(NULL));
	init_resources_dir();
	debug("Starting %s", VERSION_STRING);

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE|SDL_INIT_TIMER);
	atexit(SDL_Quit);
	
#ifdef WIN32
	init_icon();
#endif
	
	default_settings();
	load_config(TOSTRING(CONFIG_PATH), false);
		
	domain = gfx_create_domain(VERSION_STRING, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL|((mused.flags & WINDOW_MAXIMIZED)?SDL_WINDOW_MAXIMIZED:0), mused.window_w, mused.window_h, mused.pixel_scale);
	domain->fps = 30;
	domain->scale = mused.pixel_scale;
	gfx_domain_update(domain, false);
	
	MusInstrument instrument[NUM_INSTRUMENTS];
	MusPattern pattern[NUM_PATTERNS];
	MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES];
	MusChannel channel[CYD_MAX_CHANNELS];
	
	init(instrument, pattern, sequence, channel);
	
	load_config(TOSTRING(CONFIG_PATH), true);
	
	post_config_load();
	
	init_scrollbars();
	
	cyd_init(&mused.cyd, mused.mix_rate, MUS_MAX_CHANNELS);
	mus_init_engine(&mused.mus, &mused.cyd);
	new_song();
	
	enable_callback(true);
	
	for (int i = 0 ; i < CYD_MAX_FX_CHANNELS ; ++i)
		cydfx_set(&mused.cyd.fx[i], &mused.song.fx[i]);
	
	cyd_register(&mused.cyd, mused.mix_buffer);
	
	if (argc > 1)
	{
		cyd_lock(&mused.cyd, 1);
		FILE *f = fopen(argv[1], "rb");
		if (f)
		{
			open_song(f);
			fclose(f);
		}
		cyd_lock(&mused.cyd, 0);
	}
	
#ifdef MIDI
	midi_init();
#endif
	
#ifdef DEBUG
	float draw_calls = 0;
	int total_frames = 0;
#endif
	int active = 1;
	
	if (!(mused.flags & DISABLE_NOSTALGY))
		nos_decrunch(domain);
		
	while (1)
	{
		SDL_Event e = { 0 };
		int got_event = 0, menu_closed = 0;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
			{
				translate_key_event(&e.key);
			}
		
			switch (e.type)
			{
				case SDL_QUIT:
				quit_action(0,0,0);
				break;
				
				case SDL_WINDOWEVENT:
					switch (e.window.event) {
						case SDL_WINDOWEVENT_MINIMIZED:
						case SDL_WINDOWEVENT_RESTORED:
							mused.flags &= ~WINDOW_MAXIMIZED;
							break;
							
						case SDL_WINDOWEVENT_MAXIMIZED:
							mused.flags |= WINDOW_MAXIMIZED;
							break;
							
						case SDL_WINDOWEVENT_RESIZED:
							domain->screen_w = my_max(320, e.window.data1 / domain->scale);
							domain->screen_h = my_max(240, e.window.data2 / domain->scale);
							mused.window_w = domain->screen_w * domain->scale;
							mused.window_h = domain->screen_h * domain->scale;
							gfx_domain_update(domain, !(mused.flags & WINDOW_MAXIMIZED));
							break;
					}
					break;
					
				
				/*case SDL_ACTIVEEVENT:
				if (e.active.state & SDL_APPACTIVE)
				{	
					active = e.active.gain;
					debug("Window %s focus", active ? "gained" : "lost");
				}
				break;*/
				
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
						my_open_menu(mainmenu, NULL);
					}
					else if (e.button.button == SDL_BUTTON_LEFT && mused.mode == MENU)
					{
						menu_closed = 1;
					}
				break;
				
				case SDL_MOUSEBUTTONUP:
				{
					if (e.button.button == SDL_BUTTON_LEFT)
					{
						mouse_released(&e);
						
						if (mused.focus == EDITFX)
							mus_set_fx(&mused.mus, &mused.song); // for the chorus effect which does a heavy precalc 
					}
					else if (e.button.button == SDL_BUTTON_RIGHT)	
						menu_closed = 1;
				}
				break;
				
				case SDL_TEXTEDITING:
				case SDL_TEXTINPUT:
					switch (mused.focus)
					{
						case EDITBUFFER:
						edit_text(&e);
						break;
					}
					break;
				
				case SDL_KEYUP:
				case SDL_KEYDOWN:
				{
/*#ifdef DUMPKEYS
					debug("SDL_KEYDOWN: time = %.1f sym = %x mod = %x unicode = %x scancode = %x", (double)SDL_GetTicks() / 1000.0, e.key.keysym.sym, e.key.keysym.mod, e.key.keysym.unicode, e.key.keysym.scancode);
#endif*/
					// Translate F12 into SDLK_INSERT (Issue 37)
					if (e.key.keysym.sym == SDLK_F12) e.key.keysym.sym = SDLK_INSERT;

					// Special multimedia keys look like a-z keypresses but the unicode value is zero
					// We don't care about the special keys and don't want fake keypresses either
					/*if (e.type == SDL_KEYDOWN && e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z)
						break;*/
						
					// key events should go only to the edited text field
									
					if (mused.focus != EDITBUFFER) 
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
							
							case EDITFX:
							fx_event(&e);
							break;
							
							case EDITWAVETABLE:
							wave_event(&e);
							break;
							
							case EDITSONGINFO:
							songinfo_event(&e);
							break;
						}
						
						cyd_lock(&mused.cyd, 0);
					}
				}
				break;
				
				case MSG_NOTEON:
				case MSG_NOTEOFF:
				case MSG_PROGRAMCHANGE:
					note_event(&e);
					break;
			}
			
			if (mused.focus == EDITBUFFER && e.type == SDL_KEYDOWN) e.type = SDL_USEREVENT;
			
			if (e.type != SDL_MOUSEMOTION || (e.motion.state) || (e.type == SDL_MOUSEMOTION && mused.mode == MENU)) ++got_event;
			
			// ensure the last event is a mouse click so it gets passed to the draw/event code
			
			if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && e.motion.state)) 
				break; 
		}
		
		int prev_position = mused.stat_song_position;
		
		if (active) mus_poll_status(&mused.mus, &mused.stat_song_position, mused.stat_pattern_position, mused.stat_pattern, channel, mused.vis.cyd_env, mused.stat_note, &mused.time_played);
		
		if (active && (got_event || gfx_domain_is_next_frame(domain) || prev_position != mused.stat_song_position))
		{
			if ((mused.flags & FOLLOW_PLAY_POSITION) && (mused.flags & SONG_PLAYING))
			{
				mused.current_sequencepos = mused.stat_song_position - mused.stat_song_position % mused.sequenceview_steps;
				mused.current_patternpos = mused.stat_song_position;
				update_position_sliders();
			}
		
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
					
					my_draw_view(tab[m], &foo, domain);
					
					draw_menu(domain, &e);
					
					if (menu_closed) 
					{
						const Menu *cm = get_current_menu();
						const Menu *cm_action = get_current_menu_action();
					
						close_menu();
						
						if (SDL_GetModState() & KMOD_SHIFT)
							my_open_menu(cm, cm_action);
					}
				}
				else
				{
					my_draw_view(tab[m], &e, domain);
				}
				
				e.type = 0;
				
				// agh
				mused.current_patternpos = mused.pattern_position;
			}
			while (mused.mode != prev_mode); // Eliminates the one-frame long black screen
			
#ifdef DEBUG
			total_frames++;
			draw_calls += domain->calls_per_frame;
#endif
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(4);
		
		if (mused.done) 
		{
			int r;
			if (mused.modified) 
				r = confirm_ync(domain, mused.slider_bevel, &mused.largefont, "Save song?");
			else
				break;
			
			if (r == 0) mused.done = 0;
			if (r == -1) break;
			if (r == 1) 
			{ 
				int r;
				
				open_data(MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_SAVE), &r);
				
				if (!r) mused.done = 0; else break; 
			}
		}
	}
	
#ifdef MIDI
	midi_deinit();
#endif
	
	//Mix_CloseAudio();
	
	cyd_unregister(&mused.cyd);
	debug("cyd_deinit");
	cyd_deinit(&mused.cyd);
	
	gfx_domain_free(domain);
	
	save_config(TOSTRING(CONFIG_PATH));
	
	debug("deinit");
	deinit();
	
#ifdef WIN32
	deinit_icon();
#endif

#ifdef DEBUG
	if (total_frames > 0)
		debug("Draw calls per frame: %.1f", draw_calls / total_frames);
#endif

	debug("klystrack has left the building.");
	
	return 0;
}
