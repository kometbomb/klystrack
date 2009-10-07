/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

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
#include "mouse.h"
#include "dialog.h"
#include "bevel.h"

#define BG_CURSOR 0xffff4040
#define BG_PLAYERPOS 0xff004000
#define BG_SELECTION 0xff80ff80

#define COLOR_SELECTED_SEQUENCE_ROW 0xffffffff
#define COLOR_SEQUENCE_BAR 0xffa0a0a0
#define COLOR_SEQUENCE_BEAT 0xff404040
#define COLOR_SEQUENCE_NORMAL 0xff202020
#define COLOR_SEQUENCE_BAR_DISABLED 0xffa0a0b0
#define COLOR_SEQUENCE_BEAT_DISABLED 0xff404080
#define COLOR_SEQUENCE_NORMAL_DISABLED 0xff202040

#define timesig(i, bar, beat, normal) ((((i)%((mused.time_signature>>8)*(mused.time_signature&0xff))) == 0)?(bar):((((i)%(mused.time_signature&0xff))==0)?(beat):(normal)))

extern Mused mused;

void draw_view(const View* views, const SDL_Event *event)
{
	for (int i = 0 ; views[i].handler ; ++i)
	{
		const View *view = &views[i];

		memcpy(&mused.console->clip, &view->position, sizeof(view->position));
		view->handler(&view->position, event, view->param);
		SDL_UpdateRect(mused.console->surface, view->position.x, view->position.y, view->position.w, view->position.h);
	}
}


static void update_rect(const SDL_Rect *parent, SDL_Rect *rect)
{
	rect->x += rect->w + 4;
	
	if (rect->x + rect->w - 4 >= parent->x + parent->w)
	{
		rect->x = parent->x;
		rect->y += rect->h;
	}
}


static void separator(const SDL_Rect *parent, SDL_Rect *rect)
{
	while (rect->x > parent->x) update_rect(parent, rect);
	
	SDL_Rect r;
	copy_rect(&r, rect);
	
	rect->x = parent->x;
	rect->y += 6;
	
	r.y += 2;
	r.h = 4;
	r.w = parent->w;
	
	bevel(&r, mused.slider_bevel, BEV_SEPARATOR);
}


char * notename(Uint8 note)
{	
	static char buffer[4];
	static const char * notename[] =
	{
		"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
	};
	sprintf(buffer, "%s%d", notename[note % 12], (note - note % 12) / 12);
	return buffer;
}


void adjust_rect(SDL_Rect *rect, int margin)
{
	rect->x += margin;
	rect->y += margin;
	rect->w -= margin * 2;
	rect->h -= margin * 2;
}


void copy_rect(SDL_Rect *dest, const SDL_Rect *src)
{
	memcpy(dest, src, sizeof(*dest));
}


static void label(const char *_label, const SDL_Rect *area)
{
	SDL_Rect r;
	
	copy_rect(&r, area);
	
	r.y = r.y + area->h / 2 - mused.smallfont.h / 2;
	
	font_write(&mused.smallfont, mused.console->surface, &r, _label);
}



static int generic_field(const SDL_Event *e, const SDL_Rect *area, int param, const char *_label, const char *format, void *value, int width)
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
	
	bevel(&field, mused.slider_bevel, BEV_FIELD);
	
	adjust_rect(&field, 1);
	
	font_write_args(&mused.largefont, mused.console->surface, &field, format, value);

	return spinner(e, &spinner_area, param);
}



