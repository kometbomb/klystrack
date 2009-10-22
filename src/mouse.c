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

#include "mouse.h"
#include "macros.h"

static void (*motion_target)(int,int,void*) = NULL;
static void *motion_param = NULL;
static SDL_TimerID repeat_timer_id = 0;
static SDL_Event repeat_event;
int event_hit = 0;

Uint32 repeat_timer(Uint32 interval, void *param)
{
	SDL_PushEvent(&repeat_event);
	return 0;
}


void set_repeat_timer(const SDL_Event *event)
{
	if (repeat_timer_id != 0)
	{
		SDL_RemoveTimer(repeat_timer_id);
	}
	
	if (event)
	{
		memcpy(&repeat_event, event, sizeof(repeat_event));
		repeat_event.type = SDL_USEREVENT;
		repeat_timer_id = SDL_AddTimer(repeat_timer_id ? SDL_DEFAULT_REPEAT_INTERVAL : SDL_DEFAULT_REPEAT_DELAY, repeat_timer, NULL);
	}
	else
	{
		repeat_timer_id = 0;
	}
}


void mouse_released(const SDL_Event *event)
{
	if (event->button.button == SDL_BUTTON_LEFT)
	{
		set_repeat_timer(NULL);
		motion_target = NULL;
		motion_param = NULL;
	}
}


int check_event(const SDL_Event *event, const SDL_Rect *rect, void (*action)(void*,void*,void*), void *param1, void *param2, void *param3)
{
	switch (event->type) 
	{
		case SDL_MOUSEBUTTONDOWN:
		{
			if (event->button.button == SDL_BUTTON_LEFT)
			{
				if ((event->button.x >= rect->x) && (event->button.y >= rect->y) 
					&& (event->button.x < rect->x + rect->w) && (event->button.y < rect->y + rect->h))
				{
					if (action) 
					{	
						event_hit = 1;
						set_repeat_timer(event);
						action(param1, param2, param3);
					}
					return 1;
				}
			}
		}
		break;
	}
	
	return 0;
}


void set_motion_target(void (*action)(int,int,void*), void *param)
{
	motion_target = action;
	motion_param = param;
}


int check_drag_event(const SDL_Event *event, const SDL_Rect *rect, void (*action)(int,int,void*), void *param)
{
	switch (event->type) 
	{
		case SDL_MOUSEMOTION:
		{
			if (event->motion.state & (SDL_BUTTON(SDL_BUTTON_LEFT)|SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				if (!motion_target)
				{
					int x = event->motion.x - event->motion.xrel;
					int y = event->motion.y - event->motion.yrel;
					if ((x >= rect->x) && (y >= rect->y) 
						&& (x < rect->x + rect->w) && (y < rect->y + rect->h))
					{
						if (action) set_repeat_timer(NULL);
						set_motion_target(action, param);
					}
				}
				
				if (motion_target)
				{
					set_repeat_timer(NULL);
					motion_target(event->motion.x, event->motion.y, motion_param);
				}
			}
		}
		break;
	}
	
	return motion_param == param; // For identifying individual elements
}


