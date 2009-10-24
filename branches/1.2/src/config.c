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

#include "config.h"
#include "mused.h"
#include "toolutil.h"
#include "action.h"

extern Mused mused;

enum { C_END, C_BOOL, C_STR };

static const struct { int type; const char *name; void *param; int mask; } confitem[] =
{
	{ C_BOOL, "fullscreen", &mused.flags, FULLSCREEN },
	{ C_BOOL, "big_pixels", &mused.flags, BIG_PIXELS },
	{ C_BOOL, "compact", &mused.flags, COMPACT_VIEW },
	{ C_STR, "theme", mused.themename, sizeof(mused.themename) - 1 },
	{ C_END }
};


static void apply_config()
{
	change_fullscreen(0, 0, 0);
	change_pixel_scale(0, 0, 0);
	load_theme_action(mused.themename, 0, 0);
}


void load_config(const char *path)
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
			
			if (sscanf(line, "%[^ =]", name) == 1)
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
								if (sscanf(line, "%[^ =]%*[= ]%9[^\n\r]", name, value) == 2)
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
							
							case C_STR:
							{
								char value[100];
								if (sscanf(line, "%[^ =]%*[= ]%99[^\n\r]", name, value) == 2)
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
		
		apply_config();
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
}
