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
#include "diskop.h"
#include "gui/mouse.h"
#include "gui/dialog.h"
#include "gui/bevel.h"
#include "theme.h"
#include "mybevdefs.h"
#include "snd/freqs.h"
#include "view/visu.h"
#include "view/sequence.h"
#include <stdbool.h>
#include "edit.h"
#include "mymsg.h"
#include "command.h"
#include <string.h>

extern Mused mused;

extern int event_hit;

/*

Cyd envelope length in milliseconds

*/

#define envelope_length(slope) (slope!=0?(float)(((slope) * (slope) * 256 / (ENVELOPE_SCALE * ENVELOPE_SCALE))) / ((float)CYD_BASE_FREQ / 1000.0f) :0.0f)

float percent_to_dB(float percent)
{
	return 10 * log10(percent);
}

bool is_selected_param(int focus, int p)
{
	if (focus == mused.focus)
	switch (focus)
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

		case EDITSONGINFO:
			return p == mused.songinfo_param;
			break;
	}

	return false;
}


void my_separator(const SDL_Rect *parent, SDL_Rect *rect)
{
	separator(domain, parent, rect, mused.slider_bevel, BEV_SEPARATOR);
}

// note: since we need to handle the focus this piece of code is duplicated from gui/view.c

void my_draw_view(const View* views, const SDL_Event *_event, GfxDomain *domain)
{
	gfx_rect(domain, NULL, colors[COLOR_BACKGROUND]);

	SDL_Event event;
	memcpy(&event, _event, sizeof(event));

	int orig_focus = mused.focus;

	for (int i = 0 ; views[i].handler ; ++i)
	{
		const View *view = &views[i];
		SDL_Rect area;
		area.x = view->position.x >= 0 ? view->position.x : domain->screen_w + view->position.x;
		area.y = view->position.y >= 0 ? view->position.y : domain->screen_h + view->position.y;
		area.w = *(Sint16*)&view->position.w > 0 ? *(Sint16*)&view->position.w : domain->screen_w + *(Sint16*)&view->position.w - view->position.x;
		area.h = *(Sint16*)&view->position.h > 0 ? *(Sint16*)&view->position.h : domain->screen_h + *(Sint16*)&view->position.h - view->position.y;

		memcpy(&mused.console->clip, &area, sizeof(view->position));
		int iter = 0;
		do
		{
			event_hit = 0;
			view->handler(domain, &area, &event, view->param);
			if (event_hit)
			{
				event.type = MSG_EVENTHIT;
				if (view->focus != -1 && mused.focus != view->focus && orig_focus != EDITBUFFER && mused.focus != EDITBUFFER)
				{
					mused.focus = view->focus;
					clear_selection(0,0,0);
				}
				++iter;
			}
		}
		while (event_hit && iter <= 1);

		if (!event_hit && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT
			&& event.button.x >= area.x && event.button.x < area.x + area.w
			&& event.button.y >= area.y && event.button.y < area.y + area.h)
		{
			if (view->focus != -1 && mused.focus != view->focus)
			{
				if (orig_focus == EDITBUFFER)
					change_mode(view->focus);

				mused.focus = view->focus;
				clear_selection(0,0,0);
			}
		}
	}

	mused.cursor.w = (mused.cursor_target.w + mused.cursor.w * 2) / 3;
	mused.cursor.h = (mused.cursor_target.h + mused.cursor.h * 2) / 3;
	mused.cursor.x = (mused.cursor_target.x + mused.cursor.x * 2) / 3;
	mused.cursor.y = (mused.cursor_target.y + mused.cursor.y * 2) / 3;

	if (mused.cursor.w < mused.cursor_target.w) ++mused.cursor.w;
	if (mused.cursor.w > mused.cursor_target.w) --mused.cursor.w;

	if (mused.cursor.h < mused.cursor_target.h) ++mused.cursor.h;
	if (mused.cursor.h > mused.cursor_target.h) --mused.cursor.h;

	if (mused.cursor.x < mused.cursor_target.x) ++mused.cursor.x;
	if (mused.cursor.x > mused.cursor_target.x) --mused.cursor.x;

	if (mused.cursor.y < mused.cursor_target.y) ++mused.cursor.y;
	if (mused.cursor.y > mused.cursor_target.y) --mused.cursor.y;

	if (mused.cursor.w > 0) bevelex(domain, &mused.cursor, mused.slider_bevel, (mused.flags & EDIT_MODE) ? BEV_EDIT_CURSOR : BEV_CURSOR, BEV_F_STRETCH_ALL|BEV_F_DISABLE_CENTER);
}


char * notename(int note)
{
	note = my_min(my_max(0, note), FREQ_TAB_SIZE - 1);
	static char buffer[4];
	static const char * notename[] =
	{
		"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
	};
	sprintf(buffer, "%s%d", notename[note % 12], note / 12);
	return buffer;
}


void label(const char *_label, const SDL_Rect *area)
{
	SDL_Rect r;

	copy_rect(&r, area);

	r.y = r.y + area->h / 2 - mused.smallfont.h / 2;

	font_write(&mused.smallfont, domain, &r, _label);
}


void set_cursor(const SDL_Rect *location)
{
	if (location == NULL)
	{
		mused.cursor_target.w = 0;
		mused.cursor.w = 0;
		return;
	}

	if (mused.flags & ANIMATE_CURSOR)
	{
		copy_rect(&mused.cursor_target, location);

		if (mused.cursor.w == 0 || mused.cursor.h == 0)
			copy_rect(&mused.cursor, location);
	}
	else
	{
		copy_rect(&mused.cursor_target, location);
		copy_rect(&mused.cursor, location);
	}
}


bool check_mouse_hit(const SDL_Event *e, const SDL_Rect *area, int focus, int param)
{
	if (param < 0) return false;
	if (check_event(e, area, NULL, NULL, NULL, NULL))
	{
		switch (focus)
		{
			case EDITINSTRUMENT:
				mused.selected_param = param;
				break;

			case EDITFX:
				mused.edit_reverb_param = param;
				break;

			case EDITWAVETABLE:
				mused.wavetable_param = param;
				break;

			case EDITSONGINFO:
				mused.songinfo_param = param;
				break;
		}

		mused.focus = focus;

		return true;
	}

	return false;
}


int generic_field(const SDL_Event *e, const SDL_Rect *area, int focus, int param, const char *_label, const char *format, void *value, int width)
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

	bevelex(domain, &field, mused.slider_bevel, BEV_FIELD, BEV_F_STRETCH_ALL);

	adjust_rect(&field, 1);

	font_write_args(&mused.largefont, domain, &field, format, value);

	int r =  spinner(domain, e, &spinner_area, mused.slider_bevel, (Uint32)area->x << 16 | area->y);

	check_mouse_hit(e, area, focus, param);

	if (is_selected_param(focus, param))
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -2);

		set_cursor(&r);
	}

	return r * (SDL_GetModState() & KMOD_SHIFT ? 16 : 1);
}


