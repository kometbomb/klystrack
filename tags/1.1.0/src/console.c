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

#include "console.h"
#include "util/bundle.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


void console_set_background(Console * c, int enabled)
{
	c->background = enabled;
	c->font.surface = c->fontsurface[enabled];
	
	int l = strlen(c->font.charmap);
			
	for (int i = 0 ; i < l ; ++i)
		c->font.tiledescriptor[i].surface = c->font.surface;
}


void console_set_color(Console* console, Uint32 color, int idx)
{
	SDL_Color rgb = { color, color >> 8, color >> 16 };
	SDL_SetColors(console->fontsurface[0], &rgb, idx, 1);
	SDL_SetColors(console->fontsurface[1], &rgb, idx, 1);
}


const SDL_Rect * console_write(Console* console, const char *string)
{
	static SDL_Rect bounds;
	bounds.w = bounds.h = 0;
	font_write_cursor(&console->font, console->surface, &console->clip, &console->cursor, &bounds, string);
	return &bounds;
}


const SDL_Rect * console_write_args(Console* console, const char *string, ...)
{
	static SDL_Rect bounds;
	bounds.w = bounds.h = 0;
	va_list va;
	va_start(va, string);
	font_write_va(&console->font, console->surface, &console->clip, &console->cursor, &bounds, string, va);
	va_end(va);
	return &bounds;
}


void console_clear(Console *console)
{
	SDL_FillRect(console->surface, &console->clip, 0);
	console->cursor = 0;
}


Console * console_create(SDL_Surface *surface)
{
	Console * c = malloc(sizeof(*c));
	
	c->cursor = 0;
		
	Bundle b;
	
	if (bnd_open(&b, TOSTRING(RES_PATH) "/res/data"))
	{
		font_load(&c->font, &b, "8x8.fnt");
		bnd_free(&b);
	}
	
	// let's use a 8-bit surface so we can change the text color using the per surface palette
	
	for (int i = 0 ; i < 2 ; ++i)
	{
		SDL_Surface * paletted = SDL_CreateRGBSurface(SDL_HWSURFACE, c->font.surface->w, c->font.surface->h, 8, 0, 0, 0, 0);
		
		if (paletted)
		{
			{
				SDL_Color palette[2] = {{0, 0, 0}, { 255, 255, 255 }};
				SDL_SetColors(paletted, palette, 0, 2);
			}
		
			SDL_BlitSurface(c->font.surface, NULL, paletted, NULL);
			
			if (i == 0) SDL_SetColorKey(paletted, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(paletted->format, 0, 0, 0));
			
			c->fontsurface[i] = paletted;
		}
		else
		{
			exit(1);
		}
	}
	
	SDL_FreeSurface(c->font.surface);
	
	console_set_background(c, 0);
	
	c->surface = surface;
	
	c->clip.x = 0;
	c->clip.y = 0;
	c->clip.w = surface->w;
	c->clip.h = surface->h;
	
	return c;
}


void console_destroy(Console *c)
{
	c->font.surface = c->fontsurface[0];
	font_destroy(&c->font);
	SDL_FreeSurface(c->fontsurface[1]);
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

