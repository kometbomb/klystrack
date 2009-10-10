#ifndef VIEW_H
#define VIEW_H

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

typedef struct
{
	const SDL_Rect position;
	
	/*
	This is a combined drawing and mouse event handler.
	*/
	
	void (*handler)(const SDL_Rect *dest, const SDL_Event *event, void *param);
	void *param;
	/*
	When clicked the focus set to the following or if the param equals -1 it will leave as it is
	*/
	int focus;
} View;


void draw_view(const View* views, const SDL_Event *event);
void adjust_rect(SDL_Rect *rect, int margin);
void copy_rect(SDL_Rect *dest, const SDL_Rect *src);
void update_rect(const SDL_Rect *parent, SDL_Rect *rect);

/* 
"Controls"
*/

void song_name_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_name_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_disk_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void program_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void info_line(const SDL_Rect *dest, const SDL_Event *event, void *param);
void sequence_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void pattern_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void info_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_list(const SDL_Rect *dest, const SDL_Event *event, void *param);
void reverb_view(const SDL_Rect *dest, const SDL_Event *event, void *param);
void bevel_view(const SDL_Rect *dest, const SDL_Event *event, void *param);

#endif
