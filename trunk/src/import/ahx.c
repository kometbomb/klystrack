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

#include "ahx.h"
#include "edit.h"
#include "mused.h"
#include "event.h"
#include "SDL_endian.h"
#include <math.h>
#include "snd/freqs.h"

extern Mused mused;


static Uint16 find_command_ahx(Uint8 command, Uint8 data, Uint8 *ctrl)
{
	switch (command)
	{
		case 0xc:
		{
			if (data <= 0x40)
				return MUS_FX_SET_VOLUME | ((int)data * 2);
			else if (data >= 0x50 && data <= 0x90)
				return MUS_FX_SET_GLOBAL_VOLUME | ((int)(data - 0x50) * 2); // not totally correct
			else if (data >= 0xa0 && data <= 0xe0)
				return MUS_FX_SET_CHANNEL_VOLUME | ((int)(data - 0xa0) * 2);
		}
		break;
		
		case 0xa:
		{
	_0xa00:
			return MUS_FX_FADE_VOLUME | (my_min(0xf, (data & 0x0f) * 2)) | (my_min(0xf, ((data & 0xf0) >> 4) * 2) << 4);
		}
		break;
		
		case 0x3:
		{
			if (data < 0x20)
			{
				return MUS_FX_SLIDE | my_min(0xff, data * 16);
			}
			else
			{
				*ctrl |= MUS_CTRL_LEGATO;
				return 0;
			}
		}
		break;
		
		case 0x5:
		{
			*ctrl |= MUS_CTRL_SLIDE|MUS_CTRL_LEGATO;
			goto _0xa00;
		}
		break;
		
		case 0x1:
		{
			return MUS_FX_PORTA_UP | my_min(0xff, data * 8);
		}
		break;
		
		case 0x2:
		{
			return MUS_FX_PORTA_DN | my_min(0xff, data * 8);
		}
		break;
		
		case 0x4:
		{
			return MUS_FX_CUTOFF_SET | ((int)(data & 63) * 255 / 63);
		}
		break;
		
		case 0x9:
		{
			return MUS_FX_PW_SET | ((int)(data & 63) * 0x80 / 63);
		}
		break;
		
		case 0xf:
		{
			return MUS_FX_SET_SPEED | (data & 0xf);
		}
		break;
		
		case 0xe:
		{
			if ((data & 0xf0) == 0xc0)
			{
				return MUS_FX_EXT_NOTE_CUT | (data & 0xf);
			}
			else if ((data & 0xf0) == 0xd0)
			{
				return MUS_FX_EXT_NOTE_DELAY | (data & 0xf);
			}
			else if ((data & 0xf0) == 0x40)
			{
				return MUS_FX_VIBRATO | (data & 0xf);
			}
			else if ((data & 0xf0) == 0x10)
			{
				return MUS_FX_EXT_PORTA_UP | (data & 0xf);
			}
			else if ((data & 0xf0) == 0x20)
			{
				return MUS_FX_EXT_PORTA_DN | (data & 0xf);
			}
			else if ((data & 0xf0) == 0xa0)
			{
				return MUS_FX_EXT_FADE_VOLUME_UP | (data & 0xf);
			}
			else if ((data & 0xf0) == 0xb0)
			{
				return MUS_FX_EXT_FADE_VOLUME_DN | (data & 0xf);
			}
		}
		break;
		
		default: break;
	}
	
	return 0;
}


