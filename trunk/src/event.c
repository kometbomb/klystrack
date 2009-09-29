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


#include "event.h"
#include "mused.h"
#include <string.h>


extern Mused mused;

#define set_edit_buffer(s) { if (mused.edit_backup_buffer) mused.edit_backup_buffer = strdup(s); mused.edit_backup_buffer = strdup(s); mused.edit_buffer = (char*)s; mused.edit_buffer_size = sizeof(s); change_mode(EDITBUFFER); } 
#define clamp(val, add, _min, _max) { if ((int)val+(add) > _max) val = _max; else if ((int)val+(add) < _min) val = _min; else val+=(add); } 
#define flipbit(val, bit) { val ^= bit; };

void editparambox(int v)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	Uint16 *param = &inst->program[mused.selected_param-P_PARAMS];
	Uint32 mask = 0xffff0fff >> (mused.editpos*4);
	
	if (*param == MUS_FX_NOP)
		*param = 0;
	
	*param = (*param & mask) | ((v&0xf) <<((3-mused.editpos)*4));
	
	if (++mused.editpos > 3)
		change_mode(EDITINSTRUMENT);
}


static void move_position(int *cursor, int *scroll, SliderParam *param, int d, int top)
{
	if (*cursor + d < top)
		*cursor += d;
	
	if (*cursor < 0) *cursor = 0;
	
	if (param->visible_first > *cursor)
		*scroll = *cursor;
		
	if (param->visible_last < *cursor)
		*scroll = *cursor - (param->visible_last - param->visible_first);
}


int find_note(int sym, int oct)
{
	static const int keys[] = 
	{
	SDLK_z, SDLK_s, SDLK_x, SDLK_d, SDLK_c, SDLK_v, SDLK_g, SDLK_b, SDLK_h, SDLK_n, SDLK_j, SDLK_m,
	SDLK_q, SDLK_2, SDLK_w, SDLK_3, SDLK_e, SDLK_r, SDLK_5, SDLK_t, SDLK_6, SDLK_y, SDLK_7, SDLK_u, 
	SDLK_i, SDLK_9, SDLK_o, SDLK_0, SDLK_p, -1};
	
	int n = 0;
	for (const int *i = keys ; *i != -1 ; ++i, ++n)
	{
		if (*i == sym)
			return n + oct*12;
	}

	return -1;
}


static void instrument_add_param(int a)
{
	MusInstrument *i = &mused.song.instrument[mused.current_instrument];

	switch (mused.selected_param)
	{
		case P_BASENOTE:
		
		clamp(i->base_note, a, 0, 95);
		
		break;
		
		case P_LOCKNOTE:
		
		flipbit(i->flags, MUS_INST_LOCK_NOTE);
		
		break;
		
		case P_DRUM:
		
		flipbit(i->flags, MUS_INST_DRUM);
		
		break;
		
		case P_METAL:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_METAL);
		
		break;
		
		case P_KEYSYNC:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_KEY_SYNC);
		
		break;
		
		case P_SYNC:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_SYNC);
		
		break;
		
		case P_SYNCSRC:
		
		if (i->sync_source == 0xff && a < 0) break;
		if ((Uint8)(i->sync_source+a) >= MUS_CHANNELS && a > 0) break;
		i->sync_source+=a;
		
		break;
		
		case P_PULSE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_PULSE);
		
		break;
		
		case P_SAW:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_SAW);
		
		break;
		
		case P_TRIANGLE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_TRIANGLE);
		
		break;
		
		case P_NOISE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_NOISE);
		
		break;
		
		case P_REVERB:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_REVERB);
		
		break;
	
		case P_ATTACK:
		
		clamp(i->adsr.a, a, 0, 31);
			
		break;
		
		case P_DECAY:
		
		clamp(i->adsr.d, a, 0, 31);
			
		break;
		
		case P_SUSTAIN:
		
		clamp(i->adsr.s, a, 0, 31);
			
		break;
		
		case P_RELEASE:
		
		clamp(i->adsr.r, a, 0, 31);
			
		break;
		
		case P_PW:
		
		clamp(i->pw, a*16, 0, 0x7ff);
			
		break;
		
		case P_VOLUME:
		
		clamp(i->volume, a, 0, 0x7f);
			
		break;
		
		case P_PROGPERIOD:
		
		clamp(i->prog_period, a, 0, 0xff);
		
		break;
		
		case P_SLIDESPEED:
		
		clamp(i->slide_speed, a, 0, 0xff);
		
		break;
		
		case P_VIBDEPTH:
		
		clamp(i->vibrato_depth, a, 0, 0xff);
		
		break;
		
		case P_VIBSPEED:
		
		clamp(i->vibrato_speed, a, 0, 0xff);
		
		break;
		
		case P_PWMDEPTH:
		
		clamp(i->pwm_depth, a, 0, 0xff);
		
		break;
		
		case P_PWMSPEED:
		
		clamp(i->pwm_speed, a, 0, 0xff);
		
		break;
		
		case P_RINGMOD:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_RING_MODULATION);
		
		break;
		
		case P_SETPW:
		
		flipbit(i->flags, MUS_INST_SET_PW);
		
		break;
		
		case P_SETCUTOFF:
		
		flipbit(i->flags, MUS_INST_SET_CUTOFF);
		
		break;
		
		case P_INVVIB:
		
		flipbit(i->flags, MUS_INST_INVERT_VIBRATO_BIT);
		
		break;
		
		case P_FILTER:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_FILTER);
		
		break;
		
		case P_RINGMODSRC:
		
		if (i->ring_mod == 0xff && a < 0) break;
		if ((Uint8)(i->ring_mod+a) >= MUS_CHANNELS && a > 0) break;
		i->ring_mod+=a;
		
		break;
		
		case P_CUTOFF:
		
		clamp(i->cutoff, a*16, 0, 2047);
		
		break;
		
		case P_RESONANCE:
		
		clamp(i->resonance, a, 0, 3);
		
		break;
		
		case P_FLTTYPE:
		
		clamp(i->flttype, a, 0, 2);
		
		break;
	
		default:
		break;
	}
	
}


