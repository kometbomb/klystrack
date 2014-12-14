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


#include "event.h"
#include "mused.h"
#include "action.h"
#include "edit.h"
#include "util/rnd.h"
#include <string.h>
#include "snd/cydwave.h"
#include "snd/freqs.h"
#include "mymsg.h"
#include "command.h"

extern Mused mused;

#define flipbit(val, bit) { val ^= bit; };

void editparambox(int v)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	Uint16 *param = &inst->program[mused.current_program_step];
	Uint32 mask = 0xffff0fff >> (mused.editpos*4);
	
	if (*param == MUS_FX_NOP)
		*param = 0;
	
	if (mused.editpos != 0 || v < 0xf)
	{
		// Keeps the exec next command bit intact
		*param = (*param & 0x8000) | (((*param & mask) | ((v&0xf) <<((3-mused.editpos)*4))) & 0x7fff);
	}
	else
	{
		*param = ((*param & mask) | ((v&0xf) <<((3-mused.editpos)*4)));
	}
	
	if (++mused.editpos > 3)
	{
		*param = validate_command(*param);
		mused.editpos = 3;
	}
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


void instrument_add_param(int a)
{
	MusInstrument *i = &mused.song.instrument[mused.current_instrument];
	
	if (a < 0) a = -1; else if (a > 0) a = 1;
	
	if (SDL_GetModState() & KMOD_SHIFT)
	{
		switch (mused.selected_param)
		{
			case P_BASENOTE: a *= 12; break;
			default: a *= 16; break;
		}
	}

	switch (mused.selected_param)
	{
		case P_INSTRUMENT:
		
		clamp(mused.current_instrument, a, 0, NUM_INSTRUMENTS - 1);
		
		break;
	
		case P_BASENOTE:
		
		clamp(i->base_note, a, 0, 95);
		
		break;
		
		case P_FINETUNE:
		
		clamp(i->finetune, a, -128, 127);
		
		break;
		
		case P_LOCKNOTE:
		
		flipbit(i->flags, MUS_INST_LOCK_NOTE);
		
		break;
		
		case P_DRUM:
		
		flipbit(i->flags, MUS_INST_DRUM);
		
		break;
		
		case P_KEYSYNC:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_KEY_SYNC);
		
		break;
		
		case P_SYNC:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_SYNC);
		
		break;
		
		case P_SYNCSRC:
		{
			int x = (Uint8)(i->sync_source+1);
			clamp(x, a, 0, MUS_MAX_CHANNELS);
			i->sync_source = x-1;
		}
		break;
		
		case P_WAVE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_WAVE);
		
		break;
		
		case P_WAVE_ENTRY:
		
		clamp(i->wavetable_entry, a, 0, CYD_WAVE_MAX_ENTRIES - 1);
		
		break;
		
		case P_WAVE_OVERRIDE_ENV:
		
		flipbit(i->cydflags, CYD_CHN_WAVE_OVERRIDE_ENV);
		
		break;
		
		case P_WAVE_LOCK_NOTE:
		
		flipbit(i->flags, MUS_INST_WAVE_LOCK_NOTE);
		
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
		
		case P_LFSR:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_LFSR);
		
		break;
		
		case P_LFSRTYPE:
		
		clamp(i->lfsr_type, a, 0, CYD_NUM_LFSR - 1);
		
		break;
				
		case P_NOISE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_NOISE);
		
		break;
		
		case P_METAL:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_METAL);
		
		break;
		
		
		case P_FX:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_FX);
		
		break;
		
		case P_FXBUS:
		
		clamp(i->fx_bus, a, 0, CYD_MAX_FX_CHANNELS - 1);
		
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
		
		case P_BUZZ:
		
		flipbit(i->flags, MUS_INST_YM_BUZZ);
			
		break;
		
		case P_BUZZ_SHAPE:
		
		clamp(i->ym_env_shape, a, 0, 3);
			
		break;
		
		case P_BUZZ_FINE:
		{
			clamp(i->buzz_offset, a, -32768, 32767);
		}	
		break;
		
		case P_BUZZ_SEMI:
		{
			int f = i->buzz_offset >> 8;
			clamp(f, a, -99, 99);
			i->buzz_offset = (i->buzz_offset & 0xff) | f << 8;
		}	
		break;
		
		case P_PW:
		
		clamp(i->pw, a*16, 0, 0x7ff);
			
		break;
		
		case P_1_4TH:
		flipbit(i->flags, MUS_INST_QUARTER_FREQ);
		break;
		
		case P_VOLUME:
		
		clamp(i->volume, a, 0, 255); // 255 = ~2x boost
			
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
		
		case P_VIBDELAY:
		
		clamp(i->vib_delay, a, 0, 0xff);
		
		break;
		
		case P_VIBSHAPE:
		
		clamp(i->vib_shape, a, 0, MUS_NUM_SHAPES - 1);
		
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
		
		case P_PWMSHAPE:
		
		clamp(i->pwm_shape, a, 0, MUS_NUM_SHAPES - 1);
		
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
		{
			int x = (Uint8)(i->ring_mod+1);
			clamp(x, a, 0, MUS_MAX_CHANNELS);
			i->ring_mod = x-1;
		}
		break;
		
		case P_CUTOFF:
		
		clamp(i->cutoff, a*16, 0, 2047);
		
		break;
		
		case P_RESONANCE:
		
		clamp(i->resonance, a, 0, 3);
		
		break;
		
		case P_FLTTYPE:
		
		clamp(i->flttype, a, 0, FLT_TYPES - 1);
		
		break;
		
		case P_NORESTART:
		
		flipbit(i->flags, MUS_INST_NO_PROG_RESTART);
		
		break;
		
		case P_FM_MODULATION:
		
		clamp(i->fm_modulation, a, 0, 0x7f);
			
		break;
		
		case P_FM_FEEDBACK:
		
		clamp(i->fm_feedback, a, 0, 0x7);
			
		break;
		
		case P_FM_ATTACK:
		
		clamp(i->fm_adsr.a, a, 0, 31);
			
		break;
		
		case P_FM_DECAY:
		
		clamp(i->fm_adsr.d, a, 0, 31);
			
		break;
		
		case P_FM_SUSTAIN:
		
		clamp(i->fm_adsr.s, a, 0, 31);
			
		break;
		
		case P_FM_RELEASE:
		
		clamp(i->fm_adsr.r, a, 0, 31);
			
		break;
		
		case P_FM_TRIANGLE:
		
		flipbit(i->fm_flags, CYD_FM_ENABLE_TRIANGLE);
		
		break;
		
		case P_FM_WAVE:
		
		flipbit(i->fm_flags, CYD_FM_ENABLE_WAVE);
		
		break;
		
		case P_FM_ENABLE:
		
		flipbit(i->cydflags, CYD_CHN_ENABLE_FM);
		
		break;
		
		case P_FM_HARMONIC_CARRIER:
		{
			Uint8 carrier = (i->fm_harmonic >> 4);
			Uint8 modulator = i->fm_harmonic & 0xf;
		
			clamp(carrier, a, 0, 15);
		
			i->fm_harmonic = carrier << 4 | modulator;
		}
		break;
		
		case P_FM_HARMONIC_MODULATOR:
		{
			Uint8 carrier = (i->fm_harmonic >> 4);
			Uint8 modulator = i->fm_harmonic & 0xf;
		
			clamp(modulator, a, 0, 15);
		
			i->fm_harmonic = carrier << 4 | modulator;
		}
		break;
		
		
		case P_FM_WAVE_ENTRY:
		
		clamp(i->fm_wave, a, 0, CYD_WAVE_MAX_ENTRIES - 1);
		
		break;
		
	
		default:
		break;
	}
	
}