void sequence_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	char text[200];
	
	console_set_color(mused.console,0,CON_BACKGROUND);
	console_clear(mused.console);
	
	int start = mused.sequence_position;
	
	int p[MUS_CHANNELS] ={ 0 };
	
	int draw_colon[MUS_CHANNELS] = {0};
	int draw_colon_id[MUS_CHANNELS] = {0xff};
	
	bevel(dest, mused.slider_bevel, BEV_SEQUENCE_BORDER);
	
	SDL_Rect content;
	copy_rect(&content, dest);
	adjust_rect(&content, 1);
	
	SDL_Rect loop = { 0, 0, 0, 0 };
	
	for (int i = start, s = 0, y = 0 ; y < content.h ; i += mused.sequenceview_steps, ++s, y += mused.console->font.h + 1)
	{
		SDL_Rect pos = { content.x, content.y + y - 1, content.w, mused.console->font.h + 2 };
		
		if (i <= mused.song.loop_point)
		{
			loop.x = pos.x;
			loop.y = pos.y;
		}
		
		if (i < mused.song.song_length)
		{
			loop.w = pos.x - loop.x + pos.w;
			loop.h = pos.y - loop.y + pos.h;
		}
		
		console_set_color(mused.console,(mused.current_sequencepos == i) ? COLOR_SELECTED_SEQUENCE_ROW : 
			timesig((start/mused.sequenceview_steps+s), 
				((i < mused.song.song_length) ? COLOR_SEQUENCE_BAR : COLOR_SEQUENCE_BAR_DISABLED),
				((i < mused.song.song_length) ? COLOR_SEQUENCE_BEAT : COLOR_SEQUENCE_BEAT_DISABLED), 
				((i < mused.song.song_length) ? COLOR_SEQUENCE_NORMAL : COLOR_SEQUENCE_NORMAL_DISABLED)), CON_CHARACTER);
		console_set_background(mused.console, 0);
		
		if (mused.current_sequencepos == i)
			bevel(&pos, mused.slider_bevel, BEV_SELECTED_SEQUENCE_ROW);
		
		++pos.y;
		
		console_set_clip(mused.console, &pos);
		console_reset_cursor(mused.console);
		
		check_event(event, console_write_args(mused.console, "%04X", i), select_sequence_position, (void*)-1, (void*)i, 0);
		
		pos.x += 4;
		pos.w -= 4;
		
		console_set_clip(mused.console, &pos);
		
		for (int c = 0 ; c < MUS_CHANNELS ; ++c)
		{
			console_set_background(mused.console, 0);
			Uint32 bg = 0;
			if (mused.stat_song_position >= i && mused.stat_song_position < i + mused.sequenceview_steps)
			{
				bg = BG_PLAYERPOS;
				console_set_background(mused.console, 1);
			}
			
			if (c == mused.current_sequencetrack && i >= mused.selection.start && i < mused.selection.end)
			{
				bg = BG_SELECTION;
				console_set_background(mused.console, 1);
			}
		
			sprintf(text, "--   ");
 			
			if ((draw_colon[c]) > mused.sequenceview_steps)
			{
				draw_colon[c] -=  mused.sequenceview_steps;
				sprintf(text, "::   ");
			}
			else
			{
				draw_colon_id[c] = 0xff;
			}
			
			for (; p[c] < mused.song.num_sequences[c] ; ++p[c])
			{
				if (mused.song.sequence[c][p[c]].position >= i && mused.song.sequence[c][p[c]].position < i + mused.sequenceview_steps && draw_colon_id[c] != mused.song.sequence[c][p[c]].position)
				{
					if (mused.song.sequence[c][p[c]].position != i )
					{	
						//sprintf(text, "%02x+%02x %+3d ", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].position - i, mused.song.sequence[c][p[c]].note_offset);
						sprintf(text, "%02X+%02X", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].position - i);
					}
					else
					{
						//sprintf(text, "%02x   %+3d  ", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].note_offset);
						sprintf(text, "%02X   ", mused.song.sequence[c][p[c]].pattern);
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
			copy_rect(&r, console_write(mused.console,text));
			
			check_event(event, &r, select_sequence_position, (void*)c, (void*)i, 0);
			
			if (mused.current_sequencepos == i && mused.current_sequencetrack == c)
			{
				adjust_rect(&r, -2);
				bevel(&r, mused.slider_bevel, BEV_CURSOR);
			}
			
			console_set_color(mused.console,bg,CON_BACKGROUND);
			console_write(mused.console," ");
		}
		
		console_write(mused.console,"\n");
		
		if (i < mused.song.song_length)
			slider_set_params(&mused.sequence_slider_param, 0, mused.song.song_length - mused.sequenceview_steps, start, i, &mused.sequence_position, mused.sequenceview_steps, SLIDER_VERTICAL);
	}
	
	if (loop.w != 0)
		bevel(&loop, mused.slider_bevel, BEV_SEQUENCE_LOOP);
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


void pattern_view_inner(const SDL_Rect *dest, const SDL_Event *event, int current_pattern, int channel)
{
	SDL_Rect content;
	copy_rect(&content, dest);
	adjust_rect(&content, 2);
	
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_clip(mused.console, &content);
	console_clear(mused.console);
	
	adjust_rect(&content, 2);
	console_set_clip(mused.console, &content);
			
	int start = mused.pattern_position;
	
	console_set_color(mused.console,0xff808080,CON_CHARACTER);
	
	SDL_Rect selected_rect = { 0 };
	
	for (int i = start, y = 0 ; i < mused.song.pattern[current_pattern].num_steps && y < content.h; ++i, y += mused.console->font.h)
	{
		console_set_clip(mused.console, &content);
	
		if (mused.current_patternstep == i)
		{
			SDL_Rect row = { content.x - 2, content.y + y - 1, content.w + 4, mused.console->font.h + 1};
			bevel(&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, 0xffffffff, CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, timesig(i, 0xffffffff, 0xffffffc0, 0xffc0c0c0), CON_CHARACTER);
		}
		
		console_set_background(mused.console, 0);
		
		Uint32 bg = 0;
		
		if (channel != -1)
		{
			if (i == mused.stat_pattern_position[channel] && mused.mus.song_track[channel].pattern == &mused.song.pattern[current_pattern])
			{
				bg = BG_PLAYERPOS;
				console_set_background(mused.console, 1);
			}
		}
		
		if (current_pattern == mused.current_pattern && i >= mused.selection.start && i < mused.selection.end)
		{
			bg = BG_SELECTION;
			console_set_background(mused.console, 1);
		}
		
		const SDL_Rect *r;
		
		if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_RELEASE)
			r = console_write(mused.console, "---");
		else if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_NONE)
			r = console_write(mused.console, "...");
		else
			r = console_write(mused.console, notename(mused.song.pattern[current_pattern].step[i].note));
			
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_NOTE);
			
		check_event(event, r, select_pattern_param, (void*)PED_NOTE, (void*)i, (void*)current_pattern);
		
		mused.console->clip.x += 4;
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument >> 4);
		else
			r = console_write(mused.console, ".");
			
		check_event(event, r, select_pattern_param, (void*)PED_INSTRUMENT1, (void*)i, (void*)current_pattern);
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT1);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%X", mused.song.pattern[current_pattern].step[i].instrument & 0xf);
		else
			r = console_write(mused.console, ".");
			
		check_event(event, r, select_pattern_param, (void*)PED_INSTRUMENT2, (void*)i, (void*)current_pattern);
		
		update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_INSTRUMENT2);
		
		mused.console->clip.x += 4;
		
		for (int p = PED_CTRL ; p < PED_COMMAND1 ; ++p)
		{
			char *bitname = "LSV";
		
			console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == p)?BG_CURSOR:bg,CON_BACKGROUND);
			check_event(event, r = console_write_args(mused.console, "%c", mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)) ? bitname[p - PED_CTRL] : '.'), 
				select_pattern_param, (void*)p, (void*)i, (void*)current_pattern);
			update_pattern_cursor(r, &selected_rect, current_pattern, i, p);
		}
		
		console_set_color(mused.console,bg,CON_BACKGROUND);
		mused.console->clip.x += 4;
		
		for (int p = 0 ; p < 4 ; ++p)
		{
			console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == (PED_COMMAND1+p))?BG_CURSOR:bg,CON_BACKGROUND);
			check_event(event, r = console_write_args(mused.console, "%X", (mused.song.pattern[current_pattern].step[i].command >> ((3-p)*4)) & 0xf), 
				select_pattern_param, (void*)(PED_COMMAND1 + p), (void*)i, (void*)current_pattern);
			update_pattern_cursor(r, &selected_rect, current_pattern, i, PED_COMMAND1 + p);
		}
		
		console_write(mused.console,"\n");
		
		if (current_pattern == mused.current_pattern)
			slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps - 1, start, i, &mused.pattern_position, 1, SLIDER_VERTICAL);
	}
	
	if (selected_rect.w)
	{
		bevel(&selected_rect, mused.slider_bevel, BEV_CURSOR);
	}
	
	bevel(dest, mused.slider_bevel, BEV_THIN_FRAME);
}