void edit_instrument_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				begin_selection(mused.selected_param);
			break;
		
			case SDLK_PERIOD:
			if (mused.selected_param >= P_PARAMS) 
				mused.song.instrument[mused.current_instrument].program[mused.selected_param-P_PARAMS] = MUS_FX_NOP;
			break;
		
			case SDLK_RETURN:
			{
				if (mused.selected_param >= P_PARAMS) change_mode(EDITPROG);
				else
				if (mused.selected_param == P_NAME) 
					set_edit_buffer(mused.song.instrument[mused.current_instrument].name);
			}
			break;
			
			case SDLK_DOWN:
			{
				++mused.selected_param;
				if (mused.selected_param >= P_PARAMS + MUS_PROG_LEN) mused.selected_param = P_PARAMS + MUS_PROG_LEN - 1;
				
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.selected_param);
				}
			}
			break;
			
			case SDLK_UP:
			{
				--mused.selected_param;
				if (mused.selected_param < 0) mused.selected_param = 0;
				
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.selected_param);
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				if (mused.selected_param >= P_PARAMS) 
				{
					for (int i = MUS_PROG_LEN-2; i >= mused.selected_param - P_PARAMS ; --i)
						mused.song.instrument[mused.current_instrument].program[i] = mused.song.instrument[mused.current_instrument].program[i-1];
					mused.song.instrument[mused.current_instrument].program[mused.selected_param - P_PARAMS] = MUS_FX_NOP;
				}
			}
			break;
			
			case SDLK_DELETE:
			{
				if (mused.selected_param >= P_PARAMS) 
				{
					for (int i = mused.selected_param - P_PARAMS  ; i < MUS_PROG_LEN-1 ; ++i)
						mused.song.instrument[mused.current_instrument].program[i] = mused.song.instrument[mused.current_instrument].program[i+1];
					mused.song.instrument[mused.current_instrument].program[MUS_PROG_LEN-1] = MUS_FX_NOP;
				}
			}
			break;
		
			case SDLK_RIGHT:
			{
				instrument_add_param(+1);
			}
			break;
			
			case SDLK_TAB:
				for (int i = 0 ; i < MUS_CHANNELS ; ++i)
					cyd_enable_gate(mused.mus.cyd, &mused.mus.cyd->channel[i], 0);
			break;
			
			case SDLK_LEFT:
			{
				instrument_add_param(-1);
			}
			break;
		
			default:
			{
			int note = find_note(e->key.keysym.sym, mused.octave);
			if (note != -1) 
			{
				mus_trigger_instrument(&mused.mus, -1, &mused.song.instrument[mused.current_instrument], note);
			}
			}
			break;
		}
		
		break;
	}
}