static void play_the_jams(int sym, int chn, int state)
{
	if (sym == SDLK_SPACE && state == 0)
	{
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			cyd_enable_gate(mused.mus.cyd, &mused.mus.cyd->channel[i], 0);
	}
	else
	{
		int note = find_note(sym, mused.octave);
		if (note != -1) 
		{
			if (mused.flags & MULTIKEY_JAMMING)
			{
				SDL_Event e;
				e.type = state ? MSG_NOTEON : MSG_NOTEOFF;
				e.user.code = note;
				e.user.data1 = NULL;
				SDL_PushEvent(&e);
			}
			else
			{
				if (state == 1)
				{
					if (chn == -1 && !(mused.flags & MULTICHANNEL_PREVIEW))
						chn = 0;
				
					mus_trigger_instrument(&mused.mus, chn, &mused.song.instrument[mused.current_instrument], note, CYD_PAN_CENTER);
				}
			}
		}
	}
	
}


static void wave_the_jams(int sym)
{
	if (sym == SDLK_SPACE)
	{
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			cyd_enable_gate(mused.mus.cyd, &mused.mus.cyd->channel[i], 0);
	}
	else
	{
		int note = find_note(sym, mused.octave);
		if (note != -1) 
		{
			static MusInstrument inst;
			mus_get_default_instrument(&inst);
			inst.wavetable_entry = mused.selected_wavetable;
			inst.cydflags &= ~WAVEFORMS;
			inst.cydflags |= CYD_CHN_WAVE_OVERRIDE_ENV | CYD_CHN_ENABLE_WAVE;
			inst.flags &= ~MUS_INST_DRUM;
			mus_trigger_instrument(&mused.mus, 0, &inst, note, CYD_PAN_CENTER);
		}
	}
}



void edit_instrument_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_RETURN:
			{
				if (mused.selected_param == P_NAME) 
					set_edit_buffer(mused.song.instrument[mused.current_instrument].name, sizeof(mused.song.instrument[mused.current_instrument].name));
			}
			break;
			
			case SDLK_DOWN:
			{
				++mused.selected_param;
				
				if (mused.mode == EDITINSTRUMENT)
				{
					if (mused.selected_param >= P_PARAMS) mused.selected_param = P_PARAMS - 1;
				}
				else
				{
					if (mused.selected_param >= P_NAME) mused.selected_param = P_NAME;
				}
			}
			break;
			
			case SDLK_UP:
			{
				--mused.selected_param;
				
				if (mused.selected_param < 0) mused.selected_param = 0;
			}
			break;
		
			
		
			case SDLK_RIGHT:
			{
				instrument_add_param(+1);
			}
			break;
			
			
			case SDLK_LEFT:
			{
				instrument_add_param(-1);
			}
			break;
		
			default:
			{
				play_the_jams(e->key.keysym.sym, -1, 1);
			}
			break;
		}
		
		break;
		
		case SDL_KEYUP:
		
			play_the_jams(e->key.keysym.sym, -1, 0);
			
		break;
	}
}


static int gethex(int key)
{
	if (key >= SDLK_0 && key <= SDLK_9)
	{
		return key - SDLK_0;
	}
	else if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
	{
		return key - SDLK_KP_0;
	}
	else if (key >= SDLK_a && key <= SDLK_f)
	{
		return key - SDLK_a + 0xa;
	}
	else return -1;
}