void pattern_view_stepcounter(const SDL_Rect *dest, const SDL_Event *event, void *param)
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
			bevel(&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, 0xffffffff, CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, timesig(row, 0xffffffff, 0xffffffc0, 0xffc0c0c0), CON_CHARACTER);
		}
		
		console_write_args(mused.console, "%03X\n", row);
	}
	
	bevel(dest, mused.slider_bevel, BEV_THIN_FRAME);
}


static void pattern_header(const SDL_Event *event, int x, int channel, const SDL_Rect *topleft, int pattern_width, int *pattern_var)
{
	SDL_Rect button, pattern;
	copy_rect(&button, topleft);
	copy_rect(&pattern, topleft);
			
	pattern.w = 50;
	pattern.x = x;
	
	int d = generic_field(event, &pattern, channel, "", "%02X", (void*)*pattern_var, 2);
	
	if (d < 0)
	{
		*pattern_var = my_max(0, *pattern_var - 1);
	}
	else if (d > 0)
	{
		*pattern_var = my_min(NUM_PATTERNS - 1, *pattern_var + 1);
	}
			
	button.x = x + pattern_width - button.w;
	
	if (channel != -1)
		button_event(event, &button, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, 
			(mused.mus.channel[channel].flags & MUS_CHN_DISABLED) ? DECAL_AUDIO_DISABLED : DECAL_AUDIO_ENABLED, enable_channel, (void*)channel, 0, 0);
}


