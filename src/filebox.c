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

#include "filebox.h"
#include "msgbox.h"
#include "bevel.h"
#include "dialog.h"
#include "gfx/gfx.h"
#include "mused.h"
#include "view.h"
#include "mouse.h"
#include <dirent.h>
#include <sys/stat.h>

extern GfxDomain *domain;
extern Mused mused;

#define WIDTH 280
#define HEIGHT 180
#define TOP_LEFT (SCREEN_WIDTH / 2 - WIDTH / 2)
#define TOP_RIGHT (SCREEN_HEIGHT / 2 - HEIGHT / 2)
#define MARGIN 8
#define TITLE 14

static struct
{
	const char *title;
	char **files;
	int n_files;
	SliderParam scrollbar;
	int list_position;
} data;

static void file_list_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void title_view(const SDL_Rect *area, const SDL_Event *event, void *param);

static const View filebox_view[] =
{
	{{ TOP_LEFT, TOP_RIGHT, WIDTH, HEIGHT }, bevel_view, (void*)BEV_MENU, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN, WIDTH, TITLE }, title_view, &data, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE, WIDTH - MARGIN * 2 - SCROLLBAR, HEIGHT - MARGIN * 2 - TITLE }, file_list_view, &data, -1},
	{{ TOP_LEFT + WIDTH - MARGIN - SCROLLBAR, TOP_RIGHT + MARGIN + TITLE, SCROLLBAR, HEIGHT - MARGIN * 2 - TITLE }, slider, &data.scrollbar, -1},
	{{0, 0, 0, 0}, NULL}
};


void title_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	const char* title = data.title;
	font_write(&mused.largefont, mused.console->surface,  area, title);
}


void file_list_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content, pos;
	copy_rect(&content, area);
	adjust_rect(&content, 2);
	copy_rect(&pos, &content);
	pos.h = mused.largefont.h;
	bevel(area, mused.slider_bevel, BEV_FIELD);
	
	SDL_SetClipRect(mused.console->surface, &content);
	
	for (int i = data.list_position ; i < data.n_files && pos.y < content.h + content.y ; ++i)
	{
		font_write(&mused.largefont, mused.console->surface,  &pos, data.files[i]);
		
		if (pos.y + pos.h <= content.h + content.y) slider_set_params(&data.scrollbar, 0, data.n_files - 1, data.list_position, i, &data.list_position, 1, SLIDER_VERTICAL);
		
		update_rect(&content, &pos);
	}
	
	SDL_SetClipRect(mused.console->surface, NULL);
}


static void free_files()
{
	if (data.files) 
	{
		for (int i = 0 ; data.files[i] ; ++i)
			free(data.files[i]);
		free(data.files);
	}
	
	data.files = NULL;
	data.n_files = 0;
	data.list_position = 0;
}


static void populate_files(const char *dirname)
{
	DIR * dir = opendir(dirname);
	struct dirent *de = NULL;
	
	free_files();
	
	slider_set_params(&data.scrollbar, 0, 0, 0, 0, &data.list_position, 1, SLIDER_VERTICAL);
		
	while ((de = readdir(dir)) != NULL)
	{
		if ((data.n_files & 255) == 0)
			data.files = realloc(data.files, sizeof(char*) * (data.n_files + 256));
		
		data.files[data.n_files++] = strdup(de->d_name);
	}
	
	closedir(dir);
}


int filebox(const char *title, int mode)
{
	set_repeat_timer(NULL);
	
	memset(&data, 0, sizeof(data));
	data.title = title;
	
	populate_files(".");
	
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
						
						free_files();
						return FB_CANCEL;
						
						break;
						
						case SDLK_RETURN:
						
						free_files();
						return FB_OK;
						
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
			draw_view(filebox_view, &e);
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(5);
	}
}
