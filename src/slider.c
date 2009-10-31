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
#include "dialog.h"

extern Mused mused;

int quant(int v, int g)
{
	return v - v % (g);
}


static void modify_position(void *delta, void *_param, void *unused)
{
	SliderParam *param = _param;
	*param->position = quant(*param->position + (int)delta, param->granularity);
	if (*param->position < param->first) *param->position = param->first;
	if (*param->position > param->last - param->margin) *param->position = param->last - param->margin;
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


static void drag_begin(void *event, void *_param, void *area)
{
	set_motion_target(drag_motion, _param);
	SliderParam *param = _param;
	param->drag_begin_coordinate = param->orientation == SLIDER_HORIZONTAL ? ((SDL_Event*)event)->button.x : ((SDL_Event*)event)->button.y;
	param->drag_begin_position = *param->position;
	if (param->drag_begin_position > param->last - param->margin) param->drag_begin_position = param->last - param->margin;
	if (param->drag_begin_position < param->first) param->drag_begin_position = param->first;
	param->drag_area_size = (param->orientation == SLIDER_HORIZONTAL) ? ((SDL_Rect*)area)->w : ((SDL_Rect*)area)->h;
}


void slider(const SDL_Rect *_area, const SDL_Event *event, void *_param)
{
	SliderParam *param = _param;
	
	int button_size = (param->orientation == SLIDER_HORIZONTAL) ? _area->h : _area->w;
	int area_size = ((param->orientation == SLIDER_HORIZONTAL) ? _area->w : _area->h) - button_size * 2;
	int area_start = ((param->orientation == SLIDER_HORIZONTAL) ? _area->x : _area->y) + button_size;
	int bar_size = area_size;
	int bar_top = area_start;
	
	
	if (param->visible_last > param->visible_first && param->last != param->first)
	{
		bar_top = (param->visible_first - param->first) * area_size / (param->last - param->first) + area_start;
		int bar_bottom;
		
		if (param->visible_last < param->last) 
		{
			bar_bottom = (param->visible_last - param->first) * area_size / (param->last - param->first) + area_start;
		}
		else
		{
			bar_bottom = area_size + area_start;
			bar_top = bar_bottom - (param->visible_last - param->visible_first) * area_size / (param->last - param->first);
		}
		
		bar_size = bar_bottom - bar_top;
		
		if (bar_size < 4) bar_size = 4;
		if (bar_size > area_size) { bar_size = area_size; bar_top = area_start; }
		if (bar_top + bar_size > area_size + area_start) bar_top = area_size - bar_size + area_start;
	}
	
	SDL_Rect dragarea = { _area->x, _area->y, _area->w, _area->h };
	
	if (param->orientation == SLIDER_HORIZONTAL)
	{
		dragarea.x += button_size;
		dragarea.w -= button_size * 2;
	}
	else
	{
		dragarea.y += button_size;
		dragarea.h -= button_size * 2;
	}
	
	bevel(&dragarea, mused.slider_bevel, BEV_SLIDER_BG);
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.w = bar_top - dragarea.x;
			area.x = dragarea.x;
		}
		else
		{
			area.h = bar_top - dragarea.y;
			area.y = dragarea.y;
		}
		
		check_event(event, &area, modify_position, MAKEPTR(-(param->visible_last - param->visible_first)), param, 0);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.x += bar_top - _area->x;
			area.w = bar_size;
		}
		else
		{
			area.y += bar_top - _area->y;
			area.h = bar_size;
		}
		
		int pressed = check_event(event, &area, drag_begin, MAKEPTR(event), param, MAKEPTR(&dragarea));
		pressed |= check_drag_event(event, &area, drag_motion, MAKEPTR(param));
		button(&area, mused.slider_bevel, pressed ? BEV_SLIDER_HANDLE_ACTIVE : BEV_SLIDER_HANDLE, (param->orientation == SLIDER_HORIZONTAL) ? DECAL_GRAB_HORIZ : DECAL_GRAB_VERT);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, _area->w, _area->h };
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.x = bar_top + bar_size;
			area.w = dragarea.x + dragarea.w - bar_size - bar_top;
		}
		else
		{
			area.y = bar_top + bar_size;
			area.h = dragarea.y + dragarea.h - bar_size - bar_top;
		}
		
		check_event(event, &area, modify_position, MAKEPTR(param->visible_last - param->visible_first), param, 0);
	}
	
	{
		SDL_Rect area = { _area->x, _area->y, SCROLLBAR, SCROLLBAR };
		
		button_event(event, &area, mused.slider_bevel, BEV_BUTTON, BEV_BUTTON_ACTIVE, param->orientation == SLIDER_HORIZONTAL ? DECAL_LEFTARROW : DECAL_UPARROW, modify_position, MAKEPTR(-param->granularity), param, 0);
		
		if (param->orientation == SLIDER_HORIZONTAL)
		{
			area.x += area_size + button_size;
		}
		else
		{
			area.y += area_size + button_size;
		}
		
		button_event(event, &area, mused.slider_bevel, BEV_BUTTON, BEV_BUTTON_ACTIVE, param->orientation == SLIDER_HORIZONTAL ? DECAL_RIGHTARROW : DECAL_DOWNARROW, modify_position, MAKEPTR(param->granularity), param, 0);
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


void check_mouse_wheel_event(const SDL_Event *event, const SDL_Rect *rect, SliderParam *slider)
{
	switch (event->type) 
	{
		case SDL_MOUSEBUTTONDOWN:
		{
			if ((event->button.x >= rect->x) && (event->button.y >= rect->y) 
					&& (event->button.x < rect->x + rect->w) && (event->button.y < rect->y + rect->h))
			{
				int p = (slider->visible_last - slider->visible_first) / 2;
				switch (event->button.button)
				{
					case 4: 
						*slider->position -= p;
					break;
					case 5: 
						*slider->position += p;
					break;
				}
				*slider->position = my_max(my_min(*slider->position, slider->last - (slider->visible_last - slider->visible_first)), slider->first);
			}
		}
		break;
	}
}
