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

extern Mused mused;

void select_sequence_position(void *channel, void *position, void* unused)
{
	mused.current_sequencepos = (int)position;
	if ((int)channel != -1)
		mused.current_sequencetrack = (int)channel;
}


void select_pattern_param(void *id, void *position, void *pattern)
{
	mused.current_pattern = (int)pattern;
	mused.current_patternstep = (int)position;
	mused.current_patternx = (int)id;
}


void select_instrument(void *idx, void *unused1, void *unused2)
{
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


void play(void *from, void *unused1, void *unused2)
{
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
	mus_set_song(&mused.mus, &mused.song, (int)from);
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
