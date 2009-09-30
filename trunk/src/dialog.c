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

#include "dialog.h"
#include "bevel.h"
#include "mused.h"
#include "mouse.h"

extern Mused mused;


static void flip(void *bits, void *mask, void *unused)
{
	*(Uint32*)bits ^= (Uint32)mask;
}


int checkbox(const SDL_Event *event, const char* label, Uint32 *flags, Uint32 mask)
{
	SDL_Rect area = { (mused.console->font.w * (mused.console->cursor & 0xff)), mused.console->font.h * (mused.console->cursor >> 8) , 8, 8 };
	int pressed = button_event(event, &area, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, *flags & mask ? DECAL_GRAB_VERT : -1, flip, flags, (void*)mask, 0);
	mused.console->cursor += 0x01;
	pressed |= check_event(event, console_write(mused.console, label), flip, flags, (void*)mask, 0);
	
	return pressed;
}


static void delegate(void *p1, void *p2, void *p3)
{
	set_motion_target(NULL, p3);
	((void(*)(void*,void*,void*))p1)(((void **)p2)[0], ((void **)p2)[1], ((void **)p2)[2]);
}


int button_event(const SDL_Event *event, const SDL_Rect *area, SDL_Surface *gfx, int offset, int offset_pressed, int decal, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	Uint32 mask = (Uint32)param1 ^ (Uint32)param2;
	void *p[3] = { param1, param2, param3 };
	int pressed = check_event(event, area, delegate, action, p, (void*)mask);
	pressed |= check_drag_event(event, area, NULL, (void*)mask);
	button(area, mused.slider_bevel, pressed ? offset_pressed : offset, decal);
	
	return pressed;
}
