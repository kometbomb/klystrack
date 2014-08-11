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

#include "console.h"
#include "util/bundle.h"
#include "mused.h"
#include "theme.h"
#include <string.h>

extern Mused mused;
extern Uint32 colors[];

void console_set_background(Console * c, int enabled)
{
	c->background = enabled;
	/*c->font.surface->surface = c->fontsurface[enabled];
	
	int l = strlen(c->font.charmap);
			
	for (int i = 0 ; i < l ; ++i)
		c->font.tiledescriptor[i].surface = c->font.surface;*/
}


void console_set_color(Console* console, Uint32 color)
{
	if (console->current_color != color)
	{
		console->current_color = color;
		font_set_color(&console->font, console->current_color);
	}
}


const SDL_Rect * console_write(Console* console, const char *string)
{
	static SDL_Rect bounds;
	bounds.w = bounds.h = 0;
	font_write_cursor(&console->font, domain, &console->clip, &console->cursor, &bounds, string);
	return &bounds;
}


const SDL_Rect * console_write_args(Console* console, const char *string, ...)
{
	static SDL_Rect bounds;
	bounds.w = bounds.h = 0;
	va_list va;
	va_start(va, string);
	font_write_va(&console->font, domain, &console->clip, &console->cursor, &bounds, string, va);
	va_end(va);
	return &bounds;
}


void console_clear(Console *console)
{
	gfx_rect(domain, &console->clip, colors[COLOR_BACKGROUND]);
	console->cursor = 0;
}


Console * console_create(Bundle *b)
{
	Console * c = calloc(1, sizeof(*c));
	
	c->cursor = 0;
		
	font_load(domain, &c->font, b, "8x8.fnt");
	
	console_set_background(c, 0);
	
	c->clip.x = 0;
	c->clip.y = 0;
	c->clip.w = domain->screen_w;
	c->clip.h = domain->screen_h;
	
	return c;
}


void console_destroy(Console *c)
{
	font_destroy(&c->font);
	
	free(c);
}


void console_set_clip(Console * c, const SDL_Rect *rect)
{
	memcpy(&c->clip, rect, sizeof(*rect));
}


void console_reset_cursor(Console * c)
{
	c->cursor = 0;
}

