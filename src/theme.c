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

#include "macros.h"
#include "theme.h"
#include <string.h>
#include "util/bundle.h"
#include "gfx/gfx.h"
#include "mused.h"
#include "gui/menu.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <strings.h>
#include "action.h"
#include "console.h"

extern Mused mused;

#define MAX_THEMES 64

Menu thememenu[MAX_THEMES + 1];
extern const Menu prefsmenu[];

Uint32 colors[NUM_COLORS];

static void load_colors(const char *cfg)
{
	char *temp = strdup(cfg);

	char *token = strtok(temp, "\n");

	while (token)
	{
		char name[51], from[51];
		Uint32 color;

		static const char *names[NUM_COLORS] =
		{
			"sequence_counter",
			"sequence_normal",
			"pattern_selected",
			"pattern_bar",
			"pattern_beat",
			"pattern_instrument",
			"pattern_instrument_bar",
			"pattern_instrument_beat",
			"pattern_volume",
			"pattern_volume_bar",
			"pattern_volume_beat",
			"pattern_ctrl",
			"pattern_ctrl_bar",
			"pattern_ctrl_beat",
			"pattern_command",
			"pattern_command_bar",
			"pattern_command_beat",
			"pattern_normal",
			"pattern_disabled",
			"program_selected",
			"program_even",
			"program_odd",
			"instrument_selected",
			"instrument_normal",
			"menu",
			"menu_selected",
			"menu_header",
			"menu_header_selected",
			"menu_shortcut",
			"menu_shortcut_selected",
			"main_text",
			"small_text",
			"background",
			"button_text",
			"text_shadow",
			"pattern_empty_data",
			"wavetable_sample",
			"wavetable_background",
			"progress_bar",
			"pattern_seq_number",
			"catometer_eyes",
			"statusbar_text"
		};

		if (sscanf(token, "%50[^ =]%*[ =]%x", name, &color) == 2)
		{
			int i;
			for (i = 0 ; i < NUM_COLORS ; ++i)
			{
				if (strcasecmp(names[i], name) == 0)
				{
					colors[i] = color;
					FIX_ENDIAN(colors[i]);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
// fix so that the color is in the 24 bits of RGB8
					colors[i] = (colors[i] >> 8) | ((colors[i] & 0xff) << 24);
#endif
					break;
				}
			}

			if (i >= NUM_COLORS) warning("Unknown color name '%s'", name);
		}
		else if (sscanf(token, "%50[^ =]%*[ =]%50s", name, from) == 2)
		{
			int from_i, to_i;

			for (from_i = 0 ; from_i < NUM_COLORS ; ++from_i)
			{
				if (strcasecmp(names[from_i], from) == 0)
				{
					break;
				}
			}

			if (from_i >= NUM_COLORS)
				warning("Unknown color name '%s'", name);
			else
			{
				for (to_i = 0 ; to_i < NUM_COLORS ; ++to_i)
				{
					if (strcasecmp(names[to_i], name) == 0)
					{
						colors[to_i] = color;
						break;
					}
				}

				if (to_i >= NUM_COLORS) warning("Unknown color name '%s'", name);
				else
				{
					debug("%s=%s", name, from);
					colors[to_i] = colors[from_i];
				}
			}

		}

		token = strtok(NULL, "\n");
	}

	free(temp);
}



int font_load_and_set_color(Font *font, Bundle *b, char *name, Uint32 color)
{
	if (font_load(domain, font, b, name))
	{
		font_set_color(font, color);
		return 1;
	}
	else
	{
		return 0;
	}
}


static SDL_RWops *load_img_if_exists(Bundle *res, const char *base_name)
{
	/* load base_name.bmp or .png */

	char name[100];
	const char *ext[] = {"bmp", "png", NULL}, **e;

	for (e = ext ; *e ; ++e)
	{
		snprintf(name, sizeof(name), "%s.%s", base_name, *e);

		if (bnd_exists(res, name))
		{
			SDL_RWops *rw = SDL_RWFromBundle(res, name);
			if (rw)
				return rw;
		}
	}

	return NULL;
}


static char cwd[1000] = "";

