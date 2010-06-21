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

#include "view.h"
#include "event.h"
#include "mused.h"
#include "action.h"
#include "gui/mouse.h"
#include "gui/dialog.h"
#include "gui/bevel.h"
#include "theme.h"
#include "mybevdefs.h"
#include "snd/freqs.h"
#include <stdbool.h>

#define swap(a,b) { a ^= b; b ^= a; a ^= b; }

#define timesig(i, bar, beat, normal) ((((i)%((mused.time_signature>>8)*(mused.time_signature&0xff))) == 0)?(bar):((((i)%(mused.time_signature&0xff))==0)?(beat):(normal)))

extern Mused mused;

extern int event_hit;

bool is_selected_param(int p)
{
	switch (mused.focus)
	{
		case EDITINSTRUMENT:
			return p == mused.selected_param;
			break;
	
		case EDITFX:
			return p == mused.edit_reverb_param;
			break;
			
		case EDITWAVETABLE:	
			return p == mused.wavetable_param;
			break;
	}
	
	return false;
}

void my_separator(const SDL_Rect *parent, SDL_Rect *rect)
{
	separator(mused.screen, parent, rect, mused.slider_bevel->surface, BEV_SEPARATOR);
}

// note: since we need to handle the focus this piece of code is duplicated from gui/view.c

void my_draw_view(const View* views, const SDL_Event *_event, const SDL_Surface *screen)
{
	SDL_Event event;
	memcpy(&event, _event, sizeof(event));
	for (int i = 0 ; views[i].handler ; ++i)
	{
		const View *view = &views[i];
		SDL_Rect area;
		area.x = view->position.x >= 0 ? view->position.x : screen->w + view->position.x;
		area.y = view->position.y >= 0 ? view->position.y : screen->h + view->position.y;
		area.w = *(Sint16*)&view->position.w > 0 ? *(Sint16*)&view->position.w : screen->w + *(Sint16*)&view->position.w - view->position.x;
		area.h = *(Sint16*)&view->position.h > 0 ? *(Sint16*)&view->position.h : screen->h + *(Sint16*)&view->position.h - view->position.y;

		memcpy(&mused.console->clip, &area, sizeof(view->position));
		int iter = 0;
		do
		{
			event_hit = 0;
			view->handler(mused.screen, &area, &event, view->param);
			if (event_hit) 
			{
				event.type = SDL_USEREVENT + 1;
				if (view->focus != -1 && mused.focus != view->focus) 
				{
					mused.focus = view->focus;
					clear_selection(0,0,0);
				}
				++iter;
			}
		}
		while (event_hit && iter <= 1);
	}
	
	mused.cursor.w = (mused.cursor_target.w + mused.cursor.w) / 2;
	mused.cursor.h = (mused.cursor_target.h + mused.cursor.h) / 2;
	mused.cursor.x = (mused.cursor_target.x + mused.cursor.x) / 2;
	mused.cursor.y = (mused.cursor_target.y + mused.cursor.y) / 2;
	
	if (mused.cursor.w < mused.cursor_target.w) ++mused.cursor.w;
	if (mused.cursor.w > mused.cursor_target.w) --mused.cursor.w;
	
	if (mused.cursor.h < mused.cursor_target.h) ++mused.cursor.h;
	if (mused.cursor.h > mused.cursor_target.h) --mused.cursor.h;
	
	if (mused.cursor.x < mused.cursor_target.x) ++mused.cursor.x;
	if (mused.cursor.x > mused.cursor_target.x) --mused.cursor.x;
	
	if (mused.cursor.y < mused.cursor_target.y) ++mused.cursor.y;
	if (mused.cursor.y > mused.cursor_target.y) --mused.cursor.y;
	
	if (mused.cursor.w > 0) bevel(mused.screen, &mused.cursor, mused.slider_bevel->surface, BEV_CURSOR);
}


char * notename(int note)
{	
	note = my_min(my_max(0, note), FREQ_TAB_SIZE - 1);
	static char buffer[4];
	static const char * notename[] =
	{
		"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
	};
	sprintf(buffer, "%s%d", notename[note % 12], (note - note % 12) / 12);
	return buffer;
}


static void label(const char *_label, const SDL_Rect *area)
{
	SDL_Rect r;
	
	copy_rect(&r, area);
	
	r.y = r.y + area->h / 2 - mused.smallfont.h / 2;
	
	font_write(&mused.smallfont, mused.screen, &r, _label);
}


void set_cursor(const SDL_Rect *location)
{
	copy_rect(&mused.cursor_target, location);
	
	if (mused.cursor.w == 0 || mused.cursor.h == 0)
		copy_rect(&mused.cursor, location);
}


int generic_field(const SDL_Event *e, const SDL_Rect *area, int param, const char *_label, const char *format, void *value, int width)
{
	label(_label, area);
	
	SDL_Rect field, spinner_area;
	copy_rect(&field, area);
	
	field.w = width * mused.console->font.w + 2;
	field.x = area->x + area->w - field.w;
	
	copy_rect(&spinner_area, &field);
	
	spinner_area.x += field.w;
	spinner_area.w = 16;
	field.x -= spinner_area.w;
	spinner_area.x -= spinner_area.w;
	
	bevel(mused.screen,&field, mused.slider_bevel->surface, BEV_FIELD);
	
	adjust_rect(&field, 1);
	
	font_write_args(&mused.largefont, mused.screen, &field, format, value);

	int r =  spinner(mused.screen, e, &spinner_area, mused.slider_bevel->surface, (Uint32)area->x << 16 | area->y);
	
	if (is_selected_param(param))
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -2);
		
		set_cursor(&r);
	}
	
	return r;
}


void generic_flags(const SDL_Event *e, const SDL_Rect *_area, int p, const char *label, Uint32 *flags, Uint32 mask)
{
	SDL_Rect area;
	copy_rect(&area, _area);
	area.y += 1;
	
	if (checkbox(mused.screen, e, &area, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_TICK,label, flags, mask)) 
	{
		switch (mused.focus)
		{
			case EDITINSTRUMENT:
				mused.selected_param = p;
				break;
				
			case EDITFX:
				mused.edit_reverb_param = p;
				break;
				
			case EDITWAVETABLE:
				mused.wavetable_param = p;
				break;
		}
		
	}
	
	if (is_selected_param(p))
	{
		SDL_Rect r;
		copy_rect(&r, &area);
		adjust_rect(&r, -2);
		r.h -= 2;
		r.w -= 2;
		set_cursor(&r);
	}
}