static int getalphanum(const SDL_Keysym *keysym)
{
	int key = keysym->sym;

	if (key >= SDLK_0 && key <= SDLK_9)
	{
		return key - SDLK_0;
	}
	else if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
	{
		return key - SDLK_KP_0;
	}
	else if (!(keysym->mod & KMOD_SHIFT) && key >= SDLK_a && key <= SDLK_z)
	{
		return key - SDLK_a + 0xa;
	}
	else if ((keysym->mod & KMOD_SHIFT) && key >= SDLK_a && key <= SDLK_z)
	{
		return key - SDLK_a + 0xa + (SDLK_z - SDLK_a) + 1;
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


void add_sequence(int channel, int position, int pattern, int offset)
{
	for (int i = 0 ; i < mused.song.num_sequences[channel] ; ++i)
		if (mused.song.sequence[channel][i].position == position)
		{
			mused.song.sequence[channel][i].pattern = pattern;
			mused.song.sequence[channel][i].note_offset = offset;
			return;
		}
			
	if (mused.song.num_sequences[channel] >= NUM_SEQUENCES)
		return;
			
	mused.song.sequence[channel][mused.song.num_sequences[channel]].position = position;
	mused.song.sequence[channel][mused.song.num_sequences[channel]].pattern = pattern;
	mused.song.sequence[channel][mused.song.num_sequences[channel]].note_offset = offset;
	
	++mused.song.num_sequences[channel];
	
	qsort(mused.song.sequence[channel], mused.song.num_sequences[channel], sizeof(mused.song.sequence[channel][0]), seqsort);
}


Uint8 get_pattern_at(int channel, int position)
{
	for (int i = 0 ; i < mused.song.num_sequences[channel] ; ++i)
		if (mused.song.sequence[channel][i].position == position)
		{
			return mused.song.sequence[channel][i].pattern;
		}
		
	return 0;
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


static void update_sequence_slider(int d)
{
	int o = mused.current_sequencepos - mused.current_patternpos;

	slider_move_position(&mused.current_sequencepos, &mused.sequence_position, &mused.sequence_slider_param, d);
	
	if (!(mused.flags & SONG_PLAYING))
		mused.pattern_position = mused.current_patternpos = mused.current_sequencepos - o;
}



static void update_pattern_slider(int d)
{
	/*if (mused.flags & CENTER_PATTERN_EDITOR) 
	{
		mused.pattern_position = current_patternstep() + d;
		
		if (mused.pattern_position < 0) mused.pattern_position += mused.song.pattern[current_pattern()].num_steps;
		if (mused.pattern_position >= mused.song.pattern[current_pattern()].num_steps) mused.pattern_position -= mused.song.pattern[current_pattern()].num_steps;
		
		current_patternstep() = mused.pattern_position;
	}
	else */
	slider_move_position(&mused.current_patternpos, &mused.pattern_position, &mused.pattern_slider_param, d);
	mused.pattern_position = mused.current_patternpos;
	
	if (!(mused.flags & SONG_PLAYING))
		mused.current_sequencepos = mused.current_patternpos - mused.current_patternpos % mused.sequenceview_steps;
}


void update_position_sliders()
{
	update_pattern_slider(0);
	update_sequence_slider(0);
}


void update_horiz_sliders()
{
	slider_move_position(&mused.current_sequencetrack, &mused.sequence_horiz_position, &mused.sequence_horiz_slider_param, 0);
	slider_move_position(&mused.current_sequencetrack, &mused.pattern_horiz_position, &mused.pattern_horiz_slider_param, 0);
}


void sequence_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_SPACE:
			{
				if ((mused.flags & TOGGLE_EDIT_ON_STOP) || !(mused.flags & SONG_PLAYING))
						mused.flags ^= EDIT_MODE;
			
				if (mused.flags & SONG_PLAYING) stop(0, 0, 0);
			}
			break;
		
			case SDLK_RETURN:
			{
				if (mused.mode != EDITCLASSIC) change_mode(EDITPATTERN);
				else mused.focus = EDITPATTERN;
				
				//mused.current_patternpos = mused.current_sequencepos;
				
				update_position_sliders();
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
					snapshot(S_T_SEQUENCE);
					add_note_offset(-1);
					break;
				}
			
				int steps = mused.sequenceview_steps;
				if (e->key.keysym.sym == SDLK_PAGEDOWN)
					steps *= 16;
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					snapshot_cascade(S_T_SONGINFO, SI_LOOP, -1);
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						change_loop_point(MAKEPTR(steps), 0, 0);
					}
					else
					{
						change_song_length(MAKEPTR(steps), 0, 0);
					}
				}
				else
				{
					update_sequence_slider(steps);
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
					snapshot(S_T_SEQUENCE);
					add_note_offset(1);
					break;
				}
			
				int steps = mused.sequenceview_steps;
				if (e->key.keysym.sym == SDLK_PAGEUP)
					steps *= 16;
				
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					snapshot_cascade(S_T_SONGINFO, SI_LOOP, -1);
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						change_loop_point(MAKEPTR(-steps), 0,0);
					}
					else
					{
						change_song_length(MAKEPTR(-steps), 0, 0);
					}
				}
				else
				{
					update_sequence_slider(-steps);
				}
				
				if (((e->key.keysym.mod & KMOD_SHIFT) && !(e->key.keysym.mod & KMOD_CTRL)) )
				{
					select_range(mused.current_sequencepos);
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				snapshot(S_T_SEQUENCE);
			
				for (int i = 0; i < mused.song.num_sequences[mused.current_sequencetrack] ; ++i)
				{
					if (mused.song.sequence[mused.current_sequencetrack][i].position >= mused.current_sequencepos)
						mused.song.sequence[mused.current_sequencetrack][i].position += mused.sequenceview_steps;
				}
			}
			break;
			
			case SDLK_PERIOD:
			{
				snapshot(S_T_SEQUENCE);
				
				del_sequence(mused.current_sequencepos, mused.current_sequencepos+mused.sequenceview_steps, mused.current_sequencetrack);
				if (mused.song.song_length > mused.current_sequencepos + mused.sequenceview_steps)
					mused.current_sequencepos += mused.sequenceview_steps;
			}
			break;
			
			case SDLK_DELETE:
			{
				snapshot(S_T_SEQUENCE);
			
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
					change_seq_steps((void *)1, 0, 0);
				}
				else
				{
					if (mused.flags & EDIT_SEQUENCE_DIGITS)
					{
						if (mused.sequence_digit < 1)
						{
							++mused.sequence_digit;
							break;
						}
					}
				
					mused.sequence_digit = 0;
					++mused.current_sequencetrack;
					if (mused.current_sequencetrack >= mused.song.num_channels)
						mused.current_sequencetrack = 0;
						
					update_horiz_sliders();
				}
			}
			break;
			
			case SDLK_LEFT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					change_seq_steps((void *)-1, 0, 0);
				}
				else
				{
					if (mused.flags & EDIT_SEQUENCE_DIGITS)
					{
						if (mused.sequence_digit > 0)
						{
							--mused.sequence_digit;
							break;
						}
					}
				
					mused.sequence_digit = 0;
					--mused.current_sequencetrack;
					if (mused.current_sequencetrack < 0)
						mused.current_sequencetrack = mused.song.num_channels - 1;
						
					update_horiz_sliders();
				}
			}
			break;
		
			default:
			{
				if (mused.flags & EDIT_SEQUENCE_DIGITS)
				{
					int k;
				
					if ((k = gethex(e->key.keysym.sym)) != -1)
					{
						snapshot(S_T_SEQUENCE);
						
						Uint8 p = get_pattern_at(mused.current_sequencetrack, mused.current_sequencepos);
		
						if (mused.sequence_digit == 0)
							p = (p & (0x0f)) | (k << 4);
						else
							p = (p & (0xf0)) | k;
		
						add_sequence(mused.current_sequencetrack, mused.current_sequencepos, p, 0);
						
						if (mused.song.song_length > mused.current_sequencepos + mused.sequenceview_steps)
						{
							mused.current_sequencepos += mused.sequenceview_steps;
							mused.current_patternpos += mused.sequenceview_steps;
							update_position_sliders();
						}
					}
				}
				else
				{
					int p = getalphanum(&e->key.keysym);
					if (p != -1 && p < NUM_PATTERNS)
					{
						snapshot(S_T_SEQUENCE);
						add_sequence(mused.current_sequencetrack, mused.current_sequencepos, p, 0);
						if (mused.song.song_length > mused.current_sequencepos + mused.sequenceview_steps)
						{
							mused.current_sequencepos += mused.sequenceview_steps;
							mused.current_patternpos += mused.sequenceview_steps;
							update_position_sliders();
						}
					}
				}
			}
			break;
		}
		
		break;
	}
	
	//update_ghost_patterns();
}


