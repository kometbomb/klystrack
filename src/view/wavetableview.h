#ifndef WAVETABLEVIEW_H
#define WAVETABLEVIEW_H

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

#include "SDL.h"
#include "gfx/gfx.h"

void wavetable_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void wavetablelist_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void wavetable_sample_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void wavetable_tools_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void wavetable_edit_area(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void wavetable_sample_area(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param);
void invalidate_wavetable_view();

#endif
