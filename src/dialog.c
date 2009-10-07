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
#include "view.h"
#include "gfx/font.h"

extern Mused mused;


static void flip(void *bits, void *mask, void *unused)
{
	*(Uint32*)bits ^= (Uint32)mask;
}


int checkbox(const SDL_Event *event, const SDL_Rect *area, const char* _label, Uint32 *flags, Uint32 mask)
{
	SDL_Rect tick, label;
	copy_rect(&tick, area);
	copy_rect(&label, area);
	tick.h = tick.w = 8;
	label.w -= tick.w + 4;
	label.x += tick.w + 4;
	label.y += 1;
	label.h -= 1;
	int pressed = button_event(event, &tick, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, (*flags & mask) ? DECAL_TICK : -1, flip, flags, (void*)mask, 0);
	font_write(&mused.smallfont, mused.console->surface, &label, _label);
	pressed |= check_event(event, &label, flip, flags, (void*)mask, 0);
	
	return pressed;
}


static void delegate(void *p1, void *p2, void *p3)
{
	set_motion_target(NULL, p3);
	
	if (p1)
	{
		((void(*)(void*,void*,void*))p1)(((void **)p2)[0], ((void **)p2)[1], ((void **)p2)[2]);
	}
}


int button_event(const SDL_Event *event, const SDL_Rect *area, SDL_Surface *gfx, int offset, int offset_pressed, int decal, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	Uint32 mask = (Uint32)param1 ^ (Uint32)param2 ^ (Uint32)action;
	void *p[3] = { param1, param2, param3 };
	int pressed = check_event(event, area, delegate, action, p, (void*)mask);
	pressed |= check_drag_event(event, area, NULL, (void*)mask) << 1;
	button(area, mused.slider_bevel, pressed ? offset_pressed : offset, decal);
	
	return pressed;
}


int button_text_event(const SDL_Event *event, const SDL_Rect *area, SDL_Surface *gfx, int offset, int offset_pressed, const char * label, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	Uint32 mask = (Uint32)param1 ^ (Uint32)param2 ^ (Uint32)action;
	void *p[3] = { param1, param2, param3 };
	int pressed = check_event(event, area, delegate, action, p, (void*)mask);
	pressed |= check_drag_event(event, area, NULL, (void*)mask) << 1;
	button_text(area, mused.slider_bevel, pressed ? offset_pressed : offset, label);
	
	return pressed;
}


int spinner(const SDL_Event *event, const SDL_Rect *_area, int param)
{
	int plus, minus;
	SDL_Rect area;
	copy_rect(&area, _area);
	area.w /= 2;
	minus = button_event(event, &area, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, DECAL_MINUS, NULL, (void*)(0x80000000 | param), 0, NULL) & 1;

	area.x += area.w;
	plus = button_event(event, &area, mused.slider_bevel, BEV_SLIDER_HANDLE, BEV_SLIDER_HANDLE_ACTIVE, DECAL_PLUS, NULL, (void*)(0x81000000 | param), 0, NULL) & 1;
	
	return plus ? +1 : (minus ? -1 : 0);
}