void sequence_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	char text[200];
	
	console_clear(mused.console);
	
	int start = mused.sequence_position;
	
	int p[MUS_MAX_CHANNELS] ={ 0 };
	
	int draw_colon[MUS_MAX_CHANNELS] = {0};
	int draw_colon_id[MUS_MAX_CHANNELS] = {0xff};
	
	const int POS_WIDTH = 4 * mused.console->font.w + 4;
	
	{
		SDL_Rect tmp;
		copy_rect(&tmp, dest);
		tmp.w = POS_WIDTH;
		bevel(mused.screen, &tmp, mused.slider_bevel->surface, BEV_SEQUENCE_BORDER);
		copy_rect(&tmp, dest);
		tmp.x += POS_WIDTH;
		tmp.w -= POS_WIDTH;
		bevel(mused.screen, &tmp, mused.slider_bevel->surface, BEV_SEQUENCE_BORDER);
	}
	
	SDL_Rect content;
	copy_rect(&content, dest);
	adjust_rect(&content, 1);
	
	int loop_begin = -1, loop_end = -1;
	SDL_Rect selection_begin = {-1, -1}, selection_end = {-1, -1};
	
	const int channel_chars = (mused.flags & COMPACT_VIEW) ? 2 : (5 + ((mused.flags & SHOW_PATTERN_POS_OFFSET) ? 3 : 0));
	const int CHANNEL_WIDTH = channel_chars * mused.console->font.w + 4; 
	int vert_scrollbar = 0;
	SDL_Rect scrollbar;
	
	if (CHANNEL_WIDTH * mused.song.num_channels + POS_WIDTH > dest->w)
	{
		content.h -= SCROLLBAR;
		SDL_Rect temp = { dest->x, dest->y + dest->h - SCROLLBAR, dest->w, SCROLLBAR };
		copy_rect(&scrollbar, &temp);
		vert_scrollbar = 1;
	}
	
	SDL_Rect clip;
	SDL_GetClipRect(mused.screen, &clip);
	clip.h = content.h;
	clip.y = content.y;
	SDL_SetClipRect(mused.screen, &clip);
	
	slider_set_params(&mused.sequence_horiz_slider_param, 0, 0, 0, mused.song.num_channels - 1, &mused.sequence_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
		
	for (int i = start, s = 0, y = 0 ; y < content.h ; i += mused.sequenceview_steps, ++s, y += mused.console->font.h + 1)
	{
		SDL_Rect pos = { content.x, content.y + y - 1, content.w, mused.console->font.h + 2 };
		
		if (i <= mused.song.loop_point)
		{
			loop_begin = pos.y;
		}
		
		if (i >= mused.song.song_length && loop_end == -1)
		{
			loop_end = pos.y;
		}
		
		if (i + mused.sequenceview_steps >= mused.song.song_length && loop_end == -1)
		{
			loop_end = pos.y + pos.h;
		}
		
		console_set_color(mused.console,(mused.current_sequencepos == i) ? colors[COLOR_SEQUENCE_SELECTED] : 
			(i < mused.song.song_length) ? timesig((start/mused.sequenceview_steps+s), 
				colors[COLOR_SEQUENCE_BAR],	colors[COLOR_SEQUENCE_BEAT], colors[COLOR_SEQUENCE_NORMAL]) : colors[COLOR_SEQUENCE_DISABLED], CON_CHARACTER);
		console_set_background(mused.console, 0);
		
		if (mused.current_sequencepos == i)
			bevel(mused.screen,&pos, mused.slider_bevel->surface, BEV_SELECTED_SEQUENCE_ROW);
			
		if ((mused.flags & SONG_PLAYING) && mused.stat_song_position >= i && mused.stat_song_position < i + mused.sequenceview_steps)
		{
			SDL_Rect play;
			copy_rect(&play, &pos);
			play.y += (mused.console->font.h * (mused.stat_song_position % mused.sequenceview_steps) / mused.sequenceview_steps) + 1;
			play.h = 2;
			bevel(mused.screen,&play, mused.slider_bevel->surface, BEV_SEQUENCE_PLAY_POS);
		}
			
		++pos.y;
		
		check_event(event, &pos, select_sequence_position, MAKEPTR(-1), MAKEPTR(i), 0);
		
		console_set_clip(mused.console, &pos);
		console_reset_cursor(mused.console);
		
		console_write_args(mused.console, "%04X", i);
		
		pos.x += POS_WIDTH;
		pos.w = CHANNEL_WIDTH;
		
		int first = mused.sequence_horiz_position;
		int last = 0;
		
		for (int c = mused.sequence_horiz_position ; c < mused.song.num_channels && pos.x <= content.w + content.x ; ++c)
		{
			first = my_min(first, c);
			if (pos.x + pos.w <= content.w + content.x) last = c;
			console_set_clip(mused.console, &pos);
			console_reset_cursor(mused.console);
			console_set_background(mused.console, 0);
			
			for (int i =0 ; i < channel_chars ; ++i)
				text[i] = '\7';
			text[channel_chars] = '\0';
 			
			if ((draw_colon[c]) > mused.sequenceview_steps)
			{
				draw_colon[c] -=  mused.sequenceview_steps;
				
				if (draw_colon[c] <= mused.sequenceview_steps)
				{
					text[0] = '\1';
					for (int i = 1 ; i < channel_chars - 1 ; ++i)
						text[i] = '\6';
					text[channel_chars-1] = '\2';
				}
				else	
				{
					text[0] = '\3';
					for (int i = 1 ; i < channel_chars - 1 ; ++i)
						text[i] = '\5';
					text[channel_chars-1] = '\4';
				}
			}
			else
			{
				draw_colon_id[c] = 0xff;
			}
			
			for (; p[c] < mused.song.num_sequences[c] ; ++p[c])
			{
				if (mused.song.sequence[c][p[c]].position >= i && mused.song.sequence[c][p[c]].position < i + mused.sequenceview_steps && draw_colon_id[c] != mused.song.sequence[c][p[c]].position)
				{
					if (!(mused.flags & COMPACT_VIEW))
					{	
						if (mused.song.sequence[c][p[c]].position != i && (mused.flags & SHOW_PATTERN_POS_OFFSET))
							sprintf(text, "%02X+%02d%+02d", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].position - i, mused.song.sequence[c][p[c]].note_offset);
						else
							sprintf(text, "%02X%+02d", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].note_offset);
					}
					else
					{
						//sprintf(text, "%02x   %02X  ", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].note_offset);
						sprintf(text, "%02X", mused.song.sequence[c][p[c]].pattern);
					}
					draw_colon[c] = mused.song.pattern[mused.song.sequence[c][p[c]].pattern].num_steps;
					draw_colon_id[c] = mused.song.sequence[c][p[c]].position;
					break;
				}
				if (mused.song.sequence[c][p[c]].position >= i)
				{
					break;
				}
			}
			
			SDL_Rect r;
			copy_rect(&r, console_write_args(mused.console,"%*s", -channel_chars, text));
			
			clip_rect(&r, &content);
			
			check_event(event, &r, select_sequence_position, MAKEPTR(c), MAKEPTR(i), 0);
			
			if (mused.current_sequencepos == i && mused.current_sequencetrack == c && mused.focus == EDITSEQUENCE)
			{
				adjust_rect(&r, -2);
				set_cursor(&r);
			}
			
			if (mused.focus == EDITSEQUENCE && c == mused.current_sequencetrack)
			{
				selection_begin.x = r.x - 2;
				
				if (i <= mused.selection.start)
				{
					selection_begin.y = pos.y - 1;
				}
				
				selection_end.x = r.x + r.w + 2;
				
				if (i < mused.selection.end)
				{
					selection_end.y = pos.y + pos.h;
				}
			}
			
			pos.x += CHANNEL_WIDTH;
		}
		
		if (vert_scrollbar) slider_set_params(&mused.sequence_horiz_slider_param, 0, mused.song.num_channels-1, first, last, &mused.sequence_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
		
		if ((mused.song.song_length > i) && (y + mused.console->font.h <= content.h))
			slider_set_params(&mused.sequence_slider_param, 0, mused.song.song_length - mused.sequenceview_steps, start, i, &mused.sequence_position, mused.sequenceview_steps, SLIDER_VERTICAL, mused.slider_bevel->surface);
	}
	
	if (loop_begin == -1) loop_begin = content.y - (mused.console->font.h + 1);
	if (loop_end == -1) loop_end = content.y + content.h + (mused.console->font.h + 1);
	
	if (mused.focus == EDITSEQUENCE && mused.selection.start != mused.selection.end
		&& !(mused.selection.start > mused.sequence_slider_param.visible_last || mused.selection.end <= mused.sequence_slider_param.visible_first))
	{
		if (selection_begin.y == -1) selection_begin.y = content.y - 8;
		if (selection_end.y == -1) selection_end.y = content.y + content.h + 8;
		
		SDL_Rect selection = { selection_begin.x, selection_begin.y, selection_end.x - selection_begin.x, selection_end.y - selection_begin.y - 1 };
		adjust_rect(&selection, -4);
		bevel(mused.screen,&selection, mused.slider_bevel->surface, BEV_SELECTION);
	}
	
	SDL_Rect loop = { content.x, loop_begin, content.w, loop_end - loop_begin };
	
	bevel(mused.screen,&loop, mused.slider_bevel->surface, BEV_SEQUENCE_LOOP);
		
	SDL_SetClipRect(mused.screen, NULL);
	
	if (vert_scrollbar) 
	{
		slider(dest_surface, &scrollbar, event, &mused.sequence_horiz_slider_param); 
	}
	
	check_mouse_wheel_event(event, dest, &mused.sequence_slider_param);
}


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
	
	SDL_Rect selected_rect = { 0 };
	int selection_begin = -1, selection_end = -1;
	
	SDL_Rect clipped;
	copy_rect(&clipped, &content);
	adjust_rect(&clipped, -2);
	clip_rect(&clipped, limits);
	SDL_SetClipRect(mused.screen, &clipped);
	
	for (int i = start, y = 0 ; i < mused.song.pattern[current_pattern].num_steps && y < content.h; ++i, y += mused.console->font.h)
	{
		console_set_clip(mused.console, &content);
		
		SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
	
		if (mused.current_patternstep == i)
		{
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, timesig(i, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL]), CON_CHARACTER);
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
		
		if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_RELEASE)
			r = console_write(mused.console, "\x08\x09\x0b");
		else if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_NONE)
			r = console_write(mused.console, "...");
		else
			r = console_write(mused.console, notename(mused.song.pattern[current_pattern].step[i].note));
			
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_NOTE);
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
		
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_NOTE), MAKEPTR(i), MAKEPTR(current_pattern));
		
		if (!(mused.flags & COMPACT_VIEW)) mused.console->clip.x += 4;
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument >> 4);
		else
			r = console_write(mused.console, ".");
			
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
			
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_INSTRUMENT1), MAKEPTR(i), MAKEPTR(current_pattern));
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT1);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument & 0xf);
		else
			r = console_write(mused.console, ".");
			
		copy_rect(&clipped, r);
		clip_rect(&clipped, &content);
			
		check_event(event, &clipped, select_pattern_param, MAKEPTR(PED_INSTRUMENT2), MAKEPTR(i), MAKEPTR(current_pattern));
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT2);
		
		if (!(mused.flags & COMPACT_VIEW))
		{
			mused.console->clip.x += 4;
			
			for (int p = PED_CTRL ; p < PED_COMMAND1 ; ++p)
			{
				char *bitname = "LSV";
				r = console_write_args(mused.console, "%c", mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)) ? bitname[p - PED_CTRL] : '.');
				copy_rect(&clipped, r);
				clip_rect(&clipped, &content);
			
				check_event(event, &clipped, select_pattern_param, MAKEPTR(p), MAKEPTR(i), MAKEPTR(current_pattern));
				update_pattern_cursor(r, &selected_rect, current_pattern, i, p);
			}
			
			mused.console->clip.x += 4;
		}
		
		for (int p = 0 ; p < 4 ; ++p)
		{
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
				bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SEQUENCE_PLAY_POS);
			}
		}
		
		console_write(mused.console,"\n");
		
		if (current_pattern == mused.current_pattern && row.y + row.h < content.y + content.h)
			slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps - 1, start, i, &mused.pattern_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	}
	
	if (current_pattern == mused.current_pattern && mused.focus == EDITPATTERN && mused.selection.start != mused.selection.end 
		&& !(mused.selection.start > mused.pattern_slider_param.visible_last || mused.selection.end <= mused.pattern_slider_param.visible_first))
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