void generic_flags(const SDL_Event *e, const SDL_Rect *_area, int focus, int p, const char *label, Uint32 *_flags, Uint32 mask)
{
	SDL_Rect area;
	copy_rect(&area, _area);
	area.y += 1;

	int hit = check_mouse_hit(e, _area, focus, p);
	Uint32 flags = *_flags;

	if (checkbox(domain, e, &area, mused.slider_bevel, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_TICK, label, &flags, mask))
	{

	}
	else if (hit)
	{
		// so that the gap between the box and label works too
		flags ^= mask;
	}

	if (*_flags != flags)
	{
		switch (focus)
		{
			case EDITINSTRUMENT: snapshot(S_T_INSTRUMENT); break;
			case EDITFX: snapshot(S_T_FX); break;
		}
		*_flags = flags;
	}

	if (is_selected_param(focus, p))
	{
		SDL_Rect r;
		copy_rect(&r, &area);
		adjust_rect(&r, -2);
		r.h -= 2;
		r.w -= 2;
		set_cursor(&r);
	}
}


int generic_button(const SDL_Event *e, const SDL_Rect *area, int focus, int param, const char *_label, void (*action)(void*,void*,void*), void *p1, void *p2, void *p3)
{
	if (is_selected_param(focus, param))
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -2);
		r.h -= 2;
		r.w -= 2;
		set_cursor(&r);
	}

	return button_text_event(domain, e, area, mused.slider_bevel, &mused.smallfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, _label, action, p1, p2, p3);
}


void songinfo1_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);

	area.w = my_min(320, area.w);

	adjust_rect(&area, 2);
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

	int d;

	d = generic_field(event, &r, EDITSONGINFO, SI_LENGTH, "LEN", "%04X", MAKEPTR(mused.song.song_length), 4);
	songinfo_add_param(d);
	update_rect(&area, &r);

	d = generic_field(event, &r, EDITSONGINFO, SI_LOOP, "LOOP","%04X", MAKEPTR(mused.song.loop_point), 4);
	songinfo_add_param(d);
	update_rect(&area, &r);

	d = generic_field(event, &r, EDITSONGINFO, SI_STEP, "STEP","%04X", MAKEPTR(mused.sequenceview_steps), 4);
	songinfo_add_param(d);
	update_rect(&area, &r);
}


void songinfo2_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);

	area.w = my_min(320, area.w);

	adjust_rect(&area, 2);
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

	int d, tmp = r.w;

	r.w -= 26;

	d = generic_field(event, &r, EDITSONGINFO, SI_SPEED1, "SPD","%01X", MAKEPTR(mused.song.song_speed), 1);
	songinfo_add_param(d);

	r.x += r.w;
	r.w = 26;

	d = generic_field(event, &r, EDITSONGINFO, SI_SPEED2, "","%01X", MAKEPTR(mused.song.song_speed2), 1);
	songinfo_add_param(d);
	update_rect(&area, &r);

	r.w = tmp;

	d = generic_field(event, &r, EDITSONGINFO, SI_RATE, "RATE","%4d", MAKEPTR(mused.song.song_rate), 4);
	songinfo_add_param(d);
	update_rect(&area, &r);

	sprintf(speedtext, "%d/%d", mused.time_signature >> 8, mused.time_signature & 0xff);
	d = generic_field(event, &r, EDITSONGINFO, SI_TIME, "TIME","%4s", speedtext, 4);
	songinfo_add_param(d);
	update_rect(&area, &r);
}


void songinfo3_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);

	area.w = my_min(320, area.w);

	adjust_rect(&area, 2);
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

	int d;

	d = generic_field(event, &r, EDITSONGINFO, SI_OCTAVE, "OCTAVE","%02X", MAKEPTR(mused.octave), 2);
	songinfo_add_param(d);
	update_rect(&area, &r);

	d = generic_field(event, &r, EDITSONGINFO, SI_CHANNELS, "CHANLS","%02X", MAKEPTR(mused.song.num_channels), 2);
	songinfo_add_param(d);
	update_rect(&area, &r);
}


void playstop_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);

	area.w = my_min(PLAYSTOP_INFO_W * 2, area.w);

	adjust_rect(&area, 2);
	area.x++;
	area.w -= 2;
	SDL_Rect r;
	copy_rect(&r, &area);
	r.w = area.w;

	r.h = 10;
	console_set_clip(mused.console, &area);

	SDL_Rect button;
	copy_rect(&button, &r);
	button.w = my_max(button.w / 2, PLAYSTOP_INFO_W/2 - 4);
	button_text_event(domain, event, &button, mused.slider_bevel, &mused.buttonfont, (mused.flags & SONG_PLAYING) ? BEV_BUTTON_ACTIVE : BEV_BUTTON, BEV_BUTTON_ACTIVE, "PLAY", play, NULL, NULL, NULL);
	button.x -= ELEMENT_MARGIN;
	update_rect(&area, &button);
	button_text_event(domain, event, &button, mused.slider_bevel, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "STOP", stop, NULL, NULL, NULL);

	r.y = button.y + button.h;

	int d;

	if (mused.mus.cyd->flags & CYD_CLIPPING)
		d = generic_field(event, &r, EDITSONGINFO, SI_MASTERVOL, "\2VOL","%02X", MAKEPTR(mused.song.master_volume), 2);
	else
		d = generic_field(event, &r, EDITSONGINFO, SI_MASTERVOL, "\1VOL","%02X", MAKEPTR(mused.song.master_volume), 2);
	songinfo_add_param(d);
	update_rect(&area, &r);
}


