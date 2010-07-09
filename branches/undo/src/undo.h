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

typedef enum
{
	UNDO_PATTERN,
	UNDO_SEQUENCE,
	UNDO_INSTRUMENT,
	UNDO_SONGINFO,
	UNDO_MODE
} UndoType;

typedef union
{	
	struct { int channel; MusSeqPattern *seq; int n_seq; } sequence;
	struct { int idx; MusInstrument instrument; } instrument;
	struct { int idx; MusStep *step; int n_steps; } pattern;
	struct { int old_mode, focus; } mode;
	struct { } songinfo;
} UndoEvent;

typedef struct UndoFrame_t
{
	UndoType type;
	struct UndoFrame_t *prev;
	UndoEvent event;
} UndoFrame;

typedef UndoFrame *UndoStack;

void undo_init(UndoStack *stack);
void undo_deinit(UndoStack *stack);
void undo_destroy_frame(UndoFrame *frame);
void undo_add_frame(UndoStack *stack, UndoFrame *frame);

UndoFrame *undo(UndoStack *stack);

void undo_store_mode(UndoStack *stack, int old_mode, int focus);
void undo_store_instrument(UndoStack *stack, const MusInstrument *instrument);
void undo_store_sequence(UndoStack *stack, int channel, const MusSeqPattern *sequence, int n_seq);
void undo_store_pattern(UndoStack *stack, int idx, const MusPattern *pattern);

#endif