void pattern_view_stepcounter(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_clear(mused.console);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_set_background(mused.console, 0);
	
	int y = 0;
	for (int row = mused.pattern_position ; y < content.h ; ++row, y += mused.console->font.h)
	{
		if (mused.current_patternstep == row)
		{
			SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, timesig(row, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL]), CON_CHARACTER);
		}
		
		console_write_args(mused.console, "%03X\n", row);
	}
	
	bevel(mused.screen,dest, mused.slider_bevel->surface, BEV_THIN_FRAME);
}


static void pattern_header(SDL_Surface *dest_surface, const SDL_Event *event, int x, int channel, const SDL_Rect *topleft, int pattern_width, Uint16 *pattern_var)
{
	SDL_Rect button, pattern, area;
	copy_rect(&area, topleft);
	area.x = x;
	area.w = pattern_width;
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_BACKGROUND);
	copy_rect(&button, topleft);
	copy_rect(&pattern, topleft);
			
	pattern.w = 8 * 2 + 2 + 16 + ((mused.flags & COMPACT_VIEW) ? 12 : 40) + 6; 
	pattern.x = x + 4;
	pattern.y += 1;
	pattern.h = 10;
	
	char label[10] = "";
	
	if (channel != -1)
	{
		if (mused.flags & COMPACT_VIEW)
			sprintf(label, "%X", channel);
		else
			sprintf(label, "CHN %X", channel);
	}
	
	int d = generic_field(event, &pattern, channel, label, "%02X", MAKEPTR(*pattern_var), 2);
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
			
	button.x = x + pattern_width - button.w - 2;
	
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
	compact.w = pos.w - 2;
	
	bevel(mused.screen,&compact, mused.slider_bevel->surface, BEV_BACKGROUND);
	
	adjust_rect(&compact, 2);
	
	checkbox(mused.screen, event, &compact, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_TICK, "S", &mused.flags, COMPACT_VIEW);
	
	const int pattern_width = mused.flags & COMPACT_VIEW ? mused.console->font.w * 16 - 4 - 4 - 4 - 4 - mused.console->font.w * 3 - 4 - 4 : mused.console->font.w * 16 - 4 - 4 - 4;
		
	pos.y += track_header_size;
	pos.h -= track_header_size;
		
	SDL_Rect button_topleft = { dest->x + pos.w, dest->y, 12, track_header_size };
	
	if (pattern_width * pv + pos.w > dest->w)
	{
		pos.h -= SCROLLBAR;
		SDL_Rect temp = { dest->x, dest->y + dest->h - SCROLLBAR, dest->w, SCROLLBAR };
		copy_rect(&scrollbar, &temp);
		vert_scrollbar = 1;
	}
	
	SDL_SetClipRect(mused.screen, &pos);
	
	pattern_view_stepcounter(dest_surface, &pos, event, param);
	
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
		
		pattern_header(dest_surface, event, pos.x, -1, &button_topleft, pattern_width, (Uint16*)&mused.current_pattern);
	
		pattern_view_inner(dest_surface, &pos, dest, event, mused.current_pattern, -1);
	}
	
	check_mouse_wheel_event(event, dest, &mused.pattern_slider_param);
}