void pattern_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	int pv = 0;
	const int pattern_width = 128 - 4 - 4 - 4;
	
	for (int i = 0 ; i < MUS_CHANNELS ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
		{
			++pv;
		}
	}
	
	SDL_Rect pos;
	copy_rect(&pos, dest);
	
	pos.w = mused.console->font.w * 3 + 2 * 3 + 2;
	
	int vert_scrollbar = 0;
	SDL_Rect scrollbar;
	
	const int track_header_size = 12;
	
	pos.y += track_header_size;
	pos.h -= track_header_size;
	
	console_set_clip(mused.console, dest);
	console_clear(mused.console);
	
	SDL_Rect button_topleft = { dest->x + pos.w, dest->y, track_header_size, track_header_size };
	
	if (pattern_width * pv + pos.w > dest->w)
	{
		pos.h -= SCROLLBAR;
		SDL_Rect temp = { pos.w + dest->x, dest->y + dest->h - SCROLLBAR, dest->w - pos.w, SCROLLBAR };
		copy_rect(&scrollbar, &temp);
		vert_scrollbar = 1;
	}
	
	SDL_SetClipRect(mused.console->surface, &pos);
	
	pattern_view_stepcounter(&pos, event, param);
	
	SDL_SetClipRect(mused.console->surface, dest);
	
	pos.x += pos.w - 2;
	pos.w = pattern_width;
	
	int first = MUS_CHANNELS, last = 0;
	for (int i = 0 ; i < MUS_CHANNELS ; ++i)
	{
		if (mused.ghost_pattern[i] != NULL)
		{
			console_set_clip(mused.console, &pos);
			console_clear(mused.console);
	
			if (mused.pattern_horiz_position <= i)
			{
				pattern_header(event, pos.x, i, &button_topleft, pattern_width, mused.ghost_pattern[i]);
				first = my_min(first, i);
				// Only consider fully visible pattern drawn
				if (pos.x + pos.w < dest->x + dest->w) last = my_max(last, i);
				
				pattern_view_inner(&pos, event, *mused.ghost_pattern[i], i);
				pos.x += pos.w - 2;
								
				if (vert_scrollbar) slider_set_params(&mused.pattern_horiz_slider_param, 0, pv - 1, first, last, &mused.pattern_horiz_position, 1, SLIDER_HORIZONTAL);
			}
		}
	}
	
	SDL_SetClipRect(mused.console->surface, NULL);
	
	if (vert_scrollbar) 
	{
		slider(&scrollbar, event, &mused.pattern_horiz_slider_param); 
	}
	
	if (!pv)
	{
		console_set_clip(mused.console, &pos);
		console_clear(mused.console);
		
		pattern_header(event, 0, -1, &button_topleft, pattern_width, &mused.current_pattern);
	
		pattern_view_inner(&pos, event, mused.current_pattern, -1);
	}
}


