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

#include "copypaste.h"
#include "clipboard.h"
#include "mused.h"
#include "event.h"

#define swap(a,b) { a ^= b; b ^= a; a ^= b; }


extern Clipboard cp;
extern Mused mused;

void copy()
{
	switch (mused.mode)
	{
		case EDITPATTERN:
		
		if (mused.selection.start == mused.selection.end)
			cp_copy_items(&mused.cp, CP_PATTERN, mused.song.pattern[mused.current_pattern].step, sizeof(mused.song.pattern[mused.current_pattern].step[0]), mused.song.pattern[mused.current_pattern].num_steps);
		else
			cp_copy_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[mused.current_pattern].step[mused.selection.start], sizeof(mused.song.pattern[mused.current_pattern].step[0]), 
				mused.selection.end-mused.selection.start);
		
		break;
		
		case EDITINSTRUMENT:
		{
			if (mused.selection.start < P_PARAMS)
				cp_copy(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], sizeof(mused.song.instrument[mused.current_instrument]));
			else 
				cp_copy_items(&mused.cp, CP_PROGRAM, &mused.song.instrument[mused.current_instrument].program[mused.selection.start - P_PARAMS], mused.selection.end-mused.selection.start, 
					sizeof(mused.song.instrument[mused.current_instrument].program[0]));
		}
		break;
		
		case EDITSEQUENCE:
		{
		
		int first = 0, last = 0;
		
		for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		{
			if (mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection.start)
				first = i + 1;
				
			if (mused.song.sequence[mused.current_sequencetrack][i].position >= mused.selection.start && mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection.end)
				last = i;
		}
		
		cp_copy_items(&mused.cp, CP_SEQUENCE, &mused.song.sequence[mused.current_sequencetrack][first], last-first+1, sizeof(mused.song.sequence[mused.current_sequencetrack][0]));
		
		}
		break;
	}
	
	mused.selection.start = mused.selection.end = 0;
}

void cut()
{
	copy();
	delete();
}


void delete()
{
/*	switch (mused.mode)
	{
		case EDITPATTERN:
		
		if (mused.selection_start == mused.selection_end)
			clear_pattern(&mused.song.pattern[mused.current_pattern]);
		else
			cp_copy_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[mused.current_pattern].step[mused.selection_start], sizeof(mused.song.pattern[mused.current_pattern].step[0]), mused.selection_end-mused.selection_start);
		
		break;
		
		case EDITINSTRUMENT:
		
		cp_copy(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], sizeof(mused.song.instrument[mused.current_instrument]));
		
		break;
	}*/
}


void paste()
{
	switch (mused.mode)
	{
		case EDITSEQUENCE:
		{
			if (mused.cp.type != CP_SEQUENCE) break;
			
			int items = cp_get_item_count(&mused.cp, sizeof(mused.song.sequence[0]));
			
			if (items < 1) break;
			
			int first = ((MusSeqPattern*)mused.cp.data)[0].position;
			int last = ((MusSeqPattern*)mused.cp.data)[items-1].position;
			
			del_sequence(mused.current_sequencepos, last-first+mused.current_sequencepos, mused.current_sequencetrack);
			
			for (int i = 0 ; i < items ; ++i)
			{
				add_sequence(((MusSeqPattern*)mused.cp.data)[i].position-first+mused.current_sequencepos, ((MusSeqPattern*)mused.cp.data)[i].pattern, ((MusSeqPattern*)mused.cp.data)[i].note_offset);
			}
		}
		break;
		
		case EDITPATTERN:
		{
			if (mused.cp.type == CP_PATTERN)
				cp_paste_items(&mused.cp, CP_PATTERN, mused.song.pattern[mused.current_pattern].step, NUM_STEPS, sizeof(mused.song.pattern[mused.current_pattern].step[0]));
			else if (mused.cp.type == CP_PATTERNSEGMENT)
				cp_paste_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[mused.current_pattern].step[mused.current_patternstep], NUM_STEPS-mused.current_patternstep, 
					sizeof(mused.song.pattern[mused.current_pattern].step[0]));
		}
		break;
	
		case EDITINSTRUMENT:
		{
			if (mused.cp.type == CP_INSTRUMENT)
			{
				cp_paste_items(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], 1, sizeof(mused.song.instrument[mused.current_instrument]));
			}
			else if (mused.cp.type == CP_PROGRAM)
			{
				if (mused.selected_param >= P_PARAMS)
					cp_paste_items(&mused.cp, CP_PROGRAM, &mused.song.instrument[mused.current_instrument].program[mused.selected_param - P_PARAMS], MUS_PROG_LEN - (mused.selected_param - P_PARAMS), 
						sizeof(mused.song.instrument[mused.current_instrument].program[0]));
			}
		}
		break;
	}
}


void begin_selection(int position)
{
	mused.selection.keydown = position;
}


void select_range(int position)
{
	mused.selection.start = mused.selection.keydown;
	
	if (mused.selection.end == position)
		mused.selection.end = position + 1; // so we can select the last row (can't move the cursor one element after the last)
	else
		mused.selection.end = position;
		
	if (mused.selection.end < mused.selection.start)
	{
		swap(mused.selection.start, mused.selection.end);
	}
}