void info_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_BACKGROUND);
	
	area.w = my_min(320, area.w);
	
	adjust_rect(&area, 3);
	SDL_Rect r;
	copy_rect(&r, &area);
	r.w = 100-8;
	
	if (area.w > r.w)
	{
		r.w = area.w / (int)(area.w / r.w) - 5;
	}
	else
	{
		r.w = area.w;
	}
	
	r.h = 10;
	console_set_clip(mused.console, &area);
	
	char speedtext[10];
	if (mused.song.song_speed == mused.song.song_speed2)
	{
		snprintf(speedtext, 10, "%d", mused.song.song_speed);
	}
	else
	{
		snprintf(speedtext, 10, "%d+%d", mused.song.song_speed, mused.song.song_speed2);
	}
	
	int d;
	
	if (mused.mus.cyd->flags & CYD_CLIPPING)
		d = generic_field(event, &r, 0, "\2VOL","%02X", MAKEPTR(mused.song.master_volume), 2);
	else
		d = generic_field(event, &r, 0, "\1VOL","%02X", MAKEPTR(mused.song.master_volume), 2);
	if (d) change_master_volume(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
	
	d = generic_field(event, &r, 0, "LEN", "%04X", MAKEPTR(mused.song.song_length), 4);
	if (d) change_song_length(MAKEPTR(d * mused.sequenceview_steps), 0, 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "LOOP","%04X", MAKEPTR(mused.song.loop_point), 4);
	if (d) change_loop_point(MAKEPTR(d * mused.sequenceview_steps), 0, 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "STEP","%02X", MAKEPTR(mused.sequenceview_steps), 2);
	if (d) change_seq_steps(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "SPD1","%02X", MAKEPTR(mused.song.song_speed), 2);
	if (d) change_song_speed(MAKEPTR(0), MAKEPTR(d), 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "SPD2","%02X", MAKEPTR(mused.song.song_speed2), 2);
	if (d) change_song_speed(MAKEPTR(1), MAKEPTR(d), 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "RATE","%3d", MAKEPTR(mused.song.song_rate), 3);
	if (d) change_song_rate(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
	sprintf(speedtext, "%d/%d", mused.time_signature >> 8, mused.time_signature & 0xff);
	d = generic_field(event, &r, 0, "TIME","%5s", speedtext, 5);
	if (d) change_timesig(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "OCT","%02X", MAKEPTR(mused.octave), 2);
	if (d) change_octave(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
	d = generic_field(event, &r, 0, "CHNS","%02X", MAKEPTR(mused.song.num_channels), 2);
	if (d) change_channels(MAKEPTR(d), 0, 0);
	update_rect(&area, &r);
}


void get_command_desc(char *text, Uint16 inst)
{
	static const struct { Uint16 opcode; char* name; }  instructions[] =
	{
		{MUS_FX_ARPEGGIO, "Arpeggio"},
		{MUS_FX_ARPEGGIO_ABS, "Absolute arpeggio"},
		{MUS_FX_SET_EXT_ARP, "Set external arpeggio notes"},
		{MUS_FX_PORTA_UP, "Portamento up"},
		{MUS_FX_PORTA_DN, "Portamento down"},
		{MUS_FX_VIBRATO, "Vibrato"},
		{MUS_FX_SLIDE, "Slide"},
		{MUS_FX_PORTA_UP_SEMI, "Portamento up (semitones)"},
		{MUS_FX_PORTA_DN_SEMI, "Portamento down (semitones)"},
		{MUS_FX_CUTOFF_UP, "Filter cutoff up"},
		{MUS_FX_CUTOFF_DN, "Filter cutoff down"},
		{MUS_FX_CUTOFF_SET, "Set filter cutoff"},
		{MUS_FX_FILTER_TYPE, "Set filter type"},
		{MUS_FX_PW_DN, "PW down"},
		{MUS_FX_PW_UP, "PW up"},
		{MUS_FX_PW_SET, "Set PW"},
		{MUS_FX_SET_VOLUME, "Set volume"},
		{MUS_FX_SET_WAVEFORM, "Set waveform"},
		{MUS_FX_SET_SPEED, "Set speed"},
		{MUS_FX_SET_RATE, "Set rate"},
		{MUS_FX_END, "Program end"},
		{MUS_FX_NOP, "No operation"},
		{MUS_FX_JUMP, "Goto"},
		{MUS_FX_LABEL, "Loop begin"},
		{MUS_FX_LOOP, "Loop end"},
		{MUS_FX_TRIGGER_RELEASE, "Trigger release"},
		{MUS_FX_FADE_VOLUME, "Fade volume"},
		{MUS_FX_EXT_FADE_VOLUME_UP, "Fine fade volume in"},
		{MUS_FX_EXT_FADE_VOLUME_DN, "Fine fade volume out"},
		{MUS_FX_EXT_PORTA_UP, "Fine portamento up"},
		{MUS_FX_EXT_PORTA_DN, "Fine portamento down"},
		{MUS_FX_EXT_NOTE_CUT, "Note cut"},
		{MUS_FX_EXT_RETRIGGER, "Retrigger"},
		{MUS_FX_WAVETABLE_OFFSET, "Wavetable offset"},
		{MUS_FX_SET_PANNING, "Set panning"},
		{MUS_FX_PAN_LEFT, "Pan left"},
		{MUS_FX_PAN_RIGHT, "Pan right"},
		{MUS_FX_BUZZ_UP, "Tune buzz up"},
		{MUS_FX_BUZZ_DN, "Tune buzz down" },
		{MUS_FX_BUZZ_SHAPE, "Set buzz shape" },
		{MUS_FX_BUZZ_SET, "Set buzz finetune" },
		{MUS_FX_CUTOFF_FINE_SET, "Set filter cutoff (fine)"},
		{MUS_FX_BUZZ_SET_SEMI, "Set buzz semitone" },
		{0, NULL}
	};
	
	char *name = NULL;
	Uint16 fi = 0;
	for (int i = 0 ; instructions[i].name != NULL ; ++i)
	{
		if (instructions[i].opcode == (inst) ||instructions[i].opcode == (inst & 0xff00) || instructions[i].opcode == (inst & 0x7f00) || instructions[i].opcode == (inst & 0x7ff0) 
			|| (((instructions[i].opcode == MUS_FX_CUTOFF_FINE_SET) || (instructions[i].opcode == MUS_FX_WAVETABLE_OFFSET)) && instructions[i].opcode == (inst & 0x7000)))
		{
			name = instructions[i].name;
			fi = instructions[i].opcode;
			break;
		}
	}

	if ((fi & 0x7f00) == MUS_FX_SET_WAVEFORM)
	{
		sprintf(text, "%s (%s%s%s%s)\n", name, (inst & CYD_CHN_ENABLE_NOISE) ? "N" : "", (inst & CYD_CHN_ENABLE_SAW) ? "S" : "", (inst & CYD_CHN_ENABLE_TRIANGLE) ? "T" : "", (inst & CYD_CHN_ENABLE_PULSE) ? "P" : "");
	}
	else if ((fi & 0x7f00) == MUS_FX_FILTER_TYPE)
	{
		static const char *fn[FLT_TYPES] = {"LP", "HP", "BP"};
		sprintf(text, "%s (%s)\n", name, fn[(fi & 0xf) % FLT_TYPES]);
	}
	else if (name == NULL) sprintf(text, "Unknown\n");
	else sprintf(text, "%s\n", name);
}


void info_line(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_THIN_FRAME);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);
	console_set_color(mused.console,colors[COLOR_NORMAL],CON_CHARACTER);
	
	console_clear(mused.console);
	
	char text[200]="";
	
	switch (mused.focus)
	{
		case EDITPROG:
		{
			Uint16 inst = mused.song.instrument[mused.current_instrument].program[mused.current_program_step];
			get_command_desc(text, inst);
		}
		break;
		
		case EDITINSTRUMENT:
		{
			static const char * param_desc[] = 
			{
				"Select instrument",
				"Edit instrument name",
				"Base note",
				"Lock to base note",
				"Drum",
				"Sync oscillator on keydown",
				"Reverse vibrato bit",
				"Set PW on keydown",
				"Set cutoff on keydown",
				"Send signal to FX chain",
				"Pulse wave",
				"Pulse width",
				"Saw wave",
				"Triangle wave",
				"Noise",
				"Loop noise",
				"Quarter frequency",
				"Wavetable",
				"Wavetable entry",
				"Override volume envelope for wavetable",
				"Lock wave to base note",
				"Volume",
				"Relative volume commands",
				"Envelope attack",
				"Envelope decay",
				"Envelope sustain",
				"Envelope release",
				"Buzz",
				"Buzz semi",
				"Buzz fine",
				"Buzz shape",
				"Sync channel",
				"Sync master channel",
				"Ring modulation",
				"Ring modulation source",
				"Enable filter",
				"Filter type",
				"Filter cutoff frequency",
				"Filter resonance",
				"Slide speed",
				"Program period",
				"Vibrato speed",
				"Vibrato depth",
				"Vibrato shape",
				"Vibrato delay",
				"Pulse width modulation speed",
				"Pulse width modulation depth",
				"Pulse width modulation shape",
				"FX bus"
			};
			strcpy(text, param_desc[mused.selected_param]);
		}
		
		break;
		
		case EDITPATTERN:
		
		if (mused.current_patternx >= PED_COMMAND1)
		{
			Uint16 inst = mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].command;
			get_command_desc(text, inst);
		}
		
		break;
	}
	
	console_write(mused.console,text);
}