void info_line(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	area.w -= N_VIEWS * dest->h;
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_THIN_FRAME, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);
	console_set_color(mused.console, colors[COLOR_STATUSBAR_TEXT]);

	console_clear(mused.console);

	char text[200]="";

	if (mused.info_message[0] != '\0')
		strncpy(text, mused.info_message, sizeof(text) - 1);
	else
	{
		switch (mused.focus)
		{
			case EDITPROG:
			{
				Uint16 inst = mused.song.instrument[mused.current_instrument].program[mused.current_program_step];
				get_command_desc(text, sizeof(text) - 1, inst);
			}
			break;

			case EDITWAVETABLE:
			{
				static const char * param_desc[] =
				{
					"Wavetable item",
					"Item name",
					"Sample rate",
					"Base note",
					"Base note finetune",
					"Interpolate",
					"Enable looping",
					"Loop begin",
					"Ping-pong looping",
					"Loop end",
					"Number of oscillators",
					"Oscillator type",
					"Frequency multiplier",
					"Phase shift",
					"Phase exponent",
					"Absolute",
					"Negative",
					"Wave length",
					"Randomize & generate",
					"Generate",
					"Randomize",
					"Toolbox"
				};
				strcpy(text, param_desc[mused.wavetable_param]);
			}
			break;

			case EDITINSTRUMENT:
			{
				static const char * param_desc[] =
				{
					"Select instrument",
					"Edit instrument name",
					"Base note",
					"Finetune",
					"Lock to base note",
					"Drum",
					"Sync oscillator on keydown",
					"Reverse vibrato bit",
					"Set PW on keydown",
					"Set cutoff on keydown",
					"Slide speed",
					"Pulse wave",
					"Pulse width",
					"Saw wave",
					"Triangle wave",
					"Noise",
					"Metallic noise",
					"LFSR enable",
					"LFSR type",
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
					"Send signal to FX chain",
					"FX bus",
					"Vibrato speed",
					"Vibrato depth",
					"Vibrato shape",
					"Vibrato delay",
					"Pulse width modulation speed",
					"Pulse width modulation depth",
					"Pulse width modulation shape",
					"Program period",
					"Don't restart program on keydown",
					"Enable multi oscillator",
					"FM enable",
					"FM modulation",
					"FM feedback",
					"FM carrier multiplier",
					"FM modulator multiplier",
					"FM attack",
					"FM decay",
					"FM sustain",
					"FM release",
					"FM env start",
					"FM use wavetable",
					"FM wavetable entry"
				};

				if (mused.selected_param == P_FXBUS)
					snprintf(text, sizeof(text) - 1, "%s (%s)", param_desc[mused.selected_param], mused.song.fx[mused.song.instrument[mused.current_instrument].fx_bus].name);
				else if (mused.selected_param == P_WAVE_ENTRY)
					snprintf(text, sizeof(text) - 1, "%s (%s)", param_desc[mused.selected_param], mused.song.wavetable_names[mused.song.instrument[mused.current_instrument].wavetable_entry]);
				else if (mused.selected_param == P_FM_WAVE_ENTRY)
					snprintf(text, sizeof(text) - 1, "%s (%s)", param_desc[mused.selected_param], mused.song.wavetable_names[mused.song.instrument[mused.current_instrument].fm_wave]);
				else if (mused.selected_param == P_VOLUME)
					snprintf(text, sizeof(text) - 1, "%s (%+.1f dB)", param_desc[mused.selected_param], percent_to_dB((float)mused.song.instrument[mused.current_instrument].volume / MAX_VOLUME));
				else if (mused.selected_param == P_ATTACK)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].adsr.a));
				else if (mused.selected_param == P_DECAY)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].adsr.d));
				else if (mused.selected_param == P_RELEASE)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].adsr.r));
				else if (mused.selected_param == P_FM_ATTACK)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].fm_adsr.a));
				else if (mused.selected_param == P_FM_DECAY)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].fm_adsr.d));
				else if (mused.selected_param == P_FM_RELEASE)
					snprintf(text, sizeof(text) - 1, "%s (%.1f ms)", param_desc[mused.selected_param], envelope_length(mused.song.instrument[mused.current_instrument].fm_adsr.r));
				else
					strcpy(text, param_desc[mused.selected_param]);
			}

			break;

			case EDITFX:
			{
				static const char * param_desc[] =
				{
					"Enable multiplex",
					"Multiplex period",
					"Pitch inaccuracy",
					"FX bus",
					"FX bus name",
					"Enable bitcrusher",
					"Drop bits",
					"Downsample",
					"Dither",
					"Crush gain",
					"Enable stereo chorus",
					"Min. delay",
					"Max. delay",
					"Phase separation",
					"Modulation frequency",
					"Enable reverb",
					"Room size",
					"Reverb volume",
					"Decay",
					"Snap taps to ticks",
					"Tap enabled",
					"Selected tap",
					"Tap delay",
					"Tap gain",
					"Tap panning",
				};

				strcpy(text, param_desc[mused.edit_reverb_param]);
			} break;

			case EDITPATTERN:
				if (get_current_pattern())
				{
					if (mused.current_patternx >= PED_COMMAND1)
					{
						Uint16 inst = mused.song.pattern[current_pattern()].step[current_patternstep()].command;

						if (inst != 0)
							get_command_desc(text, sizeof(text), inst);
						else
							strcpy(text, "Command");
					}
					else if (mused.current_patternx == PED_VOLUME1 || mused.current_patternx == PED_VOLUME2)
					{
						Uint16 vol = mused.song.pattern[current_pattern()].step[current_patternstep()].volume;

						if (vol != MUS_NOTE_NO_VOLUME && vol > MAX_VOLUME)
						{
							switch (vol & 0xf0)
							{
								case MUS_NOTE_VOLUME_FADE_UP:
									strcpy(text, "Fade volume up");
									break;

								case MUS_NOTE_VOLUME_FADE_DN:
									strcpy(text, "Fade volume down");
									break;

								case MUS_NOTE_VOLUME_PAN_LEFT:
									strcpy(text, "Pan left");
									break;

								case MUS_NOTE_VOLUME_PAN_RIGHT:
									strcpy(text, "Pan right");
									break;

								case MUS_NOTE_VOLUME_SET_PAN:
									strcpy(text, "Set panning");
									break;
							}
						}
						else if (vol == MUS_NOTE_NO_VOLUME)
							strcpy(text, "Volume");
						else
							sprintf(text, "Volume (%+.1f dB)", percent_to_dB((float)vol / MAX_VOLUME));
					}
					else
					{
						static const char *pattern_txt[] =
						{
							"Note", "Instrument", "Instrument", "", "", "Legato", "Slide", "Vibrato"
						};

						strcpy(text, pattern_txt[mused.current_patternx]);
					}
				}
			break;
		}
	}

	console_write(mused.console,text);

	SDL_Rect button = { dest->x + area.w + 6, dest->y, dest->h, dest->h };

	for (int i = 0 ; i < N_VIEWS ; ++i)
	{
		button_event(domain, event, &button, mused.slider_bevel,
			(mused.mode != i) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
			(mused.mode != i) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
			DECAL_MODE_PATTERN + i + (mused.mode == i ? DECAL_MODE_PATTERN_SELECTED -  DECAL_MODE_PATTERN : 0), (mused.mode != i) ? change_mode_action : NULL, (mused.mode != i) ? MAKEPTR(i) : 0, 0, 0);

		button.x += button.w;
	}
}


static void write_command(const SDL_Event *event, const char *text, int cmd_idx, int cur_idx)
{
	int i = 0;

	for (const char *c = text ; *c ; ++c, ++i)
	{
		const SDL_Rect *r;
		check_event(event, r = console_write_args(mused.console, "%c", *c),
			select_program_step, MAKEPTR(cmd_idx), MAKEPTR(i), 0);

		if (mused.focus == EDITPROG && mused.editpos == i && cmd_idx == cur_idx)
		{
			SDL_Rect cur;
			copy_rect(&cur, r);
			adjust_rect(&cur, -2);
			set_cursor(&cur);
		}
	}
}


