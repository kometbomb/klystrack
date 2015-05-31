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
#include "wavegen.h"
#include "diskop.h"

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
	SHOW_ANALYZER = 2048,
#ifdef MIDI
	MIDI_SYNC = 4096,
#endif
	SHOW_LOGO = 8192,
	SHOW_DECIMALS = 16384,
	LOOP_POSITION = 32768,
	LOCK_SEQUENCE_STEP_AND_PATTERN_LENGTH = 65536,
	SHOW_DELAY_IN_TICKS = 65536 << 1,
	EDIT_SEQUENCE_DIGITS = 65536 << 2,
	EXPAND_ONLY_CURRENT_TRACK = 65536 << 3,
	DISABLE_NOSTALGY = 65536 << 4,
	TOGGLE_EDIT_ON_STOP = 65536 << 5,
	STOP_EDIT_ON_PLAY = 65536 << 6,
	MULTIKEY_JAMMING = 65536 << 7,
	DISABLE_VU_METERS = 65536 << 8,
	WINDOW_MAXIMIZED = 65536 << 9,
	SHOW_WAVEGEN = 65536 << 10,
	DISABLE_RENDER_TO_TEXTURE = 65536 << 11,
	DISABLE_BACKUPS = 65536 << 12,
	START_WITH_TEMPLATE = 65536 << 13,
};

enum
{
	VC_INSTRUMENT = 1,
	VC_VOLUME = 2,
	VC_CTRL = 4,
	VC_COMMAND = 8
};

enum
{
	VIS_SPECTRUM = 0,
	VIS_CATOMETER = 1,
	VIS_NUM_TOTAL
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
		current_patternx, current_patternpos, current_sequencepos, sequenceview_steps, single_pattern_edit, 
		prev_mode, current_sequenceparam, instrument_list_position,
		pattern_position, sequence_position, pattern_horiz_position, sequence_horiz_position,
		program_position, current_program_step,
		edit_reverb_param, selected_wavetable, wavetable_param, songinfo_param,
		loop_store_length, loop_store_loop, note_jump, wavetable_list_position, wavetable_preview_idx, sequence_digit;
	int current_sequencetrack;
	Uint16 time_signature;
	Clipboard cp;
	char * edit_buffer;
	int edit_buffer_size;
	SliderParam sequence_slider_param, pattern_slider_param, program_slider_param, instrument_list_slider_param, 
		pattern_horiz_slider_param, sequence_horiz_slider_param, wavetable_list_slider_param;
	char previous_song_filename[1000], previous_wav_filename[1000], previous_filebox_path[OD_T_N_TYPES][1000];
	/*---*/
	char * edit_backup_buffer;
	Selection selection;
	/* -- stat -- */
	int stat_song_position;
	int stat_pattern_position[MUS_MAX_CHANNELS];
	MusPattern *stat_pattern[MUS_MAX_CHANNELS];
	MusChannel *channel;
	int stat_pattern_number[MUS_MAX_CHANNELS], stat_note[MUS_MAX_CHANNELS];
	Uint64 time_played, play_start_at;
	/* ---- */
	char info_message[256];
	SDL_TimerID info_message_timer;
	GfxSurface *slider_bevel, *vu_meter, *analyzer, *logo, *catometer;
	Font smallfont, largefont, tinyfont, tinyfont_sequence_counter, tinyfont_sequence_normal;
	
	/* for menu */
	Font menufont, menufont_selected, headerfont, headerfont_selected, shortcutfont, shortcutfont_selected, buttonfont;
	
	char themename[100], keymapname[100];
	int pixel_scale;
	int mix_rate, mix_buffer;
	int window_w, window_h;
	int fx_bus, fx_room_size, fx_room_vol, fx_room_dec, fx_tap, fx_axis, fx_room_ticks, fx_room_prev_x, fx_room_prev_y;
	/*---vis---*/
	int current_visualizer;
	struct 
	{
		int cyd_env[MUS_MAX_CHANNELS];
		int spec_peak[96], spec_peak_decay[96];
		float prev_a;
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
	GfxSurface *wavetable_preview;
	Uint16 wavetable_bits;
	int prev_wavetable_x, prev_wavetable_y;
	
#ifdef MIDI
	Uint32 midi_device;
	Uint8 midi_channel;
	bool midi_start;
	Uint16 midi_rate;
	Uint32 midi_last_clock;
	Uint8 tick_ctr;
#endif

	WgSettings wgset;
	int selected_wg_osc, selected_wg_preset;
	
	int oversample;
} Mused;

extern Mused mused;
extern GfxDomain *domain;
extern Uint32 pattern_color[16];

#define NUM_PATTERNS 4096
#define NUM_INSTRUMENTS 128
#define NUM_SEQUENCES 2048

void default_settings();
void change_mode(int newmode);
void clear_pattern(MusPattern *pat);
void clear_pattern_range(MusPattern *pat, int first, int last);
void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES], MusChannel *channel);
void deinit();
void new_song();
void kt_default_instrument(MusInstrument *instrument);
void set_edit_buffer(char *buffer, size_t size);
void change_pixel_scale(void *a, void*b, void*c);
void mirror_flags();
void resize_pattern(MusPattern * pattern, Uint16 new_size);
void init_scrollbars();
void my_open_menu();
int viscol(int col);
void post_config_load();
void enable_callback(bool state);
int get_pattern(int abspos, int track);
int get_patternstep(int abspos, int track);
int current_pattern();
int current_pattern_for_channel(int channel);
int current_patternstep();
MusStep * get_current_step();
MusPattern * get_current_pattern();
void change_visualizer(int vis);
void set_info_message(const char *string, ...)  __attribute__ ((format (printf, 1, 2)));
void set_channels(int channels);
Uint32 get_playtime_at(int position);

#endif