static void write_command(const SDL_Event *event, const char *text, int cmd_idx, int cur_idx)
{
	int i = 0;

	for (const char *c = text ; *c ; ++c, ++i)
	{
		const SDL_Rect *r;
		check_event(event, r = console_write_args(mused.console, "%c", *c), 
			select_program_step, MAKEPTR(cmd_idx), 0, 0);
			
		if (mused.focus == EDITPROG && mused.editpos == i && cmd_idx == cur_idx)
		{
			SDL_Rect cur;
			copy_rect(&cur, r);
			adjust_rect(&cur, -2);
			set_cursor(&cur);
		}
	}
}


void program_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area, clip;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_THIN_FRAME);
	adjust_rect(&area, 2);
	copy_rect(&clip, &area);
	adjust_rect(&area, 1);
	console_set_clip(mused.console, &area);
	
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];

	//separator("----program-----");
	
	int start = mused.program_position;
	
	int pos = 0, prev_pos = -1;
	int selection_begin = -1, selection_end = -1;
	
	for (int i = 0 ; i < start ; ++i)
	{
		prev_pos = pos;
		if (!(inst->program[i] & 0x8000) || (inst->program[i] & 0xf000) == 0xf000) ++pos;
	}
	
	SDL_SetClipRect(mused.screen, &clip);
	
	for (int i = start, s = 0, y = 0 ; i < MUS_PROG_LEN && y < area.h; ++i, ++s, y += mused.console->font.h)
	{
		SDL_Rect row = { area.x - 1, area.y + y - 1, area.w + 2, mused.console->font.h + 1};
	
		if (mused.current_program_step == i && mused.focus == EDITPROG)
		{
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PROGRAM_SELECTED], CON_CHARACTER);
		}
		else
			console_set_color(mused.console,pos & 1 ? colors[COLOR_PROGRAM_ODD] : colors[COLOR_PROGRAM_EVEN],CON_CHARACTER);
			
		if (i <= mused.selection.start)
		{
			selection_begin = row.y;
		}
		
		if (i < mused.selection.end)
		{
			selection_end = row.y + row.h + 1;
		}
		
		char box[6], cur = ' ';
		
		for (int c = 0 ; c < CYD_MAX_CHANNELS ; ++c)
			if (mused.channel[c].instrument == inst && (mused.cyd.channel[c].flags & CYD_CHN_ENABLE_GATE) && (mused.channel[c].flags & MUS_CHN_PROGRAM_RUNNING) && mused.channel[c].program_tick == i) cur = '½';
		
		if (inst->program[i] == MUS_FX_NOP)
		{
			strcpy(box, "....");
		}
		else
		{
			sprintf(box, "%04X", ((inst->program[i] & 0xf000) != 0xf000) ? (inst->program[i] & 0x7fff) : inst->program[i]);
		}
		
		if (pos == prev_pos)
		{
			check_event(event, console_write_args(mused.console, "%02X%c   ", i, cur),
				select_program_step, MAKEPTR(i), 0, 0);
			write_command(event, box, i, mused.current_program_step);
			check_event(event, console_write_args(mused.console, "%c ", (!(inst->program[i] & 0x8000) || (inst->program[i] & 0xf000) == 0xf000) ? '´' : '|'), 
				select_program_step, MAKEPTR(i), 0, 0);
		}
		else
		{
			check_event(event, console_write_args(mused.console, "%02X%c%02X ", i, cur, pos),
				select_program_step, MAKEPTR(i), 0, 0);
			write_command(event, box, i, mused.current_program_step);
			check_event(event, console_write_args(mused.console, "%c ", ((inst->program[i] & 0x8000) && (inst->program[i] & 0xf000) != 0xf000) ? '`' : ' '), 
				select_program_step, MAKEPTR(i), 0, 0);
		}
		
		if ((inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO || (inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO_ABS)
		{
			if ((inst->program[i] & 0xff) != 0xf0 && (inst->program[i] & 0xff) != 0xf1)
				console_write_args(mused.console, "%s\n", notename(((inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO_ABS ? 0 : inst->base_note) + (inst->program[i] & 0xff)));
			else
				console_write_args(mused.console, "EXT%x\n", inst->program[i] & 0x0f);
		}
		else
			console_write_args(mused.console, "\n");
			
		if (row.y + row.h < area.y + area.h)
			slider_set_params(&mused.program_slider_param, 0, MUS_PROG_LEN - 1, start, i, &mused.program_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
		
		prev_pos = pos;
		
		if (!(inst->program[i] & 0x8000) || (inst->program[i] & 0xf000) == 0xf000) ++pos;
	}
	
	if (mused.focus == EDITPROG && mused.selection.start != mused.selection.end
		&& !(mused.selection.start > mused.program_slider_param.visible_last || mused.selection.end <= mused.program_slider_param.visible_first))
	{
		if (selection_begin == -1) selection_begin = area.y - 8;
		if (selection_end == -1) selection_end = area.y + area.h + 8;
		
		if (selection_begin > selection_end) swap(selection_begin, selection_end);
		
		SDL_Rect selection = { area.x, selection_begin - 1, area.w, selection_end - selection_begin + 1 };
		adjust_rect(&selection, -4);
		bevel(mused.screen,&selection, mused.slider_bevel->surface, BEV_SELECTION);
	}
	
	SDL_SetClipRect(mused.screen, NULL);
	
	check_mouse_wheel_event(event, dest, &mused.program_slider_param);
}


static void inst_flags(const SDL_Event *e, const SDL_Rect *_area, int p, const char *label, Uint32 *flags, Uint32 mask)
{
	generic_flags(e, _area, p, label, flags, mask);
}


static void inst_text(const SDL_Event *e, const SDL_Rect *area, int p, const char *_label, const char *format, void *value, int width)
{
	//check_event(e, area, select_instrument_param, (void*)p, 0, 0);
	
	int d = generic_field(e, area, p, _label, format, value, width);
	if (d) mused.selected_param = p;
	if (d < 0) instrument_add_param(-1);
	else if (d >0) instrument_add_param(1);
	
	/*if (p == mused.selected_param && mused.focus == EDITINSTRUMENT)
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -1);
		bevel(mused.screen,&r, mused.slider_bevel->surface, BEV_CURSOR);
	}*/
}


static void inst_field(const SDL_Event *e, const SDL_Rect *area, int p, int length, char *text)
{
	console_set_color(mused.console,colors[COLOR_NORMAL],CON_CHARACTER);
	console_set_clip(mused.console, area);
	console_clear(mused.console);
	
	bevel(mused.screen,area, mused.slider_bevel->surface, BEV_FIELD);
	
	SDL_Rect field;
	copy_rect(&field, area);
	adjust_rect(&field, 1);
	
	console_set_clip(mused.console, &field);
	
	int got_pos = 0;
	
	if (mused.edit_buffer == text && mused.focus == EDITBUFFER && mused.selected_param == p)
	{
		int i = my_max(0, mused.editpos - field.w / mused.console->font.w + 1), c = 0;
		for ( ; text[i] && c < my_min(length, field.w / mused.console->font.w) ; ++i, ++c)
		{
			const SDL_Rect *r = console_write_args(mused.console, "%c", mused.editpos == i ? '§' : text[i]);
			if (check_event(e, r, NULL, NULL, NULL, NULL))
			{
				mused.editpos = i;
				got_pos = 1;
			}
		}
		
		if (mused.editpos == i && c <= length) 
			console_write(mused.console, "§"); 
	}
	else
	{
		char temp[1000];
		strncpy(temp, text, my_min(sizeof(temp), length));
		
		temp[my_max(0, my_min(sizeof(temp), field.w / mused.console->font.w))] = '\0';
		
		console_write_args(mused.console, "%s", temp);
	}
	
	int c = 1;
	
	if (!got_pos && (c = check_event(e, &field, select_instrument_param, MAKEPTR(p), 0, 0)))
	{
		if (mused.focus == EDITBUFFER && mused.edit_buffer == text)
			mused.editpos = strlen(text);
		else
			set_edit_buffer(text, length);
	}
	
	if (!c && mused.focus == EDITBUFFER && e->type == SDL_MOUSEBUTTONDOWN) change_mode(mused.prev_mode);
}



void instrument_name_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect farea, larea, tarea;
	copy_rect(&farea,dest);
	copy_rect(&larea,dest);
	copy_rect(&tarea,dest);
	
	farea.w = 2 * mused.console->font.w + 2 + 16;
	
	if (!param)
	{
		larea.w = 0;
	}
	else
	{
		larea.w = 32;
		label("INST", &larea);
	}
	
	tarea.w = dest->w - farea.w - larea.w;
	farea.x = larea.w + dest->x;
	tarea.x = farea.x + farea.w;
	
	inst_text(event, &farea, P_INSTRUMENT, "", "%02X", MAKEPTR(mused.current_instrument), 2);
	inst_field(event, &tarea, P_NAME, sizeof(mused.song.instrument[mused.current_instrument].name), mused.song.instrument[mused.current_instrument].name);
	
	if (mused.selected_param == P_NAME || (mused.edit_buffer == mused.song.instrument[mused.current_instrument].name && mused.focus == EDITBUFFER))
	{
		SDL_Rect r;
		copy_rect(&r, &tarea);
		adjust_rect(&r, -2);
		set_cursor(&r);
	}
}


void instrument_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	
	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevel(mused.screen,&frame, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&frame, 4);
	copy_rect(&r, &frame);
	r.w = r.w / 2 - 2;
	r.h = 10;
	r.y += r.h + 1;
		
	{
		SDL_Rect note;
		copy_rect(&note, &frame);
		
		
		
		note.w = frame.w / 2 + 2;
		note.h = 10;
		
		inst_text(event, &note, P_BASENOTE, "BASE", "%s", notename(inst->base_note), 3);
		note.x += note.w + 2;
		note.w = frame.w - note.x;
		
		inst_flags(event, &note, P_LOCKNOTE, "LOCK", &inst->flags, MUS_INST_LOCK_NOTE);
		inst_flags(event, &r, P_DRUM, "DRUM", &inst->flags, MUS_INST_DRUM);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_KEYSYNC, "KSYNC", &inst->cydflags, CYD_CHN_ENABLE_KEY_SYNC);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_INVVIB, "VIB", &inst->flags, MUS_INST_INVERT_VIBRATO_BIT);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_SETPW, "SET PW", &inst->flags, MUS_INST_SET_PW);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_SETCUTOFF, "SET CUT", &inst->flags, MUS_INST_SET_CUTOFF);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_FX, "FX", &inst->cydflags, CYD_CHN_ENABLE_FX);
		update_rect(&frame, &r);
	}
	
	{
		int tmp = r.w;
		r.w = frame.w / 3 - 2;
		my_separator(&frame, &r);
		inst_flags(event, &r, P_PULSE, "PUL", &inst->cydflags, CYD_CHN_ENABLE_PULSE);
		update_rect(&frame, &r);
		r.w = frame.w / 2 - 2;
		inst_text(event, &r, P_PW, "PW", "%03X", MAKEPTR(inst->pw), 3);
		update_rect(&frame, &r);
		r.w = frame.w / 3 - 2;
		inst_flags(event, &r, P_SAW, "SAW", &inst->cydflags, CYD_CHN_ENABLE_SAW);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_TRIANGLE, "TRI", &inst->cydflags, CYD_CHN_ENABLE_TRIANGLE);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_NOISE, "NOI", &inst->cydflags, CYD_CHN_ENABLE_NOISE);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_METAL, "METAL", &inst->cydflags, CYD_CHN_ENABLE_METAL);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_1_4TH, "1/4TH", &inst->flags, MUS_INST_QUARTER_FREQ);
		update_rect(&frame, &r);
		
		r.w = tmp;
	}
	
	{
		my_separator(&frame, &r);
		int tmp = r.w;
		r.w = 42;

		inst_flags(event, &r, P_WAVE, "WAVE", &inst->cydflags, CYD_CHN_ENABLE_WAVE);
		r.x += 44;
		r.w = 32;
		inst_text(event, &r, P_WAVE_ENTRY, "", "%02X", MAKEPTR(inst->wavetable_entry), 2);
		update_rect(&frame, &r);
		r.w = 42;
		inst_flags(event, &r, P_WAVE_OVERRIDE_ENV, "OENV", &inst->cydflags, CYD_CHN_WAVE_OVERRIDE_ENV);
		r.x += r.w;
		r.w = 20;
		inst_flags(event, &r, P_WAVE_LOCK_NOTE, "L", &inst->flags, MUS_INST_WAVE_LOCK_NOTE);
		update_rect(&frame, &r);
		
		r.w = tmp;
	}
	
	my_separator(&frame, &r);
	inst_text(event, &r, P_VOLUME, "VOL", "%02X", MAKEPTR(inst->volume), 2);
	update_rect(&frame, &r);
	inst_flags(event, &r, P_RELVOL, "RELATIVE", &inst->flags, MUS_INST_RELATIVE_VOLUME);
	update_rect(&frame, &r);
	inst_text(event, &r, P_ATTACK, "ATK", "%02X", MAKEPTR(inst->adsr.a), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_DECAY, "DEC", "%02X", MAKEPTR(inst->adsr.d), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_SUSTAIN, "SUS", "%02X", MAKEPTR(inst->adsr.s), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RELEASE, "REL", "%02X", MAKEPTR(inst->adsr.r), 2);
	update_rect(&frame, &r);
	
	{
		my_separator(&frame, &r);
		int tmp = r.w;
		r.w = frame.w / 3 + 8;
		inst_flags(event, &r, P_BUZZ, "BUZZ", &inst->flags, MUS_INST_YM_BUZZ);
		r.x += r.w + 2;
		r.w = frame.w - r.x + 4;
		inst_text(event, &r, P_BUZZ_SEMI, "DETUNE", "%+3d", MAKEPTR((inst->buzz_offset + 0x80) >> 8), 3);
		update_rect(&frame, &r);
		r.w = frame.w / 2 - 8;
		inst_text(event, &r, P_BUZZ_SHAPE, "SHAPE", "%01X", MAKEPTR(inst->ym_env_shape), 1);
		r.x += r.w + 2;
		r.w = frame.w - r.x + 4;
		inst_text(event, &r, P_BUZZ_FINE, "FINE", "%+4d", MAKEPTR((Sint8)(inst->buzz_offset & 0xff)), 4);
		update_rect(&frame, &r);
		r.w = tmp;
	}
	
	my_separator(&frame, &r);
	inst_flags(event, &r, P_SYNC, "SYNC", &inst->cydflags, CYD_CHN_ENABLE_SYNC);
	update_rect(&frame, &r);
	inst_text(event, &r, P_SYNCSRC, "SRC", "%02X", MAKEPTR(inst->sync_source), 2);
	update_rect(&frame, &r);
	inst_flags(event, &r, P_RINGMOD, "RING MOD", &inst->cydflags, CYD_CHN_ENABLE_RING_MODULATION);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RINGMODSRC, "SRC", "%02X", MAKEPTR(inst->ring_mod), 2);
	update_rect(&frame, &r);
	
	static const char *flttype[] = { "LP", "HP", "BP" };
	
	my_separator(&frame, &r);
	inst_flags(event, &r, P_FILTER, "FILTER", &inst->cydflags, CYD_CHN_ENABLE_FILTER);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FLTTYPE, "TYPE", "%s", (char*)flttype[inst->flttype], 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_CUTOFF, "FRQ", "%03X", MAKEPTR(inst->cutoff), 3);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RESONANCE, "RES", "%1X", MAKEPTR(inst->resonance), 1);
	update_rect(&frame, &r);
}


