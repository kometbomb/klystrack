#ifndef MUSED_H
#define MUSED_H

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

#include "snd/music.h"
#include "gfx/font.h"
#include "console.h"
#include "gui/slider.h"
#include "edit.h"

enum
{
	EDITPATTERN,
	EDITSEQUENCE,
	EDITCLASSIC,
	EDITINSTRUMENT,
	EDITFX,
	EDITWAVETABLE,
	/* Focuses */
	EDITPROG,
	EDITSONGINFO,
	/* Virtual modes, i.e. what are not modes itself but should be considered happening "inside" prev_mode */
	EDITBUFFER,
	MENU
};

#define N_VIEWS EDITPROG
#define VIRTUAL_MODE EDITBUFFER

#include "clipboard.h"
#include "copypaste.h"
#include "gui/menu.h"
#include "undo.h"
#include <stdbool.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240


enum
{
	COMPACT_VIEW = 1,
	SONG_PLAYING = 2,
	FULLSCREEN = 4,
	MULTICHANNEL_PREVIEW = 8,
	SHOW_PATTERN_POS_OFFSET = 16,
	FOLLOW_PLAY_POSITION = 32,
	ANIMATE_CURSOR = 64,
	HIDE_ZEROS = 128,
	DELETE_EMPTIES = 256,
	EDIT_MODE = 512,
	CENTER_PATTERN_EDITOR = 1024,
	SHOW_ANALYZER = 2048,
#ifdef MIDI
	MIDI_SYNC = 4096,
#endif
	SHOW_LOGO = 8192,
	SHOW_DECIMALS = 16384,
	LOOP_POSITION = 32768,
	LOCK_SEQUENCE_STEP_AND_PATTERN_LENGTH = 65536,
	SHOW_DELAY_IN_TICKS = 65536 << 1,
	EDIT_SEQUENCE_DIGITS = 65536 << 2
};

enum
{
	VC_INSTRUMENT = 1,
	VC_VOLUME = 2,
	VC_CTRL = 4,
	VC_COMMAND = 8
};

typedef struct
{
	Uint32 flags, visible_columns;
	int done;
	Console *console;
	MusSong song;
	CydEngine cyd;
	MusEngine mus;
	int octave, instrument_page, current_instrument, default_pattern_length, selected_param, editpos, mode, focus,
		current_patternstep, current_pattern, current_patternx, 
		current_sequencepos, sequenceview_steps, single_pattern_edit, 
		prev_mode, current_sequenceparam, instrument_list_position,
		pattern_position, sequence_position, pattern_horiz_position, sequence_horiz_position,
		program_position, current_program_step,
		edit_reverb_param, selected_wavetable, wavetable_param, songinfo_param,
		loop_store_length, loop_store_loop, note_jump, wavetable_list_position, wavetable_preview_idx, sequence_digit;
	Uint16 *ghost_pattern[MUS_MAX_CHANNELS];
	int current_sequencetrack;
	Uint16 time_signature;
	Clipboard cp;
	char * edit_buffer;
	int edit_buffer_size;
	SliderParam sequence_slider_param, pattern_slider_param, program_slider_param, instrument_list_slider_param, 
		pattern_horiz_slider_param, sequence_horiz_slider_param, wavetable_list_slider_param;
	/*---*/
	char * edit_backup_buffer;
	Selection selection;
	/* -- stat -- */
	int stat_song_position;
	int stat_pattern_position[MUS_MAX_CHANNELS];
	MusPattern *stat_pattern[MUS_MAX_CHANNELS];
	MusChannel *channel;
	int stat_pattern_number[MUS_MAX_CHANNELS], stat_note[MUS_MAX_CHANNELS];
	/* ---- */
	GfxSurface *slider_bevel, *vu_meter, *analyzer, *logo;
	Font smallfont, largefont;
	
	/* for menu */
	Font menufont, menufont_selected, headerfont, headerfont_selected, shortcutfont, shortcutfont_selected, buttonfont;
	
	SDL_Surface *screen;
	char themename[100], keymapname[100];
	int pixel_scale;
	int mix_rate, mix_buffer;
	int window_w, window_h;
	int fx_bus, fx_room_size, fx_room_vol, fx_room_dec;
	/*---vis---*/
	struct 
	{
		int cyd_env[MUS_MAX_CHANNELS];
		int spec_peak[96], spec_peak_decay[96];
	} vis;
	
	/*---*/
	SDL_Rect cursor_target, cursor;
	/*----------*/
	UndoStack undo;
	UndoStack redo;
	SHType last_snapshot;
	int last_snapshot_a, last_snapshot_b;
	bool modified;
	/*------------*/
	SDL_Surface *wavetable_preview;
	
	
#ifdef MIDI
	Uint32 midi_device;
	Uint8 midi_channel;
	bool midi_start;
	Uint16 midi_rate;
	Uint32 midi_last_clock;
	Uint8 tick_ctr;
#endif
} Mused;

extern Mused mused;

#define NUM_PATTERNS 1024
#define NUM_INSTRUMENTS 128
#define NUM_SEQUENCES 1024

void change_mode(int newmode);
void clear_pattern(MusPattern *pat);
void clear_pattern_range(MusPattern *pat, int first, int last);
void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES], MusChannel *channel, SDL_Surface *screen);
void deinit();
void new_song();
void kt_default_instrument(MusInstrument *instrument);
void set_edit_buffer(char *buffer, size_t size);
void update_ghost_patterns();
void change_pixel_scale(void *a, void*b, void*c);
void mirror_flags();
void resize_pattern(MusPattern * pattern, Uint16 new_size);
void init_scrollbars();
void my_open_menu();
int viscol(int col);
void post_config_load();
void enable_callback(bool state);

#endif
