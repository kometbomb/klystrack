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

#include "shortcutdefs.h"
#include "SDL.h"
#include "action.h"
#include "diskop.h"
#include "edit.h"
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
	{ KMOD_SHIFT, SDLK_F3, change_mode_action, (void*)EDITFX, 0, 0},
	{ 0, SDLK_F4, change_mode_action, (void*)EDITSEQUENCE, 0, 0},
	{ KMOD_SHIFT, SDLK_F4, change_mode_action, (void*)EDITCLASSIC, 0, 0},
	{ KMOD_SHIFT, SDLK_F2, change_mode_action, (void*)EDITWAVETABLE, 0, 0},
	{ 0, SDLK_F5, play, 0, 0, 0 },
	{ 0, SDLK_F6, play, (void*)1, 0, 0 },
	{ KMOD_SHIFT, SDLK_F6, play_position, 0, 0, 0 },
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
	{ KMOD_CTRL, SDLK_s, open_data, MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_SAVE), 0 },
	{ KMOD_CTRL,  SDLK_o, open_data, MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_OPEN), 0 },
	{ KMOD_CTRL,  SDLK_c, generic_action, copy, 0, 0 },
	{ KMOD_CTRL, SDLK_v, generic_action, paste, 0, 0 },
	{ KMOD_CTRL, SDLK_x, generic_action, cut, 0, 0 },
	{ KMOD_CTRL, SDLK_DELETE, generic_action, delete, 0, 0 },
	{ KMOD_SHIFT, SDLK_INSERT, generic_action, paste, 0, 0 },
	{ KMOD_CTRL, SDLK_j, generic_action, join_paste, 0, 0 },
	{ KMOD_CTRL, SDLK_INSERT, generic_action, copy, 0, 0 },
	{ KMOD_CTRL, SDLK_a, select_all, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_d, clear_selection, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_r, toggle_pixel_scale, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_b, begin_selection_action, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_e, end_selection_action, 0, 0, 0 },
	{ KMOD_CTRL|KMOD_SHIFT, SDLK_e, expand_pattern, MAKEPTR(2), 0, 0 },
	{ KMOD_ALT, SDLK_RETURN, toggle_fullscreen, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_k, clone_pattern, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_u, get_unused_pattern, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_i, interpolate, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_m, unmute_all_action, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_z, do_undo, 0, 0, 0 },
	{ KMOD_CTRL, SDLK_y, do_undo, MAKEPTR(1), 0, 0 },
	{ KMOD_CTRL, SDLK_0, set_note_jump, MAKEPTR(0), 0, 0 },
	{ KMOD_CTRL, SDLK_1, set_note_jump, MAKEPTR(1), 0, 0 },
	{ KMOD_CTRL, SDLK_2, set_note_jump, MAKEPTR(2), 0, 0 },
	{ KMOD_CTRL, SDLK_3, set_note_jump, MAKEPTR(3), 0, 0 },
	{ KMOD_CTRL, SDLK_4, set_note_jump, MAKEPTR(4), 0, 0 },
	{ KMOD_CTRL, SDLK_5, set_note_jump, MAKEPTR(5), 0, 0 },
	{ KMOD_CTRL, SDLK_6, set_note_jump, MAKEPTR(6), 0, 0 },
	{ KMOD_CTRL, SDLK_7, set_note_jump, MAKEPTR(7), 0, 0 },
	{ KMOD_CTRL, SDLK_8, set_note_jump, MAKEPTR(8), 0, 0 },
	{ KMOD_CTRL, SDLK_9, set_note_jump, MAKEPTR(9), 0, 0 },
	{ KMOD_CTRL, SDLK_KP0, select_instrument_page, MAKEPTR(0), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP1, select_instrument_page, MAKEPTR(1), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP2, select_instrument_page, MAKEPTR(2), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP3, select_instrument_page, MAKEPTR(3), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP4, select_instrument_page, MAKEPTR(4), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP5, select_instrument_page, MAKEPTR(5), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP6, select_instrument_page, MAKEPTR(6), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP7, select_instrument_page, MAKEPTR(7), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP8, select_instrument_page, MAKEPTR(8), 0, MAKEPTR(1) },
	{ KMOD_CTRL, SDLK_KP9, select_instrument_page, MAKEPTR(9), 0, MAKEPTR(1) },
	{ 0, SDLK_KP0, select_instrument, MAKEPTR(0), 0, MAKEPTR(1) },
	{ 0, SDLK_KP1, select_instrument, MAKEPTR(1), 0, MAKEPTR(1) },
	{ 0, SDLK_KP2, select_instrument, MAKEPTR(2), 0, MAKEPTR(1) },
	{ 0, SDLK_KP3, select_instrument, MAKEPTR(3), 0, MAKEPTR(1) },
	{ 0, SDLK_KP4, select_instrument, MAKEPTR(4), 0, MAKEPTR(1) },
	{ 0, SDLK_KP5, select_instrument, MAKEPTR(5), 0, MAKEPTR(1) },
	{ 0, SDLK_KP6, select_instrument, MAKEPTR(6), 0, MAKEPTR(1) },
	{ 0, SDLK_KP7, select_instrument, MAKEPTR(7), 0, MAKEPTR(1) },
	{ 0, SDLK_KP8, select_instrument, MAKEPTR(8), 0, MAKEPTR(1) },
	{ 0, SDLK_KP9, select_instrument, MAKEPTR(9), 0, MAKEPTR(1) },
	/* Null terminated */
	{ 0, 0, NULL, 0, 0, 0 }
};