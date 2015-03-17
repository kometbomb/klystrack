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

#include "diskop.h"
#include "action.h"
#include "gui/toolutil.h"
#include "mused.h"
#include "macros.h"
#include "gui/msgbox.h"
#include <stdbool.h>
#include "wave.h"
#include "snd/freqs.h"
#include "snd/pack.h"
#include "view/wavetableview.h"
#include <string.h>
#include "memwriter.h"

extern Mused mused;
extern GfxDomain *domain;


static void write_wavetable_entry(SDL_RWops *f, const CydWavetableEntry *write_wave, bool write_wave_data)
{
	Uint32 flags = write_wave->flags & ~(CYD_WAVE_COMPRESSED_DELTA|CYD_WAVE_COMPRESSED_GRAY); // need to unmask bits because they're set by bitpack
	Uint32 sample_rate = write_wave->sample_rate;
	Uint32 samples = write_wave->samples, loop_begin = write_wave->loop_begin, loop_end = write_wave->loop_end;
	Uint16 base_note = write_wave->base_note;
	
	if (!write_wave_data)
	{
		// if the wave is not used and the data is not written, set these to zero too
		loop_begin = 0;
		loop_end = 0;
		samples = 0;
	}
	
	Uint8 *packed = NULL;
	Uint32 packed_size = 0;
	
	if (write_wave->samples > 0 && write_wave_data)
	{
		int pack_flags;
		packed = bitpack_best(write_wave->data, write_wave->samples, &packed_size, &pack_flags);
		
		flags |= (Uint32)pack_flags << 3;
	}
	
	FIX_ENDIAN(flags);
	FIX_ENDIAN(sample_rate);
	FIX_ENDIAN(samples);
	FIX_ENDIAN(loop_begin);
	FIX_ENDIAN(loop_end);
	FIX_ENDIAN(base_note);
	
	SDL_RWwrite(f, &flags, sizeof(flags), 1);
	SDL_RWwrite(f, &sample_rate, sizeof(sample_rate), 1);
	SDL_RWwrite(f, &samples, sizeof(samples), 1);
	SDL_RWwrite(f, &loop_begin, sizeof(loop_begin), 1);
	SDL_RWwrite(f, &loop_end, sizeof(loop_end), 1);
	SDL_RWwrite(f, &base_note, sizeof(base_note), 1);
	
	
	if (packed)
	{
		FIX_ENDIAN(packed_size);
		SDL_RWwrite(f, &packed_size, sizeof(packed_size), 1);
		
		SDL_RWwrite(f, packed, sizeof(packed[0]), (packed_size + 7) / 8);
		
		free(packed);
	}
}

	
static void save_instrument_inner(SDL_RWops *f, MusInstrument *inst, const CydWavetableEntry *write_wave, const CydWavetableEntry *write_wave_fm)
{
	Uint32 temp32 = inst->flags;
	FIX_ENDIAN(temp32);
	SDL_RWwrite(f, &temp32, sizeof(temp32), 1);
	temp32 = inst->cydflags;
	FIX_ENDIAN(temp32);
	SDL_RWwrite(f, &temp32, sizeof(temp32), 1);
	SDL_RWwrite(f, &inst->adsr, sizeof(inst->adsr), 1);
	SDL_RWwrite(f, &inst->sync_source, sizeof(inst->sync_source), 1);
	SDL_RWwrite(f, &inst->ring_mod, sizeof(inst->ring_mod), 1); 
	Uint16 temp16 = inst->pw;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, sizeof(temp16), 1);
	SDL_RWwrite(f, &inst->volume, sizeof(inst->volume), 1);
	Uint8 progsteps = 0;
	for (int i = 0 ; i < MUS_PROG_LEN ; ++i)
		if (inst->program[i] != MUS_FX_NOP) progsteps = i+1;
	SDL_RWwrite(f, &progsteps, sizeof(progsteps), 1);
	for (int i = 0 ; i < progsteps ; ++i)
	{
		temp16 = inst->program[i];
		FIX_ENDIAN(temp16);
		SDL_RWwrite(f, &temp16, sizeof(temp16), 1);
	}
	
	SDL_RWwrite(f, &inst->prog_period, sizeof(inst->prog_period), 1); 
	SDL_RWwrite(f, &inst->vibrato_speed, sizeof(inst->vibrato_speed), 1); 
	SDL_RWwrite(f, &inst->vibrato_depth, sizeof(inst->vibrato_depth), 1); 
	SDL_RWwrite(f, &inst->pwm_speed, sizeof(inst->pwm_speed), 1); 
	SDL_RWwrite(f, &inst->pwm_depth, sizeof(inst->pwm_depth), 1); 
	SDL_RWwrite(f, &inst->slide_speed, sizeof(inst->slide_speed), 1);
	SDL_RWwrite(f, &inst->base_note, sizeof(inst->base_note), 1);
	SDL_RWwrite(f, &inst->finetune, sizeof(inst->finetune), 1);
	Uint8 len = strlen(inst->name);
	SDL_RWwrite(f, &len, sizeof(len), 1);
	if (len)
		SDL_RWwrite(f, &inst->name, sizeof(inst->name[0]), len);
	temp16 = inst->cutoff;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, sizeof(temp16), 1);
	SDL_RWwrite(f, &inst->resonance, sizeof(inst->resonance), 1);
	SDL_RWwrite(f, &inst->flttype, sizeof(inst->flttype), 1);
	SDL_RWwrite(f, &inst->ym_env_shape, sizeof(inst->ym_env_shape), 1);
	temp16 = inst->buzz_offset;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, sizeof(temp16), 1);
	SDL_RWwrite(f, &inst->fx_bus, sizeof(inst->fx_bus), 1);
	SDL_RWwrite(f, &inst->vib_shape, sizeof(inst->vib_shape), 1);
	SDL_RWwrite(f, &inst->vib_delay, sizeof(inst->vib_delay), 1);
	SDL_RWwrite(f, &inst->pwm_shape, sizeof(inst->pwm_shape), 1);
	SDL_RWwrite(f, &inst->lfsr_type, sizeof(inst->lfsr_type), 1);
	
	if (write_wave)
	{
		Uint8 temp = 0xff;
		SDL_RWwrite(f, &temp, sizeof(temp), 1);
	}
	else
	{
		SDL_RWwrite(f, &inst->wavetable_entry, sizeof(inst->wavetable_entry), 1);
	}
	
	SDL_RWwrite(f, &inst->fm_flags, sizeof(inst->fm_flags), 1);
	SDL_RWwrite(f, &inst->fm_modulation, sizeof(inst->fm_modulation), 1);
	SDL_RWwrite(f, &inst->fm_feedback, sizeof(inst->fm_feedback), 1);
	SDL_RWwrite(f, &inst->fm_harmonic, sizeof(inst->fm_harmonic), 1);
	SDL_RWwrite(f, &inst->fm_adsr, sizeof(inst->fm_adsr), 1);
	SDL_RWwrite(f, &inst->fm_attack_start, sizeof(inst->fm_attack_start), 1);
	
	if (write_wave_fm)
	{
		Uint8 temp = (inst->wavetable_entry == inst->fm_wave) ? 0xfe : 0xff;
		SDL_RWwrite(f, &temp, sizeof(temp), 1);
	}
	else
	{
		SDL_RWwrite(f, &inst->fm_wave, sizeof(inst->fm_wave), 1);
	}
	
	if (write_wave)
	{
		write_wavetable_entry(f, write_wave, true);
	}
	
	if (write_wave_fm)
	{
		if (inst->wavetable_entry != inst->fm_wave)
			write_wavetable_entry(f, write_wave_fm, true);
	}
}


