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
#include "event.h"
#include "msgbox.h"
#include "version.h"
#include "../../klystron/src/version.h"
#include "gfx/gfx.h"
#include "theme.h"
#include "menu.h"

extern Mused mused;
extern GfxDomain *domain;
extern Menu pixelmenu[];

void select_sequence_position(void *channel, void *position, void* unused)
{
	if (CASTPTR(int,channel) != -1)
		mused.current_sequencetrack = CASTPTR(int,channel);
	
	if (CASTPTR(int,position) < mused.song.song_length)
		mused.current_sequencepos = CASTPTR(int,position);
		
	if (mused.mode == EDITCLASSIC) update_ghost_patterns();
}


void select_pattern_param(void *id, void *position, void *pattern)
{
	mused.current_pattern = CASTPTR(int,pattern);
	mused.current_patternstep = CASTPTR(int,position);
	mused.current_patternx = CASTPTR(int,id);
}


void select_instrument(void *idx, void *relative, void *unused2)
{
	if (relative)
		mused.current_instrument += CASTPTR(int,idx);
	else
		mused.current_instrument = CASTPTR(int,idx);
		
	if (mused.current_instrument >= NUM_INSTRUMENTS) mused.current_instrument = NUM_INSTRUMENTS-1;
	else if (mused.current_instrument < 0) mused.current_instrument = 0;
}


void change_octave(void *delta, void *unused1, void *unused2)
{
	mused.octave += CASTPTR(int,delta);
	if (mused.octave > 7) mused.octave = 7;
	else if (mused.octave < 0) mused.octave = 0;
}


void change_song_rate(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) > 0 && (int)mused.song.song_rate + CASTPTR(int,delta) <= 0xff)
		mused.song.song_rate += CASTPTR(int,delta);
	else if (CASTPTR(int,delta) < 0 && (int)mused.song.song_rate + CASTPTR(int,delta) >= 0x1)
		mused.song.song_rate += CASTPTR(int,delta);
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
	mused.flags |= SONG_PLAYING;
}


void stop(void *unused1, void *unused2, void *unused3)
{
	mus_set_song(&mused.mus, NULL, 0);
	mused.flags &= ~SONG_PLAYING;
}


void change_song_speed(void *speed, void *delta, void *unused)
{
	if (!speed)
	{
		if ((int)mused.song.song_speed + CASTPTR(int,delta) >= 1 && (int)mused.song.song_speed + CASTPTR(int,delta) <= 255)
			mused.song.song_speed += CASTPTR(int,delta);
	}
	else
	{
		if ((int)mused.song.song_speed2 + CASTPTR(int,delta) >= 1 && (int)mused.song.song_speed2 + CASTPTR(int,delta) <= 255)
		mused.song.song_speed2 += CASTPTR(int,delta);
	}
}


void select_instrument_param(void *idx, void *unused1, void *unused2)
{
	mused.selected_param = CASTPTR(int,idx);
}


void select_program_step(void *idx, void *unused1, void *unused2)
{
	mused.current_program_step = CASTPTR(int,idx);
}


void new_song_action(void *unused1, void *unused2, void *unused3)
{
	if (confirm("Clear song and data?"))
	{
		stop(0,0,0);
		new_song();
	}
}


void save_song_action(void *unused1, void *unused2, void *unused3)
{
	cyd_lock(&mused.cyd, 1);
	save_data();
	cyd_lock(&mused.cyd, 0);
}


void open_song_action(void *unused1, void *unused2, void *unused3)
{
	if (mused.mode != EDITINSTRUMENT && mused.mode != EDITPROG)
	{
		int r = confirm_ync("Save song?");
				
		if (r == 0) return;
		if (r == 1) { change_mode(EDITSEQUENCE); if (!save_data()) return; }
		
		stop(0,0,0);
	}
	
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
	change_mode(CASTPTR(int,mode));
}


void enable_channel(void *channel, void *unused1, void *unused2)
{
	mused.mus.channel[CASTPTR(int,channel)].flags ^= MUS_CHN_DISABLED;
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


void change_song_length(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0)
	{
		if (mused.song.song_length >= -CASTPTR(int,delta))
			mused.song.song_length += CASTPTR(int,delta);
						
		if (mused.song.loop_point >= mused.song.song_length)
			mused.song.loop_point = mused.song.song_length;
	}
	else if (CASTPTR(int,delta) > 0)
	{
		if (mused.song.song_length < 0xfffe - CASTPTR(int,delta))
			mused.song.song_length += CASTPTR(int,delta);
	}
}


void change_loop_point(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0)
	{
		if (mused.song.loop_point >= -CASTPTR(int,delta))
			mused.song.loop_point += CASTPTR(int,delta);
		else
			mused.song.loop_point = 0;
	}
	else if (CASTPTR(int,delta) > 0)
	{
		mused.song.loop_point += CASTPTR(int,delta);
		if (mused.song.loop_point >= mused.song.song_length)
			mused.song.loop_point = mused.song.song_length;
	}
}


