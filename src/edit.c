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

#include "edit.h"
#include "event.h"
#include "mused.h"
#include "gui/msgbox.h"
#include "view/wavetableview.h"
#include <string.h>

extern Mused mused;
extern GfxDomain *domain;

static int find_unused_pattern()
{
	for (int empty = 0 ; empty < NUM_PATTERNS ; ++empty)
	{
		int not_empty = 0;
		for (int s = 0 ; s < mused.song.pattern[empty].num_steps && !not_empty ; ++s)
			if ((empty == current_pattern() && mused.focus == EDITPATTERN) || (mused.song.pattern[empty].step[s].note != MUS_NOTE_NONE
				|| mused.song.pattern[empty].step[s].ctrl != 0 
				|| mused.song.pattern[empty].step[s].instrument != MUS_NOTE_NO_INSTRUMENT
				|| mused.song.pattern[empty].step[s].command != 0))
				not_empty = 1;
		
		for (int c = 0 ; c < mused.song.num_channels && !not_empty ; ++c)
		{
			for (int i = 0 ; i < mused.song.num_sequences[c] && !not_empty ; ++i)
			{
				if (mused.song.sequence[c][i].pattern == empty)
					not_empty = 1;
			}
		}
		
		if (!not_empty)
			return empty;
	}
	
	msgbox(domain, mused.slider_bevel, &mused.largefont, "Max patterns exceeded!", MB_OK);
	
	return -1;
}


static void set_pattern(int pattern)
{
	//if ((mused.focus == EDITPATTERN && !mused.single_pattern_edit) || mused.focus != EDITPATTERN)
	{
		debug("setting pattern to %d at %d", pattern, mused.current_sequencepos);
	
		snapshot(S_T_SEQUENCE);
	
		for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		{
			if (mused.song.sequence[mused.current_sequencetrack][i].position == mused.current_sequencepos
				&& mused.song.sequence[mused.current_sequencetrack][i].pattern == current_pattern())
				mused.song.sequence[mused.current_sequencetrack][i].pattern = pattern;
		}
	}
}


void clone_pattern(void *unused1, void *unused2, void *unused3)
{
	debug("Cloning pattern");

	int empty = find_unused_pattern();
	int cp = current_pattern();
	
	if (empty == -1 || cp == empty || cp < 0)
	{
		return;
	}
	
	mused.song.pattern[empty].color = mused.song.pattern[cp].color;
	
	resize_pattern(&mused.song.pattern[empty], mused.song.pattern[cp].num_steps);
	
	memcpy(mused.song.pattern[empty].step, mused.song.pattern[cp].step, 
		mused.song.pattern[cp].num_steps * sizeof(mused.song.pattern[cp].step[0]));
		
	set_pattern(empty);
	
	set_info_message("Cloned pattern %02X to %02X", cp, empty);
}


void clone_each_pattern(void *unused1, void *unused2, void *unused3)
{
	debug("Cloning each pattern");

	int temp = mused.current_sequencetrack;
	
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		mused.current_sequencetrack = i;
		clone_pattern(NULL, NULL, NULL);
	}
	
	mused.current_sequencetrack = temp;
	
	set_info_message("Cloned sequence row");
}


void get_unused_pattern(void *unused1, void *unused2, void *unused3)
{
	int empty = find_unused_pattern();
		
	if (empty == -1 || (current_pattern() == empty && mused.focus == EDITPATTERN))
	{
		return;
	}
	
	if (mused.focus == EDITSEQUENCE)
	{
		snapshot(S_T_SEQUENCE);
		
		bool found = false;
		
		for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		{
			if (mused.song.sequence[mused.current_sequencetrack][i].position == mused.current_sequencepos
				&& mused.song.sequence[mused.current_sequencetrack][i].pattern == current_pattern())
				found = true;
		}
		
		if (!found)
			add_sequence(mused.current_sequencetrack, mused.current_sequencepos, empty, 0);
	}
	
	set_pattern(empty);
}


void get_unused_pattern_all_tracks(void *unused1, void *unused2, void *unused3)
{
	int temp = mused.current_sequencetrack;
	
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		mused.current_sequencetrack = i;
		get_unused_pattern(0, 0, 0);
	}
	
	mused.current_sequencetrack = temp;
}


void zero_step(MusStep *step)
{
	step->note = MUS_NOTE_NONE;				
	step->instrument = MUS_NOTE_NO_INSTRUMENT;
	step->volume = MUS_NOTE_NO_VOLUME;
	step->ctrl = 0;
	step->command = 0;
}


void expand_pattern(void *factor, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN) return;
	
	MusPattern *pattern = get_current_pattern();
	
	if (!pattern)
		return;
	
	MusStep *temp = malloc(pattern->num_steps * sizeof(pattern->step[0]));
	memcpy(temp, pattern->step, pattern->num_steps * sizeof(pattern->step[0]));
	
	snapshot(S_T_PATTERN);
	
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
	
	MusPattern *pattern = get_current_pattern();
	
	if (!pattern)
		return;
	
	if (pattern->num_steps <= CASTPTR(int,factor)) return;
	
	snapshot(S_T_PATTERN);
	
	resize_pattern(pattern, pattern->num_steps / CASTPTR(int,factor));
	
	for (int i = 0, ti = 0 ; i < pattern->num_steps ; ++i, ti += CASTPTR(int,factor))
	{
		memcpy(&pattern->step[i], &pattern->step[ti], sizeof(pattern->step[i]));
	}
}


