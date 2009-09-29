#ifndef BEVEL_H
#define BEVEL_H

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

#define BEV_SLIDER_BG 0
#define BEV_SLIDER_HANDLE 16
#define BEV_SLIDER_HANDLE_ACTIVE 32
#define DECAL_UPARROW 0 
#define DECAL_DOWNARROW 16
#define DECAL_GRAB_VERT 32
#define DECAL_GRAB_HORIZ 64

void bevel(const SDL_Rect *area, SDL_Surface *gfx, int offset);
void button(const SDL_Rect *area, SDL_Surface *gfx, int offset, int decal);
void button_event(const SDL_Event *event, const SDL_Rect *area, SDL_Surface *gfx, int offset, int offset_pressed, int decal, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3);
void checkbox(const SDL_Event *event, const char* label, Uint32 *flags, Uint32 mask);

#endif
