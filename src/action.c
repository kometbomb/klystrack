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

#include "action.h"
#include "mused.h"
#include "diskop.h"
#include "toolutil.h"
#include "view.h"

extern Mused mused;

void select_sequence_position(void *channel, void *position, void* unused)
{
	if ((int)channel != -1)
		mused.current_sequencetrack = (int)channel;
	
	if ((int)position < mused.song.song_length)
		mused.current_sequencepos = (int)position;
		
	if (mused.mode == EDITCLASSIC) update_ghost_patterns();
}


void select_pattern_param(void *id, void *position, void *pattern)
{
	mused.current_pattern = (int)pattern;
	mused.current_patternstep = (int)position;
	mused.current_patternx = (int)id;
}


void select_instrument(void *idx, void *relative, void *unused2)
{
	if (relative)
		mused.current_instrument += (int)idx;
	else
		mused.current_instrument = (int)idx;
		
	if (mused.current_instrument >= NUM_INSTRUMENTS) mused.current_instrument = NUM_INSTRUMENTS-1;
	else if (mused.current_instrument < 0) mused.current_instrument = 0;
}


void change_octave(void *delta, void *unused1, void *unused2)
{
	mused.octave += (int)delta;
	if (mused.octave > 7) mused.octave = 7;
	else if (mused.octave < 0) mused.octave = 0;
}


void change_song_rate(void *delta, void *unused1, void *unused2)
{
	if ((int)delta > 0 && (int)mused.song.song_rate + (int)delta <= 0xff)
		mused.song.song_rate += (int)delta;
	else if ((int)delta < 0 && (int)mused.song.song_rate + (int)delta >= 0x1)
		mused.song.song_rate += (int)delta;
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
}


void change_time_signature(void *beat, void *unused1, void *unused2)
{
	if (!beat)
	{
		mused.time_signature = (mused.time_signature & 0x00ff) | (((((mused.time_signature >> 8) + 1) & 0xff) % 17) << 8);
		if ((mused.time_signature & 0xff00) == 0) mused.time_signature |= 0x100;
	}
	else
	{
		mused.time_signature = (mused.time_signature & 0xff00) | ((((mused.time_signature & 0xff) + 1) & 0xff) % 17);
		if ((mused.time_signature & 0xff) == 0) mused.time_signature |= 1;
	}
}


void play(void *from_cursor, void *unused1, void *unused2)
{
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
	mus_set_song(&mused.mus, &mused.song, from_cursor ? mused.current_sequencepos : 0);
}


void stop(void *unused1, void *unused2, void *unused3)
{
	mus_set_song(&mused.mus, NULL, 0);
}


void change_song_speed(void *speed, void *delta, void *unused)
{
	if (!speed)
	{
		if ((int)mused.song.song_speed + (int)delta > 1 && (int)mused.song.song_speed + (int)delta < 255)
			mused.song.song_speed += (int)delta;
	}
	else
	{
		if ((int)mused.song.song_speed2 + (int)delta > 1 && (int)mused.song.song_speed2 + (int)delta < 255)
		mused.song.song_speed2 += (int)delta;
	}
}


void select_instrument_param(void *idx, void *unused1, void *unused2)
{
	mused.selected_param = (int)idx;
}


void new_song_action(void *unused1, void *unused2, void *unused3)
{
	if (confirm("Clear song and data?"))
	{
		new_song();
	}
}


void save_song_action(void *unused1, void *unused2, void *unused3)
{
	mus_set_song(&mused.mus, NULL, 0);
	cyd_lock(&mused.cyd, 1);
	save_data();
	cyd_lock(&mused.cyd, 0);
}


void open_song_action(void *unused1, void *unused2, void *unused3)
{
	mus_set_song(&mused.mus, NULL, 0);
	cyd_lock(&mused.cyd, 1);
	open_data();
	cyd_lock(&mused.cyd, 0);
}


void generic_action(void *func, void *unused1, void *unused2)
{
	mus_set_song(&mused.mus, NULL, 0);
	cyd_lock(&mused.cyd, 1);
	
	((void *(*)(void))func)(); /* I love the smell of C in the morning */
	
	cyd_lock(&mused.cyd, 0);
}


void quit_action(void *unused1, void *unused2, void *unused3)
{
	mused.done = 1;
}


void change_mode_action(void *mode, void *unused1, void *unused2)
{
	change_mode((int)mode);
}


void enable_channel(void *channel, void *unused1, void *unused2)
{
	mused.mus.channel[(int)channel].flags ^= MUS_CHN_DISABLED;
}


void enable_reverb(void *unused1, void *unused2, void *unused3)
{
	mused.cyd.flags ^= CYD_ENABLE_REVERB;

	if (mused.cyd.flags & CYD_ENABLE_REVERB)
		mused.song.flags |= MUS_ENABLE_REVERB;
	else
		mused.song.flags &= ~MUS_ENABLE_REVERB;
}


void clear_selection(void *unused1, void *unused2, void *unused3)
{
	mused.selection.start = 0;
	mused.selection.end = 0;
}


void cycle_focus(void *_views, void *_focus, void *_mode)
{
	View **viewlist = _views;
	int *focus = _focus, *mode = _mode;
	View *views = viewlist[*mode];
	
	int i;
	for (i = 0 ; views[i].handler ; ++i)
	{
		if (views[i].focus == *focus) break;
	}
	
	if (!views[i].handler) return;
	
	int next;
	
	for (next = i + 1 ; i != next ; ++next)
	{
		if (views[next].handler == NULL)
		{
			next = -1;
			continue;
		}
		
		if (views[next].focus != -1 && views[next].focus != *focus) 
		{
			*focus = views[next].focus;
			break;
		}
	}
}