void change_seq_steps(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0)
	{
		if (mused.sequenceview_steps > 1)
		{
			--mused.sequenceview_steps;
			mused.current_sequencepos = (mused.current_sequencepos/mused.sequenceview_steps) * mused.sequenceview_steps;
		}
	}
	else if (CASTPTR(int,delta) > 0)
	{
		if (mused.sequenceview_steps < 128)
		{
			++mused.sequenceview_steps;
			mused.current_sequencepos = (mused.current_sequencepos/mused.sequenceview_steps) * mused.sequenceview_steps;
		}
	}
}


void show_about_box(void *unused1, void *unused2, void *unused3)
{
	msgbox(VERSION_STRING "\n" KLYSTRON_VERSION_STRING, MB_OK);
}


void change_channels(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0 && mused.song.num_channels > 1)
	{
		--mused.song.num_channels;
	}
	else if (CASTPTR(int,delta) > 0 && mused.song.num_channels < MUS_MAX_CHANNELS)
	{
		++mused.song.num_channels;
	}
}


void begin_selection_action(void *unused1, void *unused2, void *unused3)
{
	switch (mused.mode)
	{
		case EDITPATTERN:
		begin_selection(mused.current_patternstep);
		break;
		
		case EDITSEQUENCE:
		begin_selection(mused.current_sequencepos);
		break;
		
		case EDITPROG:
		begin_selection(mused.current_program_step);
		break;
	}
}


void end_selection_action(void *unused1, void *unused2, void *unused3)
{
	switch (mused.mode)
	{
		case EDITPATTERN:
		select_range(mused.current_patternstep);
		break;
		
		case EDITSEQUENCE:
		select_range(mused.current_sequencepos);
		break;
		
		case EDITPROG:
		select_range(mused.current_program_step);
		break;
	}
}


void toggle_pixel_scale(void *a, void*b, void*c)
{
	change_pixel_scale(MAKEPTR(((domain->scale) & 3) + 1), 0, 0);
}


void change_pixel_scale(void *scale, void*b, void*c)
{	
	mused.pixel_scale = CASTPTR(int,scale);
	domain->scale = mused.pixel_scale;
	gfx_domain_update(domain);
	mused.screen = gfx_domain_get_surface(domain);
	
	for (int i = 0 ; i < 4; ++i)
	{
		if (pixelmenu[i].p1 == scale)
			pixelmenu[i].flags |= MENU_BULLET;
		else
			pixelmenu[i].flags &= ~MENU_BULLET;
	}
}


void toggle_fullscreen(void *a, void*b, void*c)
{
	mused.flags ^= FULLSCREEN;
	change_fullscreen(0,0,0);
}


void change_fullscreen(void *a, void*b, void*c)
{
	domain->fullscreen = (mused.flags & FULLSCREEN) != 0;
	gfx_domain_update(domain);
	mused.screen = gfx_domain_get_surface(domain);
}


void load_theme_action(void *name, void*b, void*c)
{
	load_theme((char*)name);
}


void change_timesig(void *delta, void *b, void *c)
{
	// http://en.wikipedia.org/wiki/Time_signature says the following signatures are common. 
	// I'm a 4/4 or 3/4 man myself so I'll trust the article :)
	
	static const Uint16 sigs[] = { 0x0404, 0x0202, 0x0402, 0x0204, 0x0304, 0x0308, 0x0608, 0x0908, 0x0c08 };
	int i;
	for (i = 0 ; i < sizeof(sigs) / sizeof(sigs[0]) ; ++i)
	{
		if (sigs[i] == mused.time_signature)
			break;
	}
	
	i += CASTPTR(int,delta);
	
	if (i >= (int)(sizeof(sigs) / sizeof(sigs[0]) - 1)) i = 0;
	if (i < 0) i = sizeof(sigs) / sizeof(sigs[0]) - 1;
	mused.time_signature = sigs[i];
}


void clone_pattern(void *unused1, void *unused2, void *unused3)
{
	for (int empty = 0 ; empty < NUM_PATTERNS ; ++empty)
	{
		int not_empty = 0;
		for (int s = 0 ; s < mused.song.pattern[empty].num_steps && !not_empty ; ++s)
			if (mused.song.pattern[empty].step[s].note != MUS_NOTE_NO_INSTRUMENT
				|| mused.song.pattern[empty].step[s].ctrl != 0 
				|| mused.song.pattern[empty].step[s].instrument != MUS_NOTE_NO_INSTRUMENT
				|| mused.song.pattern[empty].step[s].command != 0)
				not_empty = 1;
		
		if (!not_empty)
		{
			mused.song.pattern[empty].num_steps = mused.song.pattern[mused.current_pattern].num_steps;
			memcpy(mused.song.pattern[empty].step, mused.song.pattern[mused.current_pattern].step, 
				mused.song.pattern[mused.current_pattern].num_steps * sizeof(mused.song.pattern[mused.current_pattern].step[0]));
			
			for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
				if (mused.song.sequence[mused.current_sequencetrack][i].position == mused.current_sequencepos
					&& mused.song.sequence[mused.current_sequencetrack][i].pattern == mused.current_pattern)
					mused.song.sequence[mused.current_sequencetrack][i].pattern = empty;
			
			mused.current_pattern = empty;
			
			update_ghost_patterns();
			
			return;
		}
	}
}
