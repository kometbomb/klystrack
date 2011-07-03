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

extern Mused mused;
extern GfxDomain *domain;


static void write_wavetable_entry(FILE *f, const CydWavetableEntry *write_wave, bool write_wave_data)
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
	
	_VER_WRITE(&flags, 0);
	_VER_WRITE(&sample_rate, 0);
	_VER_WRITE(&samples, 0);
	_VER_WRITE(&loop_begin, 0);
	_VER_WRITE(&loop_end, 0);
	_VER_WRITE(&base_note, 0);
	
	
	if (packed)
	{
		FIX_ENDIAN(packed_size);
		_VER_WRITE(&packed_size, 0);
		
		fwrite(packed, sizeof(packed[0]), (packed_size + 7) / 8, f);
		
		free(packed);
	}
}

	
static void save_instrument_inner(FILE *f, MusInstrument *inst, const CydWavetableEntry *write_wave)
{
	Uint32 temp32 = inst->flags;
	FIX_ENDIAN(temp32);
	_VER_WRITE(&temp32, 0);
	temp32 = inst->cydflags;
	FIX_ENDIAN(temp32);
	_VER_WRITE(&temp32, 0);
	_VER_WRITE(&inst->adsr, 0);
	_VER_WRITE(&inst->sync_source, 0);
	_VER_WRITE(&inst->ring_mod, 0); 
	Uint16 temp16 = inst->pw;
	FIX_ENDIAN(temp16);
	_VER_WRITE(&temp16, 0);
	_VER_WRITE(&inst->volume, 0);
	Uint8 progsteps = 0;
	for (int i = 0 ; i < MUS_PROG_LEN ; ++i)
		if (inst->program[i] != MUS_FX_NOP) progsteps = i+1;
	_VER_WRITE(&progsteps, 0);
	for (int i = 0 ; i < progsteps ; ++i)
	{
		temp16 = inst->program[i];
		FIX_ENDIAN(temp16);
		_VER_WRITE(&temp16, sizeof(inst->program[i]));
	}
	_VER_WRITE(&inst->prog_period, 0); 
	_VER_WRITE(&inst->vibrato_speed, 0); 
	_VER_WRITE(&inst->vibrato_depth, 0); 
	_VER_WRITE(&inst->pwm_speed, 0); 
	_VER_WRITE(&inst->pwm_depth, 0); 
	_VER_WRITE(&inst->slide_speed, 0);
	_VER_WRITE(&inst->base_note, 0);
	Uint8 len = strlen(inst->name);
	_VER_WRITE(&len, 0);
	if (len)
		_VER_WRITE(inst->name, len);
	temp16 = inst->cutoff;
	FIX_ENDIAN(temp16);
	_VER_WRITE(&temp16, 0);
	_VER_WRITE(&inst->resonance, 0);
	_VER_WRITE(&inst->flttype, 0);
	_VER_WRITE(&inst->ym_env_shape, 0);
	temp16 = inst->buzz_offset;
	FIX_ENDIAN(temp16);
	_VER_WRITE(&temp16, 0);
	_VER_WRITE(&inst->fx_bus, 0);
	_VER_WRITE(&inst->vib_shape, 0);
	_VER_WRITE(&inst->vib_delay, 0);
	_VER_WRITE(&inst->pwm_shape, 0);
		
	if (write_wave)
	{
		Uint8 temp = 0xff;
		_VER_WRITE(&temp, 0);
		write_wavetable_entry(f, write_wave, true);
	}
	else
	{
		_VER_WRITE(&inst->wavetable_entry, 0);
	}
}