static void write_packed_pattern(SDL_RWops *f, const MusPattern *pattern, bool skip)
{
	/* 
	
	Format: 
		00 		1 * byte			number of incoming steps
		01...	n_steps * nibble	incoming data
		...		n_steps * variable	data described by bits in nibble 
		
		If ctrl bit 7 is set, there's also a volume column incoming
	*/

	Uint16 steps = pattern->num_steps;
	
	if (skip) steps = 0;
	
	FIX_ENDIAN(steps);
	SDL_RWwrite(f, &steps, 1, sizeof(steps));
	SDL_RWwrite(f, &pattern->color, 1, sizeof(pattern->color));
	
	Uint8 buffer = 0;
	for (int i = 0 ; i < steps ; ++i)
	{
		if (pattern->step[i].note != MUS_NOTE_NONE)
			buffer |= MUS_PAK_BIT_NOTE;
			
		if (pattern->step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			buffer |= MUS_PAK_BIT_INST;
			
		if (pattern->step[i].ctrl != 0 || pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
			buffer |= MUS_PAK_BIT_CTRL;
			
		if (pattern->step[i].command != 0)
			buffer |= MUS_PAK_BIT_CMD;
			
		if (i & 1 || i + 1 >= steps)
			SDL_RWwrite(f, &buffer, 1, sizeof(buffer));
			
		buffer <<= 4;
	}
	
	for (int i = 0 ; i < steps ; ++i)
	{
		if (pattern->step[i].note != MUS_NOTE_NONE)
			SDL_RWwrite(f, &pattern->step[i].note, 1, sizeof(pattern->step[i].note));
			
		if (pattern->step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			SDL_RWwrite(f, &pattern->step[i].instrument, 1, sizeof(pattern->step[i].instrument));
			
		if (pattern->step[i].ctrl != 0 || pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
		{
			Uint8 ctrl = pattern->step[i].ctrl;
			if (pattern->step[i].volume != MUS_NOTE_NO_VOLUME) 
				ctrl |= MUS_PAK_BIT_VOLUME;
			SDL_RWwrite(f, &ctrl, 1, sizeof(pattern->step[i].ctrl));
		}
			
		if (pattern->step[i].command != 0)
		{
			Uint16 c = pattern->step[i].command;
			FIX_ENDIAN(c);
			SDL_RWwrite(f, &c, 1, sizeof(pattern->step[i].command));
		}
		
		if (pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
			SDL_RWwrite(f, &pattern->step[i].volume, 1, sizeof(pattern->step[i].volume));
	}
}


int open_song(FILE *f)
{
	new_song();

	if (!mus_load_song_file(f, &mused.song, mused.mus.cyd->wavetable_entries)) return 0;
	
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.num_instruments = NUM_INSTRUMENTS;
	
	// Use kewlkool heuristics to determine sequence spacing
	
	mused.sequenceview_steps = mused.song.sequence_step;
	
	if (mused.sequenceview_steps == 0)
	{
		mused.sequenceview_steps = 1000;
		
		for (int c = 0 ; c < MUS_MAX_CHANNELS ; ++c)
			for (int s = 1 ; s < mused.song.num_sequences[c] && mused.song.sequence[c][s-1].position < mused.song.song_length ; ++s)
				if (mused.sequenceview_steps > mused.song.sequence[c][s].position - mused.song.sequence[c][s-1].position)
				{
					mused.sequenceview_steps = mused.song.sequence[c][s].position - mused.song.sequence[c][s-1].position;
				}
		
		for (int c = 0 ; c < MUS_MAX_CHANNELS ; ++c)	
			if (mused.song.num_sequences[c] > 0 && mused.song.sequence[c][mused.song.num_sequences[c]-1].position < mused.song.song_length)
			{
				if (mused.sequenceview_steps > mused.song.song_length - mused.song.sequence[c][mused.song.num_sequences[c]-1].position)
				{
					mused.sequenceview_steps = mused.song.song_length - mused.song.sequence[c][mused.song.num_sequences[c]-1].position;
				}
			}
		
		if (mused.sequenceview_steps < 1) mused.sequenceview_steps = 1;
		if (mused.sequenceview_steps == 1000) mused.sequenceview_steps = 16;
	}
	
	mus_set_fx(&mused.mus, &mused.song);
	enable_callback(true);
	mirror_flags();
	
	if (!mused.song.time_signature) mused.song.time_signature = 0x404;
	
	mused.time_signature = mused.song.time_signature;
	
	mused.flags &= ~EDIT_MODE;
	
	unmute_all_action(NULL, NULL, NULL);
	
	for (int i = 0 ; i < mused.song.num_patterns ; ++i)
		if(mused.song.pattern[i].num_steps == 0)
			resize_pattern(&mused.song.pattern[i], mused.default_pattern_length);
	
	set_channels(mused.song.num_channels);
	
	return 1;
}


int save_instrument(SDL_RWops *f)
{
	const Uint8 version = MUS_VERSION;
				
	SDL_RWwrite(f, MUS_INST_SIG, strlen(MUS_INST_SIG), sizeof(MUS_INST_SIG[0]));
	
	SDL_RWwrite(f, &version, 1, sizeof(version));
	
	save_instrument_inner(f, &mused.song.instrument[mused.current_instrument], &mused.mus.cyd->wavetable_entries[mused.song.instrument[mused.current_instrument].wavetable_entry], &mused.mus.cyd->wavetable_entries[mused.song.instrument[mused.current_instrument].fm_wave]);
	
	return 1;
}


int save_song_inner(SDL_RWops *f, SongStats *stats)
{
	bool kill_unused_things = false;

	Uint8 n_inst = mused.song.num_instruments;
	if (!confirm(domain, mused.slider_bevel, &mused.largefont, "Save unused song elements?"))
	{
		int maxpat = -1;
		for (int c = 0 ; c < mused.song.num_channels ; ++c)
		{
			for (int i = 0 ; i < mused.song.num_sequences[c] ; ++i)
				 if (maxpat < mused.song.sequence[c][i].pattern)
					maxpat = mused.song.sequence[c][i].pattern;
		}
		
		n_inst = 0;
		
		for (int i = 0 ; i <= maxpat ; ++i)
			for (int s = 0 ; s < mused.song.pattern[i].num_steps ; ++s)
				if (mused.song.pattern[i].step[s].instrument != MUS_NOTE_NO_INSTRUMENT)
					n_inst = my_max(n_inst, mused.song.pattern[i].step[s].instrument + 1);
		
		mused.song.num_patterns = maxpat + 1;
		
		kill_unused_things = true;
	}

	SDL_RWwrite(f, MUS_SONG_SIG, strlen(MUS_SONG_SIG), sizeof(MUS_SONG_SIG[0]));
	
	const Uint8 version = MUS_VERSION;
	
	mused.song.time_signature = mused.time_signature;
	mused.song.sequence_step = mused.sequenceview_steps;
	
	SDL_RWwrite(f, &version, 1, sizeof(version));
	
	SDL_RWwrite(f, &mused.song.num_channels, 1, sizeof(mused.song.num_channels));
	Uint16 temp16 = mused.song.time_signature;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.time_signature));
	temp16 = mused.song.sequence_step;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.sequence_step));
	SDL_RWwrite(f, &n_inst, 1, sizeof(mused.song.num_instruments));
	temp16 = mused.song.num_patterns;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.num_patterns));
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		temp16 = mused.song.num_sequences[i];
		FIX_ENDIAN(temp16);
		SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.num_sequences[i]));
	}
	temp16 = mused.song.song_length;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.song_length));
	temp16 = mused.song.loop_point;
	FIX_ENDIAN(temp16);
	SDL_RWwrite(f, &temp16, 1, sizeof(mused.song.loop_point));
	SDL_RWwrite(f, &mused.song.master_volume, 1, 1);
	SDL_RWwrite(f, &mused.song.song_speed, 1, sizeof(mused.song.song_speed));
	SDL_RWwrite(f, &mused.song.song_speed2, 1, sizeof(mused.song.song_speed2));
	SDL_RWwrite(f, &mused.song.song_rate, 1, sizeof(mused.song.song_rate));
	Uint32 temp32 = mused.song.flags;
	FIX_ENDIAN(temp32);
	SDL_RWwrite(f, &temp32, 1, sizeof(mused.song.flags));
	SDL_RWwrite(f, &mused.song.multiplex_period, 1, sizeof(mused.song.multiplex_period));
	SDL_RWwrite(f, &mused.song.pitch_inaccuracy, 1, sizeof(mused.song.pitch_inaccuracy));
	
	Uint8 len = strlen(mused.song.title);
	SDL_RWwrite(f, &len, 1, 1);
	if (len)
		SDL_RWwrite(f, mused.song.title, 1, len);
	
	if (stats)
		stats->size[STATS_HEADER] = SDL_RWtell(f);
	
	Uint8 n_fx = kill_unused_things ? 0 : CYD_MAX_FX_CHANNELS;
	
	if (kill_unused_things)
	{
		for (int i = 0 ; i < n_inst ; ++i)
			if (mused.song.instrument[i].cydflags & CYD_CHN_ENABLE_FX) n_fx = my_max(n_fx, mused.song.instrument[i].fx_bus + 1);
	}	
	
	SDL_RWwrite(f, &n_fx, 1, sizeof(n_fx));
	
	debug("Saving %d fx", n_fx);
	for (int fx = 0 ; fx < n_fx ; ++fx)
	{
		CydFxSerialized temp;
		memcpy(&temp, &mused.song.fx[fx], sizeof(temp));
		
		FIX_ENDIAN(temp.flags);
		for (int i = 0 ; i < CYDRVB_TAPS ; ++i)	
		{
			FIX_ENDIAN(temp.rvb.tap[i].gain);
			FIX_ENDIAN(temp.rvb.tap[i].delay);
		}
		
		Uint8 len = strlen(temp.name);
		SDL_RWwrite(f, &len, sizeof(len), 1);
		if (len)
			SDL_RWwrite(f, temp.name, sizeof(temp.name[0]), len);
		
		SDL_RWwrite(f, &temp.flags, sizeof(temp.flags), 1);
		SDL_RWwrite(f, &temp.crush.bit_drop, sizeof(temp.crush.bit_drop), 1);
		SDL_RWwrite(f, &temp.chr.rate, sizeof(temp.chr.rate), 1);
		SDL_RWwrite(f, &temp.chr.min_delay, sizeof(temp.chr.min_delay), 1);
		SDL_RWwrite(f, &temp.chr.max_delay, sizeof(temp.chr.max_delay), 1);
		SDL_RWwrite(f, &temp.chr.sep, sizeof(temp.chr.sep), 1);
		
		SDL_RWwrite(f, &temp.rvb.spread, sizeof(temp.rvb.spread), 1);
		
		for (int i = 0 ; i < CYDRVB_TAPS ; ++i)	
		{
			SDL_RWwrite(f, &temp.rvb.tap[i].delay, sizeof(temp.rvb.tap[i].delay), 1);
			SDL_RWwrite(f, &temp.rvb.tap[i].gain, sizeof(temp.rvb.tap[i].gain), 1);
		}

		SDL_RWwrite(f, &temp.crushex.downsample, sizeof(temp.crushex.downsample), 1); 
		SDL_RWwrite(f, &temp.crushex.gain, sizeof(temp.crushex.gain), 1);
	}
	
	if (stats)
		stats->size[STATS_FX] = SDL_RWtell(f);
	
	SDL_RWwrite(f, &mused.song.default_volume[0], sizeof(mused.song.default_volume[0]), mused.song.num_channels);
	SDL_RWwrite(f, &mused.song.default_panning[0], sizeof(mused.song.default_panning[0]), mused.song.num_channels);
	
	if (stats)
		stats->size[STATS_DEFVOLPAN] = SDL_RWtell(f);
	
	int max_wt = kill_unused_things ? 0 : CYD_WAVE_MAX_ENTRIES;
	
	debug("Saving %d instruments", n_inst);
	for (int i = 0 ; i < n_inst ; ++i)
	{
		save_instrument_inner(f, &mused.song.instrument[i], NULL, NULL);
		
		if (kill_unused_things)
		{
			if (mused.song.instrument[i].cydflags & CYD_CHN_ENABLE_WAVE)
				max_wt = my_max(max_wt, mused.song.instrument[i].wavetable_entry + 1);
		}
	}
	
	if (stats)
		stats->size[STATS_INSTRUMENTS] = SDL_RWtell(f);
	
	bool *used_pattern = calloc(sizeof(bool), mused.song.num_patterns);
	
	for (int i = 0 ; i < mused.song.num_channels; ++i)
	{
		for (int s= 0 ; s < mused.song.num_sequences[i] ; ++s)
		{
			temp16 = mused.song.sequence[i][s].position;
			FIX_ENDIAN(temp16);
			SDL_RWwrite(f, &temp16, 1, sizeof(temp16));
			
			used_pattern[mused.song.sequence[i][s].pattern] = true;
			
			temp16 = mused.song.sequence[i][s].pattern;
			FIX_ENDIAN(temp16);
			SDL_RWwrite(f, &temp16, 1, sizeof(temp16));
			SDL_RWwrite(f, &mused.song.sequence[i][s].note_offset, 1, sizeof(mused.song.sequence[i][s].note_offset));
		}
	}
	
	if (stats)
		stats->size[STATS_SEQUENCE] = SDL_RWtell(f);
	
	for (int i = 0 ; i < mused.song.num_patterns; ++i)
	{
		write_packed_pattern(f, &mused.song.pattern[i], !used_pattern[i]);
	}
	
	if (stats)
		stats->size[STATS_PATTERNS] = SDL_RWtell(f);
	
	free(used_pattern);
	
	FIX_ENDIAN(max_wt);
	
	SDL_RWwrite(f, &max_wt, 1, sizeof(Uint8));
	
	debug("Saving %d wavetable items", max_wt);
	for (int i = 0 ; i < max_wt ; ++i)
	{
		bool used = false;
		
		for (int ins = 0 ; ins < n_inst ; ++ins)
			if (mused.song.instrument[ins].wavetable_entry == i)
				used = true;
		
		write_wavetable_entry(f, &mused.mus.cyd->wavetable_entries[i], used);
	}
	
	if (stats)
		stats->size[STATS_WAVETABLE] = SDL_RWtell(f);
	
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.num_instruments = NUM_INSTRUMENTS;
	mused.modified = false;
	
	if (stats)
	{
		for (int i = N_STATS - 1 ; i > 0 ; --i)
		{
			stats->size[i] -= stats->size[i - 1];
		}
		
		stats->total_size = 0;
		
		for (int i = 0 ; i < N_STATS ; ++i)
		{
			stats->total_size += stats->size[i];
		}
	}
	
	return 1;
}