static int gethex(int key)
{
	if (key >= SDLK_0 && key <= SDLK_9)
	{
		return key - SDLK_0;
	}
	else if (key >= SDLK_KP0 && key <= SDLK_KP9)
	{
		return key - SDLK_KP0;
	}
	else if (key >= SDLK_a && key <= SDLK_f)
	{
		return key - SDLK_a + 0xa;
	}
	else return -1;
}


static int getalphanum(int key)
{
	if (key >= SDLK_0 && key <= SDLK_9)
	{
		return key - SDLK_0;
	}
	else if (key >= SDLK_KP0 && key <= SDLK_KP9)
	{
		return key - SDLK_KP0;
	}
	else if (key >= SDLK_a && key <= SDLK_z)
	{
		return key - SDLK_a + 0xa;
	}
	else return -1;
}


static int seqsort(const void *_a, const void *_b)
{
	const MusSeqPattern *a = _a;
	const MusSeqPattern *b = _b;
	if (a->position > b->position) return 1;
	if (a->position < b->position) return -1;
	
	return 0;
}


void add_sequence(int position, int pattern, int offset)
{
	for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		if (mused.song.sequence[mused.current_sequencetrack][i].position == position)
		{
			mused.song.sequence[mused.current_sequencetrack][i].pattern = pattern;
			mused.song.sequence[mused.current_sequencetrack][i].note_offset = offset;
			return;
		}
			
	if (mused.song.num_sequences[mused.current_sequencetrack] >= NUM_SEQUENCES)
		return;
			
	mused.song.sequence[mused.current_sequencetrack][mused.song.num_sequences[mused.current_sequencetrack]].position = position;
	mused.song.sequence[mused.current_sequencetrack][mused.song.num_sequences[mused.current_sequencetrack]].pattern = pattern;
	mused.song.sequence[mused.current_sequencetrack][mused.song.num_sequences[mused.current_sequencetrack]].note_offset = offset;
	
	++mused.song.num_sequences[mused.current_sequencetrack];
	
	qsort(mused.song.sequence[mused.current_sequencetrack], mused.song.num_sequences[mused.current_sequencetrack], sizeof(mused.song.sequence[mused.current_sequencetrack][0]), seqsort);
}