static void write_packed_pattern(FILE *f, const MusPattern *pattern)
{
	/* 
	
	Format: 
		00 		1 * byte			number of incoming steps
		01...	n_steps * nibble	incoming data
		...		n_steps * variable	data described by bits in nibble 
		
		If ctrl bit 7 is set, there's also a volume column incoming
	*/

	Uint16 steps = pattern->num_steps;
	FIX_ENDIAN(steps);
	fwrite(&steps, 1, sizeof(steps), f);
	
	Uint8 buffer = 0;
	for (int i = 0 ; i < pattern->num_steps ; ++i)
	{
		if (pattern->step[i].note != MUS_NOTE_NONE)
			buffer |= MUS_PAK_BIT_NOTE;
			
		if (pattern->step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			buffer |= MUS_PAK_BIT_INST;
			
		if (pattern->step[i].ctrl != 0 || pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
			buffer |= MUS_PAK_BIT_CTRL;
			
		if (pattern->step[i].command != 0)
			buffer |= MUS_PAK_BIT_CMD;
			
		if (i & 1 || i + 1 >= pattern->num_steps)
			fwrite(&buffer, 1, sizeof(buffer), f);
			
		buffer <<= 4;
	}
	
	for (int i = 0 ; i < pattern->num_steps ; ++i)
	{
		if (pattern->step[i].note != MUS_NOTE_NONE)
			fwrite(&pattern->step[i].note, 1, sizeof(pattern->step[i].note), f);
			
		if (pattern->step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			fwrite(&pattern->step[i].instrument, 1, sizeof(pattern->step[i].instrument), f);
			
		if (pattern->step[i].ctrl != 0 || pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
		{
			Uint8 ctrl = pattern->step[i].ctrl;
			if (pattern->step[i].volume != MUS_NOTE_NO_VOLUME) 
				ctrl |= MUS_PAK_BIT_VOLUME;
			fwrite(&ctrl, 1, sizeof(pattern->step[i].ctrl), f);
		}
			
		if (pattern->step[i].command != 0)
		{
			Uint16 c = pattern->step[i].command;
			FIX_ENDIAN(c);
			fwrite(&c, 1, sizeof(pattern->step[i].command), f);
		}
		
		if (pattern->step[i].volume != MUS_NOTE_NO_VOLUME)
			fwrite(&pattern->step[i].volume, 1, sizeof(pattern->step[i].volume), f);
	}
}


int open_song(FILE *f)
{
	new_song();

	if (!mus_load_song_file(f, &mused.song, mused.mus.cyd->wavetable_entries)) return 0;
	
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.num_instruments = NUM_INSTRUMENTS;
	
	// Use kewlkool heuristics to determine sequence spacing
	
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
	
	mus_set_fx(&mused.mus, &mused.song);
	cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
	mirror_flags();
	
	if (!mused.song.time_signature) mused.song.time_signature = 0x404;
	
	mused.time_signature = mused.song.time_signature;
	
	mused.flags &= ~EDIT_MODE;
	
	return 1;
}


int save_instrument(FILE *f)
{
	const Uint8 version = MUS_VERSION;
				
	fwrite(MUS_INST_SIG, strlen(MUS_INST_SIG), sizeof(MUS_INST_SIG[0]), f);
	
	fwrite(&version, 1, sizeof(version), f);
	
	save_instrument_inner(f, &mused.song.instrument[mused.current_instrument], &mused.mus.cyd->wavetable_entries[mused.song.instrument[mused.current_instrument].wavetable_entry]);
	
	return 1;
}


int save_song(FILE *f)
{
	bool kill_unused_things = false;

	Uint8 n_inst = mused.song.num_instruments;
	if (!confirm(domain, mused.slider_bevel->surface, &mused.largefont, "Save unused song elements?"))
	{
		int maxpat = -1;
		for (int c = 0 ; c < mused.song.num_channels ; ++c)
		{
			for (int i = 0 ; i < mused.song.num_sequences[c] ; ++i)
				 if (maxpat < mused.song.sequence[c][i].pattern)
					maxpat = mused.song.sequence[c][i].pattern;
		}
		
		n_inst = 0;
		
		for (int i = 0 ; i < maxpat ; ++i)
			for (int s = 0 ; s < mused.song.pattern[i].num_steps ; ++s)
				if (mused.song.pattern[i].step[s].instrument != MUS_NOTE_NO_INSTRUMENT)
					n_inst = my_max(n_inst, mused.song.pattern[i].step[s].instrument + 1);
		
		mused.song.num_patterns = maxpat + 1;
		
		kill_unused_things = true;
	}

	fwrite(MUS_SONG_SIG, strlen(MUS_SONG_SIG), sizeof(MUS_SONG_SIG[0]), f);
	
	const Uint8 version = MUS_VERSION;
	
	mused.song.time_signature = mused.time_signature;
	
	fwrite(&version, 1, sizeof(version), f);
	
	fwrite(&mused.song.num_channels, 1, sizeof(mused.song.num_channels), f);
	Uint16 temp16 = mused.song.time_signature;
	FIX_ENDIAN(temp16);
	fwrite(&temp16, 1, sizeof(mused.song.time_signature), f);
	fwrite(&n_inst, 1, sizeof(mused.song.num_instruments), f);
	temp16 = mused.song.num_patterns;
	FIX_ENDIAN(temp16);
	fwrite(&temp16, 1, sizeof(mused.song.num_patterns), f);
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		temp16 = mused.song.num_sequences[i];
		FIX_ENDIAN(temp16);
		fwrite(&temp16, 1, sizeof(mused.song.num_sequences[i]), f);
	}
	temp16 = mused.song.song_length;
	FIX_ENDIAN(temp16);
	fwrite(&temp16, 1, sizeof(mused.song.song_length), f);
	temp16 = mused.song.loop_point;
	FIX_ENDIAN(temp16);
	fwrite(&temp16, 1, sizeof(mused.song.loop_point), f);
	fwrite(&mused.song.master_volume, 1, 1, f);
	fwrite(&mused.song.song_speed, 1, sizeof(mused.song.song_speed), f);
	fwrite(&mused.song.song_speed2, 1, sizeof(mused.song.song_speed2), f);
	fwrite(&mused.song.song_rate, 1, sizeof(mused.song.song_rate), f);
	Uint32 temp32 = mused.song.flags;
	FIX_ENDIAN(temp32);
	fwrite(&temp32, 1, sizeof(mused.song.flags), f);
	fwrite(&mused.song.multiplex_period, 1, sizeof(mused.song.multiplex_period), f);
	fwrite(&mused.song.pitch_inaccuracy, 1, sizeof(mused.song.pitch_inaccuracy), f);
	
	Uint8 len = strlen(mused.song.title);
	fwrite(&len, 1, 1, f);
	if (len)
		fwrite(mused.song.title, 1, len, f);
	
	Uint8 n_fx = CYD_MAX_FX_CHANNELS;
	
	if (kill_unused_things)
	{
		for (int i = 0 ; i < n_inst ; ++i)
			if (mused.song.instrument[i].cydflags & CYD_CHN_ENABLE_FX) n_fx = my_max(n_fx, mused.song.instrument[i].fx_bus + 1);
	}	
	
	fwrite(&n_fx, 1, sizeof(n_fx), f);
	
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
		
		fwrite(&temp, 1, sizeof(temp), f);
	}
	
	fwrite(&mused.song.default_volume[0], sizeof(mused.song.default_volume[0]), mused.song.num_channels, f);
	fwrite(&mused.song.default_panning[0], sizeof(mused.song.default_panning[0]), mused.song.num_channels, f);
	
	int max_wt = CYD_WAVE_MAX_ENTRIES;
	
	for (int i = 0 ; i < n_inst ; ++i)
	{
		save_instrument_inner(f, &mused.song.instrument[i], NULL);
		
		if (kill_unused_things)
		{
			if (mused.song.instrument[i].cydflags & CYD_CHN_ENABLE_WAVE)
				max_wt = my_max(max_wt, mused.song.instrument[i].wavetable_entry + 1);
		}
	}
	
	for (int i = 0 ; i < mused.song.num_channels; ++i)
	{
		for (int s= 0 ; s < mused.song.num_sequences[i] ; ++s)
		{
			temp16 = mused.song.sequence[i][s].position;
			FIX_ENDIAN(temp16);
			fwrite(&temp16, 1, sizeof(temp16), f);
			temp16 = mused.song.sequence[i][s].pattern;
			FIX_ENDIAN(temp16);
			fwrite(&temp16, 1, sizeof(temp16), f);
			fwrite(&mused.song.sequence[i][s].note_offset, 1, sizeof(mused.song.sequence[i][s].note_offset), f);
		}
	}
	
	for (int i = 0 ; i < mused.song.num_patterns; ++i)
	{
		write_packed_pattern(f, &mused.song.pattern[i]);
	}
	
	FIX_ENDIAN(max_wt);
	
	fwrite(&max_wt, 1, sizeof(Uint8), f);
	
	for (int i = 0 ; i < max_wt ; ++i)
	{
		bool used = false;
		
		for (int ins = 0 ; ins < n_inst ; ++ins)
			if (mused.song.instrument[ins].wavetable_entry == i)
				used = true;
		
		write_wavetable_entry(f, &mused.mus.cyd->wavetable_entries[i], used);
	}
	
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.num_instruments = NUM_INSTRUMENTS;
	mused.modified = false;
	
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


void open_data(void *type, void *action, void *_ret)
{
	int t = CASTPTR(int, type);
	int a = CASTPTR(int, action);
	int *ret = _ret;
	
	if (a == OD_A_OPEN && t == OD_T_SONG)
	{
		int r = -1;
		if (mused.modified) r = confirm_ync(domain, mused.slider_bevel->surface, &mused.largefont, "Save song?");
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
		int (*open[2])(FILE *);
	} open_stuff[] = {
		{ "song", "kt", { open_song, save_song } },
		{ "instrument", "ki", { open_instrument, save_instrument } },
		{ "wave", "wav", {open_wavetable, NULL } },
		{ "raw signed", "", {open_wavetable_raw, NULL} },
		{ "raw unsigned", "", {open_wavetable_raw_u, NULL} }
	};
	
	const char *mode[] = { "rb", "wb" };
	const char *modename[] = { "Open", "Save" };
	char str[1000];
	snprintf(str, sizeof(str), "%s %s", modename[a], open_stuff[t].name);
	
	stop(0,0,0);
	
	const char *def = NULL;
	
	if (a == OD_A_SAVE)
	{
		switch (t)
		{
			case OD_T_INSTRUMENT:
			{
				def = mused.song.instrument[mused.current_instrument].name;
			}
			break;
			
			case OD_T_SONG:
			{
				def = mused.song.title;
			}
			break;
		}
	}
	
	FILE * f = open_dialog(mode[a], str, open_stuff[t].ext, domain, mused.slider_bevel->surface, &mused.largefont, &mused.smallfont, def);
	
	if (f)
	{
		int return_val = 1;
	
		if (open_stuff[t].open[a])
		{
			cyd_lock(&mused.cyd, 1);
			int r = open_stuff[t].open[a](f);
			cyd_lock(&mused.cyd, 0);
			
			if (!r)
			{
				snprintf(str, sizeof(str), "Could not open %s!", open_stuff[t].name);
				msgbox(domain, mused.slider_bevel->surface, &mused.largefont, str, MB_OK);
				
				return_val = 0;
			}
			else if (a == OD_A_OPEN && t != OD_T_SONG)
				mused.modified = true;
		}	
		fclose(f);
		
		if (ret) *ret = return_val;
	}
	else
		if (ret) *ret = 0;
	
	change_mode(mused.mode);
}

