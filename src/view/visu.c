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

#include "visu.h"
#include "gui/bevel.h"
#include "mybevdefs.h"
#include "mused.h"
#include "snd/freqs.h"
#include "gfx/gfx.h"
#include "theme.h"

extern Uint32 colors[NUM_COLORS];

void spectrum_analyzer_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	
	SDL_Rect clip;
	gfx_domain_get_clip(domain, &clip);
	gfx_domain_set_clip(domain, &content);
	
	int spec[255] = { 0 };
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
	{
		Uint8 note = (mused.stat_note[i] & 0xff00) >> 8;
		spec[note] = my_max(spec[note], mused.vis.cyd_env[i]);
		if (note > 0) spec[note - 1] = my_max(spec[note] / 3, spec[note - 1]);
		if (note < 255) spec[note + 1] = my_max(spec[note] / 3, spec[note + 1]);
	}
	
	for (int i = 0 ; i < 96 ; ++i)
	{
		if (spec[i] >= mused.vis.spec_peak[i])
			mused.vis.spec_peak_decay[i] = 0;
		mused.vis.spec_peak_decay[i] = my_min(64, mused.vis.spec_peak_decay[i] + 1);
		mused.vis.spec_peak[i] = my_max(0, my_max(mused.vis.spec_peak[i], spec[i]) - my_min(1, my_max(0, /*mused.vis.spec_peak_decay[i] - 20*/ 2)) * 4);
	}
	
	const int w = mused.analyzer->surface->w / 2;
	SDL_Rect bar = {content.x, 0, w, 0};
	SDL_Rect src = { 0, 0, w, content.h };
	
	for (int i = (MIDDLE_C - content.w / w / 2 + 12) ; i < FREQ_TAB_SIZE && bar.x < content.x + content.w ; ++i, bar.x += bar.w)
	{
		if (i >= 0)
		{
			/*bar.h = mused.vis.spec_peak[i] * content.h / MAX_VOLUME;
			bar.y = content.y + content.h - bar.h;
			
			SDL_FillRect(mused.screen, &bar, 0x404040);*/
			
			src.x = 0;
			src.y = 0;
			src.w = w;
			src.h = content.h;
			
			bar.h = content.h;
			bar.y = content.y;
			
			SDL_Rect temp;
			copy_rect(&temp, &bar);
			
			my_BlitSurface(mused.analyzer, &src, dest_surface, &temp);
			
			bar.h = my_min(MAX_VOLUME, mused.vis.spec_peak[i]) * content.h / MAX_VOLUME;
			bar.y = content.y + content.h - bar.h;
			
			src.h = my_min(MAX_VOLUME, mused.vis.spec_peak[i]) * content.h / MAX_VOLUME;
			src.y = mused.analyzer->surface->h - bar.h;
			src.h = bar.h;
			src.x = w;
			
			copy_rect(&temp, &bar);
			
			my_BlitSurface(mused.analyzer, &src, dest_surface, &temp);
		}
	}
	
	gfx_domain_set_clip(dest_surface, &clip);
}


void catometer_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	
	SDL_Rect clip, cat;
	copy_rect(&cat, &content);
	cat.w = mused.catometer->surface->w;
	cat.x = cat.x + content.w / 2 - mused.catometer->surface->w / 2;
	gfx_domain_set_clip(domain, &clip);
	gfx_domain_set_clip(domain, &content);
	my_BlitSurface(mused.catometer, NULL, dest_surface, &cat);
	
	int v = 0;
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
	{
		v += mused.vis.cyd_env[i];
	}
	
	float a = ((float)v * M_PI / (MAX_VOLUME * 4) + M_PI) * 0.25 + mused.vis.prev_a * 0.75;
	
	if (a < M_PI)
		a = M_PI;
		
	if (a > M_PI * 2)
		a = M_PI * 2;
	
	mused.vis.prev_a = a;
	
	int ax = cos(a) * 12;
	int ay = sin(a) * 12;
	int eye1 = 31;
	int eye2 = -30;
	
	for (int w = -3 ; w <= 3 ; ++w)
	{
		gfx_line(dest_surface, dest->x + dest->w / 2 + eye1 + w, dest->y + dest->h / 2 + 6, dest->x + dest->w / 2 + ax + eye1, dest->y + dest->h / 2 + ay + 6, colors[COLOR_CATOMETER_EYES]);
		gfx_line(dest_surface, dest->x + dest->w / 2 + eye2 + w, dest->y + dest->h / 2 + 6, dest->x + dest->w / 2 + ax + eye2, dest->y + dest->h / 2 + ay + 6, colors[COLOR_CATOMETER_EYES]);
	}
	
	gfx_domain_set_clip(dest_surface, &clip);
}
