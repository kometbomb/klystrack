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


#include "optimize.h"
#include "edit.h"
#include "macros.h"

static bool is_pattern_used(const MusSong *song, int p)
{
	for (int c = 0 ; c < song->num_channels ; ++c)
	{
		for (int s = 0 ; s < song->num_sequences[c] ; ++s)
		{
			if (song->sequence[c][s].pattern == p)
			{
				return true;
			}
		}
	}

	return false;
}


static void replace_pattern(MusSong *song, int from, int to)
{
	for (int c = 0 ; c < song->num_channels ; ++c)
	{
		for (int s = 0 ; s < song->num_sequences[c] ; ++s)
		{
			if (song->sequence[c][s].pattern == from)
			{
				song->sequence[c][s].pattern = to;
			}
		}
	}
}


bool is_pattern_equal(const MusPattern *a, const MusPattern *b)
{
	if (b->num_steps != a->num_steps)
		return false;
		
	for (int i = 0 ; i < a->num_steps ; ++i)
		if (a->step[i].note != b->step[i].note 
			|| a->step[i].instrument != b->step[i].instrument
			|| a->step[i].volume != b->step[i].volume
			|| a->step[i].ctrl != b->step[i].ctrl
			|| a->step[i].command != b->step[i].command)
			return false;

	return true;
}


bool is_pattern_empty(const MusPattern *a)
{
	for (int i = 0 ; i < a->num_steps ; ++i)
		if (a->step[i].note != MUS_NOTE_NONE
			|| a->step[i].instrument != MUS_NOTE_NO_INSTRUMENT
			|| a->step[i].volume != MUS_NOTE_NO_VOLUME
			|| a->step[i].ctrl != 0
			|| a->step[i].command != 0)
			return false;

	return true;
}


static void remove_pattern(MusSong *song, int p)
{
	void * temp = song->pattern[p].step;
	
	for (int i = 0 ; i < song->pattern[p].num_steps ; ++i)
		zero_step(&song->pattern[p].step[i]);

	for (int a = p ; a < song->num_patterns - 1 ; ++a)
	{
		memcpy(&song->pattern[a], &song->pattern[a + 1], sizeof(song->pattern[a]));
		replace_pattern(song, a + 1, a);
	}
	
	if (song->num_patterns - 1 >= 0)
		song->pattern[song->num_patterns - 1].step = temp;
	
	--song->num_patterns;
}



void optimize_duplicate_patterns(MusSong *song)
{
	debug("Optimizing song patterns");
	
	int orig_count = song->num_patterns;

	for (int a = 0 ; a < song->num_patterns ; ++a)
	{	
		if (is_pattern_used(song, a))
		{
			for (int b = a + 1 ; b < song->num_patterns ; ++b)
			{	
				if (is_pattern_used(song, b) && is_pattern_equal(&song->pattern[a], &song->pattern[b]))
				{
					replace_pattern(song, b, a);
				}
			}
		}
	}
	
	for (int a = 0 ; a < song->num_patterns ; )
	{	
		if (!is_pattern_used(song, a))
		{
			remove_pattern(song, a);
		}
		else 
			++a;
	}
	
	debug("Reduced number of patterns from %d to %d", orig_count, song->num_patterns);
}


void optimize_song(MusSong *song)
{
	debug("Optimizing song");
	optimize_duplicate_patterns(song);
}