int open_wavetable(FILE *f)
{
	Wave *w = wave_load(f);
				
	if (w)
	{
		cyd_wave_entry_init(&mused.mus.cyd->wavetable_entries[mused.selected_wavetable], w->data, w->length, w->bits_per_sample == 16 ? CYD_WAVE_TYPE_SINT16 : CYD_WAVE_TYPE_SINT8, w->channels, 1, 1);
		
		mused.mus.cyd->wavetable_entries[mused.selected_wavetable].flags = 0;
		mused.mus.cyd->wavetable_entries[mused.selected_wavetable].sample_rate = w->sample_rate;
		mused.mus.cyd->wavetable_entries[mused.selected_wavetable].base_note = MIDDLE_C << 8;
		
		wave_destroy(w);
		
		invalidate_wavetable_view();
		
		return 1;
	}
	
	return 0;
}


static int open_wavetable_raw_inner(FILE *f, int t)
{
	size_t pos = ftell(f);
	fseek(f, 0, SEEK_END);
	size_t s = ftell(f) - pos;
	fseek(f, pos, SEEK_SET);
	
	if (s > 0)
	{
		Sint8 *w = calloc(s, sizeof(Sint8));
					
		if (w)
		{
			fread(w, 1, s, f);
		
			cyd_wave_entry_init(&mused.mus.cyd->wavetable_entries[mused.selected_wavetable], w, s, t, 1, 1, 1);
			
			mused.mus.cyd->wavetable_entries[mused.selected_wavetable].flags = 0;
			mused.mus.cyd->wavetable_entries[mused.selected_wavetable].sample_rate = 8000;
			mused.mus.cyd->wavetable_entries[mused.selected_wavetable].base_note = MIDDLE_C << 8;
			
			free(w);
			
			invalidate_wavetable_view();
			
			return 1;
		}
	}
	
	return 0;
}