static void switch_track(int d)
{
	mused.current_sequencetrack = (mused.current_sequencetrack + (int)mused.song.num_channels + d) % mused.song.num_channels;
	update_horiz_sliders();
	mused.sequence_digit = 0;
}


void pattern_event(SDL_Event *e)
{
	int last_param = PED_PARAMS - 1;
	if (!viscol(PED_COMMAND1)) last_param = my_max(PED_COMMAND1 - 1, last_param);
	else if (!viscol(PED_CTRL)) last_param = my_max(PED_CTRL - 1, last_param);
	else if (!viscol(PED_VOLUME1)) last_param = my_max(PED_VOLUME1 - 1, last_param);
	else if (!viscol(PED_INSTRUMENT1)) last_param = my_max(PED_INSTRUMENT1 - 1, last_param);

	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			
			case SDLK_RETURN:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					Uint8 p = get_pattern_at(mused.current_sequencetrack, mused.current_sequencepos);
					add_sequence(mused.current_sequencetrack, mused.current_patternpos, p, 0);
				}
				else
				{
					if (mused.mode != EDITCLASSIC) change_mode(EDITSEQUENCE);
					else mused.focus = EDITSEQUENCE;
					
					mused.flags &= ~SHOW_LOGO; // hide logo when jumping to sequence
					
					mused.current_sequencepos = mused.current_patternpos;
					
					update_position_sliders();
				}
			}
			break;
			
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				begin_selection(mused.current_patternpos);
			break;
		
			case SDLK_END:
			case SDLK_PAGEDOWN:
			case SDLK_DOWN:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					update_sequence_slider(mused.sequenceview_steps);
				}
				else
				{
					int steps = 1;
					if (e->key.keysym.sym == SDLK_PAGEDOWN) steps = steps * 16;
					
					if (get_current_pattern())
					{
						if (e->key.keysym.sym == SDLK_END) steps = get_current_pattern()->num_steps - current_patternstep() - 1;
						
						if ((e->key.keysym.mod & KMOD_SHIFT) && get_current_pattern())
							steps = my_min(steps, get_current_pattern()->num_steps - current_patternstep() - 1);
					}
					
					update_pattern_slider(steps);
					
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						select_range(mused.current_patternpos);
					}
				}
			}
			break;
			
			case SDLK_HOME:
			case SDLK_PAGEUP:
			case SDLK_UP:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					update_sequence_slider(-mused.sequenceview_steps);
				}
				else
				{
					
					int steps = 1;
					if (e->key.keysym.sym == SDLK_PAGEUP) steps = steps * 16;
					if (e->key.keysym.sym == SDLK_HOME) steps = current_patternstep();
					
					if (e->key.keysym.mod & KMOD_SHIFT)
						steps = my_min(steps, current_patternstep());
					
					update_pattern_slider(-steps);
				
					if (e->key.keysym.mod & KMOD_SHIFT)
					{
						select_range(mused.current_patternpos);
					}
					
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				if (!(mused.flags & EDIT_MODE)) break;
				
				if (get_current_step())
				{
					snapshot(S_T_PATTERN);
				
					if ((e->key.keysym.mod & KMOD_ALT)) 
					{
						resize_pattern(get_current_pattern(), get_current_pattern()->num_steps + 1);
						zero_step(&mused.song.pattern[current_pattern()].step[mused.song.pattern[current_pattern()].num_steps - 1]);
						break;
					}
						
					for (int i = mused.song.pattern[current_pattern()].num_steps-1; i >= current_patternstep() ; --i)
						memcpy(&mused.song.pattern[current_pattern()].step[i], &mused.song.pattern[current_pattern()].step[i-1], sizeof(mused.song.pattern[current_pattern()].step[0]));
					
					zero_step(&mused.song.pattern[current_pattern()].step[current_patternstep()]);
				}
			}
			break;
			
			case SDLK_BACKSPACE:
			case SDLK_DELETE:
			{
				if (!(mused.flags & EDIT_MODE)) break;
			
				if (get_current_step())
				{
				
					if (e->key.keysym.sym == SDLK_BACKSPACE)
					{
						if (current_patternstep() > 0) update_pattern_slider(-1);
						else break;
					}
					
					snapshot(S_T_PATTERN);
					
					if ((e->key.keysym.mod & KMOD_ALT)) 
					{
						if (mused.song.pattern[current_pattern()].num_steps > 1)
							resize_pattern(&mused.song.pattern[current_pattern()], mused.song.pattern[current_pattern()].num_steps - 1);
							
						if (current_patternstep() >= mused.song.pattern[current_pattern()].num_steps) --mused.current_patternpos;
						break;
					}
				
					if (!(mused.flags & DELETE_EMPTIES) || e->key.keysym.sym == SDLK_BACKSPACE)
					{
						for (int i = current_patternstep()  ; i < mused.song.pattern[current_pattern()].num_steps ; ++i)
							memcpy(&mused.song.pattern[current_pattern()].step[i], &mused.song.pattern[current_pattern()].step[i+1], sizeof(mused.song.pattern[current_pattern()].step[0]));
							
						zero_step(&mused.song.pattern[current_pattern()].step[mused.song.pattern[current_pattern()].num_steps - 1]);				
						
						if (current_patternstep() >= mused.song.pattern[current_pattern()].num_steps) --mused.current_patternpos;
					}
					else
					{
						if (e->key.keysym.mod & KMOD_SHIFT)
						{
							zero_step(&mused.song.pattern[current_pattern()].step[current_patternstep()]);				
						}
						else
						{
							switch (mused.current_patternx)
							{
								case PED_NOTE:
									mused.song.pattern[current_pattern()].step[current_patternstep()].note = MUS_NOTE_NONE;	
									mused.song.pattern[current_pattern()].step[current_patternstep()].instrument = MUS_NOTE_NO_INSTRUMENT;	
									break;
									
								case PED_INSTRUMENT1:
								case PED_INSTRUMENT2:
									mused.song.pattern[current_pattern()].step[current_patternstep()].instrument = MUS_NOTE_NO_INSTRUMENT;	
									break;
									
								case PED_VOLUME1:
								case PED_VOLUME2:
									mused.song.pattern[current_pattern()].step[current_patternstep()].volume = MUS_NOTE_NO_VOLUME;	
									break;
									
								case PED_LEGATO:
								case PED_SLIDE:
								case PED_VIB:
									mused.song.pattern[current_pattern()].step[current_patternstep()].ctrl = 0;	
									break;
								
								case PED_COMMAND1:
								case PED_COMMAND2:
								case PED_COMMAND3:
								case PED_COMMAND4:
									mused.song.pattern[current_pattern()].step[current_patternstep()].command = 0;	
									break;
							}
						}
					
						update_pattern_slider(mused.note_jump);
					}
				}
			}
			break;
		
			case SDLK_RIGHT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
						int x = mused.current_patternx;
						
						switch_track(+1);
						
						if (e->key.keysym.mod & KMOD_SHIFT)
							mused.current_patternx = x;
				}
				else
				{
					++mused.current_patternx;
					
					if (!viscol(mused.current_patternx) && (mused.current_patternx == PED_INSTRUMENT1 || mused.current_patternx == PED_INSTRUMENT2))
						mused.current_patternx = PED_VOLUME1;
						
					if (!viscol(mused.current_patternx) && (mused.current_patternx == PED_VOLUME1 || mused.current_patternx == PED_VOLUME2))
						mused.current_patternx = PED_CTRL;
						
					if (!viscol(mused.current_patternx) && (mused.current_patternx >= PED_CTRL && mused.current_patternx < PED_COMMAND1))
						mused.current_patternx = PED_COMMAND1;
						
					if (!viscol(mused.current_patternx) && (mused.current_patternx >= PED_COMMAND1 && mused.current_patternx <= PED_COMMAND4))
						mused.current_patternx = PED_PARAMS;
						
					if (mused.current_patternx >= PED_PARAMS)
					{
						mused.current_patternx = 0;
						switch_track(+1);
					}
				}
			}
			break;
			
			case SDLK_LEFT:
			{
				if (e->key.keysym.mod & KMOD_CTRL)
				{
					int x = mused.current_patternx;
					
					switch_track(-1);
						
					if (e->key.keysym.mod & KMOD_SHIFT)
						mused.current_patternx = x;
				}
				else
				{
					--mused.current_patternx;
					
					if (mused.current_patternx < 0)
					{
							mused.current_patternx = PED_PARAMS - 1;
							
							switch_track(-1);
					}
					
					if (!viscol(mused.current_patternx) && (mused.current_patternx >= PED_COMMAND1 && mused.current_patternx <= PED_COMMAND4))
						mused.current_patternx = PED_VIB;
					
					if (!viscol(mused.current_patternx) && (mused.current_patternx < PED_COMMAND1 && mused.current_patternx > PED_VOLUME2))
						mused.current_patternx = PED_VOLUME2;
										
					if (!viscol(mused.current_patternx) && (mused.current_patternx == PED_VOLUME1 || mused.current_patternx == PED_VOLUME2))
						mused.current_patternx = PED_INSTRUMENT2;
						
					if (!viscol(mused.current_patternx) && (mused.current_patternx == PED_INSTRUMENT1 || mused.current_patternx == PED_INSTRUMENT2))
						mused.current_patternx = PED_NOTE;
					
					if (mused.current_patternx < 0)
					{
						if (mused.single_pattern_edit)
						{
							mused.current_patternx = 0;
						}
						else
						{
							mused.current_patternx = last_param;
							switch_track(-1);
						}
					}
				}
			}
			break;
			
			default:
			{
				if (e->key.keysym.sym == SDLK_SPACE)
				{
					if ((mused.flags & TOGGLE_EDIT_ON_STOP) || !(mused.flags & SONG_PLAYING))
						mused.flags ^= EDIT_MODE;
					
					if (mused.flags & SONG_PLAYING) stop(0, 0, 0);
					
					play_the_jams(e->key.keysym.sym, -1, 1);
				}
			
				if (!(mused.flags & EDIT_MODE)) 
				{
					/* still has to play a note */
				
					if (mused.current_patternx == PED_NOTE)
					{
						play_the_jams(e->key.keysym.sym, mused.current_sequencetrack, 1);
					}
					
					break;
				}
				
				if (get_current_step())
				{
					if (mused.current_patternx == PED_NOTE)
					{
						if (e->key.keysym.sym == SDLK_PERIOD)
						{
							mused.song.pattern[current_pattern()].step[current_patternstep()].note = MUS_NOTE_NONE;				
							
							snapshot(S_T_PATTERN);
								
							update_pattern_slider(mused.note_jump);
						}
						else if (e->key.keysym.sym == SDLK_1)
						{
							mused.song.pattern[current_pattern()].step[current_patternstep()].note = MUS_NOTE_RELEASE;
							
							snapshot(S_T_PATTERN);
								
							update_pattern_slider(mused.note_jump);
						}
						else
						{
							int note = find_note(e->key.keysym.sym, mused.octave);
							if (note != -1) 
							{
								play_the_jams(e->key.keysym.sym, mused.current_sequencetrack, 1);
								
								snapshot(S_T_PATTERN);
								
								mused.song.pattern[current_pattern()].step[current_patternstep()].note = note;				
								mused.song.pattern[current_pattern()].step[current_patternstep()].instrument = mused.current_instrument;
								
								update_pattern_slider(mused.note_jump);
							}
						}
					}
					else if (mused.current_patternx == PED_INSTRUMENT1 || mused.current_patternx == PED_INSTRUMENT2)
					{
						if (e->key.keysym.sym == SDLK_PERIOD)
						{
							mused.song.pattern[current_pattern()].step[current_patternstep()].instrument = MUS_NOTE_NO_INSTRUMENT;				
							
							snapshot(S_T_PATTERN);
								
							update_pattern_slider(mused.note_jump);
						}
						else if (gethex(e->key.keysym.sym) != -1)
						{
							if (mused.song.num_instruments > 0)
							{
								Uint8 inst = mused.song.pattern[current_pattern()].step[current_patternstep()].instrument;
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
								
								snapshot(S_T_PATTERN);
							
								mused.song.pattern[current_pattern()].step[current_patternstep()].instrument = inst; 
								mused.current_instrument = inst % mused.song.num_instruments;
							}
						
							update_pattern_slider(mused.note_jump);
						}
					}
					else if (mused.current_patternx == PED_VOLUME1 || mused.current_patternx == PED_VOLUME2)
					{
						switch (e->key.keysym.sym)
						{
							case  SDLK_PERIOD:
								snapshot(S_T_PATTERN);
								
								mused.song.pattern[current_pattern()].step[current_patternstep()].volume = MUS_NOTE_NO_VOLUME;				
									
								update_pattern_slider(mused.note_jump);
								break;
							
							case SDLK_u:						
							case SDLK_d:
							{
								int cmd = 0;
							
								switch (e->key.keysym.sym)
								{
									case SDLK_u: cmd = MUS_NOTE_VOLUME_FADE_UP; break;
									case SDLK_d: cmd = MUS_NOTE_VOLUME_FADE_DN; break;
									default: break;
								}
								
								snapshot(S_T_PATTERN);
								
								if (mused.song.pattern[current_pattern()].step[current_patternstep()].volume == MUS_NOTE_NO_VOLUME)
									mused.song.pattern[current_pattern()].step[current_patternstep()].volume = 0;
							
								if (mused.current_patternx == PED_VOLUME1)
									mused.song.pattern[current_pattern()].step[current_patternstep()].volume = 
										(mused.song.pattern[current_pattern()].step[current_patternstep()].volume & 0xf)
										| cmd;
										
								update_pattern_slider(mused.note_jump);
							}	
							break;
						
							default:
								if (gethex(e->key.keysym.sym) != -1)
								{
									Uint8 vol = mused.song.pattern[current_pattern()].step[current_patternstep()].volume;
									if ((vol == MUS_NOTE_NO_VOLUME)) vol = 0;
									
									switch (mused.current_patternx)
									{
										case PED_VOLUME1:
										vol = (vol & 0x0f) | ((gethex(e->key.keysym.sym) << 4) & 0xf0);
										break;
										
										case PED_VOLUME2:
										vol = (vol & 0xf0) | gethex(e->key.keysym.sym);
										break;
									}
									
									snapshot(S_T_PATTERN);
									
									if ((vol & 0xf0) != MUS_NOTE_VOLUME_FADE_UP && (vol & 0xf0) != MUS_NOTE_VOLUME_FADE_DN)
										mused.song.pattern[current_pattern()].step[current_patternstep()].volume = my_min(MAX_VOLUME, vol); 
									else mused.song.pattern[current_pattern()].step[current_patternstep()].volume = vol;
								
									update_pattern_slider(mused.note_jump);
								}
								break;
						}
					}
					else if (mused.current_patternx >= PED_COMMAND1 && mused.current_patternx <= PED_COMMAND4)
					{
						if (gethex(e->key.keysym.sym) != -1)
						{
							Uint16 inst = mused.song.pattern[current_pattern()].step[current_patternstep()].command;
							
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
							
							snapshot(S_T_PATTERN);
							
							mused.song.pattern[current_pattern()].step[current_patternstep()].command = validate_command(inst) & 0x7fff; 
							
							update_pattern_slider(mused.note_jump);
						}
					}
					else if (mused.current_patternx >= PED_CTRL && mused.current_patternx < PED_COMMAND1)
					{
						if (e->key.keysym.sym == SDLK_PERIOD || e->key.keysym.sym == SDLK_0)
						{
							snapshot(S_T_PATTERN);
						
							mused.song.pattern[current_pattern()].step[current_patternstep()].ctrl &= ~(MUS_CTRL_BIT << (mused.current_patternx - PED_CTRL));				
								
							update_pattern_slider(mused.note_jump);
						}
						if (e->key.keysym.sym == SDLK_1)
						{
							snapshot(S_T_PATTERN);
						
							mused.song.pattern[current_pattern()].step[current_patternstep()].ctrl |= (MUS_CTRL_BIT << (mused.current_patternx - PED_CTRL));				
								
							update_pattern_slider(mused.note_jump);
						}
					}
				}
			}
			break;
		}
		
		break;
		
		case SDL_KEYUP:
		
			play_the_jams(e->key.keysym.sym, -1, 0);
			
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
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
				begin_selection(mused.current_program_step);
			break;
		
			case SDLK_PERIOD:
				snapshot(S_T_INSTRUMENT);
			
				mused.song.instrument[mused.current_instrument].program[mused.current_program_step] = MUS_FX_NOP;
			break;
			
			case SDLK_SPACE:
			{
				snapshot(S_T_INSTRUMENT);
				
				if ((mused.song.instrument[mused.current_instrument].program[mused.current_program_step] & 0xf000) != 0xf000) 
					mused.song.instrument[mused.current_instrument].program[mused.current_program_step] ^= 0x8000;
			}
			break;
			
			case SDLK_RETURN:
			{
				MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
				Uint16 *param = &inst->program[mused.current_program_step];
				*param = validate_command(*param);
			}
			break;
			
			case SDLK_PAGEDOWN:
			case SDLK_DOWN:
			{
				MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
				Uint16 *param = &inst->program[mused.current_program_step];
				*param = validate_command(*param);
			
				int steps = 1;
				if (e->key.keysym.sym == SDLK_PAGEDOWN) steps *= 16;
				
				slider_move_position(&mused.current_program_step, &mused.program_position, &mused.program_slider_param, steps);
				
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.current_program_step);
				}
			}
			break;
			
			case SDLK_PAGEUP:
			case SDLK_UP:
			{
				MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
				Uint16 *param = &inst->program[mused.current_program_step];
				*param = validate_command(*param);
			
				int steps = 1;
				if (e->key.keysym.sym == SDLK_PAGEUP) steps *= 16;
				
				slider_move_position(&mused.current_program_step, &mused.program_position, &mused.program_slider_param, -steps);
				
				if (e->key.keysym.mod & KMOD_SHIFT)
				{
					select_range(mused.current_program_step);
				}
			}
			break;
		
			case SDLK_INSERT:
			{
				snapshot(S_T_INSTRUMENT);
				for (int i = MUS_PROG_LEN-2; i >= mused.current_program_step ; --i)
					mused.song.instrument[mused.current_instrument].program[i] = mused.song.instrument[mused.current_instrument].program[i-1];
				mused.song.instrument[mused.current_instrument].program[mused.current_program_step] = MUS_FX_NOP;
			}
			break;
			
			case SDLK_BACKSPACE:
			case SDLK_DELETE:
			{
				snapshot(S_T_INSTRUMENT);
				if (e->key.keysym.sym == SDLK_BACKSPACE)
				{
					if (mused.current_program_step > 0) slider_move_position(&mused.current_program_step, &mused.program_position, &mused.program_slider_param, -1);
					else break;
				}
			
				if (!(mused.flags & DELETE_EMPTIES) || e->key.keysym.sym == SDLK_BACKSPACE)
				{
					for (int i = mused.current_program_step  ; i < MUS_PROG_LEN-1 ; ++i)
						mused.song.instrument[mused.current_instrument].program[i] = mused.song.instrument[mused.current_instrument].program[i+1];
					mused.song.instrument[mused.current_instrument].program[MUS_PROG_LEN-1] = MUS_FX_NOP;
				}
				else 
				{
					mused.song.instrument[mused.current_instrument].program[mused.current_program_step] = MUS_FX_NOP;
					++mused.current_program_step;
					if (mused.current_program_step >= MUS_PROG_LEN)
						mused.current_program_step = MUS_PROG_LEN - 1;
				}
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
					snapshot(S_T_INSTRUMENT);
					editparambox(v);
				}
			}
			break;
		}
		
		break;
		
		case SDL_KEYUP:
		
			play_the_jams(e->key.keysym.sym, -1, 0);
			
		break;
	}
}