void program_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area, clip;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_THIN_FRAME, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 2);
	copy_rect(&clip, &area);
	adjust_rect(&area, 1);
	area.w = 1000;
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

	gfx_domain_set_clip(domain, &clip);

	for (int i = start, s = 0, y = 0 ; i < MUS_PROG_LEN && y < area.h; ++i, ++s, y += mused.console->font.h)
	{
		SDL_Rect row = { area.x - 1, area.y + y - 1, area.w + 2, mused.console->font.h + 1};

		if (mused.current_program_step == i && mused.focus == EDITPROG)
		{
			bevel(domain,&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_PROGRAM_SELECTED]);
		}
		else
			console_set_color(mused.console,pos & 1 ? colors[COLOR_PROGRAM_ODD] : colors[COLOR_PROGRAM_EVEN]);

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

		if (!is_valid_command(inst->program[i]))
			console_write_args(mused.console, "???");
		else if ((inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO || (inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO_ABS)
		{
			if ((inst->program[i] & 0xff) != 0xf0 && (inst->program[i] & 0xff) != 0xf1)
				console_write_args(mused.console, "%s", notename(((inst->program[i] & 0x7f00) == MUS_FX_ARPEGGIO_ABS ? 0 : inst->base_note) + (inst->program[i] & 0xff)));
			else
				console_write_args(mused.console, "EXT%x", inst->program[i] & 0x0f);
		}
		else if (inst->program[i] != MUS_FX_NOP)
		{
			const InstructionDesc *d = get_instruction_desc(inst->program[i]);
			if (d)
				console_write(mused.console, d->shortname ? d->shortname : d->name);
		}

		console_write_args(mused.console, "\n");

		if (row.y + row.h < area.y + area.h)
			slider_set_params(&mused.program_slider_param, 0, MUS_PROG_LEN - 1, start, i, &mused.program_position, 1, SLIDER_VERTICAL, mused.slider_bevel);

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
		adjust_rect(&selection, -3);
		bevel(domain,&selection, mused.slider_bevel, BEV_SELECTION);
	}

	gfx_domain_set_clip(domain, NULL);

	if (mused.focus == EDITPROG)
		check_mouse_wheel_event(event, dest, &mused.program_slider_param);
}


static void inst_flags(const SDL_Event *e, const SDL_Rect *_area, int p, const char *label, Uint32 *flags, Uint32 mask)
{
	generic_flags(e, _area, EDITINSTRUMENT, p, label, flags, mask);
}


static void inst_text(const SDL_Event *e, const SDL_Rect *area, int p, const char *_label, const char *format, void *value, int width)
{
	//check_event(e, area, select_instrument_param, (void*)p, 0, 0);

	int d = generic_field(e, area, EDITINSTRUMENT, p, _label, format, value, width);
	if (d)
	{
		if (p >= 0) mused.selected_param = p;
		if (p != P_INSTRUMENT) snapshot_cascade(S_T_INSTRUMENT, mused.current_instrument, p);
		if (d < 0) instrument_add_param(-1);
		else if (d >0) instrument_add_param(1);
	}

	/*if (p == mused.selected_param && mused.focus == EDITINSTRUMENT)
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -1);
		bevel(domain,&r, mused.slider_bevel, BEV_CURSOR);
	}*/
}


void inst_field(const SDL_Event *e, const SDL_Rect *area, int p, int length, char *text)
{
	console_set_color(mused.console,colors[COLOR_MAIN_TEXT]);
	console_set_clip(mused.console, area);
	console_clear(mused.console);

	bevelex(domain,area, mused.slider_bevel, BEV_FIELD, BEV_F_STRETCH_ALL);

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
			const SDL_Rect *r = console_write_args(mused.console, "%c", mused.editpos == i ? '½' : text[i]);
			if (check_event(e, r, NULL, NULL, NULL, NULL))
			{
				mused.editpos = i;
				got_pos = 1;
			}
		}

		if (mused.editpos == i && c <= length)
			console_write(mused.console, "ï¿½");
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

		if (text == mused.song.title)
			snapshot(S_T_SONGINFO);
		else if (text == mused.song.instrument[mused.current_instrument].name)
			snapshot(S_T_INSTRUMENT);
		else if (text == mused.song.wavetable_names[mused.selected_wavetable])
			snapshot(S_T_WAVE_NAME);
	}

	if (!c && mused.focus == EDITBUFFER && e->type == SDL_MOUSEBUTTONDOWN) change_mode(mused.prev_mode);
}



void instrument_name_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
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

	if (is_selected_param(EDITINSTRUMENT, P_NAME) || (mused.selected_param == P_NAME && mused.mode == EDITINSTRUMENT && (mused.edit_buffer == mused.song.instrument[mused.current_instrument].name && mused.focus == EDITBUFFER)))
	{
		SDL_Rect r;
		copy_rect(&r, &tarea);
		adjust_rect(&r, -2);
		set_cursor(&r);
	}
}


void instrument_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];

	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevelex(domain,&frame, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);
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
		note.w = frame.w / 3;
		inst_text(event, &note, P_FINETUNE, "", "%+4d", MAKEPTR(inst->finetune), 4);
		note.x += note.w + 2;
		note.w = frame.w - note.x;

		inst_flags(event, &note, P_LOCKNOTE, "L", &inst->flags, MUS_INST_LOCK_NOTE);
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

		inst_text(event, &r, P_SLIDESPEED, "SLIDE", "%02X", MAKEPTR(inst->slide_speed), 2);
		update_rect(&frame, &r);
	}

	{
		int tmp = r.w;
		r.w = frame.w / 3 - 2 - 12;
		my_separator(&frame, &r);
		inst_flags(event, &r, P_PULSE, "PUL", &inst->cydflags, CYD_CHN_ENABLE_PULSE);
		update_rect(&frame, &r);
		r.w = frame.w / 2 - 2 - 25;
		inst_text(event, &r, P_PW, "", "%03X", MAKEPTR(inst->pw), 3);
		update_rect(&frame, &r);
		r.w = frame.w / 3 - 8;
		inst_flags(event, &r, P_SAW, "SAW", &inst->cydflags, CYD_CHN_ENABLE_SAW);
		update_rect(&frame, &r);
		r.w = frame.w / 3 - 8;
		inst_flags(event, &r, P_TRIANGLE, "TRI", &inst->cydflags, CYD_CHN_ENABLE_TRIANGLE);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_NOISE, "NOI", &inst->cydflags, CYD_CHN_ENABLE_NOISE);
		update_rect(&frame, &r);
		r.w = frame.w / 3;
		inst_flags(event, &r, P_METAL, "METAL", &inst->cydflags, CYD_CHN_ENABLE_METAL);
		update_rect(&frame, &r);
		inst_flags(event, &r, P_LFSR, "POKEY", &inst->cydflags, CYD_CHN_ENABLE_LFSR);
		update_rect(&frame, &r);
		r.w = frame.w / 3 - 16;
		inst_text(event, &r, P_LFSRTYPE, "", "%X", MAKEPTR(inst->lfsr_type), 1);
		update_rect(&frame, &r);
		r.w = frame.w / 3 - 2;
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
		inst_text(event, &r, P_BUZZ_SHAPE, "SHAPE", "%c", MAKEPTR(inst->ym_env_shape + 0xf0), 1);
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
	inst_text(event, &r, P_CUTOFF, "CUT", "%03X", MAKEPTR(inst->cutoff), 3);
	update_rect(&frame, &r);
	inst_text(event, &r, P_RESONANCE, "RES", "%1X", MAKEPTR(inst->resonance), 1);
	update_rect(&frame, &r);
}


