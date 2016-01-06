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

#include "copypaste.h"
#include "clipboard.h"
#include "mused.h"
#include "event.h"

#define swap(a,b) { a ^= b; b ^= a; a ^= b; }


extern Mused mused;

void copy()
{
	cp_clear(&mused.cp);

	switch (mused.focus)
	{
		case EDITPATTERN:
		
		if (mused.selection.start == mused.selection.end && get_current_pattern())
			cp_copy_items(&mused.cp, CP_PATTERN, get_current_pattern()->step, sizeof(MusStep), get_current_pattern()->num_steps, mused.selection.start);
		else if (get_pattern(mused.selection.start, mused.current_sequencetrack) != -1)
			cp_copy_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[get_pattern(mused.selection.start, mused.current_sequencetrack)].step[get_patternstep(mused.selection.start, mused.current_sequencetrack)], sizeof(MusStep), 
				mused.selection.end-mused.selection.start, mused.selection.start);
		
		break;
		
		case EDITINSTRUMENT:
		{
			cp_copy(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], sizeof(mused.song.instrument[mused.current_instrument]), 0);
		}
		break;
		
		case EDITPROG:
		{
			cp_copy_items(&mused.cp, CP_PROGRAM, &mused.song.instrument[mused.current_instrument].program[mused.selection.start], mused.selection.end-mused.selection.start, 
				sizeof(mused.song.instrument[mused.current_instrument].program[0]), mused.selection.start);
		}
		break;
		
		case EDITSEQUENCE:
		{
			int first = -1, last = -1;
			
			for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
			{
				if (first == -1 && mused.song.sequence[mused.current_sequencetrack][i].position >= mused.selection.start && mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection.end)
					first = i;
					
				if (mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection.end)
					last = i;
			}
			
			// Check if no items inside the selection
			
			if (first == -1 || first >= mused.song.num_sequences[mused.current_sequencetrack]) 
				break;
			
			cp_copy_items(&mused.cp, CP_SEQUENCE, &mused.song.sequence[mused.current_sequencetrack][first], last-first+1, sizeof(mused.song.sequence[mused.current_sequencetrack][0]), mused.selection.start);
		}
		break;
	}
}

void cut()
{
	copy();
	delete();
}


void delete()
{
	switch (mused.focus)
	{
		case EDITPATTERN:
		snapshot(S_T_PATTERN);
		if (mused.selection.start == mused.selection.end)
			clear_pattern(&mused.song.pattern[current_pattern()]);
		else
			clear_pattern_range(&mused.song.pattern[get_pattern(mused.selection.start, mused.current_sequencetrack)], get_patternstep(mused.selection.start, mused.current_sequencetrack), get_patternstep(mused.selection.end, mused.current_sequencetrack));
		
		break;
		
		case EDITSEQUENCE:
		snapshot(S_T_SEQUENCE);
		del_sequence(mused.selection.start, mused.selection.end, mused.current_sequencetrack);
		
		break;
	}
	
	mused.selection.start = mused.selection.end = 0;
}


