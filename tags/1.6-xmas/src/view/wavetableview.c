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
#include "wave_action.h"

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
		
		console_write_args(mused.console, "%02X %s\n", i, temp);
		
		check_event(event, &row, select_wavetable, MAKEPTR(i), 0, 0);
		
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
	else if (mused.wavetable_preview_idx == mused.selected_wavetable) return;
	
	mused.wavetable_preview_idx = mused.selected_wavetable;
	
	SDL_FillRect(mused.wavetable_preview, NULL, colors[COLOR_WAVETABLE_BACKGROUND]);

	const CydWavetableEntry *w = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
	
	mused.wavetable_bits = 0;
	
	if (w->samples > 0)
	{
		const int res = 4096;
		const int dadd = my_max(res, w->samples * res / area->w);
		const int gadd = my_max(res, (Uint32)area->w * res / w->samples);
		int c = 0, d = 0;
				
		for (int x = 0 ; x < area->w * res ; )
		{
			int min = 32768;
			int max = -32768;
			
			for (; d < w->samples && c < dadd ; c += res, ++d)
			{
				min = my_min(min, w->data[d]);
				max = my_max(max, w->data[d]);
				mused.wavetable_bits |= w->data[d];
			}
			
			c -= dadd;
			
			if (min < 0 && max < 0)
				max = 0;
				
			if (min > 0 && max > 0)
				min = 0;
			
			min = (32768 + min) * area->h / 65536;
			max = (32768 + max) * area->h / 65536 - min;
		
			int prev_x = x - 1;
			x += gadd;
			
			SDL_Rect r = { prev_x / res, min, my_max(1, x / res - prev_x / res), max + 1 };
			
			SDL_FillRect(mused.wavetable_preview, &r, colors[COLOR_WAVETABLE_SAMPLE]);
		}
		
		debug("Wavetable item bitmask = %x, lowest bit = %d", mused.wavetable_bits, __builtin_ffs(mused.wavetable_bits) - 1);
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
	
	int mx, my;
	
	if (mused.mode == EDITWAVETABLE && (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(1)))
	{
		mx /= mused.pixel_scale;
		my /= mused.pixel_scale;
		
		if (mx >= area.x && my >= area.y
			&& mx < area.x + area.w && my < area.y + area.h)
		{
			wavetable_draw((float)(mx - area.x) / area.w, (float)(my - area.y) / area.h, 1.0f / area.w);
		}
	}
}


void invalidate_wavetable_view()
{
	mused.wavetable_preview_idx = -1;
	mused.wavetable_bits = 0;
}


void wavetable_tools_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevel(mused.screen,&frame, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&frame, 4);
	copy_rect(&r, &frame);
	
	r.h = 12;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "DROP LOWEST BIT", wavetable_drop_lowest_bit, NULL, NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "HALVE RATE", wavetable_halve_samplerate, NULL, NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "NORMALIZE", wavetable_normalize, MAKEPTR(32768), NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "NORMALIZE 1/4", wavetable_normalize, MAKEPTR(32768 / 4), NULL, NULL);
	
	r.y += r.h;
	
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "CUT TAIL", wavetable_cut_tail, NULL, NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "CUT HEAD", wavetable_cut_head, NULL, NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "5TH", wavetable_chord, MAKEPTR(5), NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "OCTAVE", wavetable_chord, MAKEPTR(12), NULL, NULL);
	
	r.y += r.h;
	
	button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "GENERATE A", wavetable_create_one_cycle, MAKEPTR(12), NULL, NULL);
}