void del_sequence(int first,int last,int track)
{
	if (mused.song.num_sequences[track] == 0) return;

	for (int i = 0 ; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
		if (mused.song.sequence[track][i].position >= first && mused.song.sequence[track][i].position < last)
		{
			mused.song.sequence[track][i].position = 0xffff;
		}
	
	qsort(mused.song.sequence[track], mused.song.num_sequences[track], sizeof(mused.song.sequence[track][0]), seqsort);
	
	while (mused.song.num_sequences[track] > 0 && mused.song.sequence[track][mused.song.num_sequences[track]-1].position == 0xffff) --mused.song.num_sequences[track];
}


void add_note_offset(int a)
{
	{
		for (int i = (int)mused.song.num_sequences[mused.current_sequencetrack] - 1 ; i >= 0 ; --i)
		{
			if (mused.current_sequencepos >= mused.song.sequence[mused.current_sequencetrack][i].position && 
				mused.song.sequence[mused.current_sequencetrack][i].position + mused.song.pattern[mused.song.sequence[mused.current_sequencetrack][i].pattern].num_steps > mused.current_sequencepos)
			{
				mused.song.sequence[mused.current_sequencetrack][i].note_offset += a;
				break;
			}
		}
	}
}


void sequence_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_RETURN:
			{
				for (int i = (int)mused.song.num_sequences[mused.current_sequencetrack] - 1 ; i >= 0 ; --i)
				{
					if (mused.current_sequencepos >= mused.song.sequence[mused.current_sequencetrack][i].position && 
						mused.song.sequence[mused.current_sequencetrack][i].position + mused.song.pattern[mused.song.sequence[mused.current_sequencetrack][i].pattern].num_steps > mused.current_sequencepos)
					{
						change_mode(EDITPATTERN);
						mused.current_pattern = mused.song.sequence[mused.current_sequencetrack][i].pattern;
						break;
					}
				}
			}
			break;
			
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				begin_selection(mused.current_sequencepos);
			break;
		
			case SDLK_PAGEDOWN:
			case SDLK_DOWN:
			{
				if (e->key.keysym.mod & KMOD_ALT)
				{
					add_note_offset(-1);
					break;
				}
			
				int steps = mused.sequenceview_steps;
				if (e->key.keysym.sym == SDLK_PAGEDOWN)
					steps *= 16;
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						++mused.song.loop_point;
						if (mused.song.loop_point >= mused.song.song_length)
							mused.song.loop_point = mused.song.song_length;
					}
					else
					{
						if (mused.song.song_length < 0xfffe - steps)
							mused.song.song_length += steps;
					}
				}
				else
				{
					move_position(&mused.current_sequencepos, &mused.sequence_position, &mused.sequence_slider_param, steps, mused.song.song_length);
				}
				
				if (((e->key.keysym.mod & KMOD_SHIFT) && !(e->key.keysym.mod & KMOD_CTRL)) )
				{
					select_range(mused.current_sequencepos);
				}
			}
			break;
			
			case SDLK_PAGEUP:
			case SDLK_UP:
			{
				if (e->key.keysym.mod & KMOD_ALT)
				{
					add_note_offset(1);
					break;
				}
			
				int steps = mused.sequenceview_steps;
				if (e->key.keysym.sym == SDLK_PAGEUP)
					steps *= 16;
				
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						if (mused.song.loop_point > 0)
							--mused.song.loop_point;
					}
					else
					{
						if (mused.song.song_length >= steps)
							mused.song.song_length -= steps;
							
						if (mused.song.loop_point >= mused.song.song_length)
							mused.song.loop_point = mused.song.song_length;
					}
				}
				else
				{
					move_position(&mused.current_sequencepos, &mused.sequence_position, &mused.sequence_slider_param, -steps, mused.song.song_length);
				}
				
				if (((e->key.keysym.mod & KMOD_SHIFT) && !(e->key.keysym.mod & KMOD_CTRL)) )
				{
					select_range(mused.current_sequencepos);
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				for (int i = 0; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
				{
					if (mused.song.sequence[mused.current_sequencetrack][i].position >= mused.current_sequencepos)
						mused.song.sequence[mused.current_sequencetrack][i].position += mused.sequenceview_steps;
				}
			}
			break;
			
			case SDLK_PERIOD:
			{
				del_sequence(mused.current_sequencepos, mused.current_sequencepos+mused.sequenceview_steps, mused.current_sequencetrack);
				if (mused.song.song_length > mused.current_sequencepos + mused.sequenceview_steps)
					mused.current_sequencepos += mused.sequenceview_steps;
			}
			break;
			
			case SDLK_DELETE:
			{
				del_sequence(mused.current_sequencepos, mused.current_sequencepos+mused.sequenceview_steps, mused.current_sequencetrack);
				for (int i = 0; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
				{
					if (mused.song.sequence[mused.current_sequencetrack][i].position >= mused.current_sequencepos)
						mused.song.sequence[mused.current_sequencetrack][i].position -= mused.sequenceview_steps;
				}
			}
			break;
		
			case SDLK_RIGHT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					if (mused.sequenceview_steps < 64)
					{
						++mused.sequenceview_steps;
						mused.current_sequencepos = (mused.current_sequencepos/mused.sequenceview_steps) * mused.sequenceview_steps;
					}
				}
				else
				{
					++mused.current_sequencetrack;
					if (mused.current_sequencetrack >= MUS_CHANNELS)
						mused.current_sequencetrack = MUS_CHANNELS-1;
				}
			}
			break;
			
			case SDLK_LEFT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					if (mused.sequenceview_steps > 1)
					{
						--mused.sequenceview_steps;
						mused.current_sequencepos = (mused.current_sequencepos/mused.sequenceview_steps) * mused.sequenceview_steps;
					}
				}
				else
				{
					--mused.current_sequencetrack;
					if (mused.current_sequencetrack < 0)
						mused.current_sequencetrack = 0;
				}
			}
			break;
		
			default:
			{
				
				
				if (getalphanum(e->key.keysym.sym) != -1 && getalphanum(e->key.keysym.sym) < NUM_PATTERNS)
				{
					add_sequence(mused.current_sequencepos, getalphanum(e->key.keysym.sym), 0);
					if (mused.song.song_length > mused.current_sequencepos + mused.sequenceview_steps)
						mused.current_sequencepos += mused.sequenceview_steps;
				}
			}
			break;
		}
		
		break;
	}
}