void info_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	char text[200];
	
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	
	console_clear(mused.console);
	
	char speedtext[10];
	if (mused.song.song_speed == mused.song.song_speed2)
	{
		sprintf(speedtext, "%d", mused.song.song_speed);
	}
	else
	{
		sprintf(speedtext, "%d+%d", mused.song.song_speed, mused.song.song_speed2);
	}
	
	
	sprintf(text, "Song length: %04X\n"
	              "Loop point:  %04X\n"
	              "Speed:     %6s\n"
				  "Rate:      %3d hz\n"
				  "Time sig.:  %2d/%2d\n\n"
				  "Sel.instrument:%02X\n"
				  ":%-16s\n"
				  "Sel.pattern:   %02X\n"
				  "Sequence step: %02X\n"
				  "Octave:        %02X\n",
				  
				  
				  
				  mused.song.song_length, mused.song.loop_point, speedtext, mused.song.song_rate, mused.time_signature >> 8, mused.time_signature &0xf,
				  mused.current_instrument, mused.song.instrument[mused.current_instrument].name, mused.current_pattern, mused.sequenceview_steps, mused.octave);
	console_write(mused.console,text);
}


void get_command_desc(char *text, Uint16 inst)
{
	static const struct { Uint16 opcode; char* name; }  instructions[] =
	{
		{MUS_FX_ARPEGGIO, "Arpeggio"},
		{MUS_FX_SET_EXT_ARP, "Set external arpeggio notes"},
		{MUS_FX_PORTA_UP, "Portamento up"},
		{MUS_FX_PORTA_DN, "Portamento down"},
		{MUS_FX_PORTA_UP_SEMI, "Portamento up (semitones)"},
		{MUS_FX_PORTA_DN_SEMI, "Portamento down (semitones)"},
		{MUS_FX_CUTOFF_UP, "Filter cutoff up"},
		{MUS_FX_CUTOFF_DN, "Filter cutoff down"},
		{MUS_FX_CUTOFF_SET, "Set filter cutoff"},
		{MUS_FX_PW_DN, "PW down"},
		{MUS_FX_PW_UP, "PW up"},
		{MUS_FX_PW_SET, "Set PW"},
		{MUS_FX_SET_VOLUME, "Set volume"},
		{MUS_FX_SET_WAVEFORM, "Set waveform"},
		{MUS_FX_END, "Program end"},
		{MUS_FX_JUMP, "Goto"},
		{MUS_FX_LABEL, "Loop begin"},
		{MUS_FX_LOOP, "Loop end"},
		{MUS_FX_NOP, "No operation"},
		{MUS_FX_TRIGGER_RELEASE, "Trigger release"},
		{MUS_FX_FADE_VOLUME, "Fade volume"},
		{MUS_FX_EXT_FADE_VOLUME_UP, "Fine fade volume in"},
		{MUS_FX_EXT_FADE_VOLUME_DN, "Fine fade volume out"},
		{0, NULL}
	};
	
	char *name = NULL;
	Uint16 fi = 0;
	for (int i = 0 ; instructions[i].name != NULL ; ++i)
	{
		if (instructions[i].opcode == inst || instructions[i].opcode == (inst & 0x7f00) || instructions[i].opcode == (inst & 0x7ff0))
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
	else if (name == NULL) sprintf(text, "Unknown\n");
	else sprintf(text, "%s\n", name);
}


void info_line(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(&area, mused.slider_bevel, BEV_THIN_FRAME);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	
	console_clear(mused.console);
	
	char text[200]="";
	
	switch (mused.mode)
	{
		case EDITINSTRUMENT:
		
		if (mused.selected_param >= P_PARAMS)
		{
			Uint16 inst = mused.song.instrument[mused.current_instrument].program[mused.selected_param-P_PARAMS];
			get_command_desc(text, inst);
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


void program_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(&area, mused.slider_bevel, BEV_THIN_FRAME);
	adjust_rect(&area, 2);
	console_set_clip(mused.console, &area);
	
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];

	//separator("----program-----");
	
	int start = mused.program_position;
	
	int pos = 0, prev_pos = -1;
	
	for (int i = 0 ; i < start ; ++i)
	{
		prev_pos = pos;
		if (!(inst->program[i] & 0x8000)) ++pos;
	}
	
	SDL_SetClipRect(mused.console->surface, &area);
	
	for (int i = start, s = 0, y = 0 ; i < MUS_PROG_LEN && y < area.h; ++i, ++s, y += mused.console->font.h)
	{
		if (mused.selected_param == (P_PARAMS+i))
		{
			console_set_color(mused.console,0xffffffff,CON_CHARACTER);
			SDL_Rect row = { area.x - 1, area.y + y - 1, area.w + 2, mused.console->font.h + 1};
			bevel(&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, 0xffffffff, CON_CHARACTER);
		}
		else
			console_set_color(mused.console,pos & 1 ? 0xffc0c0c0 : 0xffd0d0d0,CON_CHARACTER);
		
		if (mused.selection.start - P_PARAMS <= i && mused.selection.end - P_PARAMS > i)
			console_set_color(mused.console,0xff0000ff,CON_BACKGROUND);
		else
			console_set_color(mused.console,0,CON_BACKGROUND);
		
		char box[5];
		
		if (inst->program[i] == MUS_FX_NOP)
		{
			strcpy(box, "....");
		}
		else
		{
			sprintf(box, "%04X", inst->program[i]);
		}
		
		if (mused.mode == EDITPROG && mused.selected_param == (P_PARAMS+i))
		{
			box[mused.editpos] = '§'; // Cursor character
		}
		
		if (pos == prev_pos)
			check_event(event, console_write_args(mused.console, "%02X    %s\n", i, box),
				select_instrument_param, (void*)(P_PARAMS + i), 0, 0);
		else
			check_event(event, console_write_args(mused.console, "%02X %02X %s\n", i, pos, box),
				select_instrument_param, (void*)(P_PARAMS + i), 0, 0);
			
		slider_set_params(&mused.program_slider_param, 0, MUS_PROG_LEN - 1, start, i, &mused.program_position, 1, SLIDER_VERTICAL);
		
		prev_pos = pos;
		
		if (!(inst->program[i] & 0x8000)) ++pos;
	}
	
	SDL_SetClipRect(mused.console->surface, NULL);
}


static void inst_flags(const SDL_Event *e, const SDL_Rect *area, int p, const char *label, Uint32 *flags, Uint32 mask)
{
	if (checkbox(e, area, label, flags, mask)) mused.selected_param = p;
	if (p == mused.selected_param)
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -2);
		r.h -= 2;
		r.w -= 2;
		bevel(&r, mused.slider_bevel, BEV_CURSOR);
	}
}