void init_resources_dir(void)
{
	if (SDL_getenv("KLYSTRACK") == NULL)
	{
#if RESOURCES_IN_BINARY_DIR
		strncpy(cwd, SDL_GetBasePath(), sizeof(cwd));
#else
		strncpy(cwd, TOSTRING(RES_PATH), sizeof(cwd));
#endif
	}
	else
	{
		strncpy(cwd, SDL_getenv("KLYSTRACK"), sizeof(cwd));
	}
}

char * query_resource_directory(void)
{
	return cwd;
}


void set_scaled_cursor()
{
	if (mused.mouse_cursor_surface == NULL)
		return;

	if (mused.mouse_cursor) SDL_FreeCursor(mused.mouse_cursor);

	if (mused.flags & USE_SYSTEM_CURSOR)
	{
		mused.mouse_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	}
	else
	{
		// We'll use SDL_Renderer here because SDL_BlitScaled seems to have an issue with the alpha channel
		// Additionally, transparency on a zoomed cursor seems to make the cursor an "XOR" cursor so we need
		// to set the transparent color separately after SDL_Renderer has done its thing. SDL bug maybe?

		SDL_Surface *temp = SDL_CreateRGBSurface(0, mused.mouse_cursor_surface->surface->w * mused.pixel_scale, mused.mouse_cursor_surface->surface->h * mused.pixel_scale, 32, 0, 0, 0, 0);

		SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(temp);
		SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, mused.mouse_cursor_surface->surface);

		// Draw the texture on a magic pink background
		SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
		SDL_RenderFillRect(renderer, NULL);
		SDL_RenderCopy(renderer, tex, NULL, NULL);

		SDL_DestroyTexture(tex);
		SDL_DestroyRenderer(renderer);

		// Make magic pink transparent
		SDL_SetColorKey(temp, SDL_TRUE, SDL_MapRGB(temp->format, 255, 0, 255));

		mused.mouse_cursor = SDL_CreateColorCursor(temp, 0, 0);

		SDL_FreeSurface(temp);
	}

	if (mused.mouse_cursor)
	{
		SDL_SetCursor(mused.mouse_cursor);
	}
	else
	{
		warning("SDL_SetCursor failed: %s", SDL_GetError());
	}
}


void set_app_icon()
{
	SDL_SetWindowIcon(domain->window, mused.icon_surface->surface);
}


