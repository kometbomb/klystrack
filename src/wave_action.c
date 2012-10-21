#include "wave_action.h"
#include "mused.h"
#include "view/wavetableview.h"
#include "snd/freqs.h"

void wavetable_drop_lowest_bit(void *unused1, void *unused2, void *unused3)
{
	if (!mused.wavetable_bits)
	{
		debug("Wave is silent");
		return;
	}
		
	snapshot(S_T_WAVE_DATA);
		
	const CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	Uint16 mask = 0xffff << (__builtin_ffs(mused.wavetable_bits));
	
	if (w->samples > 0)
	{
		int d = 0;
				
		for (; d < w->samples ; ++d)
		{
			w->data[d] &= mask;
		}
		
		invalidate_wavetable_view();
	}
}

void wavetable_halve_samplerate(void *unused1, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		int s = 0, d = 0;
				
		for (; s < (w->samples & (~1)) ; s += 2, ++d)
		{
			w->data[d] = (w->data[s] + w->data[s + 1]) / 2;
		}
		
		w->samples /= 2;
		w->sample_rate /= 2;
		w->loop_begin /= 2; 
		w->loop_end /= 2;
		
		invalidate_wavetable_view();
	}
}


void wavetable_normalize(void *vol, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		int m = 0;
				
		for (int s = 0 ; s < w->samples ; ++s)
		{
			m = my_max(m, abs(w->data[s]));
		}
		
		debug("Peak = %d", m);
		
		if (m != 0)
		{
			for (int s = 0 ; s < w->samples ; ++s)
			{
				w->data[s] = my_max(my_min((Sint32)w->data[s] * CASTPTR(int, vol) / m, 32767), -32768);
			}
		}
		
		invalidate_wavetable_view();
	}
}


void wavetable_cut_tail(void *unused1, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		for (int s = w->samples - 1 ; s > 0 ; --s)
		{
			if (w->data[s] != 0)
			{
				debug("Cut %d samples", w->samples - (s + 1));
				w->samples = s + 1;
				w->loop_end = my_min(w->samples, w->loop_end);
				w->loop_begin = my_min(w->samples, w->loop_begin);
				
				invalidate_wavetable_view();
				
				break;
			}
		}
	}
}


void wavetable_cut_head(void *unused1, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		for (int s = 0 ; s < 0 ; --s)
		{
			if (w->data[s] != 0 && s != 0)
			{
				debug("Cut %d samples", s);
				
				w->samples -= s;
				memmove(&w->data[0], &w->data[s], w->samples);
				
				w->loop_end = my_min(w->samples, w->loop_end - s);
				w->loop_begin = my_max(0, (int)w->loop_begin - s);
				
				invalidate_wavetable_view();
				
				break;
			}
		}
	}
}


void wavetable_chord(void *transpose, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		int denom = 1, nom = 1;
			
		// too lazy to add a LCM function so here's a table
			
		switch (CASTPTR(int, transpose))
		{
			case 4: // perfect 4th
				denom = 4; nom = 3;
				break;
				
			case 5: // perfect 5th
				denom = 3; nom = 2;
				break;
				
			default:
			case 12: // perfect octave
				denom = 2; nom = 1;
				break;
		}
		
		int new_length = nom * w->samples;
		Sint16 *new_data = malloc(sizeof(Sint16) * new_length);
		
		for (int s = 0 ; s < new_length ; ++s)
		{
			new_data[s] = ((int)w->data[s % w->samples] + (int)w->data[(s * denom / nom) % w->samples]) / 2;
		}
		
		free(w->data);
		w->data = new_data;
		w->samples = new_length;
		w->loop_begin *= nom;
		w->loop_end *= nom;
		
		invalidate_wavetable_view();
	}
}


void wavetable_create_one_cycle(void *unused1, void *unused2, void *unused3)
{
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		snapshot(S_T_WAVE_DATA);
	}
	
	int new_length = 256;
	Sint16 *new_data = malloc(sizeof(Sint16) * new_length);
	
	for (int s = 0 ; s < new_length ; ++s)
	{
		new_data[s] = sin(s * M_PI * 2 / new_length) * 8192;
	}
	
	if (w->data) free(w->data);
	w->data = new_data;
	w->sample_rate = new_length * 220;
	w->samples = new_length;
	w->loop_begin = 0;
	w->loop_end = new_length;
	w->flags = CYD_WAVE_LOOP;
	w->base_note = (MIDDLE_C + 9 - 12) << 8;
	
	invalidate_wavetable_view();
}