static void inst_text(const SDL_Event *e, const SDL_Rect *area, int p, const char *_label, const char *format, void *value, int width)
{
	check_event(e, area, select_instrument_param, (void*)p, 0, 0);
	
	int d = generic_field(e, area, p, _label, format, value, width);
	if (d) mused.selected_param = p;
	if (d < 0) instrument_add_param(-1);
	else if (d >0) instrument_add_param(1);
	
	if (p == mused.selected_param)
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -2);
		r.h -= 2;
		r.w -= 2;
		bevel(&r, mused.slider_bevel, BEV_CURSOR);
	}
}


static void inst_field(const SDL_Event *e, const SDL_Rect *area, int p, int length, char *text)
{
	console_set_color(mused.console,mused.selected_param == p?0xff0000ff:0xffffffff,CON_CHARACTER);
	console_set_clip(mused.console, area);
	console_clear(mused.console);
	
	bevel(area, mused.slider_bevel, BEV_FIELD);
	
	SDL_Rect field;
	copy_rect(&field, area);
	adjust_rect(&field, 1);
	
	console_set_clip(mused.console, &field);
	
	if (mused.edit_buffer == text && mused.mode == EDITBUFFER && mused.selected_param == p)
	{
		int i = 0;
		for ( ; text[i] && i < length ; ++i)
		{
			if (check_event(e, console_write_args(mused.console, "%c", mused.editpos == i ? '§' : text[i]), NULL, NULL, NULL, NULL))
				mused.editpos = i;
		}
		
		if (mused.editpos == i && i < length) 
			console_write(mused.console, "§"); 
	}
	else
	{
		if (check_event(e, console_write_args(mused.console, "%*s", -length, text), select_instrument_param, (void*)p, 0, 0))
			set_edit_buffer(text, length);
	}
}



