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
const int VOL_CHARS = 2;
const int CTRL_BITS = 3;
const int CMD_CHARS = 4;

const struct { bool margin; int w; int id; } pattern_params[] =
{
	{false, 3, PED_NOTE},
	{true, 1, PED_INSTRUMENT1},
	{false, 1, PED_INSTRUMENT2},
	{true, 1, PED_VOLUME1},
	{false, 1, PED_VOLUME2},
	{true, 1, PED_LEGATO},
	{false, 1, PED_SLIDE},
	{false, 1, PED_VIB},
	{true, 1, PED_COMMAND1},
	{false, 1, PED_COMMAND2},
	{false, 1, PED_COMMAND3},
	{false, 1, PED_COMMAND4},
};

#define selrow(sel, nor) ((current_patternstep() == i) ? (sel) : (nor))
#define diszero(e, c) ((!(e)) ? mix_colors(c, colors[COLOR_PATTERN_EMPTY_DATA]) : c)


/*static void update_pattern_cursor(const SDL_Rect *area, SDL_Rect *selected_rect, int current_pattern, int patternstep, int pattern_param)
{
	if (current_pattern() == current_pattern && current_patternstep() == patternstep && mused.current_patternx == pattern_param)
	{
		copy_rect(selected_rect, area);
		adjust_rect(selected_rect, -2);
		--selected_rect->h;
	}
}*/


