#ifndef CONSOLE_H
#define CONSOLE_H

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