void instrument_view2(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];

	SDL_Rect r, frame;
	copy_rect(&frame, dest);
	bevelex(domain,&frame, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);
	adjust_rect(&frame, 4);
	copy_rect(&r, &frame);
	r.w = r.w / 2 - 2;
	r.h = 10;

	inst_flags(event, &r, P_FX, "FX", &inst->cydflags, CYD_CHN_ENABLE_FX);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FXBUS,   "FXBUS", "%02X", MAKEPTR(inst->fx_bus), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBSPEED,   "VIB.S", "%02X", MAKEPTR(inst->vibrato_speed), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBDEPTH,   "VIB.D", "%02X", MAKEPTR(inst->vibrato_depth), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBSHAPE,   "VIB.SH", "%c", MAKEPTR(inst->vib_shape + 0xf4), 1);
	update_rect(&frame, &r);
	inst_text(event, &r, P_VIBDELAY,   "V.DEL", "%02X", MAKEPTR(inst->vib_delay), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMSPEED,   "PWM.S", "%02X", MAKEPTR(inst->pwm_speed), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMDEPTH,   "PWM.D", "%02X", MAKEPTR(inst->pwm_depth), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PWMSHAPE,   "PWM.SH", "%c", MAKEPTR(inst->pwm_shape + 0xf4), 1);
	update_rect(&frame, &r);
	inst_text(event, &r, P_PROGPERIOD, "P.PRD", "%02X", MAKEPTR(inst->prog_period), 2);
	update_rect(&frame, &r);
	inst_flags(event, &r, P_NORESTART, "NO RESTART", &inst->flags, MUS_INST_NO_PROG_RESTART);
	update_rect(&frame, &r);
	inst_flags(event, &r, P_MULTIOSC, "MULTIOSC", &inst->flags, MUS_INST_MULTIOSC);
	update_rect(&frame, &r);
	my_separator(&frame, &r);
	inst_flags(event, &r, P_FM_ENABLE, "FM", &inst->cydflags, CYD_CHN_ENABLE_FM);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_MODULATION, "VOL", "%02X", MAKEPTR(inst->fm_modulation), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_FEEDBACK, "FEEDBACK", "%01X", MAKEPTR(inst->fm_feedback), 1);
	update_rect(&frame, &r);
	int tmp = r.w;
	r.w -= 27;
	inst_text(event, &r, P_FM_HARMONIC_CARRIER, "MUL", "%01X", MAKEPTR(inst->fm_harmonic >> 4), 1);
	r.x += r.w + 11;
	r.w = 16;

	inst_text(event, &r, P_FM_HARMONIC_MODULATOR, "", "%01X", MAKEPTR(inst->fm_harmonic & 15), 1);
	update_rect(&frame, &r);
	r.w = tmp;
	inst_text(event, &r, P_FM_ATTACK, "ATK", "%02X", MAKEPTR(inst->fm_adsr.a), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_DECAY, "DEC", "%02X", MAKEPTR(inst->fm_adsr.d), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_SUSTAIN, "SUS", "%02X", MAKEPTR(inst->fm_adsr.s), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_RELEASE, "REL", "%02X", MAKEPTR(inst->fm_adsr.r), 2);
	update_rect(&frame, &r);
	inst_text(event, &r, P_FM_ENV_START, "E.START", "%02X", MAKEPTR(inst->fm_attack_start), 2);
	update_rect(&frame, &r);
	tmp = r.w;
	r.w = 42;
	inst_flags(event, &r, P_FM_WAVE, "WAVE", &inst->fm_flags, CYD_FM_ENABLE_WAVE);
	r.x += 44;
	r.w = 32;
	inst_text(event, &r, P_FM_WAVE_ENTRY, "", "%02X", MAKEPTR(inst->fm_wave), 2);
	update_rect(&frame, &r);
}



void instrument_list(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_THIN_FRAME, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 3);
	console_set_clip(mused.console, &area);

	int y = area.y;

	//separator("----instruments----");

	int start = mused.instrument_list_position;

	/*if (start > NUM_INSTRUMENTS - rows ) start = NUM_INSTRUMENTS - rows;
	if (start < 0 ) start = 0;*/

	for (int i = start ; i < NUM_INSTRUMENTS && y < area.h + area.y ; ++i, y += mused.console->font.h)
	{
		SDL_Rect row = { area.x - 1, y - 1, area.w + 2, mused.console->font.h + 1};

		if (i == mused.current_instrument)
		{
			bevel(domain,&row, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_SELECTED]);
		}
		else
		{
			console_set_color(mused.console, colors[COLOR_INSTRUMENT_NORMAL]);
		}

		char temp[sizeof(mused.song.instrument[i].name) + 1];

		strcpy(temp, mused.song.instrument[i].name);
		temp[my_min(sizeof(mused.song.instrument[i].name), my_max(0, area.w / mused.console->font.w - 3))] = '\0';

		console_write_args(mused.console, "%02X %-16s\n", i, temp);

		check_event(event, &row, select_instrument, MAKEPTR(i), 0, 0);

		slider_set_params(&mused.instrument_list_slider_param, 0, NUM_INSTRUMENTS - 1, start, i, &mused.instrument_list_position, 1, SLIDER_VERTICAL, mused.slider_bevel);
	}

	if (mused.focus == EDITINSTRUMENT)
		check_mouse_wheel_event(event, dest, &mused.instrument_list_slider_param);
}