void pattern_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			
			case SDLK_RETURN:
			{
				change_mode(EDITSEQUENCE);
			}
			break;
			
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				begin_selection(mused.current_patternstep);
			break;
		
			case SDLK_PAGEDOWN:
			case SDLK_DOWN:
			{
				int steps = 1;
				if (e->key.keysym.sym == SDLK_PAGEDOWN) steps *= 16;
				
				move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +steps, mused.song.pattern[mused.current_pattern].num_steps);
				
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.current_patternstep);
				}
			}
			break;
			
			case SDLK_PAGEUP:
			case SDLK_UP:
			{
				int steps = 1;
				if (e->key.keysym.sym == SDLK_PAGEUP) steps *= 16;
				
				move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, -steps, mused.song.pattern[mused.current_pattern].num_steps);
			
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.current_patternstep);
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				if (mused.song.pattern[mused.current_pattern].num_steps < NUM_STEPS)	
					++mused.song.pattern[mused.current_pattern].num_steps;
					
				if ((e->key.keysym.mod & KMOD_ALT)) break;
					
				for (int i = mused.song.pattern[mused.current_pattern].num_steps-1; i >= mused.current_patternstep ; --i)
					memcpy(&mused.song.pattern[mused.current_pattern].step[i], &mused.song.pattern[mused.current_pattern].step[i-1], sizeof(mused.song.pattern[mused.current_pattern].step[0]));
				
				mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].note = MUS_NOTE_NONE;				
				mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument = MUS_NOTE_NO_INSTRUMENT;
				mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].ctrl = 0;
				mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].command = 0;
			}
			break;
			
			case SDLK_BACKSPACE:
			case SDLK_DELETE:
			{
				if (e->key.keysym.sym == SDLK_BACKSPACE)
				{
					if (mused.current_patternstep > 0) move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, -1, mused.song.pattern[mused.current_pattern].num_steps);
					else break;
				}
				
				if (mused.song.pattern[mused.current_pattern].num_steps > 1)
					--mused.song.pattern[mused.current_pattern].num_steps;
					
				if ((e->key.keysym.mod & KMOD_ALT)) break;
			
				if (mused.song.pattern[mused.current_pattern].num_steps-mused.current_patternstep >= 1)
				{
					for (int i = mused.current_patternstep  ; i < mused.song.pattern[mused.current_pattern].num_steps ; ++i)
						memcpy(&mused.song.pattern[mused.current_pattern].step[i], &mused.song.pattern[mused.current_pattern].step[i+1], sizeof(mused.song.pattern[mused.current_pattern].step[0]));
					
					
				}
				else
				{
					mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].note = MUS_NOTE_NONE;				
					mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument = MUS_NOTE_NO_INSTRUMENT;
					mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].ctrl = 0;
				}
				
				if (mused.current_patternstep >= mused.song.pattern[mused.current_pattern].num_steps) --mused.current_patternstep;
			}
			break;
		
			case SDLK_RIGHT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					++mused.current_pattern;
					if (mused.current_pattern >= NUM_PATTERNS)
						mused.current_pattern = NUM_PATTERNS - 1;
				}
				else
				{
					++mused.current_patternx;
					if (mused.current_patternx >= PED_PARAMS)
					{
						if (mused.single_pattern_edit)
						{
							mused.current_patternx = PED_PARAMS-1;
						}
						else
						{
							mused.current_patternx = 0;
							do
							{
								mused.current_sequencetrack = (mused.current_sequencetrack + 1) % MUS_CHANNELS;
							}
							while (mused.ghost_pattern[mused.current_sequencetrack] == -1) ;
							
							mused.current_pattern = mused.ghost_pattern[mused.current_sequencetrack];
							
							if (mused.current_patternstep >= mused.song.pattern[mused.current_pattern].num_steps)
								mused.current_patternstep = mused.song.pattern[mused.current_pattern].num_steps - 1;
						}
					}
				}
			}
			break;
			
			case SDLK_LEFT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					--mused.current_pattern;
					if (mused.current_pattern < 0)
						mused.current_pattern = 0;
				}
				else
				{
					--mused.current_patternx;
					if (mused.current_patternx < 0)
					{
						if (mused.single_pattern_edit)
						{
							mused.current_patternx = 0;
						}
						else
						{
							mused.current_patternx = PED_PARAMS - 1;
							do
							{
								mused.current_sequencetrack = (mused.current_sequencetrack + MUS_CHANNELS - 1) % MUS_CHANNELS;
							}
							while (mused.ghost_pattern[mused.current_sequencetrack] == -1); 
							
							mused.current_pattern = mused.ghost_pattern[mused.current_sequencetrack];
							
							if (mused.current_patternstep >= mused.song.pattern[mused.current_pattern].num_steps)
								mused.current_patternstep = mused.song.pattern[mused.current_pattern].num_steps - 1;
						}
					}
				}
			}
			break;
			
			default:
			{
			
				if (mused.current_patternx == PED_NOTE)
				{
					if (e->key.keysym.sym == SDLK_PERIOD)
					{
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].note = MUS_NOTE_NONE;				
							
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
					else if (e->key.keysym.sym == SDLK_TAB)
					{
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].note = MUS_NOTE_RELEASE;
							
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
					else
					{
						int note = find_note(e->key.keysym.sym, mused.octave);
						if (note != -1) 
						{
							mus_trigger_instrument(&mused.mus, 0, &mused.song.instrument[mused.current_instrument], note);
							
							mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].note = note;				
							mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument = mused.current_instrument;
							
							move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
						}
					}
				}
				else if (mused.current_patternx == PED_INSTRUMENT1 || mused.current_patternx == PED_INSTRUMENT2)
				{
					if (e->key.keysym.sym == SDLK_PERIOD)
					{
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument = MUS_NOTE_NO_INSTRUMENT;				
							
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
					else if (gethex(e->key.keysym.sym) != -1)
					{
						if (mused.song.num_instruments > 0)
						{
							Uint8 inst = mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument;
							if ((inst == MUS_NOTE_NO_INSTRUMENT)) inst = 0;
							
							switch (mused.current_patternx)
							{
								case PED_INSTRUMENT1:
								inst = (inst & 0x0f) | ((gethex(e->key.keysym.sym) << 4) & 0xf0);
								break;
								
								case PED_INSTRUMENT2:
								inst = (inst & 0xf0) | gethex(e->key.keysym.sym);
								break;
							}
							
							if (inst > (mused.song.num_instruments-1)) inst = (mused.song.num_instruments-1);
						
							mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].instrument = inst; 
							mused.current_instrument = inst % mused.song.num_instruments;
						}
					
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
				}
				else if (mused.current_patternx >= PED_COMMAND1 && mused.current_patternx <= PED_COMMAND4)
				{
					if (gethex(e->key.keysym.sym) != -1)
					{
						Uint16 inst = mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].command;
						
						switch (mused.current_patternx)
						{
							case PED_COMMAND1:
							inst = (inst & 0x0fff) | ((gethex(e->key.keysym.sym) << 12) & 0xf000);
							break;
							
							case PED_COMMAND2:
							inst = (inst & 0xf0ff) | ((gethex(e->key.keysym.sym) << 8) & 0x0f00);
							break;
							
							case PED_COMMAND3:
							inst = (inst & 0xff0f) | ((gethex(e->key.keysym.sym) << 4) & 0x00f0);
							break;
							
							case PED_COMMAND4:
							inst = (inst & 0xfff0) | gethex(e->key.keysym.sym);
							break;
						}
						
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].command = inst; 
						
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
				}
				else if (mused.current_patternx >= PED_CTRL && mused.current_patternx < PED_COMMAND1)
				{
					if (e->key.keysym.sym == SDLK_PERIOD || e->key.keysym.sym == SDLK_0)
					{
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].ctrl &= ~(MUS_CTRL_BIT << (mused.current_patternx - PED_CTRL));				
							
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
					if (e->key.keysym.sym == SDLK_1)
					{
						mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].ctrl |= (MUS_CTRL_BIT << (mused.current_patternx - PED_CTRL));				
							
						move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, +1, mused.song.pattern[mused.current_pattern].num_steps);
					}
				}
			}
			break;
		}
		
		break;
	}
}

