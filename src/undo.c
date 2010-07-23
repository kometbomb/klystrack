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
#include <stdbool.h>

extern bool inside_undo;

void undo_add_frame(UndoStack *stack, UndoFrame *frame)
{
	frame->prev = *stack;
	*stack = frame;
	
	debug("Added undo frame %p to %p", frame, stack);
}

static UndoEvent * get_frame(UndoType type, UndoStack *stack)
{
	if (inside_undo) return NULL;
	
	UndoFrame *frame = calloc(sizeof(UndoFrame), 1);
	undo_add_frame(stack, frame);
	frame->type = type;
	
#ifdef DEBUG
	undo_show_stack(stack);
#endif	
	
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
			
		default: break;
	}

	free(frame);
}


void undo_store_mode(UndoStack *stack, int old_mode, int focus)
{
	UndoEvent *frame = get_frame(UNDO_MODE, stack);
	
	if (!frame) return;
	
	frame->mode.old_mode = old_mode;
	frame->mode.focus = focus;
}


void undo_store_instrument(UndoStack *stack, int idx, const MusInstrument *instrument)
{
	UndoEvent *frame = get_frame(UNDO_INSTRUMENT, stack);
	
	if (!frame) return;
	
	frame->instrument.idx = idx;
	memcpy(&frame->instrument.instrument, instrument, sizeof(*instrument));
}


void undo_store_pattern(UndoStack *stack, int idx, const MusPattern *pattern)
{
	UndoEvent *frame = get_frame(UNDO_PATTERN, stack);
	
	if (!frame) return;
	
	frame->pattern.idx = idx;
	frame->pattern.n_steps = pattern->num_steps;
	frame->pattern.step = malloc(pattern->num_steps * sizeof(frame->pattern.step[0]));
	memcpy(frame->pattern.step, pattern->step, pattern->num_steps * sizeof(frame->pattern.step[0]));
}


void undo_store_sequence(UndoStack *stack, int channel, const MusSeqPattern *sequence, int n_seq)
{
	UndoEvent *frame = get_frame(UNDO_SEQUENCE, stack);
	
	if (!frame) return;
	
	frame->sequence.channel = channel;
	frame->sequence.n_seq = n_seq;
	frame->sequence.seq = malloc(n_seq * sizeof(frame->sequence.seq[0]));
	memcpy(frame->sequence.seq, sequence, n_seq * sizeof(frame->sequence.seq[0]));
}


void undo_store_songinfo(UndoStack *stack, const MusSong *song)
{
	UndoEvent *frame = get_frame(UNDO_SONGINFO, stack);
	
	if (!frame) return;
	
	frame->songinfo.song_length = song->song_length;  
	frame->songinfo.loop_point = song->loop_point;
	frame->songinfo.song_speed = song->song_speed;
	frame->songinfo.song_speed2 = song->song_speed2; 
	frame->songinfo.song_rate = song->song_rate;
	frame->songinfo.time_signature = song->time_signature;
	frame->songinfo.flags = song->flags;
	frame->songinfo.num_channels = song->num_channels;
	strcpy(frame->songinfo.title, song->title);
	frame->songinfo.master_volume = song->master_volume;
	memcpy(frame->songinfo.default_volume, song->default_volume, sizeof(frame->songinfo.default_volume));
}


void undo_store_fx(UndoStack *stack, int idx, const CydFxSerialized *fx, Uint8 multiplex_period)
{
	UndoEvent *frame = get_frame(UNDO_FX, stack);
	
	if (!frame) return;
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
	undo_destroy_frame(undo(stack));
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
