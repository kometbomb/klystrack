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

#include "macros.h"
#include "theme.h"
#include <string.h>
#include "util/bundle.h"
#include "gfx/gfx.h"
#include "mused.h"

extern Mused mused;


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
				"pattern_normal",
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
	
	snprintf(fullpath, sizeof(fullpath) - 1, TOSTRING(RES_PATH) "/res/%s", name);
	
	if (bnd_open(&res, fullpath))
	{
		SDL_RWops *rw = SDL_RWFromBundle(&res, "bevel.bmp");
		
		if (rw)
		{
			if (mused.slider_bevel) SDL_FreeSurface(mused.slider_bevel);
			mused.slider_bevel = gfx_load_surface_RW(rw, GFX_KEYED);
		}
	
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
	}
}
