#include "edit.h"
#include "mused.h"
#include "msgbox.h"

extern Mused mused;

static int find_unused_pattern()
{
	for (int empty = 0 ; empty < NUM_PATTERNS ; ++empty)
	{
		int not_empty = 0;
		for (int s = 0 ; s < mused.song.pattern[empty].num_steps && !not_empty ; ++s)
			if (empty == mused.current_pattern || (mused.song.pattern[empty].step[s].note != MUS_NOTE_NONE
				|| mused.song.pattern[empty].step[s].ctrl != 0 
				|| mused.song.pattern[empty].step[s].instrument != MUS_NOTE_NO_INSTRUMENT
				|| mused.song.pattern[empty].step[s].command != 0))
				not_empty = 1;
		
		if (!not_empty)
			return empty;
	}
	
	msgbox("Max patterns exceeded!", MB_OK);
	
	return -1;
}


static void set_pattern(int pattern)
{
	if (!mused.single_pattern_edit)
	{
		for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
			if (mused.song.sequence[mused.current_sequencetrack][i].position == mused.current_sequencepos
				&& mused.song.sequence[mused.current_sequencetrack][i].pattern == mused.current_pattern)
				mused.song.sequence[mused.current_sequencetrack][i].pattern = pattern;
	}
	
	mused.current_pattern = pattern;
	
	update_ghost_patterns();
}


void clone_pattern(void *unused1, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN) return;
	
	int empty = find_unused_pattern();
	
	if (empty == -1 || mused.current_pattern == empty)
	{
		return;
	}
	
	resize_pattern(&mused.song.pattern[empty], mused.song.pattern[mused.current_pattern].num_steps);
	
	memcpy(mused.song.pattern[empty].step, mused.song.pattern[mused.current_pattern].step, 
		mused.song.pattern[mused.current_pattern].num_steps * sizeof(mused.song.pattern[mused.current_pattern].step[0]));
		
	set_pattern(empty);
}


void get_unused_pattern(void *unused1, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN) return;

	int empty = find_unused_pattern();
		
	if (empty == -1 || mused.current_pattern == empty)
	{
		return;
	}
	
	set_pattern(empty);
}


void zero_step(MusStep *step)
{
	step->note = MUS_NOTE_NONE;				
	step->instrument = MUS_NOTE_NO_INSTRUMENT;
	step->ctrl = 0;
	step->command = 0;
}


void expand_pattern(void *factor, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN) return;
	
	MusPattern *pattern = &mused.song.pattern[mused.current_pattern];
	
	MusStep *temp = malloc(pattern->num_steps * sizeof(pattern->step[0]));
	memcpy(temp, pattern->step, pattern->num_steps * sizeof(pattern->step[0]));
	
	resize_pattern(pattern, pattern->num_steps * CASTPTR(int,factor));
	
	for (int i = 0 ; i < pattern->num_steps ; ++i)
	{
		zero_step(&pattern->step[i]);
	}
	
	for (int i = 0, ti = 0 ; i < pattern->num_steps ; ++i)
	{
		if ((i % CASTPTR(int,factor)) == 0)
		{
			memcpy(&pattern->step[i], &temp[ti], sizeof(pattern->step[i]));
			++ti;
		}
	}
	
	free(temp);
}


void shrink_pattern(void *factor, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN) return;
	
	MusPattern *pattern = &mused.song.pattern[mused.current_pattern];
	
	if (pattern->num_steps <= CASTPTR(int,factor)) return;
	
	resize_pattern(pattern, pattern->num_steps / CASTPTR(int,factor));
	
	for (int i = 0, ti = 0 ; i < pattern->num_steps ; ++i, ti += CASTPTR(int,factor))
	{
		memcpy(&pattern->step[i], &pattern->step[ti], sizeof(pattern->step[i]));
	}
}