static void ahx_program(Uint8 fx1, Uint8 data1, int *pidx, MusInstrument *i, const int *pos, int *has_4xx)
{
	switch (fx1)
	{
		case 0: 	
			i->program[*pidx] = MUS_FX_CUTOFF_SET | ((data1 & 0x3f) * 4);
			break;
			
		case 1: 	
			i->program[*pidx] = MUS_FX_PORTA_UP | (data1 & 0xff);
			break;
			
		case 2: 	
			i->program[*pidx] = MUS_FX_PORTA_DN | (data1 & 0xff);
			break;
			
		case 3: 	
			i->program[*pidx] = MUS_FX_PW_SET | my_min(255, my_max(8, ((int)(data1) * 0x80 / 63)));
			break;
			
		case 0xf:		
			i->program[*pidx] = MUS_FX_SET_SPEED | data1;
			break;
		
		case 4:
			*has_4xx |= data1;
		case 7:
			--*pidx; // not supported
			i->program[*pidx] &= ~0x8000;
			break;
			
		case 5:
			i->program[*pidx] = MUS_FX_JUMP | (pos[data1] & (MUS_PROG_LEN - 1));
			if (*pidx > 0) i->program[*pidx - 1] &= ~0x8000;
			break;
			
		case 0xc:
		case 6:
			if (data1<=0x40)
				i->program[*pidx] = MUS_FX_SET_VOLUME | (data1 * 2);
			else if (data1>=0x50 && data1<=0x90)
				i->program[*pidx] = MUS_FX_SET_VOLUME | ((int)(data1 - 0x50) * 2); // should be relative but no can do
			
			break;
	}
}


