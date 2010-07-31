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

#ifdef WIN32

#include "windows.h"

#endif

extern Mused mused;

#define MAX_THEMES 10

Menu thememenu[MAX_THEMES + 1];
extern const Menu prefsmenu[];

Uint32 colors[NUM_COLORS];

static void load_colors(const char *cfg)
{
	char *temp = strdup(cfg);
	
	char *token = strtok(temp, "\n");
	
	while (token)
	{
		char name[51];
		Uint32 color;
		
		if (sscanf(token, "%50[^ =]%*[ =]%x", name, &color) == 2)
		{
			static const char *names[NUM_COLORS] =
			{
				"normal",
				"sequence_selected",
				"sequence_bar",
				"sequence_beat",
				"sequence_normal",
				"sequence_disabled",
				"pattern_selected",
				"pattern_bar",
				"pattern_beat",
				"pattern_instrument",
				"pattern_instrument_bar",
				"pattern_instrument_beat",
				"pattern_normal",
				"pattern_disabled",
				"program_selected",
				"program_even",
				"program_odd",
				"instrument_selected",
				"instrument_normal"
			};
			
			int i;
			for (i = 0 ; i < NUM_COLORS ; ++i)
			{
				if (strcasecmp(names[i], name) == 0)
				{
					colors[i] = color;
					break;
				}
			}
			
			if (i >= NUM_COLORS) warning("Unknown color name '%s'", name);
		}
		
		token = strtok(NULL, "\n");
	}
	
	free(temp);
}





void load_theme(const char *name)
{
	Bundle res;
	char fullpath[1000];
	
#ifdef WIN32
	// RES_PATH is relative to klystrack.exe
	char path[1000] = "";
	
	if (GetModuleFileName(NULL, path, sizeof(path)))
	{
		// Get the path component
		
		for (int i = strlen(path) - 1 ; i >= 0 && path[i] != '\\' && path[i] != '/' ; --i)
		{
			path[i] = '\0';
		}
	}
	
	snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/res/%s", path, name);
#else
	snprintf(fullpath, sizeof(fullpath) - 1, TOSTRING(RES_PATH) "/res/%s", name);
#endif
	
	debug("Loading theme '%s'", fullpath);
	
	if (bnd_open(&res, fullpath))
	{
		SDL_RWops *rw = SDL_RWFromBundle(&res, "bevel.bmp");
		
		if (rw)
		{
			if (mused.slider_bevel) gfx_free_surface(mused.slider_bevel);
			mused.slider_bevel = gfx_load_surface_RW(rw, GFX_KEYED);
			
			/* TODO: do we need to store the surface in the params? */
	
			mused.sequence_slider_param.gfx = mused.slider_bevel->surface; 
			mused.pattern_slider_param.gfx = mused.slider_bevel->surface; 
			mused.program_slider_param.gfx = mused.slider_bevel->surface; 
			mused.instrument_list_slider_param.gfx = mused.slider_bevel->surface;
			mused.pattern_horiz_slider_param.gfx = mused.slider_bevel->surface; 
			mused.sequence_horiz_slider_param.gfx = mused.slider_bevel->surface;
		}
	
		if (mused.console) console_destroy(mused.console); 
		mused.console = console_create(fullpath);
		font_destroy(&mused.smallfont);
		font_load(&mused.smallfont, &res, "7x6.fnt");
		font_destroy(&mused.largefont);
		font_load(&mused.largefont, &res, "8x8.fnt");
		
		FILE *colors = bnd_locate(&res, "colors.txt", 0);
		if (colors)
		{
			char temp[1000] = {0};
			fread(temp, 1, sizeof(temp)-1, colors);
			fclose(colors);
			
			load_colors(temp);
		}
		
		bnd_free(&res);
		strcpy(mused.themename, name);
		update_theme_menu();
		
		debug("Theme opened ok");
	}
	else
	{
		debug("Theme loading failed");
		if (strcmp(name, "Default") != 0)
			load_theme("Default");
	}
}


void enum_themes()
{
	memset(thememenu, 0, sizeof(thememenu));
	
#ifdef WIN32
// RES_PATH is relative to klystrack.exe
	char path[1000] = "", fullpath[1000];
	
	if (GetModuleFileName(NULL, path, sizeof(path)))
	{
		// Get the path component (this should be functionized and used by load_theme() and enum_theme()
		
		for (int i = strlen(path) - 1 ; i >= 0 && path[i] != '\\' && path[i] != '/' ; --i)
		{
			path[i] = '\0';
		}
	}
	
	snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/res", path);
	DIR *dir = opendir(fullpath);
	debug("Enumerating themes at " "%s" TOSTRING(RES_PATH) "/res", path);
#else
	DIR *dir = opendir(TOSTRING(RES_PATH) "/res");
	debug("Enumerating themes at " TOSTRING(RES_PATH) "/res");
#endif
	
	if (!dir)
	{
		warning("Could not enumerate themes at " TOSTRING(RES_PATH) "/res");
		return;
	}
	
	struct dirent *de = NULL;
	int themes = 0;
	
	while ((de = readdir(dir)) != NULL)
	{
		char fullpath[1000];
	
#ifdef WIN32
		snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/res/%s", path, de->d_name);
#else
		snprintf(fullpath, sizeof(fullpath) - 1, TOSTRING(RES_PATH) "/res/%s", de->d_name);
#endif
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