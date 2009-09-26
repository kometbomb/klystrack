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


#include "slider.h"
#include "mused.h"
#include "mouse.h"

extern Mused mused;

static void modify_position(void *delta, void *_param, void *unused)
{
	SliderParam *param = _param;
	*param->position += (int)delta;
	if (*param->position < param->first) *param->position = param->first;
	if (*param->position > param->last) *param->position = param->last;
}


static void drag_begin(void *event, void *_param, void *area)
{
	SliderParam *param = _param;
	param->drag_begin_coordinate = ((SDL_Event*)event)->button.y;
	param->drag_begin_position = *param->position;
	param->drag_area_size = ((SDL_Rect*)area)->h;
}


static void drag_motion(int x, int y, void *_param)
{
	SliderParam *param = _param;
	int delta = y - param->drag_begin_coordinate;
	*param->position = param->drag_begin_position + delta * (param->last - param->first) / param->drag_area_size;
	if (*param->position < param->first) *param->position = param->first;
	if (*param->position > param->last) *param->position = param->last;
}


void slider(const SDL_Rect *_area, const SDL_Event *event, void *_param)
{
	SliderParam *param = _param;
	int bar_top = 0;
	int bar_size = _area->h;
	
	if (param->last > param->first)
	{
		bar_top = (param->visible_first) * _area->h / (param->last - param->first);
		int bar_bottom = (param->visible_last) * _area->h / (param->last - param->first);
		bar_size = bar_bottom - bar_top;
		if (bar_size < 1) bar_size = 1;
		if (bar_top + bar_size > _area->h) bar_top = _area->h - bar_size;
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, bar_top };
		check_event(event, &area, modify_position, (void*)-(param->visible_last - param->visible_first), param, 0);
		SDL_FillRect(mused.console->surface, &area, 0xff808080);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y + bar_top, _area->w, bar_size };
		check_event(event, &area, drag_begin, event, param, _area);
		check_drag_event(event, &area, drag_motion, param);
		SDL_FillRect(mused.console->surface, &area, 0xffffffff);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y + bar_top + bar_size, _area->w, _area->h - (bar_top + bar_size) };
		check_event(event, &area, modify_position, (void*)(param->visible_last - param->visible_first), param, 0);
		SDL_FillRect(mused.console->surface, &area, 0xff8080f0);
	}
}


void slider_set_params(SliderParam *param, int first, int last, int first_visible, int last_visible, int *position)
{
	param->first = first;
	param->last = last;
	param->visible_first = first_visible;
	param->visible_last = last_visible;
	param->position = position;
}
