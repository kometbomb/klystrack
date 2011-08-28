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

#include "export.h"
#include "gui/bevel.h"
#include "snd/cyd.h"
#include "macros.h"
#include "mused.h"
#include "gfx/gfx.h"
#include "gui/view.h"
#include "mybevdefs.h"
#include "gfx/font.h"
#include "theme.h"

extern GfxDomain *domain;

void export_wav(MusSong *song, CydWavetableEntry * entry, FILE *f)
{
	MusEngine mus;
	CydEngine cyd;
	
	cyd_init(&cyd, 44100, MUS_MAX_CHANNELS);
	cyd.flags |= CYD_SINGLE_THREAD;
	mus_init_engine(&mus, &cyd);
	mus.volume = song->master_volume;
	mus_set_fx(&mus, song);
	CydWavetableEntry * prev_entry = cyd.wavetable_entries; // save entries so they can be free'd
	cyd.wavetable_entries = entry;
	cyd_set_callback(&cyd, mus_advance_tick, &mus, song->song_rate);
	mus_set_song(&mus, song, 0);
	song->flags |= MUS_NO_REPEAT;
	
	const int channels = 2;
	Sint16 buffer[2000 * channels];
	
	Uint32 tmp = 0;
	
	fwrite("RIFF", 4, 1, f);
	
	Uint32 riffsize = ftell(f);
	
	FIX_ENDIAN(riffsize);
	
	fwrite(&tmp, 4, 1, f);
	fwrite("WAVE", 4, 1, f);
	fwrite("fmt ", 4, 1, f);
	
	tmp = 16;
	
	FIX_ENDIAN(tmp);
	
	fwrite(&tmp, 4, 1, f);
	
	tmp = 1;
	
	FIX_ENDIAN((*(Uint16*)&tmp));
	
	fwrite(&tmp, 2, 1, f);
	
	tmp = channels;
	
	FIX_ENDIAN((*(Uint16*)&tmp));
	
	fwrite(&channels, 2, 1, f);
	
	tmp = 44100;
	
	FIX_ENDIAN(tmp);
	
	fwrite(&tmp, 4, 1, f);
	
	tmp = 44100 * channels * sizeof(buffer[0]);
	
	FIX_ENDIAN(tmp);
	
	fwrite(&tmp, 4, 1, f);
	
	tmp = channels * sizeof(buffer[0]);
	
	FIX_ENDIAN((*(Uint16*)&tmp));
	
	fwrite(&tmp, 2, 1, f);
	
	tmp = 16;
	
	FIX_ENDIAN((*(Uint16*)&tmp));
	
	fwrite(&tmp, 2, 1, f);
	
	fwrite("data", 4, 1, f);
	
	size_t chunksize = ftell(f);
	
	tmp = 0;
	fwrite(&tmp, 4, 1, f);
	
	int last_percentage = -1;
	
	for (;;)
	{
		memset(buffer, 0, sizeof(buffer)); // Zero the input to cyd
		cyd_output_buffer_stereo(0, buffer, sizeof(buffer), &cyd);
		
		if (cyd.samples_output > 0)
			fwrite(buffer, cyd.samples_output * channels * sizeof(buffer[0]), 1, f);
		
		if (mus.song_position >= song->song_length) break;
		
		if (song->song_length != 0)
		{
			int percentage = mus.song_position * 100 / song->song_length;
			
			if (percentage > last_percentage)
			{
				last_percentage = percentage;
				
				SDL_Rect area = {domain->screen_w / 2 - 140, domain->screen_h / 2 - 24, 280, 48};
				bevel(gfx_domain_get_surface(domain), &area, mused.slider_bevel->surface, BEV_MENU);
				
				adjust_rect(&area, 8);
				area.h = 16;
				
				bevel(gfx_domain_get_surface(domain), &area, mused.slider_bevel->surface, BEV_FIELD);
				
				adjust_rect(&area, 2);
				
				int t = area.w;
				area.w = area.w * percentage / 100;
				
				SDL_FillRect(gfx_domain_get_surface(domain), &area, colors[COLOR_PROGRESS_BAR]);
				
				area.y += 16 + 4 + 4;
				area.w = t;
				
				font_write_args(&mused.smallfont, gfx_domain_get_surface(domain), &area, "Exporting... Press ESC to abort.");
				
				SDL_Event e;
				
				while (SDL_PollEvent(&e))
				{
					if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
					{
						goto abort;
					}
				}
				
				gfx_domain_flip(domain);
			}
		}
	}
	
abort:;
	
	Uint32 sz = ftell(f) - 8;
	
	FIX_ENDIAN(sz);
	
	fseek(f, riffsize, SEEK_SET);
	fwrite(&sz, sizeof(sz), 1, f);
	
	sz = sz + 8 - (chunksize + 4);
	
	fseek(f, chunksize, SEEK_SET);
	
	FIX_ENDIAN(sz);
	
	fwrite(&sz, sizeof(sz), 1, f);
	
	cyd.wavetable_entries = prev_entry;
	
	cyd_deinit(&cyd);
	
	song->flags &= ~MUS_NO_REPEAT;
}