void load_theme(const char *name)
{
	char tmpname[100] = {0};
	strncpy(tmpname, name, sizeof(tmpname) - 1);

	if (strcmp(name, "Default") != 0)
		load_theme("Default"); // for default stuff not in selected theme

	Bundle res;
	char fullpath[2000] = {0};

	snprintf(fullpath, sizeof(fullpath) - 1, "%s/res/%s", query_resource_directory(), tmpname);

	debug("Loading theme '%s'", fullpath);

	if (bnd_open(&res, fullpath))
	{
		SDL_RWops *rw;

		rw = load_img_if_exists(&res, "bevel");
		if (rw)
		{
			if (mused.slider_bevel) gfx_free_surface(mused.slider_bevel);
			mused.slider_bevel = gfx_load_surface_RW(domain, rw, GFX_KEYED);

			/* TODO: do we need to store the surface in the params? */

			mused.sequence_slider_param.gfx = mused.slider_bevel;
			mused.pattern_slider_param.gfx = mused.slider_bevel;
			mused.program_slider_param.gfx = mused.slider_bevel;
			mused.instrument_list_slider_param.gfx = mused.slider_bevel;
			mused.pattern_horiz_slider_param.gfx = mused.slider_bevel;
			mused.sequence_horiz_slider_param.gfx = mused.slider_bevel;
		}

		rw = load_img_if_exists(&res, "vu");
		if (rw)
		{
			if (mused.vu_meter) gfx_free_surface(mused.vu_meter);
			mused.vu_meter = gfx_load_surface_RW(domain, rw, GFX_KEYED);
		}

		rw = load_img_if_exists(&res, "analyzor");
		if (rw)
		{
			if (mused.analyzer) gfx_free_surface(mused.analyzer);
			mused.analyzer = gfx_load_surface_RW(domain, rw, GFX_KEYED);
		}

		rw = load_img_if_exists(&res, "catometer");
		if (rw)
		{
			if (mused.catometer) gfx_free_surface(mused.catometer);
			mused.catometer = gfx_load_surface_RW(domain, rw, GFX_KEYED);
		}

		rw = load_img_if_exists(&res, "cursor");
		if (rw)
		{
			if (mused.mouse_cursor_surface) gfx_free_surface(mused.mouse_cursor_surface);
			if (mused.mouse_cursor) SDL_FreeCursor(mused.mouse_cursor);
			mused.mouse_cursor_surface = gfx_load_surface_RW(domain, rw, GFX_KEYED);

			set_scaled_cursor();
		}

		rw = load_img_if_exists(&res, "icon");
		if (rw)
		{
			if (mused.icon_surface) gfx_free_surface(mused.icon_surface);
			mused.icon_surface = gfx_load_surface_RW(domain, rw, 0);

			set_app_icon();
		}

		rw = load_img_if_exists(&res, "logo");
		if (rw)
		{
			if (mused.logo) gfx_free_surface(mused.logo);
			mused.logo = gfx_load_surface_RW(domain, rw, GFX_KEYED);
		}

		if (bnd_exists(&res, "colors.txt"))
		{
			SDL_RWops *colors = SDL_RWFromBundle(&res, "colors.txt");
			if (colors)
			{
				SDL_RWseek(colors, 0, SEEK_END);
				size_t s = SDL_RWtell(colors);
				char *temp = calloc(1, s + 2);
				SDL_RWseek(colors, 0, SEEK_SET);
				SDL_RWread(colors, temp, 1, s);

				strcat(temp, "\n");

				SDL_RWclose(colors);

				load_colors(temp);
				free(temp);
			}
		}

		if (bnd_exists(&res, "7x6.fnt"))
		{
			font_destroy(&mused.smallfont);
			font_load_and_set_color(&mused.smallfont, &res, "7x6.fnt", colors[COLOR_SMALL_TEXT]);
			font_destroy(&mused.shortcutfont);
			font_load_and_set_color(&mused.shortcutfont, &res, "7x6.fnt", colors[COLOR_MENU_SHORTCUT]);
			font_destroy(&mused.shortcutfont_selected);
			font_load_and_set_color(&mused.shortcutfont_selected, &res, "7x6.fnt", colors[COLOR_MENU_SHORTCUT_SELECTED]);
			font_destroy(&mused.headerfont);
			font_load_and_set_color(&mused.headerfont, &res, "7x6.fnt", colors[COLOR_MENU_HEADER]);
			font_destroy(&mused.headerfont_selected);
			font_load_and_set_color(&mused.headerfont_selected, &res, "7x6.fnt", colors[COLOR_MENU_HEADER_SELECTED]);
			font_destroy(&mused.buttonfont);
			font_load_and_set_color(&mused.buttonfont, &res, "7x6.fnt", colors[COLOR_BUTTON_TEXT]);
		}
		else
		{
			font_set_color(&mused.smallfont, colors[COLOR_SMALL_TEXT]);
			font_set_color(&mused.shortcutfont, colors[COLOR_MENU_SHORTCUT]);
			font_set_color(&mused.shortcutfont_selected, colors[COLOR_MENU_SHORTCUT_SELECTED]);
			font_set_color(&mused.headerfont, colors[COLOR_MENU_HEADER]);
			font_set_color(&mused.headerfont_selected, colors[COLOR_MENU_HEADER_SELECTED]);
			font_set_color(&mused.buttonfont, colors[COLOR_BUTTON_TEXT]);
		}

		if (bnd_exists(&res, "8x8.fnt"))
		{
			if (mused.console) console_destroy(mused.console);
			mused.console = console_create(&res);

			font_destroy(&mused.largefont);
			font_load_and_set_color(&mused.largefont, &res, "8x8.fnt", colors[COLOR_MAIN_TEXT]);
			font_destroy(&mused.menufont);
			font_load_and_set_color(&mused.menufont, &res, "8x8.fnt", colors[COLOR_MENU_NORMAL]);
			font_destroy(&mused.menufont_selected);
			font_load_and_set_color(&mused.menufont_selected, &res, "8x8.fnt", colors[COLOR_MENU_SELECTED]);
		}
		else
		{
			font_set_color(&mused.largefont, colors[COLOR_MAIN_TEXT]);
			font_set_color(&mused.menufont, colors[COLOR_MENU_NORMAL]);
			font_set_color(&mused.menufont_selected, colors[COLOR_MENU_SELECTED]);
		}

		if (bnd_exists(&res, "4x6.fnt"))
		{
			font_destroy(&mused.tinyfont);
			font_load_and_set_color(&mused.tinyfont, &res, "4x6.fnt", colors[COLOR_MAIN_TEXT]);
			font_destroy(&mused.tinyfont_sequence_counter);
			font_load_and_set_color(&mused.tinyfont_sequence_counter, &res, "4x6.fnt", colors[COLOR_SEQUENCE_COUNTER]);
			font_destroy(&mused.tinyfont_sequence_normal);
			font_load_and_set_color(&mused.tinyfont_sequence_normal, &res, "4x6.fnt", colors[COLOR_SEQUENCE_NORMAL]);
		}
		else
		{
			font_set_color(&mused.tinyfont, colors[COLOR_MAIN_TEXT]);
			font_set_color(&mused.tinyfont_sequence_counter, colors[COLOR_SEQUENCE_COUNTER]);
			font_set_color(&mused.tinyfont_sequence_normal, colors[COLOR_SEQUENCE_NORMAL]);
		}

		bnd_free(&res);
		strncpy(mused.themename, tmpname, sizeof(mused.themename));
		update_theme_menu();

		debug("Theme opened ok");
	}
	else
	{
		warning("Theme loading failed");

		if (strcmp(name, "Default") != 0)
		{
			load_theme("Default");
		}
		else
		{
			char message[4000] = {0};
			snprintf(message, sizeof(message) - 1, "Default theme at '%s' could not be loaded.", fullpath);

			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Theme files missing", message, domain->window);

			fatal("Default theme at '%s' could not be loaded.", fullpath);

			exit(1);
		}
	}
}


