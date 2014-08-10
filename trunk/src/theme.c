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

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef WIN32

#include "windows.h"

#endif

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
		char name[51];
		Uint32 color;
		
		if (sscanf(token, "%50[^ =]%*[ =]%x", name, &color) == 2)
		{
			static const char *names[NUM_COLORS] =
			{
				"sequence_counter",
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
				"pattern_seq_number"
			};
			
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
		return 0;
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


#ifdef __APPLE__
// Obtain resources directory - thanks Spenot
char *query_resource_directory( void )
{
	static char *buffer = NULL;
	
	if (buffer != NULL)
	{
		return buffer;
	}
	
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	
	int bufferSize;
	

	CFURLRef resourceURL = CFBundleCopyBundleURL(mainBundle);
	CFStringRef fullPathString = CFURLCopyFileSystemPath(resourceURL, kCFURLPOSIXPathStyle);
	int fullPathLength = CFStringGetLength(fullPathString);

	bufferSize = fullPathLength * 3 + 1;
	char *bundleBuffer = (char *)calloc(bufferSize, 1);
	CFStringGetCString(fullPathString, bundleBuffer, bufferSize, kCFStringEncodingUTF8);

	CFRelease(fullPathString);
	CFRelease(resourceURL);


	resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	fullPathString = CFURLCopyFileSystemPath(resourceURL, kCFURLPOSIXPathStyle);
	fullPathLength = CFStringGetLength(fullPathString);

	bufferSize = fullPathLength * 3 + 1;
	char *resourcesBuffer = (char *)calloc(bufferSize, 1);
	CFStringGetCString(fullPathString, resourcesBuffer, bufferSize, kCFStringEncodingUTF8);

	CFRelease(fullPathString);
	CFRelease(resourceURL);


	buffer = calloc(strlen(bundleBuffer) + strlen(resourcesBuffer) + 2, 1);
	strcpy(buffer, bundleBuffer);
	buffer[strlen(buffer)] = '/';
	strcat(buffer, resourcesBuffer);
	
	free(bundleBuffer);
	free(resourcesBuffer);

	return buffer;
}

void init_resources_dir(void)
{
}


#elif WIN32

static char cwd[1000] = "";

void init_resources_dir(void)
{
#if RESOURCES_IN_BINARY_DIR
	if (GetModuleFileName(NULL, cwd, sizeof(cwd)))
	{
		// Get the path component
		
		for (int i = strlen(cwd) - 1 ; i >= 0 && cwd[i] != '\\' && cwd[i] != '/' ; --i)
		{
			cwd[i] = '\0';
		}
	}
#else
	strncpy(cwd, TOSTRING(RES_PATH), sizeof(cwd));
#endif	
}

char * query_resource_directory(void)
{
	return cwd;
}

#else

static char cwd[1000] = "";

void init_resources_dir(void)
{
#if RESOURCES_IN_BINARY_DIR
	getcwd(cwd, sizeof(cwd));
#else
	strncpy(cwd, TOSTRING(RES_PATH), sizeof(cwd));
#endif
}

char * query_resource_directory(void)
{
	return cwd;
}


#endif


void load_theme(const char *name)
{
	char tmpname[1000];
	strncpy(tmpname, name, sizeof(tmpname));

	if (strcmp(name, "Default") != 0)
		load_theme("Default"); // for default stuff not in selected theme

	Bundle res;
	char fullpath[1000];
	
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
		
		if (bnd_exists(&res, "4x6.fnt"))
		{
			font_destroy(&mused.tinyfont);
			font_load_and_set_color(&mused.tinyfont, &res, "4x6.fnt", colors[COLOR_MAIN_TEXT]);
			font_destroy(&mused.tinyfont_sequence_counter);
			font_load_and_set_color(&mused.tinyfont_sequence_counter, &res, "4x6.fnt", colors[COLOR_SEQUENCE_COUNTER]);
			font_destroy(&mused.tinyfont_sequence_normal);
			font_load_and_set_color(&mused.tinyfont_sequence_normal, &res, "4x6.fnt", colors[COLOR_SEQUENCE_NORMAL]);
		}
				
		bnd_free(&res);
		strncpy(mused.themename, tmpname, sizeof(mused.themename));
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
	
	char path[1000];
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
		char fullpath[1000];
	
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
