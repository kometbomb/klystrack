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

#define HEADER_HEIGHT 12

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


void pattern_view_header(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, int channel)
{
	bevel(mused.screen, dest, mused.slider_bevel->surface, BEV_BACKGROUND);
	
	SDL_Rect area;
	copy_rect(&area, dest);
	adjust_rect(&area, 3);
	font_write_args(&mused.smallfont, dest_surface, &area, "%02d", channel);
	
	SDL_Rect mute;
	
	{	
		copy_rect(&mute, dest);
		mute.w = dest->h;
		mute.x = dest->x + dest->w - mute.w;
	
		void *action = enable_channel;
                
        if (SDL_GetModState() & KMOD_SHIFT) action = solo_channel;
        
		button_event(dest_surface, event, &mute, mused.slider_bevel->surface, 
				(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
				(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
				(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? DECAL_AUDIO_DISABLED : DECAL_AUDIO_ENABLED, action, MAKEPTR(channel), 0, 0);
	}	
	
	if (!(mused.flags & COMPACT_VIEW) && (!(mused.flags & EXPAND_ONLY_CURRENT_TRACK) || channel == mused.current_sequencetrack))
	{
		SDL_Rect vol;
		copy_rect(&vol, &mute);

		vol.x -= vol.w + 3 + 17;
		vol.x -= vol.w + 3 + 17 + 4;
		vol.w = 33;
		vol.h -= 1;
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
}


void pattern_view_inner(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event)
{
	SDL_SetClipRect(mused.screen, dest);
	
	const int height = 8;
	const int top = mused.pattern_position - dest->h / height / 2;
	const int bottom = top + dest->h / height;
	SDL_Rect row;
	copy_rect(&row, dest);
	
	adjust_rect(&row, 1);
	SDL_FillRect(mused.screen, &row, colors[COLOR_BACKGROUND]);
	
	row.y = (bottom - top) / 2 * height + row.y + 2 + HEADER_HEIGHT;
	row.h = height + 1;
	
	bevel(mused.screen, &row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
	
	slider_set_params(&mused.pattern_slider_param, 0, mused.song.song_length - 1, mused.pattern_position, mused.pattern_position, &mused.pattern_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	
	const int char_width = mused.largefont.w;
	int w = 2 * char_width + 4 + 4;
	int narrow_w = 2 * char_width + 4 + 4;
	const int SPACER = 4;
	
	for (int param = PED_NOTE ; param < PED_PARAMS ; ++param)
	{
		if (param == PED_NOTE || viscol(param))
			w += pattern_params[param].w * char_width;
		
		if (param == PED_NOTE)
			narrow_w += pattern_params[param].w * char_width;
		
		if (pattern_params[param].margin && param < PED_PARAMS - 1 && viscol(param + 1))
		{
			w += SPACER;
			
			if (param == PED_NOTE)
				narrow_w += SPACER;
		}
	}
	
	if (!(mused.flags & EXPAND_ONLY_CURRENT_TRACK))
		narrow_w = w;
		
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
		
		SDL_Rect header;
		copy_rect(&header, &track);
		header.h = HEADER_HEIGHT;
		header.w -= 2;
		
		pattern_view_header(dest_surface, &header, event, channel);
				
		track.h -= HEADER_HEIGHT;
		track.y += HEADER_HEIGHT + 1;
		
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
					if (param != PED_NOTE && !viscol(param)) continue;
				
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
					
					SDL_Rect tmp;
					copy_rect(&tmp, &pos);
					clip_rect(&tmp, &track);
					
					if (sp && event->type == SDL_MOUSEBUTTONDOWN)
					{
						check_event(event, &tmp, select_pattern_param, MAKEPTR(param), MAKEPTR(sp->position + step), MAKEPTR(channel));
						set_repeat_timer(NULL); // ugh
					}
					
					pos.x += pos.w;
					
					if (channel != mused.current_sequencetrack && (mused.flags & EXPAND_ONLY_CURRENT_TRACK))
						break;
				}
				
				text.y += height;
			}
		}
		
		if (mused.flags & SONG_PLAYING)
		{
			SDL_SetClipRect(mused.screen, &track);
			const int ah = dest->h + dest->y - row.y;
			const int w = mused.vu_meter->surface->w;
			const int h = my_min(mused.vu_meter->surface->h, mused.vis.cyd_env[channel] * ah / MAX_VOLUME);
			SDL_Rect r = { track.x + track.w / 2 - w / 2 , row.y - h, w, h };
			SDL_Rect sr = { 0, mused.vu_meter->surface->h - h, mused.vu_meter->surface->w, h };
			SDL_BlitSurface(mused.vu_meter->surface, &sr, mused.screen, &r);
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
			if (param == PED_NOTE || viscol(param)) x += (pattern_params[param].margin ? SPACER : 0) + pattern_params[param - 1].w * char_width;
	
		if (mused.current_sequencetrack >= mused.pattern_horiz_position && mused.current_sequencetrack <= my_min(mused.song.num_channels, mused.pattern_horiz_position + 1 + (dest->w - w) / narrow_w) - 1)
		{
			SDL_Rect cursor = { 1 + dest->x + narrow_w * (mused.current_sequencetrack - mused.pattern_horiz_position) + x, row.y, pattern_params[mused.current_patternx].w * char_width, row.h};
			adjust_rect(&cursor, -2);
			set_cursor(&cursor);
		}
		
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
	
	// ach
	
	if (event->type == SDL_MOUSEBUTTONDOWN)
	{
		switch (event->button.button)
		{
			case 4:
				mused.pattern_position -= 16;
				break;
			case 5:
				mused.pattern_position += 16;
				break;
		}
		
		mused.pattern_position = my_max(0, my_min(mused.song.song_length - 1, mused.pattern_position));
	}
}


static void pattern_view_stepcounter(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	
	SDL_Rect header, frame, compact;
	copy_rect(&header, dest);
	header.h = HEADER_HEIGHT;
	header.w -= 2;
	
	copy_rect(&compact, &header);
	adjust_rect(&compact, 2);
	
	bevel(mused.screen, &header, mused.slider_bevel->surface, BEV_BACKGROUND);
	
	button_event(mused.screen, event, &compact, mused.slider_bevel->surface, 
                !(mused.flags & COMPACT_VIEW) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
                !(mused.flags & COMPACT_VIEW) ? BEV_BUTTON : BEV_BUTTON_ACTIVE, 
                (mused.flags & COMPACT_VIEW) ? DECAL_COMPACT_SELETED : DECAL_COMPACT, flip_bit_action, &mused.flags, MAKEPTR(COMPACT_VIEW), 0);
	
	content.y += HEADER_HEIGHT;
	content.h -= HEADER_HEIGHT;
	copy_rect(&frame, &content);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_clear(mused.console);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
	console_set_background(mused.console, 0);
	
	int start = mused.pattern_position - dest->h / mused.console->font.h / 2;
	
	int y = 0;
	for (int row = start ; y < content.h ; ++row, y += mused.console->font.h)
	{
		if (mused.pattern_position == row)
		{
			SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
			bevel(mused.screen,&row, mused.slider_bevel->surface, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PATTERN_SELECTED], CON_CHARACTER);
		}
		else if (row < 0)
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
	
	bevel(mused.screen, &frame, mused.slider_bevel->surface, BEV_THIN_FRAME);
	
	SDL_SetClipRect(mused.screen, NULL);
}


void pattern_view2(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect pat, pos;
	copy_rect(&pat, dest);
	copy_rect(&pos, dest);
		
	pat.w -= 30;
	pat.x += 30;
	
	pos.w = 32;
		
	pattern_view_stepcounter(dest_surface, &pos, event);
	pattern_view_inner(dest_surface, &pat, event);
	
	SDL_SetClipRect(mused.screen, NULL);
	SDL_Rect scrollbar = { dest->x, dest->y + dest->h - SCROLLBAR, dest->w, SCROLLBAR };
	
	slider(dest_surface, &scrollbar, event, &mused.pattern_horiz_slider_param); 
}