static void fx_text(const SDL_Event *e, const SDL_Rect *area, int p, const char *_label, const char *format, void *value, int width)
{
	//check_event(e, area, select_instrument_param, (void*)p, 0, 0);

	int d = generic_field(e, area, EDITFX, p, _label, format, value, width);
	if (d)
	{
		if (p >= 0) mused.selected_param = p;
		if (p != R_FX_BUS) snapshot_cascade(S_T_FX, mused.fx_bus, p);
		if (d < 0) mused.fx_bus = my_max(0, mused.fx_bus - 1);
		else if (d >0) mused.fx_bus = my_min(CYD_MAX_FX_CHANNELS -1, mused.fx_bus + 1);
	}

	/*if (p == mused.selected_param && mused.focus == EDITINSTRUMENT)
	{
		SDL_Rect r;
		copy_rect(&r, area);
		adjust_rect(&r, -1);
		bevel(domain,&r, mused.slider_bevel, BEV_CURSOR);
	}*/
}


void fx_name_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect farea, larea, tarea;
	copy_rect(&farea,dest);
	copy_rect(&larea,dest);
	copy_rect(&tarea,dest);

	farea.w = 2 * mused.console->font.w + 2 + 16;

	larea.w = 16;

	label("FX", &larea);

	tarea.w = dest->w - farea.w - larea.w - 1;
	farea.x = larea.w + dest->x;
	tarea.x = farea.x + farea.w;

	fx_text(event, &farea, R_FX_BUS, "FX", "%02X", MAKEPTR(mused.fx_bus), 2);
	inst_field(event, &tarea, R_FX_BUS_NAME, sizeof(mused.song.fx[mused.fx_bus].name), mused.song.fx[mused.fx_bus].name);

	if (is_selected_param(EDITFX, R_FX_BUS_NAME) || (mused.mode == EDITFX && (mused.edit_buffer == mused.song.fx[mused.fx_bus].name && mused.focus == EDITBUFFER)))
	{
		SDL_Rect r;
		copy_rect(&r, &tarea);
		adjust_rect(&r, -2);
		set_cursor(&r);
	}
}


void fx_global_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 2);
	console_set_clip(mused.console, &area);
	SDL_Rect r;
	copy_rect(&r, &area);
	r.x += 2;
	r.h = 10;

	r.w = 96;

	generic_flags(event, &r, EDITFX, R_MULTIPLEX, "MULTIPLEX", &mused.song.flags, MUS_ENABLE_MULTIPLEX);
	update_rect(&area, &r);

	int d;

	r.x = 100;
	r.w = 80;

	if ((d = generic_field(event, &r, EDITFX, R_MULTIPLEX_PERIOD, "PERIOD", "%2X", MAKEPTR(mused.song.multiplex_period), 2)))
	{
		mused.edit_reverb_param = R_MULTIPLEX_PERIOD;
		fx_add_param(d);
	}

	update_rect(&area, &r);

	r.x += 8;
	r.w = 120;

	if ((d = generic_field(event, &r, EDITFX, R_PITCH_INACCURACY, "INACCURACY", "%2d", MAKEPTR(mused.song.pitch_inaccuracy), 2)))
	{
		mused.edit_reverb_param = R_PITCH_INACCURACY;
		fx_add_param(d);
	}
}


void fx_reverb_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain, &area, mused.slider_bevel, BEV_THIN_FRAME, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 4);
	console_set_clip(mused.console, &area);

	int c = 0;

	int row_ms = (1000 / mused.song.song_rate) * mused.song.song_speed;
	int row_ms2 = (1000 / mused.song.song_rate) * mused.song.song_speed2;

	for (int ms = 0 ; ms < CYDRVB_SIZE ; c++)
	{
		SDL_Rect r = { area.x + ms * area.w / CYDRVB_SIZE, area.y, 1, area.h};

		if (ms > 0)
		{
			Uint32 color = timesig(c, colors[COLOR_PATTERN_BAR], colors[COLOR_PATTERN_BEAT], colors[COLOR_PATTERN_NORMAL]);

			gfx_rect(dest_surface, &r, color);
		}

		if (timesig(c, 1, 1, 0))
		{
			SDL_Rect text = { r.x + 2, r.y + r.h - mused.smallfont.h, 16, mused.smallfont.h};
			font_write_args(&mused.smallfont, domain, &text, "%d", c);
		}

		if (c & 1)
			ms += row_ms2;
		else
			ms += row_ms;
	}

	c = 0;

	if (mused.fx_axis == 0)
	{
		for (int db = 0 ; db < -CYDRVB_LOW_LIMIT ; db += 100, c++)
		{
			Uint32 color = colors[COLOR_PATTERN_BAR];
			if (c & 1)
				color = colors[COLOR_PATTERN_BEAT];

			SDL_Rect r = { area.x, area.y + db * area.h / -CYDRVB_LOW_LIMIT, area.w, 1};

			if (!(c & 1))
			{
				SDL_Rect text = { r.x + r.w - 40, r.y + 2, 40, 8};
				font_write_args(&mused.smallfont, domain, &text, "%3d dB", -db / 10);
			}

			if (db != 0)
				gfx_rect(dest_surface, &r, color);
		}
	}
	else
	{
		for (int pan = CYD_PAN_LEFT ; pan < CYD_PAN_RIGHT ; pan += CYD_PAN_CENTER / 2, c++)
		{
			Uint32 color = colors[COLOR_PATTERN_BAR];
			if (c & 1)
				color = colors[COLOR_PATTERN_BEAT];

			SDL_Rect r = { area.x, area.y + pan * area.h / CYD_PAN_RIGHT, area.w, 1};

			if (pan != 0)
				gfx_rect(dest_surface, &r, color);
		}

		{
			SDL_Rect text = { area.x + area.w - 6, area.y + 4, 8, 8};
			font_write(&mused.smallfont, domain, &text, "L");
		}
		{
			SDL_Rect text = { area.x + area.w - 6, area.y + area.h - 16, 8, 8};
			font_write(&mused.smallfont, domain, &text, "R");
		}
	}

	for (int i = 0 ; i < CYDRVB_TAPS ; ++i)
	{
		int h;

		if (mused.fx_axis == 0)
			h = mused.song.fx[mused.fx_bus].rvb.tap[i].gain * area.h / CYDRVB_LOW_LIMIT;
		else
			h = mused.song.fx[mused.fx_bus].rvb.tap[i].panning * area.h / CYD_PAN_RIGHT;

		SDL_Rect r = { area.x + mused.song.fx[mused.fx_bus].rvb.tap[i].delay * area.w / CYDRVB_SIZE - mused.smallfont.w / 2,
			area.y + h - mused.smallfont.h / 2, mused.smallfont.w, mused.smallfont.h};

		if (mused.song.fx[mused.fx_bus].rvb.tap[i].flags & 1)
			font_write(&mused.smallfont, dest_surface, &r, "\2");
		else
			font_write(&mused.smallfont, dest_surface, &r, "\1");

		if (i == mused.fx_tap)
		{
			SDL_Rect sel;
			copy_rect(&sel, &r);
			adjust_rect(&sel, -4);
			bevelex(domain, &sel, mused.slider_bevel, BEV_CURSOR, BEV_F_STRETCH_ALL);
		}

		if (event->type == SDL_MOUSEBUTTONDOWN)
		{
			if (check_event(event, &r, NULL, 0, 0, 0))
			{
				mused.fx_tap = i;

				if (SDL_GetModState() & KMOD_SHIFT)
				{
					mused.song.fx[mused.fx_bus].rvb.tap[i].flags ^= 1;
				}
			}
		}
	}

	int mx, my;

	if (mused.mode == EDITFX && (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(1)))
	{
		mx /= mused.pixel_scale;
		my /= mused.pixel_scale;

		if (mx >= area.x && mx < area.x + area.w && my > area.y && my < area.y + area.h)
		{
			if (mused.fx_room_prev_x != -1)
			{
				snapshot_cascade(S_T_FX, mused.fx_bus, mused.fx_tap);

				mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].delay += (mx - mused.fx_room_prev_x) * CYDRVB_SIZE / area.w;

				if (mused.fx_axis == 0)
				{
					mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain += (my - mused.fx_room_prev_y) * CYDRVB_LOW_LIMIT / area.h;
					if (mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain > 0)
						mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain = 0;
					else if (mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain < CYDRVB_LOW_LIMIT)
						mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain = CYDRVB_LOW_LIMIT;
				}
				else
				{
					mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning += (my - mused.fx_room_prev_y) * CYD_PAN_RIGHT / area.h;

					if (mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning < CYD_PAN_LEFT)
						mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning = CYD_PAN_LEFT;

					if (mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning > CYD_PAN_RIGHT)
						mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning = CYD_PAN_RIGHT;
				}
			}

			mused.fx_room_prev_x = mx;
			mused.fx_room_prev_y = my;
		}
	}
	else
	{
		mused.fx_room_prev_x = -1;
		mused.fx_room_prev_y = -1;
	}
}


