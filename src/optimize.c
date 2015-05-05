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
#include "mused.h"
#include <string.h>

bool is_pattern_used(const MusSong *song, int p)
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


bool is_instrument_used(const MusSong *song, int instrument)
{
	for (int p = 0 ; p < song->num_patterns ; ++p)
	{
		for (int i = 0 ; i < song->pattern[p].num_steps ; ++i)
		{
			if (song->pattern[p].step[i].instrument == instrument)
				return true;
		}
	}
	
	return false;
}


static void remove_instrument(MusSong *song, int instrument)
{
	for (int p = 0 ; p < song->num_patterns ; ++p)
	{
		for (int i = 0 ; i < song->pattern[p].num_steps ; ++i)
		{
			if (song->pattern[p].step[i].instrument != MUS_NOTE_NO_INSTRUMENT && song->pattern[p].step[i].instrument > instrument)
				song->pattern[p].step[i].instrument--;
		}
	}
	
	for (int i = instrument ; i < song->num_instruments - 1 ; ++i)
		memcpy(&song->instrument[i], &song->instrument[i + 1], sizeof(song->instrument[i]));
	
	kt_default_instrument(&song->instrument[song->num_instruments - 1]);
}


bool is_wavetable_used(const MusSong *song, int wavetable)
{
	for (int i = 0 ; i < song->num_instruments ; ++i)
	{
		if (song->instrument[i].wavetable_entry == wavetable)
		{
			debug("Wavetable %x used by instrument %x wave", wavetable, i);
			return true;
		}
		
		if (song->instrument[i].fm_wave == wavetable)
		{
			debug("Wavetable %x used by instrument %x FM", wavetable, i);
			return true;
		}
		
		for (int p = 0 ; p < MUS_PROG_LEN ; ++p)
			if ((song->instrument[i].program[p] & 0x7fff) == (MUS_FX_SET_WAVETABLE_ITEM | wavetable))
			{
				debug("Wavetable %x used by instrument %x program (step %d)", wavetable, i, p);
				return true;
			}
	}
	
	for (int p = 0 ; p < song->num_patterns ; ++p)
	{
		for (int i = 0 ; i < song->pattern[p].num_steps ; ++i)
		{
			if ((song->pattern[p].step[i].command) == (MUS_FX_SET_WAVETABLE_ITEM | wavetable))
			{
				debug("Wavetable %x used by pattern %x command (step %d)", wavetable, p, i);
				return true;
			}
		}
	}
	
	return false;
}


static void remove_wavetable(MusSong *song, CydEngine *cyd, int wavetable)
{
	debug("Removing wavetable item %d", wavetable);
	
	for (int i = 0 ; i < song->num_instruments ; ++i)
	{
		if (song->instrument[i].wavetable_entry > wavetable)
			song->instrument[i].wavetable_entry--;
		
		if (song->instrument[i].fm_wave == wavetable)
			song->instrument[i].fm_wave--;
		
		for (int p = 0 ; p < MUS_PROG_LEN ; ++p)
			if ((song->instrument[i].program[p] & 0x7f00) == MUS_FX_SET_WAVETABLE_ITEM)
			{
				Uint8 param = song->instrument[i].program[p] & 0xff;
				
				if (param > wavetable)
					song->instrument[i].program[p] = (song->instrument[i].program[p] & 0x8000) | MUS_FX_SET_WAVETABLE_ITEM | (param - 1);
			}
	}
	
	for (int p = 0 ; p < song->num_patterns ; ++p)
	{
		for (int i = 0 ; i < song->pattern[p].num_steps ; ++i)
		{
			if ((song->pattern[p].step[i].command & 0xff00) == MUS_FX_SET_WAVETABLE_ITEM)
			{
				Uint8 param = song->pattern[p].step[i].command & 0xff;
				
				if (param > wavetable)
					song->pattern[p].step[i].command = MUS_FX_SET_WAVETABLE_ITEM | (param - 1);
			}
		}
	}
	
	for (int i = wavetable ; i < song->num_wavetables - 1 ; ++i)
	{
		strcpy(song->wavetable_names[i], song->wavetable_names[i + 1]);
		
		cyd->wavetable_entries[i].flags = cyd->wavetable_entries[i + 1].flags;
		cyd->wavetable_entries[i].sample_rate = cyd->wavetable_entries[i + 1].sample_rate;
		cyd->wavetable_entries[i].samples = cyd->wavetable_entries[i + 1].samples;
		cyd->wavetable_entries[i].loop_begin = cyd->wavetable_entries[i + 1].loop_begin;
		cyd->wavetable_entries[i].loop_end = cyd->wavetable_entries[i + 1].loop_end;
		cyd->wavetable_entries[i].base_note = cyd->wavetable_entries[i + 1].base_note;
		cyd->wavetable_entries[i].data = realloc(cyd->wavetable_entries[i].data, cyd->wavetable_entries[i + 1].samples * sizeof(Sint16));
		memcpy(cyd->wavetable_entries[i].data, cyd->wavetable_entries[i + 1].data, cyd->wavetable_entries[i + 1].samples * sizeof(Sint16));
	}
	
	strcpy(song->wavetable_names[song->num_wavetables - 1], "");
	cyd_wave_entry_init(&cyd->wavetable_entries[song->num_wavetables - 1], NULL, 0, 0, 0, 0, 0);
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
	
	if (song->num_patterns >= 1)
		song->pattern[song->num_patterns - 1].step = temp;
	
	--song->num_patterns;
}


void optimize_duplicate_patterns(MusSong *song)
{
	debug("Kill unused patterns");
	
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
	
	set_info_message("Reduced number of patterns from %d to %d", orig_count, song->num_patterns);
	
	song->num_patterns = NUM_PATTERNS;
}


void optimize_unused_instruments(MusSong *song)
{
	int removed = 0;
	
	debug("Kill unused instruments");
	
	for (int i = 0 ; i < song->num_instruments ; ++i)
		if (!is_instrument_used(song, i))
		{
			remove_instrument(song, i);
			++removed;
		}
		
	set_info_message("Removed %d unused instruments", removed);
}


void optimize_unused_wavetables(MusSong *song, CydEngine *cyd)
{
	int removed = 0;
	
	debug("Kill unused wavetables");
	
	for (int i = 0 ; i < song->num_wavetables ; ++i)
		if (!is_wavetable_used(song, i))
		{
			remove_wavetable(song, cyd, i);
			++removed;
		}
		
	set_info_message("Removed %d unused wavetables", removed);
}


void optimize_song(MusSong *song)
{
	debug("Optimizing song");
	optimize_duplicate_patterns(song);
	optimize_unused_instruments(&mused.song);
	optimize_unused_wavetables(&mused.song, &mused.cyd);
	
	set_info_message("Removed unused song data");
}


void optimize_patterns_action(void *unused1, void *unused2, void *unused3)
{
	optimize_duplicate_patterns(&mused.song);
}


void optimize_instruments_action(void *unused1, void *unused2, void *unused3)
{
	optimize_unused_instruments(&mused.song);
}


void optimize_wavetables_action(void *unused1, void *unused2, void *unused3)
{
	optimize_unused_wavetables(&mused.song, &mused.cyd);
}