void instrument_view2(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	
	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevel(mused.screen,&frame, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&frame, 4);
	copy_rect(&r, &frame);
	r.w = r.w / 2 - 2;
	r.h = 10;
	
	inst_text(event, &r, P_SLIDESPEED, "SLIDE", "%02X", MAKEPTR(inst->slide_speed), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PROGPERIOD, "P.PRD", "%02X", MAKEPTR(inst->prog_period), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBSPEED,   "VIB.S", "%02X", MAKEPTR(inst->vibrato_speed), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBDEPTH,   "VIB.D", "%02X", MAKEPTR(inst->vibrato_depth), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBSHAPE,   "VIB.SH", "%01X", MAKEPTR(inst->vib_shape), 1);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBDELAY,   "V.DEL", "%02X", MAKEPTR(inst->vib_delay), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMSPEED,   "PWM.S", "%02X", MAKEPTR(inst->pwm_speed), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMDEPTH,   "PWM.D", "%02X", MAKEPTR(inst->pwm_depth), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMSHAPE,   "PWM.SH", "%01X", MAKEPTR(inst->pwm_shape), 1);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FXBUS,   "FXBUS", "%02X", MAKEPTR(inst->fx_bus), 2);
	update_rect(&frame, &r);
}



void instrument_list(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_THIN_FRAME);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);
	
	int y = area.y;
	
	//separator("----instruments----");
	
	int start = mused.instrument_list_position;
	
	/*if (start > NUM_INSTRUMENTS - rows ) start = NUM_INSTRUMENTS - rows;
	if (start < 0 ) start = 0;*/
	
	for (int i = start ; i < NUM_INSTRUMENTS && y < area.h + area.y ; ++i, y += mused.console->font.h)
	{
		if (i == mused.current_instrument)
		{
			SDL_Rect row = { area.x - 1, y - 1, area.w + 2, mused.console->font.h + 1};
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_SELECTED], CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_NORMAL], CON_CHARACTER);
		}
			
		char temp[sizeof(mused.song.instrument[i].name)];
		
		strcpy(temp, mused.song.instrument[i].name);
		temp[my_max(0, area.w / mused.console->font.w - 3)] = '\0';
			
		check_event(event, console_write_args(mused.console, "%02X %-16s\n", i, temp), select_instrument, MAKEPTR(i), 0, 0);
		
		slider_set_params(&mused.instrument_list_slider_param, 0, NUM_INSTRUMENTS - 1, start, i, &mused.instrument_list_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	}
	
	check_mouse_wheel_event(event, dest, &mused.instrument_list_slider_param);
}


