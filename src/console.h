#ifndef CONSOLE_H
#define CONSOLE_H

#include "SDL.h"
#include "gfx/font.h"

typedef struct
{
	SDL_Surface *surface;
	Uint16 cursor;
	Font font;
	SDL_Rect clip;
} Console;

enum 
{
	CON_BACKGROUND,
	CON_CHARACTER
};

void console_reset_cursor(Console * c);
void console_set_clip(Console * c, const SDL_Rect *rect);
void console_clear(Console *console);
Console * console_create(SDL_Surface *surface);
void console_set_color(Console* console, Uint32 color, int flags);
const SDL_Rect * console_write(Console* console, const char *string);
const SDL_Rect * console_write_args(Console* console, const char *string, ...)  __attribute__ ((format (printf, 2, 3)));
void console_destroy(Console *c);

#endif
