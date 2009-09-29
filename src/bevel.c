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

#include "bevel.h"
#include "mused.h"
#include "mouse.h"

extern Mused mused;

void bevel(const SDL_Rect *area, SDL_Surface *gfx, int offset)
{
	/* Center */
	{
		for (int y = 4 ; y < area->h - 4 ; y += 8)
		{
			for (int x = 4 ; x < area->w - 4 ; x += 8)
			{
				SDL_Rect src = { 4 + offset, 4, my_min(8, area->w - x), my_min(8, area->h - y) };
				SDL_Rect dest = { x + area->x, y + area->y };
				SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
			}
		}
	}
	
	/* Sides */
	{
		for (int y = 4 ; y < area->h - 4 ; y += 8)
		{	
			{
				SDL_Rect src = { offset, 4, 4, my_min(8, area->h - y) };
				SDL_Rect dest = { area->x, y + area->y };
				SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
			}
			
			{
				SDL_Rect src = { 12 + offset, 4, 4, my_min(8, area->h - y) };
				SDL_Rect dest = { area->x + area->w - 4, y + area->y };
				SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
			}
		}
		
		for (int x = 4 ; x < area->w - 4 ; x += 8)
		{	
			{
				SDL_Rect src = { 4 + offset, 0, my_min(8, area->w - x), 4 };
				SDL_Rect dest = { area->x + x, area->y };
				SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
			}
			
			{
				SDL_Rect src = { 4 + offset, 12, my_min(8, area->w - x), 4 };
				SDL_Rect dest = { x + area->x, area->y + area->h - 4 };
				SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
			}
		}
	}
	
	/* Corners */
	{
		SDL_Rect src = { offset, 0, 4, 4 };
		SDL_Rect dest = { area->x, area->y };
		SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
	}
	
	{
		SDL_Rect src = { 12 + offset, 0, 4, 4 };
		SDL_Rect dest = { area->x + area->w - 4, area->y};
		SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
	}
	
	{
		SDL_Rect src = { 12 + offset, 12, 4, 4 };
		SDL_Rect dest = { area->x + area->w - 4, area->y + area->h - 4};
		SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
	}
	
	{
		SDL_Rect src = { offset, 12, 4, 4 };
		SDL_Rect dest = { area->x, area->y + area->h - 4};
		SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
	}
}


void button(const SDL_Rect *area, SDL_Surface *gfx, int offset, int decal)
{
	bevel(area, gfx, offset);
	
	SDL_Rect src = { decal, 16, 16, 16 };
	SDL_Rect dest = { area->x + area->w / 2 - 16 / 2, area->y + area->h / 2 - 16 / 2};
	SDL_BlitSurface(gfx, &src, mused.console->surface, &dest);
}


static void delegate(void *p1, void *p2, void *p3)
{
	set_motion_target(NULL, p3);
	((void(*)(void*,void*))p1)(((void **)p2)[0], ((void **)p2)[1]);
}


void button_event(const SDL_Event *event, const SDL_Rect *area, SDL_Surface *gfx, int offset, int offset_pressed, int decal, void (*action)(void*,void*), void *param1, void *param2)
{
	Uint32 mask = (Uint32)param1 ^ (Uint32)param2;
	void *p[2] = { param1, param2 };
	int pressed = check_event(event, area, delegate, action, p, (void*)mask);
	pressed |= check_drag_event(event, area, NULL, (void*)mask);
	button(area, mused.slider_bevel, pressed ? offset_pressed : offset, decal);
}
