#ifndef MUSED_H
#define MUSED_H

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

#include "snd/music.h"
#include "gfx/font.h"
#include "console.h"
#include "gui/slider.h"

enum
{
	EDITINSTRUMENT,
	EDITPATTERN,
	EDITSEQUENCE,
	EDITREVERB,
	EDITCLASSIC,
	EDITPROG,
	/* Virtual modes, i.e. what are not modes itself but should be considered happening "inside" prev_mode */
	EDITBUFFER,
	MENU
};

#define VIRTUAL_MODE EDITBUFFER

#include "clipboard.h"
#include "copypaste.h"
#include "gui/menu.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240


enum
{
	COMPACT_VIEW = 1,
	SONG_PLAYING = 2,
	FULLSCREEN = 4,
	MULTICHANNEL_PREVIEW = 8
};

typedef struct
{
	Uint32 flags;
	int done;
	Console *console;
	MusSong song;
	CydEngine cyd;
	MusEngine mus;
	int octave, current_instrument, selected_param, editpos, mode, focus,
		current_patternstep, current_pattern, current_patternx, 
		current_sequencepos, sequenceview_steps, single_pattern_edit, 
		prev_mode, current_sequenceparam, instrument_list_position,
		pattern_position, sequence_position, pattern_horiz_position, sequence_horiz_position,
		program_position, current_program_step,
		edit_reverb_param;
	Uint16 *ghost_pattern[MUS_MAX_CHANNELS];
	int current_sequencetrack;
	Uint16 time_signature;
	Clipboard cp;
	char * edit_buffer;
	int edit_buffer_size;
	SliderParam sequence_slider_param, pattern_slider_param, program_slider_param, instrument_list_slider_param, pattern_horiz_slider_param, sequence_horiz_slider_param;
	/*---*/
	char * edit_backup_buffer;
	int stat_song_position;
	Selection selection;
	int stat_pattern_position[MUS_MAX_CHANNELS];
	MusPattern *stat_pattern[MUS_MAX_CHANNELS];
	MusChannel *channel;
	int stat_pattern_number[MUS_MAX_CHANNELS];
	SDL_Surface *slider_bevel;
	Font smallfont, largefont;
	SDL_Surface *screen;
	char themename[100];
	int pixel_scale;
	int mix_rate, mix_buffer;
} Mused;

#define NUM_PATTERNS 256
#define NUM_INSTRUMENTS 32
#define NUM_SEQUENCES 1024

void change_mode(int newmode);
void clear_pattern(MusPattern *pat);
void clear_pattern_range(MusPattern *pat, int first, int last);
void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES], MusChannel *channel, SDL_Surface *screen);
void deinit();
void new_song();
void default_instrument(MusInstrument *instrument);
void set_edit_buffer(char *buffer, size_t size);
void update_ghost_patterns();
void change_pixel_scale(void *a, void*b, void*c);
void mirror_flags();
void resize_pattern(MusPattern * pattern, Uint16 new_size);
void init_scrollbars();

#endif