void edit_program_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_RETURN:
			{
				change_mode(EDITINSTRUMENT);
			}
			break;
		
			case SDLK_DOWN:
			{
				++mused.selected_param;
				if (mused.selected_param >= P_PARAMS+MUS_PROG_LEN) mused.selected_param = 0;
				if (mused.selected_param < P_PARAMS) change_mode(EDITINSTRUMENT); else change_mode(EDITPROG);
			}
			break;
			
			case SDLK_UP:
			{
				--mused.selected_param;
				if (mused.selected_param < 0) mused.selected_param = P_PARAMS+MUS_PROG_LEN-1;
				if (mused.selected_param < P_PARAMS) change_mode(EDITINSTRUMENT); else change_mode(EDITPROG);
			}
			break;
		
		
			case SDLK_RIGHT:
			{
				clamp(mused.editpos,+1,0,3);
				
			}
			break;
			
			case SDLK_LEFT:
			{
				clamp(mused.editpos,-1,0,3);
			}
			break;
		
			default:
			{
				int v = gethex(e->key.keysym.sym);
				
				if (v != -1)
				{
					editparambox(v);
				}
			}
			break;
		}
		
		break;
	}
}


void edit_text(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_ESCAPE:
			{
				strcpy(mused.edit_buffer, mused.edit_backup_buffer);
				change_mode(mused.prev_mode);
			}
			break;
			case SDLK_RETURN:
			{
				change_mode(mused.prev_mode);
			}
			break;
			
			case SDLK_BACKSPACE:
				clamp(mused.editpos,-1,0,mused.edit_buffer_size-1);
				mused.edit_buffer[mused.editpos] = '\0';
			break;
		
			case SDLK_RIGHT:
			{
				clamp(mused.editpos,+1,0,mused.edit_buffer_size-1);
			}
			break;
			
			case SDLK_LEFT:
			{
				clamp(mused.editpos,-1,0,mused.edit_buffer_size-1);
			}
			break;
		
			default:
			{
				if (mused.editpos < mused.edit_buffer_size && isprint(e->key.keysym.unicode))
				{
					mused.edit_buffer[mused.editpos] = e->key.keysym.unicode;
					mused.edit_buffer[mused.editpos + 1] = '\0';
					clamp(mused.editpos,+1,0,mused.edit_buffer_size);
				}
			}
			break;
		}
		
		break;
	}
}