void enum_themes()
{
	memset(thememenu, 0, sizeof(thememenu));

	char path[2000] = {0};
	snprintf(path, sizeof(path) - 1, "%s/res", query_resource_directory());
	DIR *dir = opendir(path);
	debug("Enumerating themes at %s", path);

	if (!dir)
	{
		warning("Could not enumerate themes at %s", path);
		return;
	}

	struct dirent *de = NULL;
	int themes = 0;

	while ((de = readdir(dir)) != NULL)
	{
		char fullpath[4000] = {0};

		snprintf(fullpath, sizeof(fullpath) - 1, "%s/res/%s", query_resource_directory(), de->d_name);

		struct stat attribute;

		if (stat(fullpath, &attribute) != -1 && !(attribute.st_mode & S_IFDIR))
		{
			if (themes >= MAX_THEMES)
			{
				warning("Maximum themes exceeded");
				break;
			}

			thememenu[themes].parent = prefsmenu;
			thememenu[themes].text = strdup(de->d_name);
			thememenu[themes].action = load_theme_action;
			thememenu[themes].p1 = (void*)thememenu[themes].text;
			++themes;
		}
	}

	debug("Got %d themes", themes);

	closedir(dir);
}


void update_theme_menu()
{
	for (int i = 0 ; thememenu[i].text ; ++i)
	{
		if (strcmp(mused.themename, (char*)thememenu[i].p1) == 0)
		{
			thememenu[i].flags |= MENU_BULLET;
		}
		else
			thememenu[i].flags &= ~MENU_BULLET;
	}
}


void free_themes()
{
	for (int i = 0 ; i < MAX_THEMES ; ++i)
	{
		if (thememenu[i].text != NULL) free((void*)thememenu[i].text);
	}

	memset(thememenu, 0, sizeof(thememenu));
}


Uint32 mix_colors(Uint32 a, Uint32 b)
{
	Sint32 ba = 255 - ((b >> 24) & 0xff);
	Sint32 ar = a & 0xff;
	Sint32 ag = (a >> 8) & 0xff;
	Sint32 ab = (a >> 16) & 0xff;
	Sint32 br = (b & 0xff) - ar;
	Sint32 bg = ((b >> 8) & 0xff) - ag;
	Sint32 bb = ((b >> 16) & 0xff) - ab;

	Uint32 fr = ar + br * ba / 256;
	Uint32 fg = ag + bg * ba / 256;
	Uint32 fb = ab + bb * ba / 256;

	return fr | (fg << 8) | (fb << 16);
}
