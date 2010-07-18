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

void spectrum_analyzer_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	adjust_rect(&content, 1);
	bevel(mused.screen, dest, mused.slider_bevel->surface, BEV_SEQUENCE_BORDER);
	
	SDL_Rect clip;
	SDL_GetClipRect(mused.screen, &clip);
	SDL_SetClipRect(mused.screen, &content);
	
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
	
	const int w = 3; // mused.analyzer->surface->w;
	SDL_Rect bar = {content.x, 0, w, 0};
	SDL_Rect src = { 0, 0, w, content.h };
	
	for (int i = (MIDDLE_C - content.w / w / 2 + 12) ; i < FREQ_TAB_SIZE && bar.x < content.x + content.w ; ++i, bar.x += bar.w)
	{
		if (i >= 0)
		{
			/*bar.h = mused.vis.spec_peak[i] * content.h / MAX_VOLUME;
			bar.y = content.y + content.h - bar.h;
			
			SDL_FillRect(mused.screen, &bar, 0x404040);*/
			
			bar.h = mused.vis.spec_peak[i] * content.h / MAX_VOLUME;
			bar.y = content.y + content.h - bar.h;
			
			src.h = mused.vis.spec_peak[i] * content.h / MAX_VOLUME;
			src.y = mused.analyzer->surface->h - bar.h;
			src.h = bar.h;
			
			SDL_Rect temp;
			copy_rect(&temp, &bar);
			
			SDL_BlitSurface(mused.analyzer->surface, &src, mused.screen, &temp);
		}
	}
	
	SDL_SetClipRect(mused.screen, &clip);
}
