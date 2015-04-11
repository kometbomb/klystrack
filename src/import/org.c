/*
Copyright (c) 2009-2011 Tero Lindeman (kometbomb)

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

#include "org.h"
#include "edit.h"
#include "mused.h"
#include "event.h"
#include "SDL_endian.h"
#include "snd/freqs.h"
#include <assert.h>
#include <string.h>

extern Mused mused;

typedef struct
{
	int n_samples;
	Uint32 sample_length;
	Sint8 *data;
	int n_drums;
	int drum_rate;
	struct {
		Uint32 length;
		Sint8 *data;
	} *drum;
} orgsamp_t;

int load_orgsamp(orgsamp_t *data)
{
	FILE *f = fopen("orgsamp.dat", "rb");
	
	memset(data, 0, sizeof(*data));
	
	if (f)
	{
		debug("Reading orgsamp.dat");
		
		Uint8 temp8 = 0;
		Uint32 temp32 = 0;
		Uint16 temp16 = 0;
		
		fread(&temp8, 1, sizeof(temp8), f);
		data->n_samples = temp8;
		
		fread(&temp8, 1, 1, f);
		temp32 = temp8 << 16;
		fread(&temp8, 1, 1, f);
		temp32 |= temp8 << 8;
		fread(&temp8, 1, 1, f);
		temp32 |= temp8;

		data->sample_length = temp32;
		
		data->data = malloc(data->sample_length * data->n_samples);
		
		fread(data->data, 1, data->sample_length * data->n_samples, f);
		
		fread(&temp8, 1, sizeof(temp8), f);
		
		data->n_drums = temp8;
		data->drum = malloc(sizeof(data->drum[0]) * data->n_drums);
		
		fread(&temp16, 1, sizeof(temp16), f);
		
		data->drum_rate = SDL_SwapBE16(temp16);
		
		for (int i = 0 ; i < data->n_drums ; ++i)
		{
			fread(&temp8, 1, 1, f);
			temp32 = temp8 << 16;
			fread(&temp8, 1, 1, f);
			temp32 |= temp8 << 8;
			fread(&temp8, 1, 1, f);
			temp32 |= temp8;
			
			data->drum[i].length = temp32;
			data->drum[i].data = malloc(temp32);
			
			fread(data->drum[i].data, 1, data->drum[i].length, f);
		}
		
		fclose(f);
		
		return 1;
	}
	
	return 0;
}

void unload_orgsamp(orgsamp_t *data)
{
	for (int i = 0 ; i < data->n_drums ; ++i)
		free(data->drum[i].data);
	
	free(data->drum);
	free(data->data);
}

int import_org(FILE *f)
{
	struct 
	{
		char sig[6]; // "Org-02"
		Uint16 tempo;
		Uint8 steps_per_bar;
		Uint8 beats_per_step;
		Uint32 loop_begin;
		Uint32 loop_end;
	} header;
	
	fread(&header.sig, 1, sizeof(header.sig), f);
	fread(&header.tempo, 1, sizeof(header.tempo), f);
	fread(&header.steps_per_bar, 1, sizeof(header.steps_per_bar), f);
	fread(&header.beats_per_step, 1, sizeof(header.beats_per_step), f);
	fread(&header.loop_begin, 1, sizeof(header.loop_begin), f);
	fread(&header.loop_end, 1, sizeof(header.loop_end), f);
	
	if (strncmp("Org-02", header.sig, 6) != 0 && strncmp("Org-03", header.sig, 6) != 0)
	{
		fatal("Not a Cave Story Organya song (sig: '%-6s')", header.sig);
		return 0;
	}
	
	FIX_ENDIAN(header.tempo);
	FIX_ENDIAN(header.loop_begin);
	FIX_ENDIAN(header.loop_end);
	
	mused.song.time_signature = (((Uint16)header.beats_per_step) << 8) | header.steps_per_bar;
	mused.time_signature = mused.song.time_signature;
	mused.song.song_length = header.loop_end + header.steps_per_bar;
	mused.song.loop_point = header.loop_begin;
	if (header.tempo > 0) 
		mused.song.song_rate = my_min(255, (6 * 1000) / header.tempo);
	mused.sequenceview_steps = header.beats_per_step * header.steps_per_bar;
	mused.song.num_channels = 16;
	
	struct 
	{
		Uint16 pitch;
		Uint8 instrument;
		Uint8 pi;
		Uint16 n_notes;
	} instrument[16];
	
	for (int i = 0 ; i < 16 ; ++i)
	{
		fread(&instrument[i].pitch, 1, sizeof(instrument[i].pitch), f);
		fread(&instrument[i].instrument, 1, sizeof(instrument[i].instrument), f);
		fread(&instrument[i].pi, 1, sizeof(instrument[i].pi), f);
		fread(&instrument[i].n_notes, 1, sizeof(instrument[i].n_notes), f);
		
		FIX_ENDIAN(instrument[i].pitch);
		FIX_ENDIAN(instrument[i].n_notes);
	}
	
	orgsamp_t orgsamp;
	
	int orgsamp_loaded = load_orgsamp(&orgsamp);
		
	int real_channels = 0;
	
	for (int i = 0 ; i < 16 ; ++i)
	{
		if (instrument[i].n_notes)
		{
			mused.song.instrument[real_channels].cydflags = CYD_CHN_ENABLE_WAVE;
			mused.song.instrument[real_channels].adsr.a = 0;
			mused.song.instrument[real_channels].adsr.d = 0x1f;
			mused.song.instrument[real_channels].adsr.s = 0x1f;
			mused.song.instrument[real_channels].adsr.r = 1;
			mused.song.instrument[real_channels].flags = 0;
			mused.song.instrument[real_channels].wavetable_entry = real_channels;
			
			if (orgsamp_loaded)
			{
				CydWavetableEntry *e = &mused.mus.cyd->wavetable_entries[real_channels];
				
				if (i < 8)
				{
					
					cyd_wave_entry_init(e, &orgsamp.data[instrument[i].instrument * orgsamp.sample_length], orgsamp.sample_length, CYD_WAVE_TYPE_SINT8, 1, 1, 1);
					e->flags |= CYD_WAVE_LOOP;
					e->loop_end = orgsamp.sample_length;
					e->sample_rate = (56320 * orgsamp.sample_length / 256);
					e->base_note = (MIDDLE_C - 3) * 256;
					
					sprintf(mused.song.instrument[real_channels].name, "Wave-%02d", instrument[i].instrument);
				}
				else
				{
					cyd_wave_entry_init(&mused.mus.cyd->wavetable_entries[real_channels], orgsamp.drum[instrument[i].instrument].data, orgsamp.drum[instrument[i].instrument].length, CYD_WAVE_TYPE_SINT8, 1, 1, 1);
					e->sample_rate = orgsamp.drum_rate * 20;
					e->base_note = (MIDDLE_C - 3) * 256;
					sprintf(mused.song.instrument[real_channels].name, "Drum-%02d", instrument[i].instrument);
				}
			}
			
			Uint32 *position = calloc(sizeof(Uint32), instrument[i].n_notes);
			Uint8 *note = calloc(sizeof(Uint8), instrument[i].n_notes), 
				*length = calloc(sizeof(Uint8), instrument[i].n_notes), 
				*volume = calloc(sizeof(Uint8), instrument[i].n_notes), 
				*panning = calloc(sizeof(Uint8), instrument[i].n_notes);
			
			fread(position, sizeof(Uint32), instrument[i].n_notes, f);
			fread(note, sizeof(Uint8), instrument[i].n_notes, f);
			fread(length, sizeof(Uint8), instrument[i].n_notes, f);
			fread(volume, sizeof(Uint8), instrument[i].n_notes, f);
			fread(panning, sizeof(Uint8), instrument[i].n_notes, f);
			
			resize_pattern(&mused.song.pattern[i], header.loop_end);
			add_sequence(real_channels, 0, i, 0);
			
			Uint8 prev_vol = 0, prev_pan = CYD_PAN_CENTER;
			
			for (int n = 0 ; n < instrument[i].n_notes ; ++n)
			{	
				if (position[n] >= header.loop_end)
					continue;
				
				MusStep *step = &mused.song.pattern[i].step[position[n]];
				
				if (note[n] != 255)
				{
					step->note = note[n] + 12;
					step->instrument = real_channels;
				}
				
				if (volume[n] == 255)
					step->volume = prev_vol;
				else
					prev_vol = step->volume = (Uint16)volume[n] * 0x80 / 254;
				
				Uint8 pan = 0;
				
				if (panning[n] == 255)
				{
					pan = prev_pan;
				}
				else
				{
					pan = prev_pan = ((Uint16)panning[n]) * CYD_PAN_RIGHT / 0xc;
				}
				
				if (pan != CYD_PAN_CENTER)
					step->command = MUS_FX_SET_PANNING | pan;
				
				if (note[n] != 255 && length[n] && !instrument[i].pi && length[n] + position[n] < header.loop_end)
				{
					mused.song.pattern[i].step[length[n] + position[n]].note = MUS_NOTE_RELEASE;
				}
			}
			
			free(position);
			free(note);
			free(length);
			free(panning);
			
			++real_channels;
		}
	}
	
	mused.song.num_channels = real_channels;
	
	if (orgsamp_loaded)
		unload_orgsamp(&orgsamp);
		
	return 1;
}
