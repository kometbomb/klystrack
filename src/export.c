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
#include <string.h>
#include "wavewriter.h"

extern GfxDomain *domain;

bool export_wav(MusSong *song, CydWavetableEntry * entry, FILE *f, int channel)
{
	bool success = false;
	
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
	
	if (channel >= 0)
	{
		// if channel is positive then only export that channel (mute other chans)
		
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			mus.channel[i].flags |= MUS_CHN_DISABLED;
		
		mus.channel[channel].flags &= ~MUS_CHN_DISABLED;
	}
	else
	{
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			mus.channel[i].flags &= ~MUS_CHN_DISABLED;
	}
	
	const int channels = 2;
	Sint16 buffer[2000 * channels];
	
	int last_percentage = -1;
	
	WaveWriter *ww = ww_create(f, cyd.sample_rate, 2);
	
	for (;;)
	{
		memset(buffer, 0, sizeof(buffer)); // Zero the input to cyd
		cyd_output_buffer_stereo(&cyd, (Uint8*)buffer, sizeof(buffer));
		
		if (cyd.samples_output > 0)
			ww_write(ww, buffer, cyd.samples_output);
		
		if (mus.song_position >= song->song_length) break;
		
		if (song->song_length != 0)
		{
			int percentage = (mus.song_position + (channel == -1 ? 0 : (channel * song->song_length))) * 100 / (song->song_length * (channel == -1 ? 1 : song->num_channels));
			
			if (percentage > last_percentage)
			{
				last_percentage = percentage;
				
				SDL_Rect area = {domain->screen_w / 2 - 140, domain->screen_h / 2 - 24, 280, 48};
				bevel(domain, &area, mused.slider_bevel, BEV_MENU);
				
				adjust_rect(&area, 8);
				area.h = 16;
				
				bevel(domain, &area, mused.slider_bevel, BEV_FIELD);
				
				adjust_rect(&area, 2);
				
				int t = area.w;
				area.w = area.w * percentage / 100;
				
				gfx_rect(domain, &area, colors[COLOR_PROGRESS_BAR]);
				
				area.y += 16 + 4 + 4;
				area.w = t;
				
				font_write_args(&mused.smallfont, domain, &area, "Exporting... Press ESC to abort.");
				
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
	
	success = true;
	
abort:;
	
	ww_finish(ww);
	
	cyd.wavetable_entries = prev_entry;
	
	cyd_deinit(&cyd);
	
	song->flags &= ~MUS_NO_REPEAT;
	
	return success;
}

