#ifndef OPTIMIZE_H
#define OPTIMIZE_H

/*
Copyright (c) 2009-2011 Tero Lindeman (kometbomb)

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

void optimize_duplicate_patterns(MusSong *song);
void optimize_song(MusSong *song);
bool is_pattern_empty(const MusPattern *a);
bool is_pattern_equal(const MusPattern *a, const MusPattern *b);
bool is_instrument_used(const MusSong *song, int instrument);
bool is_wavetable_used(const MusSong *song, int wavetable);

void optimize_patterns_action(void *unused1, void *unused2, void *unused3);
void optimize_instruments_action(void *unused1, void *unused2, void *unused3);
void optimize_wavetables_action(void *unused1, void *unused2, void *unused3);

#endif
