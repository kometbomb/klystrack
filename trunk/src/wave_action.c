#include "wave_action.h"
#include "mused.h"
#include "view/wavetableview.h"
#include "snd/freqs.h"
#include <string.h>
#include "wavegen.h"
#include "util/rnd.h"

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
		
		if (new_length < 100000000)
		{
			Sint16 *new_data = malloc(sizeof(Sint16) * new_length);
			
			if (new_data)
			{
				snapshot(S_T_WAVE_DATA);
				
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
			else
			{
				set_info_message("Out of memory!");
			}
		}
		else
		{
			set_info_message("Resulting wave was too big");
		}
	}
}


void wavetable_create_one_cycle(void *_settings, void *unused2, void *unused3)
{
	WgSettings *settings = _settings;
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		snapshot(S_T_WAVE_DATA);
	}
	
	int new_length = settings->length;
	Sint16 *new_data = malloc(sizeof(Sint16) * new_length);
	
	int lowest_mul = 999;
	
	for (int i = 0 ; i < settings->num_oscs ; ++i)
	{
		lowest_mul = my_min(lowest_mul, settings->chain[i].mult);
	}
	
	wg_gen_waveform(settings->chain, settings->num_oscs, new_data, new_length);
	
	if (w->data) free(w->data);
	w->data = new_data;
	w->sample_rate = new_length * 220 / lowest_mul;
	w->samples = new_length;
	w->loop_begin = 0;
	w->loop_end = new_length;
	w->flags = CYD_WAVE_LOOP;
	w->base_note = (MIDDLE_C + 9 - 12) << 8;
	
	invalidate_wavetable_view();
}


void wavetable_draw(float x, float y, float width)
{
	snapshot_cascade(S_T_WAVE_DATA, mused.selected_wavetable, 0);
	
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];

	if (w->samples > 0)
	{
		debug("draw %f,%f w = %f", x, y, width);
		int s = w->samples * x;
		int e = my_max(w->samples * (x + width), s + 1);
		
		for ( ; s < e && s < w->samples ; ++s)
		{
			w->data[s] = y * 65535 - 32768;
		}
		
		invalidate_wavetable_view();
	}
}


void wavegen_randomize(void *_settings, void *unused2, void *unused3)
{
	bool do_sines = !(rndu() & 3);
	bool do_shift = !(rndu() & 1);
	bool do_exp = !(rndu() & 3);
	bool do_highfreg = !(rndu() & 1);
	bool do_inharmonic = !(rndu() & 1);
	bool do_chop = !(rndu() & 3);
	
	mused.wgset.num_oscs = rnd(1, WG_CHAIN_OSCS);
	
	for (int i = 0 ; i < mused.wgset.num_oscs ; ++i)
	{
		mused.wgset.chain[i].flags = rnd(0, 3);
	
		if (do_sines)
		{
			if (do_chop && i > 0)
			{
				if (rndu() & 1)
					mused.wgset.chain[i].osc = WG_OSC_SINE;
				else
					mused.wgset.chain[i].osc = WG_OSC_SQUARE;
			}
			else
				mused.wgset.chain[i].osc = WG_OSC_SINE;
		}
		else
		{
			mused.wgset.chain[i].osc = rnd(0, WG_NUM_OSCS - 1);
		
			if (mused.wgset.chain[i].osc == WG_OSC_NOISE)
				mused.wgset.chain[i].osc = rnd(0, WG_NUM_OSCS - 1);
		}
		
		if (do_inharmonic)
			mused.wgset.chain[i].mult = rnd(1, do_highfreg ? 9 : 5);
		else
			mused.wgset.chain[i].mult = 1 << rnd(0, do_highfreg ? 3 : 2);
		
		mused.wgset.chain[i].op = rnd(0, WG_NUM_OPS - 1);
		
		if (do_shift)
			mused.wgset.chain[i].shift = rnd(0, 7);
		else
			mused.wgset.chain[i].shift = 0;
		
		if (do_exp)
			mused.wgset.chain[i].exp = rnd(5,95);
		else
			mused.wgset.chain[i].exp = 50;
	}
}


void wavegen_preset(void *_preset, void *_settings, void *unused3)
{
	WgSettings *preset = &((WgPreset*)_preset)->settings;
	WgSettings *settings = _settings;
	
	settings->num_oscs = preset->num_oscs;
	memcpy(settings->chain, preset->chain, sizeof(preset->chain[0]) * preset->num_oscs);
}


void wavetable_amp(void *_amp, void *unused2, void *unused3)
{
	snapshot(S_T_WAVE_DATA);
		
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		int amp = CASTPTR(int, _amp);
		
		debug("amp = %d", amp);
	
		for (int s = 0 ; s < w->samples ; ++s)
		{
			w->data[s] = my_max(my_min((Sint32)w->data[s] * amp / 32768, 32767), -32768);
		}
		
		invalidate_wavetable_view();
	}
}
