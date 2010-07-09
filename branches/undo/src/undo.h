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

typedef enum
{
	UNDO_PATTERN,
	UNDO_SEQUENCE,
	UNDO_INSTRUMENT,
	UNDO_SONG,
	UNDO_MODE
} UndoType;

typedef union
{	
	struct { int channel; MusSeqPattern *seq; int n_seq; } sequence;
	struct { int idx; MusInstrument instrument; } instrument;
	struct { int idx; MusStep *step; int n_steps; int pattern_length; } pattern;
	struct { int old_mode; } mode;
} UndoEvent;

typedef struct UndoFrame_t
{
	UndoType type;
	struct UndoFrame_t *prev;
	UndoEvent event;
} UndoFrame;

typedef UndoFrame **UndoStack;

void undo_init(UndoStack stack);
void undo_store_mode(UndoStack stack, int old_mode);

#endif