void fx_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&area, 4);
	console_set_clip(mused.console, &area);
	SDL_Rect r;
	copy_rect(&r, &area);
	
	r.h = 10;
	
	r.w = 112;
	
	generic_flags(event, &r, R_MULTIPLEX, "MULTIPLEX", &mused.song.flags, MUS_ENABLE_MULTIPLEX);
	update_rect(&area, &r);
		
	int d;
	
	r.x = 100;
	r.w = 80;
		
	if ((d = generic_field(event, &r, R_MULTIPLEX_PERIOD, "PERIOD", "%2X", MAKEPTR(mused.song.multiplex_period), 2))) 
	{
		mused.edit_reverb_param = R_MULTIPLEX_PERIOD;
		fx_add_param(d);
	}
	
	update_rect(&area, &r);
	my_separator(&area, &r);
	
	r.w = 320 / (CYD_MAX_FX_CHANNELS) - 5;
	
	for (int i = 0 ; i < CYD_MAX_FX_CHANNELS ; ++i)
	{
		char txt[10];
		sprintf(txt, "FX%d", i);
		if (button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.smallfont, i == mused.fx_bus ? BEV_BUTTON_ACTIVE : BEV_BUTTON, BEV_BUTTON_ACTIVE, txt, NULL, MAKEPTR(R_FX_BUS), MAKEPTR(i), NULL) & 1)
		{
			mused.edit_reverb_param = R_FX_BUS;
			mused.fx_bus = i;
			fx_add_param(0);
		}
		
		update_rect(&area, &r);
	}
	
	my_separator(&area, &r);
	
	generic_flags(event, &r, R_CRUSH, "CRUSH", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CRUSH);
	update_rect(&area, &r);
	
	r.x = 100;
	r.w = 64;
	
	if ((d = generic_field(event, &r, R_CRUSHBITS, "BITS", "%01X", MAKEPTR(mused.song.fx[mused.fx_bus].crush.bit_drop), 1)))
	{
		mused.edit_reverb_param = R_CRUSHBITS;
		fx_add_param(d);
	}
	
	update_rect(&area, &r);
	
	if ((d = generic_field(event, &r, R_CRUSHDOWNSAMPLE, "DSMP", "%02d", MAKEPTR(mused.song.fx[mused.fx_bus].crushex.downsample), 2)))
	{
		mused.edit_reverb_param = R_CRUSHDOWNSAMPLE;
		fx_add_param(d);
	}
	
	update_rect(&area, &r);
	
	my_separator(&area, &r);
	
	r.w = 60;
	
	generic_flags(event, &r, R_CHORUS, "STEREO", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CHORUS);
	
	update_rect(&area, &r);
	
	{
		// hacky-hack, we need different addresses for the different fields
		// because the address of the string is used as an ad-hoc identifier for
		// the fields...
		
		char temp1[10], temp2[10], temp3[10]; 
		
		sprintf(temp1, "%4.1f ms", (float)mused.song.fx[mused.fx_bus].chr.min_delay / 10);
		
		r.x = 100;
		r.w = 104;
		
		if ((d = generic_field(event, &r, R_MINDELAY, "MIN", temp1, NULL, 7)))
		{
			mused.edit_reverb_param = R_MINDELAY;
			fx_add_param(d);
		}
		
		update_rect(&area, &r);
		
		sprintf(temp2, "%4.1f ms", (float)mused.song.fx[mused.fx_bus].chr.max_delay / 10);
		
		if ((d = generic_field(event, &r, R_MAXDELAY, "MAX", temp2, NULL, 7)))
		{
			mused.edit_reverb_param = R_MAXDELAY;
			fx_add_param(d);
		}
		
		r.x = 100;
		r.y += r.h;
		
		if ((d = generic_field(event, &r, R_SEPARATION, "PHASE", "%02X", MAKEPTR(mused.song.fx[mused.fx_bus].chr.sep), 2)))
		{
			mused.edit_reverb_param = R_SEPARATION;
			fx_add_param(d);
		}
		
		update_rect(&area, &r);
		
		if (mused.song.fx[mused.fx_bus].chr.rate != 0)
			sprintf(temp3, "%5.2f Hz", (((float)(mused.song.fx[mused.fx_bus].chr.rate - 1) + 10) / 40));
		else
			strcpy(temp3, "OFF");
		
		if ((d = generic_field(event, &r, R_RATE, "MOD", temp3, NULL, 8)))
		{
			mused.edit_reverb_param = R_RATE;
			fx_add_param(d);
		}
		
		update_rect(&area, &r);
	}
	
	my_separator(&area, &r);
	
	generic_flags(event, &r, R_ENABLE, "REVERB", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_REVERB);
	
	update_rect(&area, &r);
	
	r.w = 80;
	
	r.x = 4;
	r.y += r.h;
	
	if ((d = generic_field(event, &r, R_SPREAD, "SPREAD", "%02X", MAKEPTR(mused.song.fx[mused.fx_bus].rvb.spread), 2)))
	{
		mused.edit_reverb_param = R_SPREAD;
		fx_add_param(d);
	}	
	
	update_rect(&area, &r);
	
	{
		r.x = 130;
		r.y -= r.h;
		
		int tmp = r.w;
		r.w = 60 + 32;
	
		if ((d = generic_field(event, &r, R_ROOMSIZE, "ROOMSIZE", "%02X", MAKEPTR(mused.fx_room_size), 2)))
		{
			mused.edit_reverb_param = R_ROOMSIZE;
			fx_add_param(d);
		}	
		
		update_rect(&area, &r);
		
		r.w = 320 - 8 - r.x;
		
		if ((d = generic_field(event, &r, R_ROOMVOL, "VOLUME", "%02X", MAKEPTR(mused.fx_room_vol), 2)))
		{
			mused.edit_reverb_param = R_ROOMVOL;
			fx_add_param(d);
		}	
		
		r.x = 130;
		r.y += r.h;
		
		r.w = 60+32;
		
		if ((d = generic_field(event, &r, R_ROOMDECAY, "DECAY", "%d", MAKEPTR(mused.fx_room_dec), 1)))
		{
			mused.edit_reverb_param = R_ROOMDECAY;
			fx_add_param(d);
		}	
		
		update_rect(&area, &r);
		
		r.w = 32;
		
		if (button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "SET", NULL, NULL, NULL, NULL) & 1)
		{
			set_room_size(mused.fx_bus, mused.fx_room_size, mused.fx_room_vol, mused.fx_room_dec);
		}
		
		update_rect(&area, &r);
		
		r.w = tmp;
	}
	
	my_separator(&area, &r);
	
	int p = R_DELAY;
	
	for (int i = 0 ; i < CYDRVB_TAPS ; ++i)
	{
		char label[20], value[20];
		
		sprintf(label, "TAP %d", i);
		
		r.w = 120;
		
		int d;
		
		if ((d = generic_field(event, &r, p, label, "%4d ms", MAKEPTR(mused.song.fx[mused.fx_bus].rvb.tap[i].delay), 7))) 
		{
			mused.edit_reverb_param = p;
			fx_add_param(d);
		}
		
		update_rect(&area, &r);
		
		r.w = 22;
		
		float spd = 1000.0 / mused.song.song_rate * ((float)(mused.song.song_speed + mused.song.song_speed2) / 2);
		
		char tmp[10];
		snprintf(tmp, sizeof(tmp), "%d", (int)((float)(mused.song.fx[mused.fx_bus].rvb.tap[i].delay / (int)spd * (int)spd) / spd));
		
		if (button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, tmp, NULL, NULL, NULL, NULL) & 1)
		{
			mused.song.fx[mused.fx_bus].rvb.tap[i].delay = (mused.song.fx[mused.fx_bus].rvb.tap[i].delay) / (int)spd * (int)spd;
			mused.edit_reverb_param = p;
			fx_add_param(0); // update taps
		}
				
		update_rect(&area, &r);
		
		++p;
		
		r.w = 80;
		
		if (mused.song.fx[mused.fx_bus].rvb.tap[i].gain <= CYDRVB_LOW_LIMIT)
			strcpy(value, "- INF");
		else
			sprintf(value, "%+5.1f", (double)mused.song.fx[mused.fx_bus].rvb.tap[i].gain * 0.1);
			
		d = generic_field(event, &r, p, "", "%s dB", value, 8);
		
		if ((d = generic_field(event, &r, p, "", "%s dB", value, 8))) 
		{
			mused.edit_reverb_param = p;
			fx_add_param(d);
		}
				
		update_rect(&area, &r);
		
		r.w = 32;
		
		if (button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "-INF", NULL, NULL, NULL, NULL) & 1)
		{
			mused.song.fx[mused.fx_bus].rvb.tap[i].gain = CYDRVB_LOW_LIMIT;
			mused.edit_reverb_param = p;
			fx_add_param(0);
		}
		
		update_rect(&area, &r);
		
		if (button_text_event(dest_surface, event, &r, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "0", NULL, NULL, NULL, NULL) & 1)
		{
			mused.song.fx[mused.fx_bus].rvb.tap[i].gain = 0;
			mused.edit_reverb_param = p;
			fx_add_param(0); // update taps
		}
		
		update_rect(&area, &r);
		
		my_separator(&area, &r);
		
		++p;
	}
	
	mirror_flags();
}


void instrument_disk_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(mused.screen,&area, mused.slider_bevel->surface, BEV_BACKGROUND);
	adjust_rect(&area, 2);
	
	SDL_Rect button = { area.x + 2, area.y, area.w / 2 - 4, area.h };
	
	int open = button_text_event(dest_surface, event, &button, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "LOAD", NULL, MAKEPTR(1), NULL, NULL);
	update_rect(&area, &button);
	int save = button_text_event(dest_surface, event, &button, mused.slider_bevel->surface, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "SAVE", NULL, MAKEPTR(2), NULL, NULL);
	update_rect(&area, &button);
	
	if (open & 1) open_song_action(0,0,0);
	else if (save & 1) save_song_action(0,0,0);
}


void song_name_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect larea, farea;
	copy_rect(&larea, dest);
	copy_rect(&farea, dest);
	larea.w = 32;
	farea.w -= larea.w;
	farea.x += larea.w;
	label("SONG", &larea);
	
	inst_field(event, &farea, 0, MUS_SONG_TITLE_LEN, mused.song.title);
}


void bevel_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	bevel(mused.screen,dest, mused.slider_bevel->surface, CASTPTR(int,param));
}

