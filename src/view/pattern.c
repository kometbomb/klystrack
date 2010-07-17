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

#include "../view.h"
#include "../event.h"
#include "../mused.h"
#include "../action.h"
#include "../diskop.h"
#include "gui/mouse.h"
#include "gui/dialog.h"
#include "gui/bevel.h"
#include "../theme.h"
#include "../mybevdefs.h"
#include "snd/freqs.h"
#include <stdbool.h>


const int NOTE_CHARS = 3;
const int SPACER = 4;
const int INST_CHARS = 2;
const int CTRL_BITS = 3;
const int CMD_CHARS = 4;


static void update_pattern_cursor(const SDL_Rect *area, SDL_Rect *selected_rect, int current_pattern, int patternstep, int pattern_param)
{
	if (mused.current_pattern == current_pattern && mused.current_patternstep == patternstep && mused.current_patternx == pattern_param)
	{
		copy_rect(selected_rect, area);
		adjust_rect(selected_rect, -2);
		--selected_rect->h;
	}
}


void pattern_view_inner(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Rect *limits, const SDL_Event *event, int current_pattern, int channel)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	adjust_rect(&content, 2);
	
	console_set_clip(mused.console, &content);
	console_clear(mused.console);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	
	int start = mused.pattern_position;

	if (mused.flags & CENTER_PATTERN_EDITOR) 
	{
		start = mused.current_patternstep - dest->h / mused.console->font.h / 2;
		slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps, mused.current_patternstep, mused.current_patternstep + 1, &mused.current_patternstep, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	}
	
	SDL_Rect selected_rect = { 0 };
	int selection_begin = -1, selection_end = -1;
	
	SDL_Rect clipped;
	copy_rect(&clipped, &content);
	adjust_rect(&clipped, -2);
	clip_rect(&clipped, limits);
	SDL_SetClipRect(mused.screen, &clipped);
	
	for (int i = start, y = 0 ; i < mused.song.pattern[current_pattern].num_steps && y < content.h; ++i, y += mused.console->font.h)
	{
		SDL_Rect row = { content.x - SPACER / 2, content.y + y - 1, content.w + SPACER, mused.console->font.h + 1};
	
		if (!(mused.flags & CENTER_PATTERN_EDITOR) && current_pattern == mused.current_pattern && row.y + row.h - 2 <= content.y + content.h)
			slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps - 1, start, i, &mused.pattern_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	
		if (i < 0) 
		{
			console_write(mused.console,"\n");
			continue;
		}
		
		console_set_clip(mused.console, &content);
		
		if (mused.current_patternstep == i)
		{
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
		}
		
		Uint32 base;
		
		if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
		{
			if (mused.current_patternstep == i)
			{		
				console_set_color(mused.console, base = colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
			}
			else
			{
				console_set_color(mused.console, base = timesig(i, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL]), CON_CHARACTER);
			}
		}
		else
		{
			console_set_color(mused.console, base = colors[COLOR_PATTERN_DISABLED], CON_CHARACTER);
		}
		
		if (current_pattern == mused.current_pattern && mused.focus == EDITPATTERN)
		{
			if (i <= mused.selection.start)
			{
				selection_begin = row.y + 1;
			}
			
			if (i < mused.selection.end)
			{
				selection_end = row.y + row.h - 1;
			}
		}
		
		const SDL_Rect *r;
		SDL_Rect clipped;
		
		const char *emptychar = "-", *emptynote = "---";
		
		if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_RELEASE)
			r = console_write(mused.console, "\x08\x09\x0b");
		else if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_NONE)
			r = console_write(mused.console, emptynote);
		else
			r = console_write(mused.console, notename(mused.song.pattern[current_pattern].step[i].note));
			
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_NOTE);
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
		
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_NOTE), MAKEPTR(i), MAKEPTR(current_pattern));
		
		if (!(mused.flags & COMPACT_VIEW)) mused.console->clip.x += SPACER;
		
		if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
			console_set_color(mused.console, timesig(i, colors[COLOR_PATTERN_INSTRUMENT_BAR], colors[COLOR_PATTERN_INSTRUMENT_BEAT], colors[COLOR_PATTERN_INSTRUMENT]), CON_CHARACTER);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument >> 4);
		else
			r = console_write(mused.console, emptychar);
			
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
			
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_INSTRUMENT1), MAKEPTR(i), MAKEPTR(current_pattern));
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT1);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument & 0xf);
		else
			r = console_write(mused.console, emptychar);
			
		if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
			console_set_color(mused.console, base, CON_CHARACTER);
			
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
			
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_INSTRUMENT2), MAKEPTR(i), MAKEPTR(current_pattern));
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT2);
		
		if (!(mused.flags & COMPACT_VIEW))
		{
			mused.console->clip.x += SPACER;
			
			for (int p = PED_CTRL ; p < PED_COMMAND1 ; ++p)
			{
				char *bitname = "LSV";
				r = console_write_args(mused.console, "%c", mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)) ? bitname[p - PED_CTRL] : emptychar[0]);
				copy_rect(&clipped, r);
				clip_rect(&clipped, &content);
			
				check_event(event, &clipped, select_pattern_param, MAKEPTR(p), MAKEPTR(i), MAKEPTR(current_pattern));
				update_pattern_cursor(r, &selected_rect, current_pattern, i, p);
			}
			
			mused.console->clip.x += SPACER;
		}
		
		for (int p = 0 ; p < 4 ; ++p)
		{
			if (mused.song.pattern[current_pattern].step[i].command == 0 && (mused.flags & HIDE_ZEROS))
				r = console_write_args(mused.console, emptychar);
			else
				r = console_write_args(mused.console, "%X", (mused.song.pattern[current_pattern].step[i].command >> ((3-p)*4)) & 0xf);
			copy_rect(&clipped, r);
			clip_rect(&clipped, &content);
			
			check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_COMMAND1 + p), MAKEPTR(i), MAKEPTR(current_pattern));
			update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_COMMAND1 + p);
		}
		
		if (channel != -1)
		{
			if (i == mused.stat_pattern_position[channel] && mused.mus.song_track[channel].pattern == &mused.song.pattern[current_pattern])
			{
				if (!((mused.flags & CENTER_PATTERN_EDITOR) && (mused.flags & FOLLOW_PLAY_POSITION)))
					bevel(mused.screen, &row, mused.slider_bevel->surface, BEV_SEQUENCE_PLAY_POS);
			}
		}
		
		console_write(mused.console,"\n");
	}
	
	if (channel != -1 && (mused.flags & CENTER_PATTERN_EDITOR) && (mused.flags & FOLLOW_PLAY_POSITION)) 
	{
		const int ah = (dest->h / mused.console->font.h);
		const int w = mused.vu_meter->surface->w;
		const int h = mused.vis.cyd_env[channel] * my_min(dest->h / 2, mused.vu_meter->surface->h) / MAX_VOLUME ;
		SDL_Rect r = { content.x + content.w / 2 - w / 2 , content.y + ah / 2 * mused.console->font.h - h - 1, w, h };
		SDL_Rect sr = { 0, mused.vu_meter->surface->h - h, mused.vu_meter->surface->w, h };
		SDL_BlitSurface(mused.vu_meter->surface, &sr, mused.screen, &r);
	}
	
	if (current_pattern == mused.current_pattern && mused.focus == EDITPATTERN && mused.selection.start != mused.selection.end 
		/*&& !(mused.selection.start > mused.pattern_slider_param.visible_last || mused.selection.end <= mused.pattern_slider_param.visible_first)*/)
	{
		if (selection_begin == -1) selection_begin = content.y - 8;
		if (selection_end == -1) selection_end = content.y + content.h + 8;
		
		if (selection_begin > selection_end) swap(selection_begin, selection_end);
		
		SDL_Rect selection = { clipped.x+2, selection_begin + 1, clipped.w-4, selection_end - selection_begin };
		adjust_rect(&selection, -4);
		bevel(mused.screen,&selection, mused.slider_bevel->surface, BEV_SELECTION);
	}
	
	SDL_SetClipRect(mused.screen, NULL);
	
	if (selected_rect.w && mused.focus == EDITPATTERN)
	{
		set_cursor(&selected_rect);
	}
	
	bevel(mused.screen,dest, mused.slider_bevel->surface, BEV_THIN_FRAME);
}