void interpolate(void *unused1, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN || mused.selection.start >= mused.selection.end - 1) return;
	
	MusPattern *pat = get_current_pattern();
	
	if (!pat)
		return;
	
	int start_step = get_patternstep(mused.selection.start, mused.current_sequencetrack);
	
	Uint16 command = pat->step[start_step].command;
	Uint16 mask = 0xff00;
	
	if ((command & 0xf000) == MUS_FX_CUTOFF_FINE_SET 
		|| (command & 0xf000) == MUS_FX_WAVETABLE_OFFSET) mask = 0xf000;
	
	command &= mask;
	
	int begin = pat->step[start_step].command & ~mask;
	int end = pat->step[start_step + mused.selection.end - 1 - mused.selection.start].command & ~mask;
	
	int l = mused.selection.end - mused.selection.start - 1;
	
	snapshot(S_T_PATTERN);
	
	for (int i = start_step, p = 0 ; p < mused.selection.end - mused.selection.start ; ++i, ++p)
	{
		if ((pat->step[i].command & mask) == command)
		{
			Uint16 val = begin + (end - begin) * p / l;
			pat->step[i].command = command | val;
		}
	}
}


void snapshot(SHType type)
{
	snapshot_cascade(type, -1, -1);
}


void snapshot_cascade(SHType type, int a, int b)
{
	if (!(a != -1 && mused.last_snapshot == type && mused.last_snapshot_a == a && mused.last_snapshot_b == b))
	{
		switch (type)
		{
			case S_T_PATTERN:
				undo_store_pattern(&mused.undo, current_pattern(), &mused.song.pattern[current_pattern()], mused.modified);
				break;
				
			case S_T_SEQUENCE:
				undo_store_sequence(&mused.undo, mused.current_sequencetrack, mused.song.sequence[mused.current_sequencetrack], mused.song.num_sequences[mused.current_sequencetrack], mused.modified);
				break;
				
			case S_T_MODE:
				undo_store_mode(&mused.undo, mused.mode, mused.focus, mused.modified);
				break;
				
			case S_T_FX:
				undo_store_fx(&mused.undo, mused.fx_bus, &mused.song.fx[mused.fx_bus], mused.song.multiplex_period, mused.modified);
				break;
				
			case S_T_SONGINFO:
				undo_store_songinfo(&mused.undo, &mused.song, mused.modified);
				break;
			
			case S_T_INSTRUMENT:
				undo_store_instrument(&mused.undo, mused.current_instrument, &mused.song.instrument[mused.current_instrument], mused.modified);
				break;
				
			case S_T_WAVE_PARAM:
				undo_store_wave_param(&mused.undo, mused.selected_wavetable, &mused.mus.cyd->wavetable_entries[mused.selected_wavetable], mused.modified);
				break;
				
			case S_T_WAVE_DATA:
				undo_store_wave_data(&mused.undo, mused.selected_wavetable, &mused.mus.cyd->wavetable_entries[mused.selected_wavetable], mused.modified);
				break;
				
			case S_T_WAVE_NAME:
				undo_store_wave_name(&mused.undo, mused.selected_wavetable, mused.song.wavetable_names[mused.selected_wavetable], mused.modified);
				break;
			
			default: warning("SHType %d not implemented", type); break;
		}
	}
	
	mused.last_snapshot = type;
	mused.last_snapshot_a = a;
	mused.last_snapshot_b = b;
	
	if (type != S_T_MODE)
		mused.modified = true;
}


void transpose_note_data(void *semitones, void *unused1, void *unused2)
{
	if (mused.focus != EDITPATTERN || mused.selection.start >= mused.selection.end - 1) return;
	
	MusPattern *pat = get_current_pattern();
	
	if (!pat)
		return;
	
	snapshot(S_T_PATTERN);
	
	debug("Transposing pattern %d (%d-%d)", current_pattern(), mused.selection.start, mused.selection.end);
	
	for (int i = get_patternstep(mused.selection.start, mused.current_sequencetrack), p = 0 ; p < mused.selection.end - mused.selection.start ; ++i, ++p)
	{
		if (pat->step[i].note != MUS_NOTE_NONE)
		{
			int semi = CASTPTR(int,semitones);
			int note = (int)pat->step[i].note + semi;
			
			if (note >= 0 && note < 12 * 8)
				pat->step[i].note = note;
		}
	}
}


void split_pattern(void *unused1, void *unused2, void *unused3)
{
	if (mused.focus != EDITPATTERN && mused.focus != EDITSEQUENCE) return;
	
	MusPattern *pat = get_current_pattern();
	
	if (!pat)
		return;
	
	int step = current_patternstep();
	
	if (step <= 0)
		return;
	
	int empty = find_unused_pattern();
		
	if (empty == -1 || (current_pattern() == empty && mused.focus == EDITPATTERN))
	{
		return;
	}
	
	int cp = current_pattern();
	MusPattern *new_pattern = &mused.song.pattern[empty];
	
	// Add new pattern in sequence
	
	snapshot(S_T_SEQUENCE);
	add_sequence(mused.current_sequencetrack, mused.current_patternpos, empty, 0);
	
	// Copy latter half to the new pattern
	
	snapshot(S_T_PATTERN);
	resize_pattern(new_pattern, pat->num_steps - step);
	memcpy(new_pattern->step, &pat->step[step], sizeof(pat->step[0]) * ((int)pat->num_steps - step));
	
	// Resize old pattern
	
	snapshot(S_T_PATTERN);
	resize_pattern(pat, step);
	
	set_info_message("Split %02X into %02X and %02X", cp, cp, empty);
}
