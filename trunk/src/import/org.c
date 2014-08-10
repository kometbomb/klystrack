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
	} __attribute__((__packed__)) header;
	
	if (fread(&header, 1, sizeof(header), f) != sizeof(header)) 
		return 0;
	
	if (strncmp("Org-02", header.sig, 6) != 0 && strncmp("Org-03", header.sig, 6) != 0)
	{
		fatal("Not a Cave Story Organya song (sig: '%-6s')", header.sig);
		return 0;
	}
	
	FIX_ENDIAN(header.tempo);
	FIX_ENDIAN(header.loop_begin);
	FIX_ENDIAN(header.loop_end);
	
	mused.song.time_signature = (((Uint16)header.beats_per_step) << 8) | header.steps_per_bar;
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
	} __attribute__((__packed__)) instrument[16];
	
	if (fread(&instrument, 1, sizeof(instrument), f) != sizeof(instrument)) 
		return 0;
		
	int real_channels = 0;
	
	for (int i = 0 ; i < 16 ; ++i)
	{
		FIX_ENDIAN(instrument[i].pitch);
		FIX_ENDIAN(instrument[i].n_notes);
		
		if (instrument[i].n_notes)
		{
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
				MusStep *step = &mused.song.pattern[i].step[position[n]];
				
				if (note[n] != 255)
				{
					step->note = note[n];
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
		
	return 1;
}
