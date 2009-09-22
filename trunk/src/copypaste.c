#include "copypaste.h"
#include "clipboard.h"
#include "mused.h"
#include "event.h"

extern Clipboard cp;
extern Mused mused;

void copy()
{
	switch (mused.mode)
	{
		/*case CP_PROGRAM:
		
		cp_copy_items(&mused.cp, CP_PROGRAM, &mused.instrument[mused.current_instrument].program[mused.selection_start], mused.selection_end-mused.selection_start, sizeof(mused.instrument[mused.current_instrument].program[mused.selection]));
		
		break;*/
		
		case EDITPATTERN:
		
		if (mused.selection_start == mused.selection_end)
			cp_copy_items(&mused.cp, CP_PATTERN, mused.song.pattern[mused.current_pattern].step, sizeof(mused.song.pattern[mused.current_pattern].step[0]), mused.song.pattern[mused.current_pattern].num_steps);
		else
			cp_copy_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[mused.current_pattern].step[mused.selection_start], sizeof(mused.song.pattern[mused.current_pattern].step[0]), mused.selection_end-mused.selection_start);
		
		break;
		
		case EDITINSTRUMENT:
		
		cp_copy(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], sizeof(mused.song.instrument[mused.current_instrument]));
		
		break;
		
		case EDITSEQUENCE:{
		
		int first = 0, last = 0;
		
		for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		{
			if (mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection_start)
				first = i + 1;
				
			if (mused.song.sequence[mused.current_sequencetrack][i].position >= mused.selection_start && mused.song.sequence[mused.current_sequencetrack][i].position < mused.selection_end)
				last = i;
		}
		
		cp_copy_items(&mused.cp, CP_SEQUENCE, &mused.song.sequence[mused.current_sequencetrack][first], last-first+1, sizeof(mused.song.sequence[mused.current_sequencetrack][0]));
		
		}break;
	}
	
	mused.selection_start = mused.selection_end = 0;
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
		/*case EDITPROG:
		
		cp_copy_items(&mused.cp, CP_PROGRAM, &mused.instrument[mused.current_instrument].program[mused.selection_start], mused.selection_end-mused.selection_start, sizeof(mused.instrument[mused.current_instrument].program[mused.selection]));
		
		break;
		*/
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
		
		if (mused.cp.type == CP_PATTERN)
			cp_paste_items(&mused.cp, CP_PATTERN, mused.song.pattern[mused.current_pattern].step, NUM_STEPS, sizeof(mused.song.pattern[mused.current_pattern].step[0]));
		else if (mused.cp.type == CP_PATTERNSEGMENT)
			cp_paste_items(&mused.cp, CP_PATTERNSEGMENT, &mused.song.pattern[mused.current_pattern].step[mused.current_patternstep], NUM_STEPS-mused.current_patternstep, sizeof(mused.song.pattern[mused.current_pattern].step[0]));
		
		break;
	
		case EDITINSTRUMENT:
		
		cp_paste_items(&mused.cp, CP_INSTRUMENT, &mused.song.instrument[mused.current_instrument], 1, sizeof(mused.song.instrument[mused.current_instrument]));
		
		break;
	}
}
