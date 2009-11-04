#include "shortcuts.h"
#include "SDL.h"
#include "action.h"
#include "mused.h"
#include "view.h"

extern Mused mused;
extern const View *tab[];

const KeyShortcut shortcuts[] =
{
	{ 0, SDLK_TAB, cycle_focus, tab, &mused.focus, &mused.mode },
	{ 0, SDLK_ESCAPE, quit_action, 0, 0, 0 },
	{ KMOD_ALT, SDLK_F4, quit_action, 0, 0, 0 },
	{ 0, SDLK_F2, change_mode_action, (void*)EDITPATTERN, 0, 0},
	{ 0, SDLK_F3, change_mode_action, (void*)EDITINSTRUMENT, 0, 0},
	{ KMOD_SHIFT, SDLK_F3, change_mode_action, (void*)EDITREVERB, 0, 0},
	{ 0, SDLK_F4, change_mode_action, (void*)EDITSEQUENCE, 0, 0},
	{ KMOD_SHIFT, SDLK_F4, change_mode_action, (void*)EDITCLASSIC, 0, 0},
	{ 0, SDLK_F5, play, 0, 0, 0 },
	{ 0, SDLK_F6, play, (void*)1, 0, 0 },
	{ 0, SDLK_F8, stop, 0, 0, 0 },
	{ 0, SDLK_F9, change_octave, (void*)-1, 0, 0 },
	{ KMOD_SHIFT, SDLK_F9, change_song_rate, (void*)-1, 0, 0 },
	{ KMOD_SHIFT|KMOD_CTRL, SDLK_F9, change_time_signature, 0, 0, 0 },
	{ 0, SDLK_F10, change_octave, (void*)+1, 0, 0 },
	{ KMOD_SHIFT, SDLK_F10, change_song_rate, (void*)+1, 0, 0 },
	{ KMOD_SHIFT|KMOD_CTRL, SDLK_F10, change_time_signature, (void*)1, 0, 0 },
	{ 0, SDLK_KP_PLUS, select_instrument, (void*)+1, (void*)1, 0 },
	{ KMOD_CTRL, SDLK_KP_PLUS, change_song_speed, 0, (void*)+1, 0 },
	{ KMOD_ALT, SDLK_KP_PLUS, change_song_speed, (void*)1, (void*)+1, 0 },
	{ 0, SDLK_KP_MINUS, select_instrument, (void*)-1, (void*)1, 0 },
	{ KMOD_CTRL, SDLK_KP_MINUS, change_song_speed, 0, (void*)-1, 0 },
	{ KMOD_ALT, SDLK_KP_MINUS, change_song_speed, (void*)1, (void*)-1, 0 },
	{ KMOD_CTRL, SDLK_n, new_song_action, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_s, save_song_action, 0, 0, 0 },
	{ KMOD_CTRL,  SDLK_o, open_song_action, 0, 0, 0 },
	{ KMOD_CTRL,  SDLK_c, generic_action, copy, 0, 0 },
	{ KMOD_CTRL, SDLK_v, generic_action, paste, 0, 0 },
	{ KMOD_CTRL, SDLK_x, generic_action, cut, 0, 0 },
	{ KMOD_SHIFT, SDLK_DELETE, generic_action, delete, 0, 0 },
	{ KMOD_SHIFT, SDLK_INSERT, generic_action, paste, 0, 0 },
	{ KMOD_CTRL, SDLK_INSERT, generic_action, copy, 0, 0 },
	{ KMOD_CTRL, SDLK_d, clear_selection, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_r, toggle_pixel_scale, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_b, begin_selection_action, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_e, end_selection_action, 0, 0, 0 },
	{ KMOD_ALT, SDLK_RETURN, toggle_fullscreen, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_k, clone_pattern, 0, 0, 0 },

	/* Null terminated */
	{ 0, 0, NULL, 0, 0, 0 }
};


void do_shortcuts(SDL_KeyboardEvent *e)
{
	for (int i = 0 ; shortcuts[i].action ; ++i)
	{
		if (e->keysym.sym == shortcuts[i].key
			&& (!(e->keysym.mod & KMOD_SHIFT) == !(shortcuts[i].mod & KMOD_SHIFT))
			&& (!(e->keysym.mod & KMOD_CTRL) == !(shortcuts[i].mod & KMOD_CTRL))
			&& (!(e->keysym.mod & KMOD_ALT) == !(shortcuts[i].mod & KMOD_ALT))
		)
		{
			cyd_lock(&mused.cyd, 1);
			shortcuts[i].action((void*)shortcuts[i].p1, (void*)shortcuts[i].p2, (void*)shortcuts[i].p3);
			cyd_lock(&mused.cyd, 0);
			e->keysym.sym = 0;
			break;
		}
	}
}