void edit_text(SDL_Event *e)
{
	int r = generic_edit_text(e, mused.edit_buffer, mused.edit_buffer_size, &mused.editpos);
	
	if (r == -1)
	{
		strcpy(mused.edit_buffer, mused.edit_backup_buffer);
		change_mode(mused.prev_mode);
	}
	else if (r == 1)
	{
		change_mode(mused.prev_mode);
	}
}


void set_room_size(int fx, int size, int vol, int dec)
{
	snapshot(S_T_FX);
	
	const int min_delay = 5;
	int ms = (CYDRVB_SIZE - min_delay) * size / 64;
	
	for (int i = 0 ; i < CYDRVB_TAPS ;++i)
	{
		int p = rnd(i * ms / CYDRVB_TAPS, (i + 1) * ms / CYDRVB_TAPS) + min_delay;
		mused.song.fx[fx].rvb.tap[i].delay = p;
		mused.song.fx[fx].rvb.tap[i].gain = CYDRVB_LOW_LIMIT-CYDRVB_LOW_LIMIT * pow(1.0 - (double)p / ms, (double)dec / 3) * vol / 16;
	}
	
	mus_set_fx(&mused.mus, &mused.song);
}


void fx_add_param(int d)
{
	if (d < 0) d = -1; else if (d > 0) d = 1;

	if (SDL_GetModState() & KMOD_SHIFT)
		d *= 10;
		
	snapshot_cascade(S_T_FX, mused.fx_bus, mused.edit_reverb_param);

	switch (mused.edit_reverb_param)
	{
		case R_FX_BUS:
		{
			clamp(mused.fx_bus, d, 0, CYD_MAX_FX_CHANNELS - 1);
		}
		break;
	
		case R_CRUSH:
		{
			flipbit(mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CRUSH);
		}
		break;
		
		case R_CRUSHDITHER:
		{
			flipbit(mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CRUSH);
		}
		break;
		
		case R_CRUSHGAIN:
		{
			clamp(mused.song.fx[mused.fx_bus].crushex.gain, d, 0, 128);
			mus_set_fx(&mused.mus, &mused.song);
		}
		break;
		
		case R_CRUSHBITS:
		{
			clamp(mused.song.fx[mused.fx_bus].crush.bit_drop, d, 0, 15);
			mus_set_fx(&mused.mus, &mused.song);
		}
		break;
		
		case R_CRUSHDOWNSAMPLE:
		{
			clamp(mused.song.fx[mused.fx_bus].crushex.downsample, d, 0, 64);
			mus_set_fx(&mused.mus, &mused.song);
		}
		break;
		
		case R_CHORUS:
		{
			flipbit(mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CHORUS);
		}
		break;
		
		case R_MINDELAY:
		{
			clamp(mused.song.fx[mused.fx_bus].chr.min_delay, d, 0, mused.song.fx[mused.fx_bus].chr.max_delay);
			clamp(mused.song.fx[mused.fx_bus].chr.min_delay, 0, mused.song.fx[mused.fx_bus].chr.min_delay, 255);
		}
		break;
		
		case R_MAXDELAY:
		{
			clamp(mused.song.fx[mused.fx_bus].chr.max_delay, d, mused.song.fx[mused.fx_bus].chr.min_delay, 255);
			clamp(mused.song.fx[mused.fx_bus].chr.min_delay, 0, 0, mused.song.fx[mused.fx_bus].chr.max_delay);
		}
		break;
		
		case R_SEPARATION:
		{
			clamp(mused.song.fx[mused.fx_bus].chr.sep, d, 0, 64);
		}
		break;
		
		case R_RATE:
		{
			clamp(mused.song.fx[mused.fx_bus].chr.rate, d, 0, 255);
		}
		break;
		
		case R_PITCH_INACCURACY:
		{
			clamp(mused.song.pitch_inaccuracy, d, 0, 12);
		}
		break;
		
		case R_MULTIPLEX:
		{
			flipbit(mused.song.fx[mused.fx_bus].flags, MUS_ENABLE_MULTIPLEX);
		}
		break;
		
		case R_MULTIPLEX_PERIOD:
		{
			clamp(mused.song.multiplex_period, d, 0, 255);
		}
		break;
	
		case R_ENABLE:
		{
			flipbit(mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_REVERB);
		}
		break;
		
		case R_ROOMSIZE:
		{
			clamp(mused.fx_room_size, d, 1, 64);
		}
		break;
		
		case R_ROOMDECAY:
		{
			clamp(mused.fx_room_dec, d, 1, 9);
		}
		break;
		
		case R_ROOMVOL:
		{
			clamp(mused.fx_room_vol, d, 1, 16);
		}
		break;
		
		case R_SPREAD:
		{
			clamp(mused.song.fx[mused.fx_bus].rvb.spread, d, 0, 0xff);
			mus_set_fx(&mused.mus, &mused.song);
		}
		break;
		
		default:
		{
			int p = mused.edit_reverb_param - R_DELAY;
			int tap = (p & ~1) / 2;
			if (!(p & 1))
			{
				clamp(mused.song.fx[mused.fx_bus].rvb.tap[tap].delay, d * 1, 0, CYDRVB_SIZE - 1);
			}
			else
			{
				clamp(mused.song.fx[mused.fx_bus].rvb.tap[tap].gain, d * 1, CYDRVB_LOW_LIMIT, 0);
			}
			
			mus_set_fx(&mused.mus, &mused.song);
		}
		break;
	}
	
}


