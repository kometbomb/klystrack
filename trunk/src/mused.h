#ifndef MUSED_H
#define MUSED_H

#include "snd/music.h"
#include "gfx/font.h"
#include "console.h"

enum
{
	EDITPROG,
	EDITINSTRUMENT,
	EDITPATTERN,
	EDITSEQUENCE,
	EDITBUFFER
};

#include "clipboard.h"


typedef struct
{
	Console *console;
	MusSong song;
	CydEngine cyd;
	MusEngine mus;
	int octave, current_instrument, selected_param, editpos, mode, 
		current_patternstep, current_pattern, current_patternx, 
		current_sequencepos, sequenceview_steps, single_pattern_edit, 
		prev_mode, current_sequenceparam;
	int ghost_pattern[MUS_CHANNELS];
	int current_sequencetrack;
	Uint16 time_signature;
	Clipboard cp;
	char * edit_buffer;
	int edit_buffer_size;
	/*---*/
	char * edit_backup_buffer;
	int stat_song_position;
	int selection_start,selection_end,selection_keydown;
	int stat_pattern_position[MUS_CHANNELS];
	MusPattern *stat_pattern[MUS_CHANNELS];
	int stat_pattern_number[MUS_CHANNELS];
} Mused;

#define NUM_STEPS 256
#define NUM_PATTERNS 32
#define NUM_INSTRUMENTS 32
#define NUM_SEQUENCES 256

void change_mode(int newmode);
void clear_pattern(MusPattern *pat);
void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_CHANNELS][NUM_SEQUENCES]);
void new_song();
void default_instrument(MusInstrument *instrument);

#endif
