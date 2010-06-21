#ifndef VIEW_H
#define VIEW_H

/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (SDL_Surface *dest_surface, the "Software"), to deal in the Software without
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

#include "gui/view.h"

void my_draw_view(const View* views, const SDL_Event *_event, const SDL_Surface *screen);
int generic_field(const SDL_Event *e, const SDL_Rect *area, int param, const char *_label, const char *format, void *value, int width);
char * notename(int note);

/* 
"Controls"
*/

void song_name_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_name_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_disk_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void program_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void info_line(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void sequence_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void pattern_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void info_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_view2(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void instrument_list(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void fx_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void bevel_view(SDL_Surface *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);

#endif