void fx_event(SDL_Event *e)
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
				fx_add_param(+1);
			}
			break;
			
			case SDLK_LEFT:
			{
				fx_add_param(-1);
			}
			break;
		
			default: 
				play_the_jams(e->key.keysym.sym, -1, 1);
			break;
		}
		
		break;
		
		case SDL_KEYUP:
		
			play_the_jams(e->key.keysym.sym, -1, 0);
			
		break;
	}
}


void wave_add_param(int d)
{
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (d < 0) d = -1; else if (d > 0) d = 1;
	
	if (SDL_GetModState() & KMOD_SHIFT)
	{
		switch (mused.wavetable_param)
		{
			case W_BASE: d *= 12; break;
			default: d *= 256; break;
		}
	}
		
	snapshot_cascade(S_T_WAVE_PARAM, mused.selected_wavetable, mused.wavetable_param);

	switch (mused.wavetable_param)
	{
		case W_INTERPOLATE:
		{
			flipbit(w->flags, CYD_WAVE_NO_INTERPOLATION);
		}
		break;
	
		case W_WAVE:
		{
			clamp(mused.selected_wavetable, d, 0, CYD_WAVE_MAX_ENTRIES - 1);
		}
		break;
	
		case W_RATE:
		{
			clamp(w->sample_rate, d, 1, 192000);
		}
		break;
	
		case W_BASE:
		{
			clamp(w->base_note, d * 256, 0, (FREQ_TAB_SIZE - 1) * 256);
		}
		break;
		
		case W_BASEFINE:
		{
			clamp(w->base_note, d, 0, (FREQ_TAB_SIZE - 1) * 256);
		}
		break;
		
		case W_LOOP:
		{
			flipbit(w->flags, CYD_WAVE_LOOP);
		}
		break;
		
		case W_LOOPPINGPONG:
		{
			flipbit(w->flags, CYD_WAVE_PINGPONG);
		}
		break;
		
		case W_LOOPBEGIN:
		{
			clamp(w->loop_begin, d, 0, my_min(w->samples, w->loop_end));
			clamp(w->loop_end, 0, w->loop_begin, w->samples);
		}
		break;
		
		case W_LOOPEND:
		{
			clamp(w->loop_end, d, w->loop_begin, w->samples);
			clamp(w->loop_begin, 0, 0, my_min(w->samples, w->loop_end));
		}
		break;
	}
	
}


