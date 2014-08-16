#ifndef EDIT_H
#define EDIT_H

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
	S_T_PATTERN,
	S_T_SEQUENCE,
	S_T_MODE,
	S_T_SONGINFO,
	S_T_FX,
	S_T_WAVE_PARAM,
	S_T_WAVE_DATA,
	S_T_INSTRUMENT
} SHType;


/* a, b = id for cascading snapshots */
void snapshot(SHType type);
void snapshot_cascade(SHType type, int a, int b);

void zero_step(MusStep *step);
void clone_pattern(void *, void *, void *);
void clone_each_pattern(void *, void *, void *);
void get_unused_pattern(void*, void*, void*);
void expand_pattern(void *factor, void *, void *);
void shrink_pattern(void *factor, void *, void *);
void interpolate(void *, void *, void *);
void transpose_note_data(void *semitones, void *unused1, void *unused2);

#endif
