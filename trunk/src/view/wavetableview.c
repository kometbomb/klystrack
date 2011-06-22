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

#include "wavetableview.h"
#include "../mused.h"
#include "../view.h"
#include "../event.h"
#include "gui/mouse.h"
#include "gui/dialog.h"
#include "gui/bevel.h"
#include "theme.h"
#include "mybevdefs.h"
#include "action.h"

extern Mused mused;

void wavetable_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevel(mused.screen,&frame, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&frame, 4);
	copy_rect(&r, &frame);
	
	CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	{
		r.w = 64;
		r.h = 10;
		
		int d;
				
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_WAVE, "WAVE", "%02X", MAKEPTR(mused.selected_wavetable), 2)) != 0)
		{
			wave_add_param(d);
		}
		
		update_rect(&frame, &r);
		
		r.w = 128;
		
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_RATE, "RATE", "%6d Hz", MAKEPTR(w->sample_rate), 9)) != 0)
		{
			wave_add_param(d);
		}
		
		update_rect(&frame, &r);
		
		r.w = 72;
		r.h = 10;
		
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_BASE, "BASE", "%s", notename((w->base_note + 0x80) >> 8), 3)) != 0)
		{
			wave_add_param(d);
		}
		
		update_rect(&frame, &r);
		r.w = 48;
		
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_BASEFINE, "", "%+4d", MAKEPTR((Sint8)w->base_note), 4)) != 0)
		{
			wave_add_param(d);
		}
		
		r.w = 128;
		
		update_rect(&frame, &r);
		
		generic_flags(event, &r, EDITWAVETABLE, W_INTERPOLATE, "NO INTERPOLATION", &w->flags, CYD_WAVE_NO_INTERPOLATION);
		
		update_rect(&frame, &r);
	}
	
	my_separator(&frame, &r);
	
	{
		r.w = 80;
		
		generic_flags(event, &r, EDITWAVETABLE, W_LOOP, "LOOP", &w->flags, CYD_WAVE_LOOP);
		
		update_rect(&frame, &r);
		
		r.w = 112;
		
		int d;
		
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_LOOPBEGIN, "BEGIN", "%7d", MAKEPTR(w->loop_begin), 7)) != 0)
		{
			wave_add_param(d);
		}
		
		update_rect(&frame, &r);
		
		r.w = 80;
		
		generic_flags(event, &r, EDITWAVETABLE, W_LOOPPINGPONG, "PINGPONG", &w->flags, CYD_WAVE_PINGPONG);
		
		update_rect(&frame, &r);
		
		
		r.w = 112;
		
		if ((d = generic_field(event, &r, EDITWAVETABLE, W_LOOPEND, "END", "%7d", MAKEPTR(w->loop_end), 7)) != 0)
		{
			wave_add_param(d);
		}
		
		update_rect(&frame, &r);
	}
}


void wavetablelist_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	const int chars = area.w / mused.console->font.w - 3;
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_THIN_FRAME);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);
	SDL_Rect tmp;
	SDL_GetClipRect(dest_surface, &tmp);
	SDL_SetClipRect(dest_surface, &area);

	int y = area.y;
	
	int start = mused.wavetable_list_position;
	
	for (int i = start ; i < CYD_WAVE_MAX_ENTRIES && y < area.h + area.y ; ++i, y += mused.console->font.h)
	{
		SDL_Rect row = { area.x - 1, y - 1, area.w + 2, mused.console->font.h + 1};
		if (i == mused.selected_wavetable)
		{
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_SELECTED], CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_NORMAL], CON_CHARACTER);
		}
			
		const CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[i];
		char temp[1000] = "";
		
		if (w->samples > 0)
			snprintf(temp, chars, "%u smp %0.1f kHz", w->samples, (float)w->sample_rate / 1000);
		
		check_event(event, console_write_args(mused.console, "%02X %s\n", i, temp), select_wavetable, MAKEPTR(i), 0, 0);
		
		slider_set_params(&mused.wavetable_list_slider_param, 0, CYD_WAVE_MAX_ENTRIES - 1, start, i, &mused.wavetable_list_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	}
	
	SDL_SetClipRect(dest_surface, &tmp);
	
	check_mouse_wheel_event(event, dest, &mused.wavetable_list_slider_param);	
}


static void update_sample_preview(const SDL_Surface *dest, const SDL_Rect* area)
{
	if (!mused.wavetable_preview || (mused.wavetable_preview->w != area->w || mused.wavetable_preview->h != area->h))
	{
		if (mused.wavetable_preview) SDL_FreeSurface(mused.wavetable_preview);
		
		mused.wavetable_preview = SDL_CreateRGBSurface(SDL_SWSURFACE, area->w, area->h, dest->format->BitsPerPixel, 
                                  dest->format->Rmask, dest->format->Gmask, dest->format->Bmask, 0);
	}
	
	if (mused.wavetable_preview_idx == mused.selected_wavetable) return;
	
	mused.wavetable_preview_idx = mused.selected_wavetable;
	
	SDL_FillRect(mused.wavetable_preview, NULL, colors[COLOR_WAVETABLE_BACKGROUND]);

	const CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	if (w->samples > 0)
	{
		const int gran = w->samples / area->w;
		Sint16 *ptr = w->data;
		int c = 0;
	
		for (int i = 0  ; i < area->w ; ++i)
		{
			int min_sample = 32768, max_sample = -32768;
			for (int x = 0 ; x < gran && c < w->samples ; ++x, ++c, ++ptr)
			{
				min_sample = my_min(min_sample, *ptr);
				max_sample = my_max(max_sample, *ptr);
			}
			
			min_sample = area->h * min_sample / 32768;
			max_sample = area->h * max_sample / 32768;
			
			SDL_Rect r = { i, area->h / 2 + min_sample / 2, 1, (max_sample - min_sample) / 2 + 1 };
			
			SDL_FillRect(mused.wavetable_preview, &r, colors[COLOR_WAVETABLE_SAMPLE]);
		}
	}
}

void wavetable_sample_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_THIN_FRAME);
	adjust_rect(&area, 3);
	update_sample_preview(dest_surface, &area);
	SDL_BlitSurface(mused.wavetable_preview, NULL, dest_surface, &area);
}
