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

#include "undo.h"
#include "macros.h"
#include "mused.h"
#include <stdbool.h>

extern bool inside_undo;

extern Mused mused;

void undo_add_frame(UndoStack *stack, UndoFrame *frame)
{
	frame->prev = *stack;
	*stack = frame;
	
	//debug("Added undo frame %p to %p", frame, stack);
}

static UndoEvent * get_frame(UndoType type, UndoStack *stack, bool modified)
{
	if (inside_undo) return NULL;
	
	UndoFrame *frame = calloc(sizeof(UndoFrame), 1);
	undo_add_frame(stack, frame);
	frame->type = type;
	frame->modified = modified;
	
/*#ifdef DEBUG
	undo_show_stack(stack);
#endif	*/
	
	return &frame->event;
}


void undo_destroy_frame(UndoFrame *frame)
{
	switch (frame->type)
	{
		case UNDO_SEQUENCE:
			free(frame->event.sequence.seq);
			break;
			
		case UNDO_PATTERN:
			free(frame->event.pattern.step);
			break;
			
		case UNDO_WAVE_DATA:
			free(frame->event.wave_data.data);
			break;
			
		default: break;
	}

	free(frame);
}


void undo_store_mode(UndoStack *stack, int old_mode, int focus, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_MODE, stack, modified);
	
	if (!frame) return;
	
	frame->mode.old_mode = old_mode;
	frame->mode.focus = focus;
}


void undo_store_instrument(UndoStack *stack, int idx, const MusInstrument *instrument, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_INSTRUMENT, stack, modified);
	
	if (!frame) return;
	
	frame->instrument.idx = idx;
	memcpy(&frame->instrument.instrument, instrument, sizeof(*instrument));
}


void undo_store_pattern(UndoStack *stack, int idx, const MusPattern *pattern, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_PATTERN, stack, modified);
	
	if (!frame) return;
	
	frame->pattern.idx = idx;
	frame->pattern.n_steps = pattern->num_steps;
	frame->pattern.step = malloc(pattern->num_steps * sizeof(frame->pattern.step[0]));
	memcpy(frame->pattern.step, pattern->step, pattern->num_steps * sizeof(frame->pattern.step[0]));
}


void undo_store_sequence(UndoStack *stack, int channel, const MusSeqPattern *sequence, int n_seq, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_SEQUENCE, stack, modified);
	
	if (!frame) return;
	
	frame->sequence.channel = channel;
	frame->sequence.n_seq = n_seq;
	frame->sequence.seq = malloc(n_seq * sizeof(frame->sequence.seq[0]));
	memcpy(frame->sequence.seq, sequence, n_seq * sizeof(frame->sequence.seq[0]));
}


void undo_store_songinfo(UndoStack *stack, const MusSong *song, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_SONGINFO, stack, modified);
	
	if (!frame) return;
	
	frame->songinfo.song_length = song->song_length;  
	frame->songinfo.loop_point = song->loop_point;
	frame->songinfo.sequence_step = mused.sequenceview_steps;
	frame->songinfo.song_speed = song->song_speed;
	frame->songinfo.song_speed2 = song->song_speed2; 
	frame->songinfo.song_rate = song->song_rate;
	frame->songinfo.time_signature = song->time_signature;
	frame->songinfo.flags = song->flags;
	frame->songinfo.num_channels = song->num_channels;
	strcpy(frame->songinfo.title, song->title);
	frame->songinfo.master_volume = song->master_volume;
	memcpy(frame->songinfo.default_volume, song->default_volume, sizeof(frame->songinfo.default_volume));
	memcpy(frame->songinfo.default_panning, song->default_panning, sizeof(frame->songinfo.default_panning));
}


void undo_store_fx(UndoStack *stack, int idx, const CydFxSerialized *fx, Uint8 multiplex_period, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_FX, stack, modified);
	
	if (!frame) return;
	
	frame->fx.idx = idx;
	memcpy(&frame->fx.fx, fx, sizeof(*fx));
	frame->fx.multiplex_period = multiplex_period;
}


void undo_init(UndoStack *stack)
{
	*stack = NULL;
}


void undo_deinit(UndoStack *stack)
{
	while (*stack)
	{
		UndoFrame *frame = *stack;
		*stack = frame->prev;
		
		undo_destroy_frame(frame);
	}
	
	*stack = NULL;
}


UndoFrame *undo(UndoStack *stack)
{
	if (!*stack) return NULL;
	
	UndoFrame *frame = *stack;
	
	*stack = (*stack)->prev;
	
	return frame;
}

void undo_pop(UndoStack *stack)
{
	UndoFrame *f = undo(stack);
	if (f)
		undo_destroy_frame(f);
}

#ifdef DEBUG
void undo_show_stack(UndoStack *stack)
{
	UndoFrame *frame = *stack;
	
	printf("%p [", stack);
	
	while (frame)
	{
		printf(" %d ", frame->type);
		frame = frame->prev;
	}
	
	printf("]\n");
}
#endif


void undo_store_wave_data(UndoStack *stack, int idx, const CydWavetableEntry *entry, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_WAVE_DATA, stack, modified);
	
	if (!frame) return;
	
	frame->wave_data.idx = idx;
	frame->wave_data.length = entry->samples;
	frame->wave_data.sample_rate = entry->sample_rate;
	frame->wave_data.samples = entry->samples; 
	frame->wave_data.loop_begin = entry->loop_begin;
	frame->wave_data.loop_end = entry->loop_end;
	frame->wave_data.flags = entry->flags;
	frame->wave_data.base_note = entry->base_note;
	frame->wave_data.data = malloc(entry->samples * sizeof(entry->data[0]));
	memcpy(frame->wave_data.data, entry->data, entry->samples * sizeof(entry->data[0]));
}


void undo_store_wave_param(UndoStack *stack, int idx, const CydWavetableEntry *entry, bool modified)
{
	UndoEvent *frame = get_frame(UNDO_WAVE_PARAM, stack, modified);
	
	if (!frame) return;
	
	frame->wave_param.idx = idx;
	frame->wave_param.flags = entry->flags;
	frame->wave_param.sample_rate = entry->sample_rate;
	frame->wave_param.samples = entry->samples; 
	frame->wave_param.loop_begin = entry->loop_begin;
	frame->wave_param.loop_end = entry->loop_end;
	frame->wave_param.base_note = entry->base_note;
}