void instrument_name_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area,dest);
	area.w = 2 * mused.console->font.w + 2 + 16;
	inst_text(event, &area, P_INSTRUMENT, "", "%02x", (void*)mused.current_instrument, 2);
	area.x += area.w;
	area.w = dest->x + dest->w - area.x;
	inst_field(event, &area, P_NAME, 16, mused.song.instrument[mused.current_instrument].name);
}


void instrument_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	
	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevel(&frame, mused.slider_bevel, BEV_SLIDER_HANDLE);
	adjust_rect(&frame, 4);
	
	{
		copy_rect(&r, &frame);
		SDL_Rect note;
		copy_rect(&note, &frame);
		
		r.w = r.w / 2 - 2;
		r.h = 10;
		r.y += r.h + 1;
		
		note.w = frame.w / 2 + 2;
		note.h = 10;
		
		inst_text(event, &note, P_BASENOTE, "BASE", "%s", notename(inst->base_note), 3);
		note.x += note.w + 2;
		note.y += 1;
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
		inst_flags(event, &r, P_REVERB, "RVB", &inst->cydflags, CYD_CHN_ENABLE_REVERB);
		update_rect(&frame, &r);
	}
	
	{
		int tmp = r.w;
		r.w = frame.w / 3 - 2;
		separator(&frame, &r);
		inst_flags(event, &r, P_PULSE, "PUL", &inst->cydflags, CYD_CHN_ENABLE_PULSE);
		update_rect(&frame, &r);
		r.w = frame.w / 2 - 2;
		inst_text(event, &r, P_PW, "PW", "%03X", (void*)inst->pw, 3);
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
		
		r.w = tmp;
	}
	
	separator(&frame, &r);
	inst_text(event, &r, P_VOLUME, "VOL", "%02X", (void*)inst->volume, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_ATTACK, "ATK", "%02X", (void*)inst->adsr.a, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_DECAY, "DEC", "%02X", (void*)inst->adsr.d, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_SUSTAIN, "SUS", "%02X", (void*)inst->adsr.s, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RELEASE, "REL", "%02X", (void*)inst->adsr.r, 2);
	update_rect(&frame, &r);
	
	separator(&frame, &r);
	inst_flags(event, &r, P_SYNC, "SYNC", &inst->cydflags, CYD_CHN_ENABLE_SYNC);
	update_rect(&frame, &r);
	inst_text(event, &r, P_SYNCSRC, "SRC", "%02X", (void*)inst->sync_source, 2);
	update_rect(&frame, &r);
	inst_flags(event, &r, P_RINGMOD, "RING MOD", &inst->cydflags, CYD_CHN_ENABLE_RING_MODULATION);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RINGMODSRC, "SRC", "%02X", (void*)inst->ring_mod, 2);
	update_rect(&frame, &r);
	
	separator(&frame, &r);
	inst_text(event, &r, P_SLIDESPEED, "SLIDE", "%02X", (void*)inst->slide_speed, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBSPEED,   "V SPD", "%02X", (void*)inst->vibrato_speed, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBDEPTH,   "V DPT", "%02X", (void*)inst->vibrato_depth, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMSPEED,   "PWM SPD", "%02X", (void*)inst->pwm_speed, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMDEPTH,   "PWM DPT", "%02X", (void*)inst->pwm_depth, 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PROGPERIOD, "P PERIOD", "%02X", (void*)inst->prog_period, 2);
	update_rect(&frame, &r);
	
	static const char *flttype[] = { "LP", "HP", "BP" };
	
	separator(&frame, &r);
	inst_flags(event, &r, P_FILTER, "FILTER", &inst->cydflags, CYD_CHN_ENABLE_FILTER);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FLTTYPE, "TYPE", "%s", (char*)flttype[inst->flttype], 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_CUTOFF, "FRQ", "%03X", (void*)inst->cutoff, 3);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RESONANCE, "RES", "%1X", (void*)inst->resonance, 1);
	update_rect(&frame, &r);
	
}