void fx_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 4);
	console_set_clip(mused.console, &area);
	SDL_Rect r;
	copy_rect(&r, &area);

	int d;

	r.h = 10;
	r.w = 48;

	generic_flags(event, &r, EDITFX, R_CRUSH, "CRUSH", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CRUSH);
	update_rect(&area, &r);

	r.x = 56;
	r.w = 56;

	if ((d = generic_field(event, &r, EDITFX, R_CRUSHBITS, "BITS", "%01X", MAKEPTR(mused.song.fx[mused.fx_bus].crush.bit_drop), 1)))
	{
		fx_add_param(d);
	}

	update_rect(&area, &r);

	r.w = 64;

	if ((d = generic_field(event, &r, EDITFX, R_CRUSHDOWNSAMPLE, "DSMP", "%02d", MAKEPTR(mused.song.fx[mused.fx_bus].crushex.downsample), 2)))
	{
		fx_add_param(d);
	}

	update_rect(&area, &r);

	generic_flags(event, &r, EDITFX, R_CRUSHDITHER, "DITHER", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CRUSH_DITHER);
	update_rect(&area, &r);

	r.w = 56;

	if ((d = generic_field(event, &r, EDITFX, R_CRUSHGAIN, "VOL", "%02X", MAKEPTR(mused.song.fx[mused.fx_bus].crushex.gain), 2)))
	{
		fx_add_param(d);
	}
	update_rect(&area, &r);

	my_separator(&area, &r);

	r.w = 60;

	generic_flags(event, &r, EDITFX, R_CHORUS, "STEREO", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_CHORUS);

	update_rect(&area, &r);

	{
		// hacky-hack, we need different addresses for the different fields
		// because the address of the string is used as an ad-hoc identifier for
		// the fields...

		char temp1[10], temp2[10], temp3[10];

		sprintf(temp1, "%4.1f ms", (float)mused.song.fx[mused.fx_bus].chr.min_delay / 10);

		r.x = 100;
		r.w = 104;

		if ((d = generic_field(event, &r, EDITFX, R_MINDELAY, "MIN", temp1, NULL, 7)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		sprintf(temp2, "%4.1f ms", (float)mused.song.fx[mused.fx_bus].chr.max_delay / 10);

		if ((d = generic_field(event, &r, EDITFX, R_MAXDELAY, "MAX", temp2, NULL, 7)))
		{
			fx_add_param(d);
		}

		r.x = 100;
		r.y += r.h;

		if ((d = generic_field(event, &r, EDITFX, R_SEPARATION, "PHASE", "%02X", MAKEPTR(mused.song.fx[mused.fx_bus].chr.sep), 2)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		if (mused.song.fx[mused.fx_bus].chr.rate != 0)
			sprintf(temp3, "%5.2f Hz", (((float)(mused.song.fx[mused.fx_bus].chr.rate - 1) + 10) / 40));
		else
			strcpy(temp3, "OFF");

		if ((d = generic_field(event, &r, EDITFX, R_RATE, "MOD", temp3, NULL, 8)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);
	}

	my_separator(&area, &r);

	generic_flags(event, &r, EDITFX, R_ENABLE, "REVERB", &mused.song.fx[mused.fx_bus].flags, CYDFX_ENABLE_REVERB);

	update_rect(&area, &r);

	r.w = 80;

	r.x = 4;
	r.y += r.h;

	{
		r.x = 130;
		r.y -= r.h;

		int tmp = r.w;
		r.w = 60 + 32;

		if ((d = generic_field(event, &r, EDITFX, R_ROOMSIZE, "ROOMSIZE", "%02X", MAKEPTR(mused.fx_room_size), 2)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		r.w = 320 - 8 - r.x;

		if ((d = generic_field(event, &r, EDITFX, R_ROOMVOL, "VOLUME", "%02X", MAKEPTR(mused.fx_room_vol), 2)))
		{
			fx_add_param(d);
		}

		r.x = 130;
		r.y += r.h;

		r.w = 60+32;

		if ((d = generic_field(event, &r, EDITFX, R_ROOMDECAY, "DECAY", "%d", MAKEPTR(mused.fx_room_dec), 1)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		r.w = 41;

		generic_flags(event, &r, EDITFX, R_SNAPTICKS, "SNAP", (Uint32*)&mused.fx_room_ticks, 1);

		update_rect(&area, &r);

		if (button_text_event(domain, event, &r, mused.slider_bevel, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "SET", NULL, NULL, NULL, NULL) & 1)
		{
			set_room_size(mused.fx_bus, mused.fx_room_size, mused.fx_room_vol, mused.fx_room_dec);
		}

		update_rect(&area, &r);

		r.w = tmp;
	}

	{
		my_separator(&area, &r);

		char value[20];
		int d;

		r.w = 32;
		r.h = 10;

		Uint32 _flags = mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].flags;

		generic_flags(event, &r, EDITFX, R_TAPENABLE, "TAP", &_flags, 1);

		mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].flags = _flags;

		update_rect(&area, &r);

		r.w = 32;

		if ((d = generic_field(event, &r, EDITFX, R_TAP, "", "%02d", MAKEPTR(mused.fx_tap), 2)))
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		r.w = 80;

		if (mused.flags & SHOW_DELAY_IN_TICKS)
		{
			char tmp[10];
			float ticks = (float)mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].delay / (1000 / (float)mused.song.song_rate);
			snprintf(tmp, sizeof(tmp), "%5.2f", ticks);
			d = generic_field(event, &r, EDITFX, R_DELAY, "", "%s t", tmp, 7);
		}
		else
		{
			d = generic_field(event, &r, EDITFX, R_DELAY, "", "%4d ms", MAKEPTR(mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].delay), 7);
		}

		if (d)
		{
			fx_add_param(d);
		}

		update_rect(&area, &r);

		r.w = 80;

		if (mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain <= CYDRVB_LOW_LIMIT)
			strcpy(value, "- INF");
		else
			sprintf(value, "%+5.1f", (double)mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].gain * 0.1);

		if ((d = generic_field(event, &r, EDITFX, R_GAIN, "", "%s dB", value, 8)))
		{
			fx_add_param(d);
		}

		r.x += r.w + 4;

		r.w = 32;

		int panning = (int)mused.song.fx[mused.fx_bus].rvb.tap[mused.fx_tap].panning - CYD_PAN_CENTER;
		char tmp[10];

		if (panning != 0)
			snprintf(tmp, sizeof(tmp), "%c%X", panning < 0 ? '\xf9' : '\xfa', panning == 63 ? 8 : ((abs((int)panning) >> 3) & 0xf));
		else
			strcpy(tmp, "\xfa\xf9");

		if ((d = generic_field(event, &r, EDITFX, R_PANNING, "", "%s", tmp, 2)))
		{
			fx_add_param(d);
		}

		r.x += r.w + 4;

		r.w = 32;

		if (button_text_event(domain, event, &r, mused.slider_bevel, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, mused.fx_axis == 0 ? "GAIN" : "PAN", NULL, NULL, NULL, NULL) & 1)
		{
			mused.fx_axis ^= 1;
		}

		r.y += r.h + 4;
		r.h = area.h - r.y + area.y;
		r.w = area.w;
		r.x = area.x;

		fx_reverb_view(dest_surface, &r, event, param);
	}

	mirror_flags();
}


void instrument_disk_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect area;
	copy_rect(&area, dest);
	console_set_clip(mused.console, &area);
	console_clear(mused.console);
	bevelex(domain,&area, mused.slider_bevel, BEV_BACKGROUND, BEV_F_STRETCH_ALL);
	adjust_rect(&area, 2);

	SDL_Rect button = { area.x + 2, area.y, area.w / 2 - 4, area.h };

	int open = button_text_event(domain, event, &button, mused.slider_bevel, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "LOAD", NULL, MAKEPTR(1), NULL, NULL);
	update_rect(&area, &button);
	int save = button_text_event(domain, event, &button, mused.slider_bevel, &mused.buttonfont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "SAVE", NULL, MAKEPTR(2), NULL, NULL);
	update_rect(&area, &button);

	if (open & 1) open_data(param, MAKEPTR(OD_A_OPEN), 0);
	else if (save & 1) open_data(param, MAKEPTR(OD_A_SAVE), 0);
}


void song_name_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
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


void bevel_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	bevelex(domain,dest, mused.slider_bevel, CASTPTR(int,param), BEV_F_STRETCH_ALL);
}


void sequence_spectrum_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	if (mused.flags & SHOW_LOGO)
	{
		if (mused.logo != NULL)
		{
			SDL_Rect a;
			copy_rect(&a, dest);
			a.w += SCROLLBAR;
			gfx_rect(dest_surface, &a, colors[COLOR_BACKGROUND]);
			SDL_Rect d, s = {0,0,a.w,a.h};
			gfx_domain_set_clip(domain, &a);
			copy_rect(&d, &a);
			d.x = d.w / 2 - mused.logo->surface->w / 2 + d.x;
			d.w = mused.logo->surface->w;
			s.w = mused.logo->surface->w;
			my_BlitSurface(mused.logo, &s, dest_surface, &d);
			gfx_domain_set_clip(domain, NULL);
			if (check_event(event, &a, NULL, NULL, NULL, NULL))
				mused.flags &= ~SHOW_LOGO;
		}
		else
		{
			mused.flags &= ~SHOW_LOGO;
		}
	}
	else if (mused.flags & SHOW_ANALYZER)
	{
		SDL_Rect a;
		copy_rect(&a, dest);
		a.w += SCROLLBAR;
		gfx_domain_set_clip(dest_surface, &a);

		check_event(event, &a, toggle_visualizer, NULL, NULL, NULL);

		switch (mused.current_visualizer)
		{
			default:
			case VIS_SPECTRUM:
				spectrum_analyzer_view(dest_surface, &a, event, param);
				break;

			case VIS_CATOMETER:
				catometer_view(dest_surface, &a, event, param);
				break;
		}

		gfx_domain_set_clip(domain, NULL);
	}
	else
	{
		sequence_view2(dest_surface, dest, event, param);
	}
}


