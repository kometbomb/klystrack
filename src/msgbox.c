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

#include "msgbox.h"
#include "bevel.h"
#include "dialog.h"
#include "gfx/gfx.h"
#include "mused.h"
#include "view.h"
#include "mouse.h"

extern GfxDomain *domain;
extern Mused mused;


static int draw_box(const SDL_Event *event, const char *msg, int buttons, int *selected)
{
	SDL_Rect area = { mused.console->surface->w / 2 - 100, mused.console->surface->h / 2 - 24, 200, 48 };
	
	bevel(&area, mused.slider_bevel, BEV_MENU);
	SDL_Rect content, pos;
	copy_rect(&content, &area);
	adjust_rect(&content, 8);
	copy_rect(&pos, &content);
	
	pos.h = 18;
	
	font_write(&mused.largefont, mused.console->surface, &pos, msg);
	update_rect(&content, &pos);
	
	int b = 0;
	for (int i = 0 ; i < BUTTON_TYPES ; ++i)
		if (buttons & (1 << i)) ++b;
		
	*selected = (*selected + b) % b;
	
	pos.w = 50;
	pos.h = 14;
	pos.x = content.x + content.w / 2 - b * (pos.w + ELEMENT_MARGIN) / 2 + ELEMENT_MARGIN / 2;
	
	int r = 0;
	static const char *label[] = { "YES", "NO", "CANCEL", "OK" };
	int idx = 0;
	
	for (int i = 0 ; i < BUTTON_TYPES ; ++i)
	{
		if (buttons & (1 << i))
		{
			int p = button_text_event(event, &pos, mused.slider_bevel, BEV_BUTTON, BEV_BUTTON_ACTIVE, label[i], NULL, 0, 0, 0);
			
			if (idx == *selected)
			{	
				if (event->type == SDL_KEYDOWN && (event->key.keysym.sym == SDLK_SPACE || event->key.keysym.sym == SDLK_RETURN))
					p = 1;
			
				bevel(&pos, mused.slider_bevel, BEV_CURSOR);
			}
			
			update_rect(&content, &pos);
			if (p & 1) r = (1 << i);
			++idx;
		}
	}
	
	return r;
}


int msgbox(const char *msg, int buttons)
{
	set_repeat_timer(NULL);
	
	int selected = 0;
	
	while (1)
	{
		SDL_Event e = { 0 };
		int got_event = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_KEYDOWN:
				{
					switch (e.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						
						if (buttons & CANCEL)
							return CANCEL;
						if (buttons & NO)
							return NO;
						return OK;
						
						break;
						
						case SDLK_SPACE:
						case SDLK_RETURN:
						
							
						
						break;
						
						case SDLK_LEFT:
						--selected;
						break;
						
						case SDLK_RIGHT:
						++selected;
						break;
						
						default: break;
					}
				}
				break;
			
				case SDL_USEREVENT:
					e.type = SDL_MOUSEBUTTONDOWN;
				break;
				
				case SDL_MOUSEMOTION:
					e.motion.xrel /= domain->scale;
					e.motion.yrel /= domain->scale;
					e.button.x /= domain->scale;
					e.button.y /= domain->scale;
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					e.button.x /= domain->scale;
					e.button.y /= domain->scale;
				break;
				
				case SDL_MOUSEBUTTONUP:
				{
					if (e.button.button == SDL_BUTTON_LEFT)
						mouse_released(&e);
				}
				break;
			}
			
			if (e.type != SDL_MOUSEMOTION || (e.motion.state)) ++got_event;
			
			// ensure the last event is a mouse click so it gets passed to the draw/event code
			
			if (e.type == SDL_MOUSEBUTTONDOWN || (e.type == SDL_MOUSEMOTION && e.motion.state)) break; 
		}
		
		if (got_event || gfx_domain_is_next_frame(domain))
		{
			int r = draw_box(&e, msg, buttons, &selected);
			gfx_domain_flip(domain);
			if (r) 
			{
				set_repeat_timer(NULL);
				return r;
			}
		}
		else
			SDL_Delay(5);
	}
}