static void pattern_view_stepcounter(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param, int max_steps)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_clear(mused.console);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_set_background(mused.console, 0);
	
	int start = mused.pattern_position;

	if (mused.flags & CENTER_PATTERN_EDITOR) start = mused.current_patternstep - dest->h / mused.console->font.h / 2;
	
	int y = 0;
	for (int row = start ; y < content.h ; ++row, y += mused.console->font.h)
	{
		if (mused.current_patternstep == row)
		{
			SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
		}
		else if (row < 0 || (max_steps != -1 && row >= max_steps))
			console_set_color(mused.console, colors[COLOR_PATTERN_DISABLED], CON_CHARACTER);
		else
		{
			console_set_color(mused.console, timesig(row, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL]), CON_CHARACTER);
		}
		
		console_write_args(mused.console, "%03X\n", row & 0xfff);
	}
	
	bevel(mused.screen,dest, mused.slider_bevel->surface, BEV_THIN_FRAME);
}


static void pattern_header(SDL_Surface *dest_surface, const SDL_Event *event, int x, int channel, const SDL_Rect *topleft, int pattern_width, Uint16 *pattern_var)
{
	SDL_Rect button, pattern, area;
	copy_rect(&area, topleft);
	area.x = x + 1;
	area.w = pattern_width - 1;
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_BACKGROUND);
	copy_rect(&button, topleft);
	copy_rect(&pattern, topleft);
			
	pattern.w = 32 + 14 + 3; 
	pattern.x = x + 4;
	pattern.h = 10;
	pattern.y += 1;
	
	char label[10] = "";
	
	if (channel != -1)
	{
		sprintf(label, "%02X", channel);
	}
	
	int d = generic_field(event, &pattern, 99, channel, label, "%02X", MAKEPTR(*pattern_var), 2);
	int old = *pattern_var;
	
	if (d < 0)
	{
		*pattern_var = my_max(0, *pattern_var - 1);
	}
	else if (d > 0)
	{
		*pattern_var = my_min(NUM_PATTERNS - 1, *pattern_var + 1);
	}
	
	if (d)
	{
		if (mused.current_pattern == old)
			mused.current_pattern = *pattern_var;
	}
			
	button.x = x + pattern_width - button.w - 1;
	
	if (!(mused.flags & COMPACT_VIEW) && channel != -1)
	{
		SDL_Rect vol;
		copy_rect(&vol, &button);
	
		vol.x -= vol.w + 3 + 17;
		vol.w = 32;
		vol.h -= 2;
		vol.y += 1;
		
		int d;
		
		if ((d = generic_field(event, &vol, 98, channel, "", "%02X", MAKEPTR(mused.song.default_volume[channel]), 2)))
			mused.song.default_volume[channel] = my_max(0, my_min(MAX_VOLUME, (int)mused.song.default_volume[channel] + d));
	}
	
	if (channel != -1)
		button_event(dest_surface, event, &button, mused.slider_bevel->surface, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? DECAL_AUDIO_DISABLED : DECAL_AUDIO_ENABLED, enable_channel, MAKEPTR(channel), 0, 0);
}


