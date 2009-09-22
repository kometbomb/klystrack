#include "console.h"

void console_set_color(Console* console, Uint32 color, int idx)
{
	SDL_Color rgb = { color, color >> 8, color >> 16 };
	SDL_SetColors(console->font.surface, &rgb, idx, 1);
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
	SDL_FillRect(console->surface, NULL, 0);
	console->cursor = 0;
}


Console * console_create(SDL_Surface *surface)
{
	Console * c = malloc(sizeof(*c));
	
	c->cursor = 0;
		
	Bundle b;
	
	bnd_open(&b, "c:\\code\\repo\\thrust\\main\\data.bundle");
	font_load(&c->font, &b, "8x8.fnt");
	bnd_free(&b);
	
	// let's use a 8-bit surface so we can change the text color using the per surface palette
	
	SDL_Surface * paletted = SDL_CreateRGBSurface(SDL_HWSURFACE, c->font.surface->w, c->font.surface->h, 8, 0, 0, 0, 0);
	
	if (paletted)
	{
		{
			SDL_Color rgb = { 255, 255, 255 };
			SDL_SetColors(paletted, &rgb, 1, 1);
		}
	
		SDL_BlitSurface(c->font.surface, NULL, paletted, NULL);
		SDL_FreeSurface(c->font.surface);
		
		int l = strlen(c->font.charmap);
		
		for (int i = 0 ; i < l ; ++i)
			c->font.tiledescriptor[i].surface = paletted;
		
		c->font.surface = paletted;
	}
	else
	{
		exit(1);
	}
	
	c->surface = surface;
	
	c->clip.x = 0;
	c->clip.y = 0;
	c->clip.w = surface->w;
	c->clip.h = surface->h;
	
	return c;
}


void console_destroy(Console *c)
{
	font_destroy(&c->font);
	free(c);
}