static void reverb_add_param(int d)
{
	if (mused.edit_reverb_param == R_ENABLE)
	{
		flipbit(mused.song.flags, MUS_ENABLE_REVERB);
	}
	else
	{
		int p = mused.edit_reverb_param - R_DELAY;
		int tap = (p & ~1) / 2;
		if (!(p & 1))
		{
			clamp(mused.song.rvbtap[tap].delay, d * 1, 0, CYDRVB_SIZE - 1);
		}
		else
		{
			clamp(mused.song.rvbtap[tap].gain, d * 1, CYDRVB_LOW_LIMIT, 0);
		}
	}
	
	mus_set_reverb(&mused.mus, &mused.song);
}


void reverb_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_DOWN:
			{
				++mused.edit_reverb_param;
				if (mused.edit_reverb_param >= R_DELAY + CYDRVB_TAPS * 2) mused.edit_reverb_param = R_DELAY + CYDRVB_TAPS * 2 - 1;
			}
			break;
			
			case SDLK_UP:
			{
				--mused.edit_reverb_param;
				if (mused.edit_reverb_param < 0) mused.edit_reverb_param = 0;
			}
			break;
		
			case SDLK_RIGHT:
			{
				reverb_add_param(e->key.keysym.mod & KMOD_SHIFT ? +10 : +1);
			}
			break;
			
			case SDLK_LEFT:
			{
				reverb_add_param(e->key.keysym.mod & KMOD_SHIFT ? -10 : -1);
			}
			break;
		
			default: break;
		}
		
		break;
	}
}