void paste()
{
	switch (mused.focus)
	{
		case EDITSEQUENCE:
		{
			if (mused.cp.type != CP_SEQUENCE) break;
			
			size_t items = cp_get_item_count(&mused.cp, sizeof(mused.song.sequence[0][0]));
			
			if (items < 1) break;
			
			snapshot(S_T_SEQUENCE);
			
			int first = ((MusSeqPattern*)mused.cp.data)[0].position;
			int last = ((MusSeqPattern*)mused.cp.data)[items-1].position;
			
			del_sequence(mused.current_sequencepos, last-first+mused.current_sequencepos, mused.current_sequencetrack);
			
			for (int i = 0 ; i < items ; ++i)
			{
				add_sequence(mused.current_sequencetrack, ((MusSeqPattern*)mused.cp.data)[i].position-mused.cp.position+mused.current_sequencepos, ((MusSeqPattern*)mused.cp.data)[i].pattern, ((MusSeqPattern*)mused.cp.data)[i].note_offset);
			}
		}
		break;
		
		case EDITPATTERN:
		{
			size_t items = cp_get_item_count(&mused.cp, sizeof(mused.song.pattern[current_pattern()].step[0]));
			
			if (items < 1) 
				break;
			
			if (mused.cp.type == CP_PATTERN)
			{
				snapshot(S_T_PATTERN);
				resize_pattern(&mused.song.pattern[current_pattern()], items);
				cp_paste_items(&mused.cp, CP_PATTERN, mused.song.pattern[current_pattern()].step, items, sizeof(mused.song.pattern[current_pattern()].step[0]));
			}
			else if (mused.cp.type == CP_PATTERNSEGMENT && (get_pattern(mused.selection.start, mused.current_sequencetrack) != -1))
			{
				snapshot(S_T_PATTERN);
				cp_paste_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[current_pattern()].step[current_patternstep()], mused.song.pattern[current_pattern()].num_steps-current_patternstep(), 
					sizeof(mused.song.pattern[current_pattern()].step[0]));
			}
		}
		break;
	
		case EDITINSTRUMENT:
		{
			if (mused.cp.type == CP_INSTRUMENT)
			{
				snapshot(S_T_INSTRUMENT);
			
				cp_paste_items(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], 1, sizeof(mused.song.instrument[mused.current_instrument]));
			}
		}
		break;
		
		case EDITPROG:
		{
			size_t items = cp_get_item_count(&mused.cp, sizeof(mused.song.instrument[mused.current_instrument].program[0]));
			
			if (items < 1) 
				break;
		
			if (mused.cp.type == CP_PROGRAM)
			{
				snapshot(S_T_INSTRUMENT);
				cp_paste_items(&mused.cp, CP_PROGRAM, &mused.song.instrument[mused.current_instrument].program[mused.current_program_step], MUS_PROG_LEN - mused.current_program_step, 
					sizeof(mused.song.instrument[mused.current_instrument].program[0]));
			}
		}
		break;
	}
}


void join_paste()
{
	switch (mused.focus)
	{
		case EDITSEQUENCE:
		{
			if (mused.cp.type != CP_SEQUENCE) break;
			
			snapshot(S_T_SEQUENCE);
			
			size_t items = cp_get_item_count(&mused.cp, sizeof(mused.song.sequence[0][0]));
			
			if (items < 1) break;
			
			int first = ((MusSeqPattern*)mused.cp.data)[0].position;
			
			for (int i = 0 ; i < items ; ++i)
			{
				add_sequence(mused.current_sequencetrack, ((MusSeqPattern*)mused.cp.data)[i].position-first+mused.current_sequencepos, ((MusSeqPattern*)mused.cp.data)[i].pattern, ((MusSeqPattern*)mused.cp.data)[i].note_offset);
			}
		}
		break;
		
		case EDITPATTERN:
		{
			size_t items = cp_get_item_count(&mused.cp, sizeof(mused.song.pattern[0].step[0]));
			
			if (items < 1) break;
			
			if (mused.cp.type == CP_PATTERNSEGMENT || mused.cp.type == CP_PATTERN)
			{
				snapshot(S_T_PATTERN);
				
				int ofs;
				
				if (mused.cp.type == CP_PATTERN) 
					ofs = 0;
				else
					ofs = current_patternstep();
				
				for (int i = 0 ; i < items && i + ofs < mused.song.pattern[current_pattern()].num_steps ; ++i)
				{
					const MusStep *s = &((MusStep*)mused.cp.data)[i];
					MusStep *d = &mused.song.pattern[current_pattern()].step[ofs + i];
					if (s->note != MUS_NOTE_NONE)
						d->note = s->note;
						
					if (s->volume != MUS_NOTE_NO_VOLUME)
						d->volume = s->volume;
						
					if (s->instrument != MUS_NOTE_NO_INSTRUMENT)
						d->instrument = s->instrument;
						
					if (s->command != 0)
						d->command = s->command;
						
					if (s->ctrl != 0)
						d->ctrl = s->ctrl;
				}
			}
		}
		break;
	}
}


void begin_selection(int position)
{
	//mused.selection.start = mused.selection.end;
	mused.selection.keydown = position;
	debug("Selected from %d", position);
}


void select_range(int position)
{
	mused.selection.start = mused.selection.keydown;
	
	if (mused.selection.end == position)
	{
		int extra = 1;
		
		if (mused.focus == EDITSEQUENCE)
			extra = mused.sequenceview_steps;
		
		mused.selection.end = position + extra; // so we can select the last row (can't move the cursor one element after the last)
	}
	else
		mused.selection.end = position;
		
	if (mused.selection.end < mused.selection.start)
	{
		swap(mused.selection.start, mused.selection.end);
	}
	
	debug("Selected from %d-%d", mused.selection.start, mused.selection.end);
}