void pattern_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	int pv = 0, top_i = 0;
	
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
		{
			++pv;
			top_i = i;
		}
	}
	
	SDL_Rect pos, compact;
	copy_rect(&pos, dest);
	copy_rect(&compact, dest);
	
	console_set_clip(mused.console, dest);
	console_clear(mused.console);
	
	
	pos.w = mused.console->font.w * 3 + 2 * 3 + 2;
	
	int vert_scrollbar = 0;
	SDL_Rect scrollbar;
	
	const int track_header_size = 12;
	
	compact.h = track_header_size;
	compact.w = pos.w - 1;
	
	bevel(mused.screen, &compact, mused.slider_bevel->surface, BEV_BACKGROUND);
	
	adjust_rect(&compact, 2);
	
	checkbox(mused.screen, event, &compact, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_TICK, "S", &mused.flags, COMPACT_VIEW);
	
	const int pattern_width = ((mused.flags & COMPACT_VIEW) 
		? (mused.console->font.w * NOTE_CHARS + mused.console->font.w * INST_CHARS + mused.console->font.w * CMD_CHARS)
		: (mused.console->font.w * NOTE_CHARS + SPACER + mused.console->font.w * INST_CHARS + SPACER + mused.console->font.w * CTRL_BITS + SPACER + mused.console->font.w * CMD_CHARS)) + 8;
		
	pos.y += track_header_size;
	pos.h -= track_header_size;
		
	SDL_Rect button_topleft = { dest->x + pos.w, dest->y, 12, track_header_size };
	
	if ((pattern_width - 1) * pv + (pos.w - 2) > dest->w)
	{
		pos.h -= SCROLLBAR;
		SDL_Rect temp = { dest->x, dest->y + dest->h - SCROLLBAR, dest->w, SCROLLBAR };
		copy_rect(&scrollbar, &temp);
		vert_scrollbar = 1;
	}
	
	SDL_SetClipRect(mused.screen, &pos);
	
	int ml = 0;
	
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
			ml = my_max(ml, mused.song.pattern[*mused.ghost_pattern[i]].num_steps);
	}
	
	pattern_view_stepcounter(dest_surface, &pos, event, param, mused.single_pattern_edit ? mused.song.pattern[mused.current_pattern].num_steps : ml);
	
	SDL_SetClipRect(mused.screen, dest);
	
	pos.x += pos.w - 2;
	pos.w = pattern_width;
	
	int first = mused.song.num_channels, last = 0;
	for (int i = 0 ; i < mused.song.num_channels && pos.x < dest->w + dest->x ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
		{
			console_set_clip(mused.console, &pos);
			console_clear(mused.console);
	
			if (mused.pattern_horiz_position <= i)
			{
				pattern_header(dest_surface, event, pos.x, i, &button_topleft, pattern_width, mused.ghost_pattern[i]);
				first = my_min(first, i);
				// Only consider fully visible pattern drawn
				if (pos.x + pos.w < dest->x + dest->w) last = my_max(last, i);
				
				pattern_view_inner(dest_surface, &pos, dest, event, *mused.ghost_pattern[i], i);
				pos.x += pos.w - 1;
			}
		}
	}
	
	if (vert_scrollbar) 
		slider_set_params(&mused.pattern_horiz_slider_param, 0, top_i, first, last, &mused.pattern_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
	else
		slider_set_params(&mused.pattern_horiz_slider_param, 0, top_i, mused.pattern_horiz_position, top_i, &mused.pattern_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
	
	SDL_SetClipRect(mused.screen, NULL);
	
	if (vert_scrollbar) 
	{
		slider(dest_surface, &scrollbar, event, &mused.pattern_horiz_slider_param); 
	}
	
	if (!pv)
	{
		console_set_clip(mused.console, &pos);
		console_clear(mused.console);
		
		pattern_header(dest_surface, event, pos.x, -1, &button_topleft, pattern_width, (Uint16*)&mused.current_pattern);
	
		pattern_view_inner(dest_surface, &pos, dest, event, mused.current_pattern, -1);
	}
	
	check_mouse_wheel_event(event, dest, &mused.pattern_slider_param);
}