int open_wavetable_raw(FILE *f)
{
	return open_wavetable_raw_inner(f, CYD_WAVE_TYPE_SINT8);
}


int open_wavetable_raw_u(FILE *f)
{
	return open_wavetable_raw_inner(f, CYD_WAVE_TYPE_UINT8);
}


int open_instrument(FILE *f)
{
	if (!mus_load_instrument_file2(f, &mused.song.instrument[mused.current_instrument], mused.mus.cyd->wavetable_entries)) return 0;
	
	mused.modified = true;
	
	invalidate_wavetable_view();
	
	return 1;
}


int save_song(SDL_RWops *ops)
{
	int r = save_song_inner(ops, NULL);
	
	return r;
}


void open_data(void *type, void *action, void *_ret)
{
	int t = CASTPTR(int, type);
	int a = CASTPTR(int, action);
	int *ret = _ret;
	
	if (a == OD_A_OPEN && t == OD_T_SONG)
	{
		int r = -1;
		if (mused.modified) r = confirm_ync(domain, mused.slider_bevel, &mused.largefont, "Save song?");
		int ret_val;
				
		if (r == 0) 
		{
			if (ret) *ret = 0;
			return;
		}
		
		if (r == 1) 
		{ 
			stop(0,0,0); // so loop positions set by pattern loop mode will be restored
			open_data(MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_SAVE), &ret_val); 
			if (!ret_val) 
			{
				if (ret) *ret = 0;
				return;
			}
		}
	}
	
	const struct 
	{
		const char *name, *ext;
		int (*open)(FILE *);
		int (*save)(SDL_RWops *);
	} open_stuff[] = {
		{ "song", "kt", open_song, save_song },
		{ "instrument", "ki", open_instrument, save_instrument },
		{ "wave", "wav", open_wavetable, NULL },
		{ "raw signed", "", open_wavetable_raw, NULL },
		{ "raw unsigned", "", open_wavetable_raw_u, NULL }
	};
	
	const char *mode[] = { "rb", "wb" };
	const char *modename[] = { "Open", "Save" };
	char str[1000];
	snprintf(str, sizeof(str), "%s %s", modename[a], open_stuff[t].name);
	
	stop(0,0,0);
	
	char _def[1024] = "";
	char *def = NULL;
	
	if (a == OD_A_SAVE)
	{
		switch (t)
		{
			case OD_T_INSTRUMENT:
			{
				strncpy(_def, mused.song.instrument[mused.current_instrument].name, sizeof(_def)-4);
				strcat(_def, ".ki");
			}
			break;
			
			case OD_T_SONG:
			{
				strncpy(_def, mused.song.title, sizeof(_def)-4);
				strcat(_def, ".kt");
			}
			break;
		}
		
		def = _def;
	}
	
	FILE * f = open_dialog(mode[a], str, open_stuff[t].ext, domain, mused.slider_bevel, &mused.largefont, &mused.smallfont, def);
	SDL_RWops *rw = NULL;
	
	if (f)
	{
		int return_val = 1;
		void * tmp;
		
		if (a == 0)
			tmp = open_stuff[t].open;
		else
			tmp = open_stuff[t].save;
	
		if (tmp)
		{
			cyd_lock(&mused.cyd, 1);
			int r;
			if (a == 0)
				r = open_stuff[t].open(f);
			else
			{	
				rw = create_memwriter(f);
				r = open_stuff[t].save(rw);
			}
			
			cyd_lock(&mused.cyd, 0);
			
			if (!r)
			{
				snprintf(str, sizeof(str), "Could not open %s!", open_stuff[t].name);
				msgbox(domain, mused.slider_bevel, &mused.largefont, str, MB_OK);
				
				return_val = 0;
			}
			else if (a == OD_A_OPEN && t != OD_T_SONG)
				mused.modified = true;
		}	
		
		if (rw)
			SDL_RWclose(rw);
		
		fclose(f);
		
		if (ret) *ret = return_val;
	}
	else
		if (ret) *ret = 0;
	
	change_mode(mused.mode);
}

