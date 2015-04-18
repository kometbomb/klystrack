#ifndef UNDO_H
#define UNDO_H

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
#include <stdbool.h>

typedef enum
{
	UNDO_PATTERN,
	UNDO_SEQUENCE,
	UNDO_INSTRUMENT,
	UNDO_FX,
	UNDO_SONGINFO,
	UNDO_MODE,
	UNDO_WAVE_PARAM,
	UNDO_WAVE_DATA,
	UNDO_WAVE_NAME
} UndoType;

typedef union
{	
	struct { int channel; MusSeqPattern *seq; int n_seq; } sequence;
	struct { int idx; MusInstrument instrument; } instrument;
	struct { int idx; MusStep *step; int n_steps; } pattern;
	struct { CydFxSerialized fx; int idx; Uint8 multiplex_period; } fx;
	struct { int old_mode, focus; } mode;
	struct { 
		Uint16 song_length, loop_point, sequence_step;
		Uint8 song_speed, song_speed2, song_rate;
		Uint16 time_signature;
		Uint32 flags;
		Uint8 num_channels;
		char title[MUS_SONG_TITLE_LEN + 1];
		Uint8 master_volume, default_volume[MUS_MAX_CHANNELS], default_panning[MUS_MAX_CHANNELS];
	} songinfo;
	struct {
		int idx;
		Uint32 flags;
		Uint32 sample_rate;
		Uint32 samples, loop_begin, loop_end;
		Uint16 base_note;
	} wave_param;
	struct {
		int idx;
		void *data;
		size_t length;
		Uint32 flags;
		Uint32 sample_rate;
		Uint32 samples, loop_begin, loop_end;
		Uint16 base_note;
	} wave_data;
	struct {
		int idx;
		char *name;
	} wave_name;
} UndoEvent;

typedef struct UndoFrame_t
{
	UndoType type;
	struct UndoFrame_t *prev;
	UndoEvent event;
	bool modified;
} UndoFrame;

typedef UndoFrame *UndoStack;

void undo_init(UndoStack *stack);
void undo_deinit(UndoStack *stack);
void undo_destroy_frame(UndoFrame *frame);
void undo_add_frame(UndoStack *stack, UndoFrame *frame);

/* Use when undo state stored but then the action is cancelled */
void undo_pop(UndoStack *stack);

/* Pops the topmost frame from stack, use undo_destroy_frame() after processed */
UndoFrame *undo(UndoStack *stack);

void undo_store_mode(UndoStack *stack, int old_mode, int focus, bool modified);
void undo_store_instrument(UndoStack *stack, int idx, const MusInstrument *instrument, bool modified);
void undo_store_sequence(UndoStack *stack, int channel, const MusSeqPattern *sequence, int n_seq, bool modified);
void undo_store_songinfo(UndoStack *stack, const MusSong *song, bool modified);
void undo_store_fx(UndoStack *stack, int idx, const CydFxSerialized *fx, Uint8 multiplex_period, bool modified);
void undo_store_pattern(UndoStack *stack, int idx, const MusPattern *pattern, bool modified);
void undo_store_wave_data(UndoStack *stack, int idx, const CydWavetableEntry *entry, bool modified);
void undo_store_wave_name(UndoStack *stack, int idx, const char *name, bool modified);
void undo_store_wave_param(UndoStack *stack, int idx, const CydWavetableEntry *entry, bool modified);

#ifdef DEBUG
void undo_show_stack(UndoStack *stack);
#endif

#endif