int import_ahx(FILE *f)
{
	char sig[4];
	
	fread(sig, 1, 4, f);
	
	int ver = 0;
	
	if (memcmp("THX\0", sig, 4) == 0)
	{
		ver = 0;
	}
	else if (memcmp("THX\1", sig, 4) == 0)
	{
		ver = 1;
	}
	else return 0;
	
	Uint8 byte;
	Uint16 word;
	
	fread(&word, 1, sizeof(word), f); // title offset
	fread(&word, 1, sizeof(word), f); 
	
	word = SDL_SwapBE16(word);
	
	int track0 = (word & 0x8000) != 0;
	
	mused.song.song_rate = (1 + ((word & 0x7000) >> 12)) * 50;
	
	int LEN = word & 0xfff;
	
	fread(&word, 1, sizeof(word), f); 
	
	mused.song.loop_point = SDL_SwapBE16(word);
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int TRL = mused.sequenceview_steps = byte;
	
	mused.song.loop_point *= TRL;
	mused.song.song_length = LEN * TRL;
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int TRK = byte;
	
	fread(&byte, 1, sizeof(byte), f);  
	
	int SMP = byte;
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int SS = byte;
	
	fseek(f, SS * 2, SEEK_CUR); // we don't need the subsong positions
	
	{
		Uint8 seq[2 * 0x1000 * 4];
		
		fread(&seq, 1, LEN * 2 * 4, f);
		
		for (int i = 0 ; i < LEN ; ++i)
		{
			for (int c = 0 ; c < 4 ; ++c)
			{
				/*if (!track0 && pat == 0)
					continue;*/
				
				add_sequence(c, i * TRL, seq[c * 2 + i * 4 * 2], *(Sint8*)&seq[c * 2 + i * 4 * 2 + 1]);
			}	
		}
	}
	
	for (int pat = 0 ; pat < TRK + 1; ++pat)
	{
		resize_pattern(&mused.song.pattern[pat], TRL);
		
		for (int s = 0 ; s < TRL ; ++s)
			zero_step(&mused.song.pattern[pat].step[s]);
		
		if (track0 && pat == 0) continue;
		
		Uint8 steps[64 * 3 + 1];
		
		fread(steps, 3, TRL, f);
		
		for (int s = 0 ; s < TRL ; ++s)
		{
			Uint32 step = SDL_SwapLE32(((Uint32)steps[s * 3] << 16) | ((Uint32)steps[s * 3 + 1] << 8) | ((Uint32)steps[s * 3 + 2]));
			
			Uint8 note = (step >> 18) & 0x3f;
			Uint8 instrument = (step >> 12) & 0x3f;
			Uint8 command = (step >> 8) & 0xf;
			Uint8 data = step & 0xff;
			
			mused.song.pattern[pat].step[s].note = note ? (note - 1) : MUS_NOTE_NONE;
			mused.song.pattern[pat].step[s].instrument = instrument ? instrument - 1 : MUS_NOTE_NO_INSTRUMENT;
			mused.song.pattern[pat].step[s].ctrl = 0;
			mused.song.pattern[pat].step[s].command = find_command_ahx(command, data, &mused.song.pattern[pat].step[s].ctrl);
			mused.song.pattern[pat].step[s].volume = MUS_NOTE_NO_VOLUME;
		}
	}
	
	for (int smp = 0 ; smp < SMP ; ++smp)
	{
		MusInstrument *i = &mused.song.instrument[smp];
		
		mus_get_default_instrument(i);
		i->flags = MUS_INST_SET_PW|MUS_INST_SET_CUTOFF|MUS_INST_INVERT_VIBRATO_BIT|MUS_INST_RELATIVE_VOLUME;
		i->cydflags = CYD_CHN_ENABLE_FILTER|CYD_CHN_ENABLE_PULSE;
		
		fread(&byte, 1, 1, f);
		
		i->volume = byte * 2;
		i->slide_speed = 0xc0;
		
		fread(&byte, 1, 1, f);
		
		int s = my_min(5, byte & 0x7);
		
		if (s >= 4)
		{
			s -= 2;
			i->flags |= MUS_INST_QUARTER_FREQ;
		}
		
		int wavelen_semitones = s * 12;
		
		i->base_note = (MIDDLE_C + 48) - wavelen_semitones;
		
		Uint8 vol, len;
		fread(&len, 1, 1, f);
		fread(&vol, 1, 1, f);
		i->adsr.a = my_min(31, vol > 0x10 ? 0 : (vol ? (len / vol) / 2 : len / 4));
		
		fread(&len, 1, 1, f);
		fread(&vol, 1, 1, f);
		
		i->adsr.d = my_min(31, sqrt((float)len / 255) * 31 * 2 + 1); // AHX envelopes are linear, we have exponential
		i->adsr.s = my_min(31, sqrt((float)vol / 64) * 31);
		
		fread(&byte, 1, 1, f);
		
		fread(&len, 1, 1, f);
		fread(&vol, 1, 1, f);
		i->adsr.r = 1;
		i->adsr.s = my_max(my_min(31, sqrt((float)vol / 64) * 31), i->adsr.s);
		
		if (vol == 0)
		{
			i->adsr.s = 0;
			i->adsr.d = my_min(i->adsr.d, sqrt((float)len / 255) * 31 * 2 + 1);
		}
		
		/* --- */
		
		fread(&byte, 1, 1, f);
		fread(&byte, 1, 1, f);
		fread(&byte, 1, 1, f);
		
		Uint8 flt_upper, flt_lower;
		
		fread(&flt_lower, 1, 1, f);
		
		flt_lower &= 63;
		
		fread(&byte, 1, 1, f);
		
		fread(&byte, 1, 1, f);
		i->vib_delay = byte;
		i->vibrato_depth = (byte & 0xf) * 8;
		
		fread(&byte, 1, 1, f);
		i->vibrato_speed = byte * 3;
		
		Uint8 lower, upper;
		fread(&lower, 1, 1, f);
		fread(&upper, 1, 1, f);
		
		i->pwm_depth = my_min(255, (int)(upper - lower) * 2);
		i->pw = ((upper + lower) / 2) * 2047 / 63;
		
		fread(&byte, 1, 1, f);
		i->pwm_speed = (16 / my_max(1, byte));
		
		fread(&flt_upper, 1, 1, f);
		
		flt_upper &= 63;
		
		if (flt_upper != flt_lower) // Probably AHX0
			i->cutoff = 2047 * (int)((flt_upper - flt_lower) & 63) / 63;
		else
			i->cutoff = 2047;
		
		fread(&byte, 1, 1, f);
		i->prog_period = byte;
		
		int PLEN;
		fread(&byte, 1, 1, f);
		
		PLEN = byte;
		
		int pidx = 0;
		int pos[256];
		int was_fixed = 0, has_4xx = 0;
		
		Uint32 steps[256];
		fread(&steps, sizeof(Uint32), PLEN, f);
		
		for (int s = 0 ; s < PLEN ; ++s)
		{
			Uint32 step = SDL_SwapBE32(steps[s]);
			
			pos[s] = pidx; // map multiple klystrack program steps to the ahx program step
			
			if (pidx < MUS_PROG_LEN - 1)
			{
				Uint8 fx2 = (step & 0xe0000000) >> 29 ;
				Uint8 fx1 = (step & 0x1c000000) >> 26;
				Uint8 wave = (step & 0x3800000) >> 23;
				Uint8 fixed_note = (step >> 22) & 1;
				Uint8 note = (step >> 16) & 63;
				Uint8 data1 = (step >> 8) & 0xff;
				Uint8 data2 = (step) & 0xff;
				
				if (wave != 0)
				{
					// 1=triangle,
                    //  2=sawtooth, 3=square, 4=noise
					
					switch (wave)
					{
						case 1:
							i->program[pidx] = MUS_FX_SET_WAVEFORM | CYD_CHN_ENABLE_TRIANGLE;
							break;
						case 2:
							i->program[pidx] = MUS_FX_SET_WAVEFORM | CYD_CHN_ENABLE_SAW;
							break;
						case 3:
							i->program[pidx] = MUS_FX_SET_WAVEFORM | CYD_CHN_ENABLE_PULSE;
							break;
						case 4:
							i->program[pidx] = MUS_FX_SET_WAVEFORM | CYD_CHN_ENABLE_NOISE;
							break;
						default:
							break;
					}
				}
				
				if (note || was_fixed)
				{
					if (wave && pidx < MUS_PROG_LEN - 1)
					{
						i->program[pidx] |= 0x8000;
						++pidx;					
					}
					
					int n = note - 1;
					
					if (fixed_note) n = my_min(0xf0, my_max(0, (int)n)) + 48 - wavelen_semitones;
					
					n = my_max(0, my_min(n, FREQ_TAB_SIZE - 1));
					
					if (pidx < MUS_PROG_LEN - 1) i->program[pidx] = (fixed_note ? MUS_FX_ARPEGGIO_ABS : MUS_FX_ARPEGGIO) | ((n) & 0xff);
				}
				
				if (fx1 == 0x5 && fx2 != 0x0)
				{
					// jump should be processed after fx2 has been processed or it will jump to wrong row
				
					fx1 ^= fx2;
					fx2 ^= fx1;
					fx1 ^= fx2;
					
					data1 ^= data2;
					data2 ^= data1;
					data1 ^= data2;
				}				
				
				if (fx1 || data1)
				{
					if ((wave || note) && pidx < MUS_PROG_LEN - 1)
					{
						i->program[pidx] |= 0x8000;
						++pidx;
					}
					
					if (pidx < MUS_PROG_LEN - 1) ahx_program(fx1, data1, &pidx, i, pos, &has_4xx);
				}
				
				if (fx2 || data2)
				{
					if ((wave || note || (fx1 || data1)) && pidx < MUS_PROG_LEN - 1)
					{
						i->program[pidx] |= 0x8000;
						++pidx;
					}
					
					if (pidx < MUS_PROG_LEN - 1) ahx_program(fx2, data2, &pidx, i, pos, &has_4xx);
				}
				
				was_fixed = fixed_note;
				
				++pidx;
			}
		}
		
		if (!(has_4xx & 0x0f))
		{
			i->pwm_depth = 0 ; // no modulation set so disable
		}
		
		if (!(has_4xx & 0xf0))
		{
			i->cutoff = 2047 ; // no modulation set so disable
		}
		
		i->program[pidx] = MUS_FX_END;
	}
	
	size_t begin_names = ftell(f);
	fseek(f, 0, SEEK_END);
	size_t bytes = ftell(f) - begin_names;
	
	char *txt = malloc(bytes);
	
	fseek(f, begin_names, SEEK_SET);
	
	fread(txt, bytes, 1, f);
	
	char *ptr = txt;
	
	strncpy(mused.song.title, ptr, sizeof(mused.song.title));
	
	ptr += strlen(ptr) + 1;
	
	for (int smp = 0 ; smp < SMP ; ++smp)
	{
		strncpy(mused.song.instrument[smp].name, ptr, sizeof(mused.song.instrument[smp].name));
		ptr += strlen(ptr) + 1;
	}
	
	free(txt);
	
	// Amiga panning
	// not completely panned to left and right
	mused.song.default_panning[0] = -48;
	mused.song.default_panning[1] = 48;
	mused.song.default_panning[2] = 48;
	mused.song.default_panning[3] = -48;
	
	return 1;
}
