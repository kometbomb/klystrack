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
#include "bevel.h"

extern Mused mused;

static int quant(int v, int g)
{
	return v - v % (g);
}

static void modify_position(void *delta, void *_param, void *unused)
{
	SliderParam *param = _param;
	*param->position = quant(*param->position + (int)delta, param->granularity);
	if (*param->position < param->first) *param->position = param->first;
	if (*param->position > param->last) *param->position = param->last;
}


static void drag_begin(void *event, void *_param, void *area)
{
	SliderParam *param = _param;
	param->drag_begin_coordinate = param->orientation == SLIDER_HORIZONTAL ? ((SDL_Event*)event)->button.x : ((SDL_Event*)event)->button.y;
	param->drag_begin_position = *param->position;
	if (param->drag_begin_position > param->last - param->margin) param->drag_begin_position = param->last - param->margin;
	if (param->drag_begin_position < param->first) param->drag_begin_position = param->first;
	param->drag_area_size = param->orientation == SLIDER_HORIZONTAL ? ((SDL_Rect*)area)->w : ((SDL_Rect*)area)->h;
}


static void drag_motion(int x, int y, void *_param)
{
	SliderParam *param = _param;
	if ((param->visible_first == param->first && param->visible_last == param->last) || param->drag_area_size == 0)
		return;
	int delta = (param->orientation == SLIDER_HORIZONTAL ? x : y) - param->drag_begin_coordinate;
	*param->position = quant(param->drag_begin_position + delta * (param->last - param->first) / param->drag_area_size, param->granularity);
	if (*param->position > param->last - param->margin) *param->position = param->last - param->margin;
	if (*param->position < param->first) *param->position = param->first;
}


void slider(const SDL_Rect *_area, const SDL_Event *event, void *_param)
{
	SliderParam *param = _param;
	int bar_top = 0;
	int area_size = param->orientation == SLIDER_HORIZONTAL ? _area->w : _area->h;
	int area_start = param->orientation == SLIDER_HORIZONTAL ? _area->x : _area->y;
	int bar_size = area_size;
	
	if (param->visible_last > param->visible_first && param->last != param->first)
	{
		bar_top = (param->visible_first - param->first) * area_size / (param->last - param->first);
		int bar_bottom;
		
		if (param->visible_last < param->last) 
		{
			bar_bottom = (param->visible_last - param->first) * area_size / (param->last - param->first);
		}
		else
		{
			bar_bottom = area_size + area_start;
			bar_top = bar_bottom - (param->visible_last - param->visible_first) * area_size / (param->last - param->first);
		}
		
		bar_size = bar_bottom - bar_top;
		
		if (bar_size < 4) bar_size = 4;
		if (bar_top + bar_size > area_size) bar_top = area_size - bar_size;
	}
	
	bevel(_area, mused.slider_bevel);
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
			area.w = bar_top;
		else
			area.h = bar_top;
		
		check_event(event, &area, modify_position, (void*)-(param->visible_last - param->visible_first), param, 0);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.x += bar_top;
			area.w = bar_size;
		}
		else
		{
			area.y += bar_top;
			area.h = bar_size;
		}
		
		check_event(event, &area, drag_begin, (void*)event, param, (void*)_area);
		check_drag_event(event, &area, drag_motion, (void*)param);
		bevel(&area, mused.slider_bevel);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.x += bar_top + bar_size;
			area.w -= bar_size + bar_top;
		}
		else
		{
			area.y += bar_top + bar_size;
			area.h -= bar_size + bar_top;
		}
		
		check_event(event, &area, modify_position, (void*)(param->visible_last - param->visible_first), param, 0);
	}
}


void slider_set_params(SliderParam *param, int first, int last, int first_visible, int last_visible, int *position, int granularity, int orientation)
{
	param->first = first;
	param->last = last;
	param->visible_first = first_visible;
	param->visible_last = last_visible;
	param->margin = (last_visible - first_visible);
	
	param->orientation = orientation;
	
	param->position = position;
	param->granularity = granularity;
}
