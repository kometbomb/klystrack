/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

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

#include "mused.h"
#include "gfx/font.h"
#include "util/bundle.h"

extern Mused mused;

void change_mode(int newmode)
{
	SDL_FillRect(mused.console->surface, NULL, 0);

	mused.selection.start = 0;
	mused.selection.end = 0;
	mused.prev_mode = mused.mode;
	switch (newmode)
	{
		case EDITBUFFER:
		case EDITPROG:
		
		mused.editpos = 0;
		
		break;
		
		case EDITINSTRUMENT:
		
		break;
		
		case EDITPATTERN:
		if (mused.mode == EDITSEQUENCE)
		{
			for (int e = 0 ; e < mused.song.num_sequences[mused.current_sequencetrack] ; ++e)
					if (mused.song.sequence[mused.current_sequencetrack][e].position == mused.current_sequencepos)
						mused.current_sequencepos = mused.song.sequence[mused.current_sequencetrack][e].position;
			int p = 0;				
			for (int i = 0 ; i < MUS_CHANNELS ; ++i)
			{
				mused.ghost_pattern[i] = -1;
				
				for (int e = 0 ; e < mused.song.num_sequences[i] ; ++e)
					if (mused.song.sequence[i][e].position == mused.current_sequencepos)
					{
						mused.ghost_pattern[i] = mused.song.sequence[i][e].pattern;
						++p;
					}
			}
			mused.current_patternstep = 0;
			mused.current_patternx = 0;
			mused.single_pattern_edit = (p <= 1);
		}
		else
		{
			mused.single_pattern_edit = 1;
			for (int i = 0 ; i < MUS_CHANNELS ; ++i)
			{
				mused.ghost_pattern[i] = -1;
			}
			mused.current_patternstep = 0;
			mused.current_patternx = 0;
		}
		break;
		
		case EDITSEQUENCE:
		
		break;
	}

	mused.mode = newmode;
}


void clear_pattern(MusPattern *pat)
{
	clear_pattern_range(pat, 0, pat->num_steps);
}


void clear_pattern_range(MusPattern *pat, int first, int last)
{
	for (int i = first ; i < last ; ++i)
	{
		pat->step[i].note = MUS_NOTE_NONE;
		pat->step[i].instrument = MUS_NOTE_NO_INSTRUMENT;
		pat->step[i].ctrl = 0;
		pat->step[i].command = 0;
	}
}


void new_song()
{
	for (int i = 0 ; i < NUM_INSTRUMENTS ; ++i)
	{
		MusInstrument *inst = &mused.song.instrument[i];
		default_instrument(inst);
		
	}
	
	mused.song.num_instruments = NUM_INSTRUMENTS;
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.song_speed = 6;
	mused.song.song_speed2 = 6;
	mused.song.song_rate = 50;
	mused.song.song_length = 0;
	mused.song.loop_point = 0;
	
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		clear_pattern(&mused.song.pattern[i]);
	}
}


void default_instrument(MusInstrument *inst)
{
	mus_get_default_instrument(inst);
}


void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_CHANNELS][NUM_SEQUENCES])
{
	mused.done = 0;
	mused.octave = 4;
	mused.current_instrument = 0;
	mused.selected_param = 0;
	mused.editpos = 0;
	mused.mode = EDITINSTRUMENT;
	mused.current_patternstep = 0;
	mused.current_pattern = 0;
	mused.current_patternx = 0;
	mused.current_sequencepos = 0;
	mused.sequenceview_steps = 16;
	mused.current_sequencetrack = 0;
	mused.time_signature = 0x0404;
	mused.prev_mode = 0;
	mused.edit_backup_buffer = NULL;
	
	change_mode(EDITSEQUENCE);
	
	memset(&mused.cp, 0, sizeof(mused.cp));
	memset(&mused.song, 0, sizeof(mused.song));
	mused.song.instrument = instrument;
	mused.song.pattern = pattern;

	for (int i = 0 ; i < MUS_CHANNELS ; ++i)
	{
		mused.song.sequence[i] = sequence[i];	
	}
	
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		mused.song.pattern[i].step = malloc(NUM_STEPS*sizeof(*pattern[i].step));
		mused.song.pattern[i].num_steps = 16;
	}
	
	new_song();
}