void wave_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_DOWN:
			{
				++mused.wavetable_param;
				if (mused.wavetable_param >= W_N_PARAMS) mused.wavetable_param = W_N_PARAMS - 1;
			}
			break;
			
			case SDLK_UP:
			{
				--mused.wavetable_param;
				if (mused.wavetable_param < 0) mused.wavetable_param = 0;
			}
			break;
		
			case SDLK_RIGHT:
			{
				wave_add_param(+1);
			}
			break;
			
			case SDLK_LEFT:
			{
				wave_add_param(-1);
			}
			break;
		
			default: 
				wave_the_jams(e->key.keysym.sym);
			break;
		}
		
		break;
	}
}


void songinfo_add_param(int d)
{
	if (d < 0) d = -1; else if (d > 0) d = 1;

	if (SDL_GetModState() & KMOD_SHIFT)
		d *= 16;
		
	if (d) snapshot_cascade(S_T_SONGINFO, mused.songinfo_param, -1);

	switch (mused.songinfo_param)
	{

		case SI_MASTERVOL:
			change_master_volume(MAKEPTR(d), 0, 0);
			break;
		
		case SI_LENGTH:
			change_song_length(MAKEPTR(d * mused.sequenceview_steps), 0, 0);
			break;
			
		case SI_LOOP:
			change_loop_point(MAKEPTR(d * mused.sequenceview_steps), 0, 0);
			break;
			
		case SI_STEP:
			change_seq_steps(MAKEPTR(d), 0, 0);
			break;
			
		case SI_SPEED1:
			change_song_speed(MAKEPTR(0), MAKEPTR(d), 0);
			break;
			
		case SI_SPEED2:
			change_song_speed(MAKEPTR(1), MAKEPTR(d), 0);
			break;
			
		case SI_RATE:
			change_song_rate(MAKEPTR(d), 0, 0);
			break;
			
		case SI_TIME:
			change_timesig(MAKEPTR(d), 0, 0);
			break;
			
		case SI_OCTAVE:
			change_octave(MAKEPTR(d), 0, 0);
			break;
			
		case SI_CHANNELS:
			change_channels(MAKEPTR(d), 0, 0);
			break;
	}
}


