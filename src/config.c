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

#include "config.h"
#include "mused.h"
#include "gui/toolutil.h"
#include "gui/filebox.h"
#include "action.h"
#include "gfx/gfx.h"
#include <strings.h>

extern Mused mused;
extern GfxDomain *domain;

enum { C_END, C_BOOL, C_STR, C_INT };

static const struct { int type; const char *name; void *param; int mask; } confitem[] =
{
	{ C_BOOL, "fullscreen", &mused.flags, FULLSCREEN },
	{ C_INT, "pixel_size", &mused.pixel_scale },
	{ C_INT, "window_width", &mused.window_w },
	{ C_INT, "window_height", &mused.window_h },
	{ C_INT, "mix_rate", &mused.mix_rate },
	{ C_INT, "mix_buffer", &mused.mix_buffer },
	{ C_BOOL, "compact", &mused.flags, COMPACT_VIEW },
	{ C_BOOL, "track_focus", &mused.flags, EXPAND_ONLY_CURRENT_TRACK },
	{ C_STR, "theme", mused.themename, sizeof(mused.themename) - 1 },
	{ C_STR, "keymap", mused.keymapname, sizeof(mused.keymapname) - 1 },
	{ C_BOOL, "multichannel_instrument_edit", &mused.flags, MULTICHANNEL_PREVIEW },
	{ C_BOOL, "show_position_offset", &mused.flags, SHOW_PATTERN_POS_OFFSET },
	{ C_BOOL, "follow_play_position", &mused.flags, FOLLOW_PLAY_POSITION },
	{ C_BOOL, "toggle_edit_on_stop", &mused.flags, TOGGLE_EDIT_ON_STOP },
	{ C_BOOL, "stop_edit_on_play", &mused.flags, STOP_EDIT_ON_PLAY },
	{ C_BOOL, "multikey_jamming", &mused.flags, MULTIKEY_JAMMING },
	{ C_BOOL, "animate_cursor", &mused.flags, ANIMATE_CURSOR },
	{ C_BOOL, "hide_zeros", &mused.flags, HIDE_ZEROS },
	{ C_BOOL, "protracker_delete", &mused.flags, DELETE_EMPTIES },
	{ C_BOOL, "center_pattern_editor", &mused.flags, CENTER_PATTERN_EDITOR },
	{ C_INT, "visible_columns", &mused.visible_columns },
	{ C_BOOL, "show_decimals", &mused.flags, SHOW_DECIMALS },
	{ C_BOOL, "show_reverb_ticks", &mused.flags, SHOW_DELAY_IN_TICKS },
	{ C_INT, "pattern_length", &mused.default_pattern_length },
	{ C_INT, "analyzer", &mused.current_visualizer },
#ifdef MIDI
	{ C_INT, "midi_device", &mused.midi_device },
	{ C_INT, "midi_channel", &mused.midi_channel },
#endif
	{ C_BOOL, "lock_pattern_length", &mused.flags, LOCK_SEQUENCE_STEP_AND_PATTERN_LENGTH },
	{ C_BOOL, "edit_sequence_digits", &mused.flags, EDIT_SEQUENCE_DIGITS },
	{ C_BOOL, "disable_nostalgy", &mused.flags, DISABLE_NOSTALGY },
	{ C_BOOL, "disable_vu_meters", &mused.flags, DISABLE_VU_METERS },
	{ C_BOOL, "maximized", &mused.flags, WINDOW_MAXIMIZED },
	{ C_INT, "oversample", &mused.oversample },
	{ C_BOOL, "disable_render_to_texture", &mused.flags, DISABLE_RENDER_TO_TEXTURE },
	{ C_BOOL, "disable_backups", &mused.flags, DISABLE_BACKUPS },
	{ C_END }
};


void apply_config()
{
	change_fullscreen(0, 0, 0);
	change_render_to_texture(0, 0, 0);
	change_pixel_scale(CASTTOPTR(void,mused.pixel_scale), 0, 0);
	load_theme_action(mused.themename, 0, 0);
	load_keymap_action(mused.keymapname, 0, 0);
	change_oversample(CASTTOPTR(void,mused.oversample), 0, 0);
}


void load_config(const char *path, bool apply)
{
	char *e = expand_tilde(path);
	FILE *f = fopen(e ? e : path, "rt");
	if (e) free(e);
	if (f)
	{
		while (!feof(f))
		{
			char line[500], name[500];
			if (!fgets(line, sizeof(line), f)) break;
			
			if (sscanf(line, "%400[^ =]", name) == 1)
			{
				int i;
				for (i = 0; confitem[i].type != C_END ; ++i)
				{
					if (strcmp(confitem[i].name, name) == 0)
					{
						switch (confitem[i].type)
						{
							case C_BOOL:
							{
								char value[10];
								if (sscanf(line, "%400[^ =]%*[= ]%9[^\n\r]", name, value) == 2)
								{
									if (strcmp(value, "yes") == 0)
									{
										*(int*)confitem[i].param |= confitem[i].mask;
									}
									else
									{
										*(int*)confitem[i].param &= ~confitem[i].mask;
									}
								}
							}
							break;
							
							case C_INT:
							{
								int value;
								if (sscanf(line, "%400[^ =]%*[= ]%d", name, &value) == 2)
								{
									*(int*)confitem[i].param = value;
								}
							}
							break;
							
							case C_STR:
							{
								char value[100];
								if (sscanf(line, "%400[^ =]%*[= ]%99[^\n\r]", name, value) == 2)
								{
									strncpy((char*)confitem[i].param, value, confitem[i].mask);
								}
							}
							break;
							
							default: 
								debug("Unhandled configtype %d", confitem[i].type);
								exit(2);
							break;
						}
					}
				}
			}
		}
	
		fclose(f);
	}
	
	if (apply) apply_config();
	
	e = expand_tilde("~/.klystrackfavorites");
	
	if (e) 
	{
		filebox_init(e);
		free(e);
	}
}


void save_config(const char *path)
{
	char *e = expand_tilde(path);
	FILE *f = fopen(e ? e : path, "wt");
	if (e) free(e);
	if (f)
	{
		for (int i = 0; confitem[i].type != C_END ; ++i)
		{
			switch (confitem[i].type)
			{
				case C_BOOL:
					fprintf(f, "%s = %s\n", confitem[i].name, *(int*)confitem[i].param & confitem[i].mask ? "yes" : "no");
				break;
				
				case C_STR:
					fprintf(f, "%s = %s\n", confitem[i].name, (char*)confitem[i].param);
				break;
				
				case C_INT:
					fprintf(f, "%s = %d\n", confitem[i].name, *(int*)confitem[i].param);
				break;
				
				default: 
					debug("Unhandled configtype %d", confitem[i].type);
					exit(2);
				break;
			}
		}
		
		fclose(f);
	}
	else
	{
		warning("Could not write config (%s)", path);
	}
	
	e = expand_tilde("~/.klystrackfavorites");
	
	if (e) 
	{
		filebox_quit(e);
		free(e);
	}
}

