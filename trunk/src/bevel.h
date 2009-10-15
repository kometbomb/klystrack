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
#define BEV_CURSOR 48
#define BEV_SELECTED_SEQUENCE_ROW 64
#define BEV_SEQUENCE_BORDER 80
#define BEV_SEQUENCE_LOOP 96
#define BEV_THIN_FRAME 112
#define BEV_SELECTED_PATTERN_ROW 128
#define BEV_FIELD 144
#define BEV_SEPARATOR 160
#define BEV_SEQUENCE_PLAY_POS 176
#define BEV_BACKGROUND 192
#define BEV_MENUBAR 208
#define BEV_MENU 224
#define BEV_MENU_SELECTED 240
#define BEV_SELECTION 256
#define DECAL_UPARROW 0 
#define DECAL_DOWNARROW 16
#define DECAL_GRAB_VERT 32
#define DECAL_GRAB_HORIZ 48
#define DECAL_TICK 64
#define DECAL_PLUS 80
#define DECAL_MINUS 96
#define DECAL_AUDIO_ENABLED 112
#define DECAL_AUDIO_DISABLED 128
#define DECAL_RIGHTARROW 144
#define DECAL_LEFTARROW 160

void bevel(const SDL_Rect *area, SDL_Surface *gfx, int offset);
void button(const SDL_Rect *area, SDL_Surface *gfx, int offset, int decal);
void button_text(const SDL_Rect *area, SDL_Surface *gfx, int offset, const char *label);

#endif