void songinfo_event(SDL_Event *e)
{
	switch (e->type)
	{
		case SDL_KEYDOWN:
		
		switch (e->key.keysym.sym)
		{
			case SDLK_DOWN:
			{
				++mused.songinfo_param;
				if (mused.songinfo_param >= SI_N_PARAMS) mused.songinfo_param = SI_N_PARAMS - 1;
			}
			break;
			
			case SDLK_UP:
			{
				--mused.songinfo_param;
				if (mused.songinfo_param < 0) mused.songinfo_param = 0;
			}
			break;
		
			case SDLK_RIGHT:
			{
				songinfo_add_param(+1);
			}
			break;
			
			case SDLK_LEFT:
			{
				songinfo_add_param(-1);
			}
			break;
			
			default: break;
		}
		
		break;
	}
}


static int note_playing[MUS_MAX_CHANNELS] = {-1};


static int find_playing_note(int n)
{
	cyd_lock(&mused.cyd, 1);
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS && i < mused.cyd.n_channels ; ++i)
	{
		if (note_playing[i] == n && mused.mus.channel[i].instrument == &mused.song.instrument[mused.current_instrument])
		{
			cyd_lock(&mused.cyd, 0);
			return i;
		}
	}
	
	cyd_lock(&mused.cyd, 0);
	
	return -1;
}


void note_event(SDL_Event *e)
{
	switch (e->type)
	{
		case MSG_NOTEON:
			if (find_playing_note(e->user.code) == -1 && e->user.code < FREQ_TAB_SIZE)
			{
				int c = mus_trigger_instrument(&mused.mus, -1, &mused.song.instrument[mused.current_instrument], e->user.code, CYD_PAN_CENTER);
				note_playing[c] = e->user.code;
			}
			break;
			
		case MSG_NOTEOFF:
		{
			int c;
			if ((c = find_playing_note(e->user.code)) != -1)
			{
				mus_release(&mused.mus, c);
				note_playing[c] = -1;
			}
		}
		break;
		
		case MSG_PROGRAMCHANGE:
		{
			if (e->user.code >= 0 && e->user.code < NUM_INSTRUMENTS)
				mused.current_instrument = e->user.code;
		}
		break;
	}
}