void instrument_list(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(&area, mused.slider_bevel, BEV_THIN_FRAME);
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
			bevel(&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, 0xffffffff, CON_CHARACTER);
		}
		else
		{
			console_set_color(mused.console, 0xffc0c0c0, CON_CHARACTER);
		}
			
		check_event(event, console_write_args(mused.console, "%02X %-16s\n", i, mused.song.instrument[i].name), select_instrument, (void*)i, 0, 0);
		
		slider_set_params(&mused.instrument_list_slider_param, 0, NUM_INSTRUMENTS - 1, start, i, &mused.instrument_list_position, 1, SLIDER_VERTICAL);
	}
}


void reverb_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	console_clear(mused.console);
	
	//separator("----reverb----");
	
	console_set_color(mused.console, mused.edit_reverb_param == R_ENABLE ? 0xff0000ff:0xffffffff,CON_CHARACTER);
	
	//if (checkbox(event, "Enabled\n", &mused.song.flags, MUS_ENABLE_REVERB)) mused.edit_reverb_param = R_ENABLE;
	
	// We need to mirror the reverb flag to the corresponding Cyd flag
	
	if (mused.song.flags & MUS_ENABLE_REVERB)
		mused.cyd.flags |= CYD_ENABLE_REVERB;
	else
		mused.cyd.flags &= ~CYD_ENABLE_REVERB;
	
	int p = R_DELAY;
	
	for (int i = 0 ; i < CYDRVB_TAPS ; ++i)
	{
		if ((i % 3) == 0 && i > 0) console_write(mused.console, "\n");
		
		console_set_color(mused.console,0xffffffff,CON_CHARACTER);
		console_write_args(mused.console, "Tap %x:", i);
	
		console_set_color(mused.console,mused.edit_reverb_param == p ? 0xff0000ff:0xffffffff,CON_CHARACTER);
		console_write_args(mused.console, " %4d ms ", mused.song.rvbtap[i].delay);
		
		++p;
		
		console_set_color(mused.console,mused.edit_reverb_param == p ? 0xff0000ff:0xffffffff,CON_CHARACTER);
		
		if (mused.song.rvbtap[i].gain <= CYDRVB_LOW_LIMIT)
			console_write(mused.console, "- INF dB");
		else
			console_write_args(mused.console, "%+5.1f dB", (double)mused.song.rvbtap[i].gain * 0.1);
		
		++p;
	}
}


void instrument_disk_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevel(&area, mused.slider_bevel, BEV_SLIDER_HANDLE);
	adjust_rect(&area, 1);
	
	SDL_Rect button = { area.x, area.y, 50, area.h };
	
	button_text_event(event, &button, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, "LOAD", open_song_action, NULL, NULL, NULL);
	update_rect(&area, &button);
	button_text_event(event, &button, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, "SAVE", save_song_action, NULL, NULL, NULL);
	update_rect(&area, &button);
}