#if 0

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
		start = current_patternstep() - dest->h / mused.console->font.h / 2;
		slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps, current_patternstep(), current_patternstep() + 1, &current_patternstep(), 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
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
	
		if (!(mused.flags & CENTER_PATTERN_EDITOR) && current_pattern == current_pattern() && row.y + row.h - 2 <= content.y + content.h)
			slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps - 1, start, i, &mused.pattern_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	
		if (i < 0) 
		{
			console_write(mused.console,"\n");
			continue;
		}
		
		console_set_clip(mused.console, &content);
		
		if (current_patternstep() == i)
		{
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
		}
		
		Uint32 base;
		
		if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
		{
			if (current_patternstep() == i)
			{		
				console_set_color(mused.console, base = colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
			}
			else
			{
				console_set_color(mused.console, base = diszero(mused.song.pattern[current_pattern].step[i].note != MUS_NOTE_NONE, timesig(i, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL])), CON_CHARACTER);
			}
		}
		else
		{
			console_set_color(mused.console, base = colors[COLOR_PATTERN_DISABLED], CON_CHARACTER);
		}
		
		if (current_pattern == current_pattern() && mused.focus == EDITPATTERN)
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
		{
			r = console_write(mused.console, emptynote);
		}
		else
			r = console_write(mused.console, notename(mused.song.pattern[current_pattern].step[i].note));
			
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_NOTE);
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
		
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_NOTE), MAKEPTR(i), MAKEPTR(current_pattern));
		
		if (viscol(VC_INSTRUMENT))
		{
			mused.console->clip.x += SPACER;
		
			if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
				console_set_color(mused.console, diszero(mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT, selrow(colors[COLOR_PATTERN_SELECTED], timesig(i, colors[COLOR_PATTERN_INSTRUMENT_BAR], colors[COLOR_PATTERN_INSTRUMENT_BEAT], colors[COLOR_PATTERN_INSTRUMENT]))), CON_CHARACTER);
			
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
		}
		
		if (viscol(VC_VOLUME))
		{
			mused.console->clip.x += SPACER;
			
			if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
				console_set_color(mused.console, diszero(mused.song.pattern[current_pattern].step[i].volume != MUS_NOTE_NO_VOLUME, selrow(colors[COLOR_PATTERN_SELECTED], timesig(i, colors[COLOR_PATTERN_VOLUME_BAR], colors[COLOR_PATTERN_VOLUME_BEAT], colors[COLOR_PATTERN_VOLUME]))), CON_CHARACTER);
						
			if (mused.song.pattern[current_pattern].step[i].volume <= MAX_VOLUME)
				r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].volume >> 4);
			else if ((mused.song.pattern[current_pattern].step[i].volume & 0xf0) == MUS_NOTE_VOLUME_FADE_UP)
				r = console_write(mused.console, "\xfb");
			else if ((mused.song.pattern[current_pattern].step[i].volume & 0xf0) == MUS_NOTE_VOLUME_FADE_DN)
				r = console_write(mused.console, "\xfc");
			else
				r = console_write(mused.console, emptychar);
				
			copy_rect(&clipped, r);
			clip_rect(&clipped, &content);
				
			check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_VOLUME1), MAKEPTR(i), MAKEPTR(current_pattern));
			
			update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_VOLUME1);
			
			if (mused.song.pattern[current_pattern].step[i].volume != MUS_NOTE_NO_VOLUME)
				r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].volume & 0xf);
			else
				r = console_write(mused.console, emptychar);
				
			if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
				console_set_color(mused.console, base, CON_CHARACTER);
				
			copy_rect(&clipped, r);
			clip_rect(&clipped, &content);
				
			check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_VOLUME2), MAKEPTR(i), MAKEPTR(current_pattern));
			
			update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_VOLUME2);
		}

		if (viscol(VC_CTRL))
		{
			mused.console->clip.x += SPACER;
			
			for (int p = PED_CTRL ; p < PED_COMMAND1 ; ++p)
			{
				if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
					console_set_color(mused.console, diszero(mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)), selrow(colors[COLOR_PATTERN_SELECTED], timesig(i, colors[COLOR_PATTERN_CTRL_BAR], colors[COLOR_PATTERN_CTRL_BEAT], colors[COLOR_PATTERN_CTRL]))), CON_CHARACTER);
			
				char *bitname = "LSV";
				r = console_write_args(mused.console, "%c", mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)) ? bitname[p - PED_CTRL] : emptychar[0]);
				copy_rect(&clipped, r);
				clip_rect(&clipped, &content);
			
				check_event(event, &clipped, select_pattern_param, MAKEPTR(p), MAKEPTR(i), MAKEPTR(current_pattern));
				update_pattern_cursor(r, &selected_rect, current_pattern, i, p);
			}
		}
		
		if (viscol(VC_COMMAND))
		{
			mused.console->clip.x += SPACER;
			
			if (channel == -1 || !(mused.mus.channel[channel].flags & MUS_CHN_DISABLED))
				console_set_color(mused.console, diszero(mused.song.pattern[current_pattern].step[i].command != 0, selrow(colors[COLOR_PATTERN_SELECTED], timesig(i, colors[COLOR_PATTERN_COMMAND_BAR], colors[COLOR_PATTERN_COMMAND_BEAT], colors[COLOR_PATTERN_COMMAND]))), CON_CHARACTER);
			
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
	
	if (channel != -1 && (mused.flags & CENTER_PATTERN_EDITOR) && (mused.flags & FOLLOW_PLAY_POSITION) && (mused.flags & SONG_PLAYING)) 
	{
		const int ah = (dest->h / mused.console->font.h);
		const int w = mused.vu_meter->surface->w;
		const int h = my_min(mused.vu_meter->surface->h, mused.vis.cyd_env[channel] * my_min(dest->h / 2, mused.vu_meter->surface->h) / MAX_VOLUME);
		SDL_Rect r = { content.x + content.w / 2 - w / 2 , content.y + ah / 2 * mused.console->font.h - h - 1, w, h };
		SDL_Rect sr = { 0, mused.vu_meter->surface->h - h, mused.vu_meter->surface->w, h };
		SDL_BlitSurface(mused.vu_meter->surface, &sr, mused.screen, &r);
	}
	
	if (current_pattern == current_pattern() && mused.focus == EDITPATTERN && mused.selection.start != mused.selection.end 
		&& mused.selection.end > start
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

	if (mused.flags & CENTER_PATTERN_EDITOR) start = current_patternstep() - dest->h / mused.console->font.h / 2;
	
	int y = 0;
	for (int row = start ; y < content.h ; ++row, y += mused.console->font.h)
	{
		if (current_patternstep() == row)
		{
			SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
		}
		else if (row < 0 || (max_steps != -1 && row >= max_steps))
			console_set_color(mused.console, colors[COLOR_PATTERN_DISABLED], CON_CHARACTER);
		else
		{
			console_set_color(mused.console, ((row == current_patternstep()) ? colors[COLOR_PATTERN_SELECTED] : timesig(row, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL])), CON_CHARACTER);
		}
		
		if (SHOW_DECIMALS & mused.flags)
			console_write_args(mused.console, "%03d\n", (row + 1000) % 1000); // so we don't get negative numbers
		else
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
		if ((mused.flags & COMPACT_VIEW))
		{
			const char *fmts[] = {"CHN %02X", "CH%02X", "%02X", "%01X", NULL};
			const char **ptr = fmts;
			
			do
			{
				snprintf(label, sizeof(label), *ptr, channel);
				++ptr;
				if (!*ptr) { break; }
				
			}
			while (strlen(label) * mused.smallfont.w > pattern_width - button.w - 2);
		}
		else
			snprintf(label, sizeof(label), "%02X", channel);
	}
	
	if (!(mused.flags & COMPACT_VIEW)) 
	{
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
			if (current_pattern() == old)
				current_pattern() = *pattern_var;
		}
	}
	else
	{
		pattern.y += 2;
		font_write(&mused.smallfont, mused.screen, &pattern, label);
	}
			
	button.x = x + pattern_width - button.w - 1;
	
	if (!(mused.flags & COMPACT_VIEW) && channel != -1)
	{
		SDL_Rect vol;
		copy_rect(&vol, &button);
	
		vol.x -= vol.w + 3 + 17;
		vol.x -= vol.w + 3 + 17 + 4;
		vol.w = 33;
		vol.h -= 2;
		vol.y += 1;
		
		int d;
		
		char tmp[4]="\xfa\xf9";
		
		if (mused.song.default_panning[channel])
			snprintf(tmp, sizeof(tmp), "%c%X", mused.song.default_panning[channel] < 0 ? '\xf9' : '\xfa', mused.song.default_panning[channel] == 63 ? 8 : ((abs((int)mused.song.default_panning[channel]) >> 3) & 0xf));
		
		if ((d = generic_field(event, &vol, 97, channel, "", "%s", tmp, 2)))
		{
			snapshot_cascade(S_T_SONGINFO, 97, channel);
			mused.song.default_panning[channel] = my_max(-64, my_min(63, (int)mused.song.default_panning[channel] + d * 8));
			if (abs(mused.song.default_panning[channel]) < 8)
				mused.song.default_panning[channel] = 0;
		}
			
		vol.x += vol.w + 2;
		
		if ((d = generic_field(event, &vol, 98, channel, "", "%02X", MAKEPTR(mused.song.default_volume[channel]), 2)))
		{
			snapshot_cascade(S_T_SONGINFO, 98, channel);
			mused.song.default_volume[channel] = my_max(0, my_min(MAX_VOLUME, (int)mused.song.default_volume[channel] + d));
		}
	}
	
	if (channel != -1)
	{
		void *action = enable_channel;
		
		if (SDL_GetModState() & KMOD_SHIFT) action = solo_channel;
	
		button_event(dest_surface, event, &button, mused.slider_bevel->surface, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? DECAL_AUDIO_DISABLED : DECAL_AUDIO_ENABLED, action, MAKEPTR(channel), 0, 0);
	}
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
	compact.w -= 2;
	compact.x += 1;
	
	button_event(mused.screen, event, &compact, mused.slider_bevel->surface, 
		!(mused.flags & COMPACT_VIEW) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
		!(mused.flags & COMPACT_VIEW) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
		(mused.flags & COMPACT_VIEW) ? DECAL_COMPACT_SELETED : DECAL_COMPACT, flip_bit_action, &mused.flags, MAKEPTR(COMPACT_VIEW), 0);
	
	const int pattern_width = 
		((mused.console->font.w * NOTE_CHARS) + viscol(VC_INSTRUMENT) * (mused.console->font.w * INST_CHARS + SPACER) + 
		viscol(VC_VOLUME) * (mused.console->font.w * VOL_CHARS + SPACER) + viscol(VC_CTRL) * (mused.console->font.w * CTRL_BITS + SPACER) + 
		viscol(VC_COMMAND) * (mused.console->font.w * CMD_CHARS + SPACER)) + 8;
		
	pos.y += track_header_size;
	pos.h -= track_header_size;
		
	SDL_Rect button_topleft = { dest->x + pos.w, dest->y, 12, track_header_size };
	
	if ((pattern_width - 2) * pv + (pos.w - 2) > dest->w)
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
	
	pattern_view_stepcounter(dest_surface, &pos, event, param, mused.single_pattern_edit ? mused.song.pattern[current_pattern()].num_steps : ml);
	
	SDL_Rect tdest;
	
	copy_rect(&tdest, dest);
	
	/*if (vert_scrollbar)
		tdest.w -= SCROLLBAR;*/
	
	SDL_SetClipRect(mused.screen, dest);
	
	pos.x += pos.w - 2;
	pos.w = pattern_width;
	
	int first = mused.song.num_channels, last = 0;
	for (int i = 0 ; i < mused.song.num_channels && pos.x < tdest.w + tdest.x ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
		{
			console_set_clip(mused.console, &pos);
			//console_clear(mused.console);
	
			if (mused.pattern_horiz_position <= i)
			{
				pattern_header(dest_surface, event, pos.x, i, &button_topleft, pattern_width, mused.ghost_pattern[i]);
				first = my_min(first, i);
				// Only consider fully visible pattern drawn
				if (pos.x + pos.w < tdest.x + tdest.w) last = my_max(last, i);
				
				pattern_view_inner(dest_surface, &pos, dest, event, *mused.ghost_pattern[i], i);
				pos.x += pos.w - 2;
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
		
		pattern_header(dest_surface, event, pos.x, -1, &button_topleft, pattern_width, (Uint16*)&current_pattern());
	
		pattern_view_inner(dest_surface, &pos, dest, event, current_pattern(), -1);
	}
	
	check_mouse_wheel_event(event, dest, &mused.pattern_slider_param);
}
#endif

void pattern_view2(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_SetClipRect(mused.screen, dest);
	
	const int height = 8;
	const int top = mused.pattern_position - dest->h / height / 2;
	const int bottom = top + dest->h / height;
	SDL_Rect row;
	copy_rect(&row, dest);
	
	adjust_rect(&row, 1);
	SDL_FillRect(mused.screen, &row, colors[COLOR_BACKGROUND]);
	
	row.y = (bottom - top) / 2 * height + row.y + 1;
	row.h = height + 2;
	
	bevel(mused.screen, &row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
	
	slider_set_params(&mused.pattern_slider_param, 0, mused.song.song_length - 1, mused.pattern_position, mused.pattern_position, &mused.pattern_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	
	const int char_width = mused.largefont.w;
	const int w = 2 * char_width + SPACER + NOTE_CHARS * char_width + SPACER + INST_CHARS * char_width + SPACER +
		VOL_CHARS * char_width + SPACER + CTRL_BITS * char_width + SPACER + CMD_CHARS * char_width + 4;
	const int narrow_w = 2 * char_width + SPACER + NOTE_CHARS * char_width + 4;
		
	slider_set_params(&mused.pattern_horiz_slider_param, 0, mused.song.num_channels - 1, mused.pattern_horiz_position, my_min(mused.song.num_channels, mused.pattern_horiz_position + 1 + (dest->w - w) / narrow_w) - 1, &mused.pattern_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
	
	int x = 0;
	
	for (int channel = mused.pattern_horiz_position ; channel < mused.song.num_channels && x < dest->w ; x += ((channel == mused.current_sequencetrack) ? w : narrow_w), ++channel)
	{
		const MusSeqPattern *sp = &mused.song.sequence[channel][0];
		
		SDL_Rect track;
		copy_rect(&track, dest);
		track.w = ((channel == mused.current_sequencetrack) ? w : narrow_w) + 2;
		track.x += x;
		
		SDL_SetClipRect(mused.screen, NULL);
		
		bevel(mused.screen, &track, mused.slider_bevel->surface, BEV_THIN_FRAME);
		adjust_rect(&track, 3);
		for (int i = 0 ; i < mused.song.num_sequences[channel] ; ++i, ++sp)
		{
			if (sp->position >= bottom) break;
			
			int len = mused.song.pattern[sp->pattern].num_steps;
			
			if (sp->position + len <= top) continue;
			
			if (i < mused.song.num_sequences[channel] - 1)
				len = my_min(len, (sp + 1)->position - sp->position);
			
			SDL_Rect pat = { track.x, (sp->position - top) * height + track.y, ((channel == mused.current_sequencetrack) ? w : narrow_w), len * height };
			SDL_Rect text;
			copy_rect(&text, &pat);
			clip_rect(&pat, &track);
			
			SDL_SetClipRect(mused.screen, &pat);
			
			for (int step = 0 ; step < len ; ++step)
			{
				MusStep *s = &mused.song.pattern[sp->pattern].step[step];
				
				SDL_Rect pos;
				copy_rect(&pos, &text);
				pos.h = height;
				
				if (step == 0)
					font_write_args(&mused.smallfont, mused.screen, &pos, "%02X", sp->pattern);
					
				pos.x += 2 * char_width + SPACER;
				
				for (int param = PED_NOTE ; param < PED_PARAMS ; ++param)
				{
					pos.w = pattern_params[param].w * char_width;
					
					if (pattern_params[param].margin)
						pos.x += SPACER;
						
					static const struct { Uint32 bar, beat, normal; } coltab[] =
					{
						{COLOR_PATTERN_BAR, COLOR_PATTERN_BEAT, COLOR_PATTERN_NORMAL},
						{COLOR_PATTERN_INSTRUMENT_BAR, COLOR_PATTERN_INSTRUMENT_BEAT, COLOR_PATTERN_INSTRUMENT},
						{COLOR_PATTERN_INSTRUMENT_BAR, COLOR_PATTERN_INSTRUMENT_BEAT, COLOR_PATTERN_INSTRUMENT},
						{COLOR_PATTERN_VOLUME_BAR, COLOR_PATTERN_VOLUME_BEAT, COLOR_PATTERN_VOLUME},
						{COLOR_PATTERN_VOLUME_BAR, COLOR_PATTERN_VOLUME_BEAT, COLOR_PATTERN_VOLUME},
						{COLOR_PATTERN_CTRL_BAR, COLOR_PATTERN_CTRL_BEAT, COLOR_PATTERN_CTRL},
						{COLOR_PATTERN_CTRL_BAR, COLOR_PATTERN_CTRL_BEAT, COLOR_PATTERN_CTRL},
						{COLOR_PATTERN_CTRL_BAR, COLOR_PATTERN_CTRL_BEAT, COLOR_PATTERN_CTRL},
						{COLOR_PATTERN_COMMAND_BAR, COLOR_PATTERN_COMMAND_BEAT, COLOR_PATTERN_COMMAND},
						{COLOR_PATTERN_COMMAND_BAR, COLOR_PATTERN_COMMAND_BEAT, COLOR_PATTERN_COMMAND},
						{COLOR_PATTERN_COMMAND_BAR, COLOR_PATTERN_COMMAND_BEAT, COLOR_PATTERN_COMMAND},
						{COLOR_PATTERN_COMMAND_BAR, COLOR_PATTERN_COMMAND_BEAT, COLOR_PATTERN_COMMAND}
					};
						
					Uint32 color;
					
					if (sp->position + step != mused.pattern_position)	
						color = timesig(step, colors[coltab[param].bar], colors[coltab[param].beat], colors[coltab[param].normal]);
					else
						console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
				
					switch (param)
					{
						case PED_NOTE:
							{
							const char *note = (s->note < MUS_NOTE_NONE) 
								? ((s->note == MUS_NOTE_RELEASE) ? "\x08\x09\x0b" : notename(s->note)) : "---";
								
							if (sp->position + step != mused.pattern_position)
								console_set_color(mused.console, diszero(mused.song.pattern[sp->pattern].step[step].note != MUS_NOTE_NONE, color), CON_CHARACTER);
								
							font_write(&mused.console->font, mused.screen, &pos, note);
							}
							break;
						
						case PED_INSTRUMENT1:
						case PED_INSTRUMENT2:
						
							if (sp->position + step != mused.pattern_position)
								console_set_color(mused.console, diszero(s->instrument != MUS_NOTE_NO_INSTRUMENT, color), CON_CHARACTER);
						
							if (s->instrument != MUS_NOTE_NO_INSTRUMENT)
								font_write_args(&mused.console->font, mused.screen, &pos, "%X", (s->instrument >> (4 - (param - PED_INSTRUMENT1) * 4)) & 0xf);
							else
								font_write(&mused.console->font, mused.screen, &pos, "-");
							break;
							
						case PED_VOLUME1:
						case PED_VOLUME2:
						
							if (sp->position + step != mused.pattern_position)
								console_set_color(mused.console, diszero(s->volume != MUS_NOTE_NO_VOLUME, color), CON_CHARACTER);
						
							if (s->volume != MUS_NOTE_NO_VOLUME)
								font_write_args(&mused.console->font, mused.screen, &pos, "%X", (s->volume >> (4 - (param - PED_VOLUME1) * 4)) & 0xf);
							else
								font_write(&mused.console->font, mused.screen, &pos, "-");
							break;
							
						case PED_LEGATO:
						case PED_SLIDE:
						case PED_VIB:
						
							if (sp->position + step != mused.pattern_position)
								console_set_color(mused.console, diszero((s->ctrl & (1 << (param - PED_LEGATO))), color), CON_CHARACTER);
						
							font_write_args(&mused.console->font, mused.screen, &pos, "%c", (s->ctrl & (1 << (param - PED_LEGATO))) ? "LSV"[param - PED_LEGATO] : '-');
							break;
							
						case PED_COMMAND1:
						case PED_COMMAND2:
						case PED_COMMAND3:
						case PED_COMMAND4:
						
							if (sp->position + step != mused.pattern_position)
								console_set_color(mused.console, diszero(s->command != 0, color), CON_CHARACTER);
						
							font_write_args(&mused.console->font, mused.screen, &pos, "%X", (s->command >> (12 - (param - PED_COMMAND1) * 4)) & 0xf);
							break;
					}
					
					pos.x += pos.w;
					
					if (channel != mused.current_sequencetrack)
						break;
				}
				
				text.y += height;
			}
		}
	}
	
	SDL_Rect pat;
	copy_rect(&pat, dest);
	adjust_rect(&pat, 2);
	SDL_SetClipRect(mused.screen, &pat);
	
	if (mused.focus == EDITPATTERN)
	{
		int x = 2 + 2 * char_width + SPACER;
		for (int param = 1 ; param <= mused.current_patternx ; ++param)
			x += (pattern_params[param].margin ? SPACER : 0) + pattern_params[param - 1].w * char_width;
	
		SDL_Rect cursor = { dest->x + narrow_w * (mused.current_sequencetrack - mused.pattern_horiz_position) + x, row.y, pattern_params[mused.current_patternx].w * char_width, row.h};
		adjust_rect(&cursor, -2);
		set_cursor(&cursor);
		
		if (mused.selection.start != mused.selection.end)
		{
			if (mused.selection.start <= bottom && mused.selection.end >= top)
			{
				SDL_Rect selection = { dest->x + narrow_w * (mused.current_sequencetrack - mused.pattern_horiz_position) + 2 * char_width + SPACER, 
					row.y + height * (mused.selection.start - mused.pattern_position), w - (2 * char_width + SPACER), height * (mused.selection.end - mused.selection.start)};
					
				adjust_rect(&selection, -3);
				selection.h += 2;
				bevel(mused.screen, &selection, mused.slider_bevel->surface, BEV_SELECTION);
			}
		}
	}
	
	SDL_SetClipRect(mused.screen, NULL);
	SDL_Rect scrollbar = { dest->x, dest->y + dest->h - SCROLLBAR, dest->w, SCROLLBAR };
	
	slider(dest_surface, &scrollbar, event, &mused.pattern_horiz_slider_param); 
}