void toolbar_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	SDL_Rect button;
	copy_rect(&button, dest);
	button.w = dest->w - 3 * (dest->h + 2);

	button_text_event(domain, event, &button, mused.slider_bevel, &mused.buttonfont,
		BEV_BUTTON, BEV_BUTTON_ACTIVE, 	"MENU", open_menu_action, 0, 0, 0);

	button.x += button.w;
	button.w = button.h + 2;

	if (button_event(domain, event, &button, mused.slider_bevel,
		!(mused.flags & SHOW_ANALYZER) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
		!(mused.flags & SHOW_ANALYZER) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
		DECAL_TOOLBAR_VISUALIZATIONS, flip_bit_action, &mused.flags, MAKEPTR(SHOW_ANALYZER), 0))
			mused.cursor.w = mused.cursor_target.w = 0;

	button.x += button.w;

	if (button_event(domain, event, &button, mused.slider_bevel,
		!(mused.flags & FULLSCREEN) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
		!(mused.flags & FULLSCREEN) ? BEV_BUTTON : BEV_BUTTON_ACTIVE,
		DECAL_TOOLBAR_FULLSCREEN, NULL, 0, 0, 0))
	{
		toggle_fullscreen(0,0,0);
		return; // dest_surface is now invalid
	}

	button.x += button.w;

	button_event(domain, event, &button, mused.slider_bevel,
		BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_TOOLBAR_QUIT, quit_action, 0, 0, 0);

	button.x += button.w;
}
