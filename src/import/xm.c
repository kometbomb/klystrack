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

#include "mod.h"
#include "edit.h"
#include "mused.h"
#include "event.h"
#include "SDL_endian.h"
#include "snd/freqs.h"
#include <assert.h>

extern Mused mused;



int import_xm(FILE *f)
{
	struct 
	{
		char sig[17];
		char name[20];
		Uint8 _1a;
		char tracker_name[20];
		Uint16 version;
		Uint32 header_size;
		Uint16 song_length;
		Uint16 restart_position;
		Uint16 num_channels;
		Uint16 num_instruments;
		Uint16 num_patterns;
		Uint16 flags;
		Uint16 default_tempo;
		Uint16 default_bpm;
		Uint8 pattern_order[256];
	} __attribute__((__packed__)) header;
	
	if (fread(&header, 1, sizeof(header), f) != sizeof(header)) 
		return 0;
	
	if (strncmp("Extended Module: ", header.sig, 17) != 0)
	{
		fatal("Not a FastTracker II module (sig: '%-17s')", header.sig);
		return 0;
	}
	
	FIX_ENDIAN(header.version);
	FIX_ENDIAN(header.header_size);
	FIX_ENDIAN(header.song_length);
	FIX_ENDIAN(header.restart_position);
	FIX_ENDIAN(header.num_channels);
	FIX_ENDIAN(header.num_instruments);
	FIX_ENDIAN(header.num_patterns);
	FIX_ENDIAN(header.flags);
	FIX_ENDIAN(header.default_tempo);
	FIX_ENDIAN(header.default_bpm);
	
	/*if (header.version > 0x0104)
	{
		fatal("XM version 0x%x not supported", header.version);
		return 0;
	}*/

	int pattern_length[256];
	
	for (int p = 0 ; p < header.num_patterns ; ++p)
	{
		struct
		{
			Uint32 header_length;
			Uint8 packing_type;
			Uint16 num_rows;
			Uint16 data_size;
		} __attribute__((__packed__)) pattern_hdr;
	
		fread(&pattern_hdr, 1, sizeof(pattern_hdr), f);
		
		FIX_ENDIAN(pattern_hdr.data_size);
		FIX_ENDIAN(pattern_hdr.num_rows);
		FIX_ENDIAN(pattern_hdr.header_length);
		
		pattern_length[p] = pattern_hdr.num_rows;
		
		Uint8 data[256*32*5];
		
		fread(&data[0], 1, pattern_hdr.data_size, f);
		
		for (int c = 0 ; c < header.num_channels ; ++c)
		{
			int pat = p * header.num_channels + c;
			resize_pattern(&mused.song.pattern[pat], pattern_hdr.num_rows);
		}
		
		Uint8 *ptr = &data[0];
		
		for (int r = 0 ; r < pattern_hdr.num_rows ; ++r)
		{
			for (int c = 0 ; c < header.num_channels ; ++c)
			{
				Uint8 note = *ptr++;
				Uint8 instrument = 0, volume = 0, fx = 0, param = 0;
				
				if (note & 0x80)
				{
					Uint8 flags = note;
					note = 0;
					
					if (flags & 1)
						note = *ptr++;
					
					if (flags & 2)
						instrument = *ptr++;
						
					if (flags & 4)
						volume = *ptr++;
						
					if (flags & 8)
						fx = *ptr++;
					
					if (flags & 16)
						param = *ptr++;
				}
				else
				{
					instrument = *ptr++;
					volume = *ptr++;
					fx = *ptr++;
					param = *ptr++;
				}
				
				int pat = p * header.num_channels + c;
				MusStep *step = &mused.song.pattern[pat].step[r];
				step->ctrl = 0;
				
				if (note != 0 && note != 97)
					step->note = note - 1;
				else if (note == 97)
					step->note = MUS_NOTE_RELEASE;
				else
					step->note = MUS_NOTE_NONE;
				
				step->instrument = instrument != 0 ? instrument - 1 : MUS_NOTE_NO_INSTRUMENT;
				step->volume = MUS_NOTE_NO_VOLUME;
				
				if (volume >= 0x10 && volume <= 0x50)
					step->volume = (volume - 0x10) * 2;
				else if (volume >= 0x60 && volume <= 0x6f)
					step->volume = MUS_NOTE_VOLUME_FADE_DN | (volume & 0xf);
				else if (volume >= 0x70 && volume <= 0x7f)
					step->volume = MUS_NOTE_VOLUME_FADE_UP | (volume & 0xf);
				else if (volume >= 0xf0 && volume <= 0xff)
					step->ctrl = MUS_CTRL_SLIDE|MUS_CTRL_LEGATO;
				else if (volume >= 0xb0 && volume <= 0xbf)
					step->ctrl = MUS_CTRL_VIB;
					
				step->command = 0;
			}
		}
	}
	
	int pos = 0;
	
	for (int s = 0 ; s < header.song_length ; ++s)
	{
		for (int c = 0 ; c < header.num_channels ; ++c)
		{
			add_sequence(c, pos, header.pattern_order[s] * header.num_channels + c, 0);
		}
		
		pos += pattern_length[s];
	}
	
	return 1;
}
