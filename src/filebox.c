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
#include "event.h"
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
#define FIELD 14

enum { FB_DIRECTORY, FB_FILE };

enum { FOCUS_LIST, FOCUS_FIELD };

typedef struct
{
	int type;
	char *name;
} File;

static struct
{
	int mode;
	const char *title;
	File *files;
	int n_files;
	SliderParam scrollbar;
	int list_position, selected_file;
	File * picked_file;
	int focus;
	char field[256];
	int editpos;
} data;

static void file_list_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void title_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void field_view(const SDL_Rect *area, const SDL_Event *event, void *param);

static const View filebox_view[] =
{
	{{ TOP_LEFT, TOP_RIGHT, WIDTH, HEIGHT }, bevel_view, (void*)BEV_MENU, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN, WIDTH - MARGIN * 2, TITLE - 2 }, title_view, &data, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE, WIDTH - MARGIN * 2, FIELD - 2 }, field_view, &data, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE + FIELD, WIDTH - MARGIN * 2 - SCROLLBAR, HEIGHT - MARGIN * 2 - TITLE - FIELD }, file_list_view, &data, -1},
	{{ TOP_LEFT + WIDTH - MARGIN - SCROLLBAR, TOP_RIGHT + MARGIN + TITLE + FIELD, SCROLLBAR, HEIGHT - MARGIN * 2 - TITLE - FIELD }, slider, &data.scrollbar, -1},
	{{0, 0, 0, 0}, NULL}
};


static void pick_file_action(void *file, void *unused1, void *unused2)
{
	if (data.selected_file == (int)file) data.picked_file = &data.files[(int)file];
	data.selected_file = (int)file;
	data.focus = FOCUS_LIST;
	strncpy(data.field, data.files[(int)file].name, sizeof(data.field));
}


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
		if (data.selected_file == i)
		{
			bevel(&pos, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
		}
	
		if (data.files[i].type == FB_FILE)
			font_write(&mused.largefont, mused.console->surface, &pos, data.files[i].name);
		else
			font_write_args(&mused.largefont, mused.console->surface, &pos, "½%s", data.files[i].name);
		
		if (pos.y + pos.h <= content.h + content.y) slider_set_params(&data.scrollbar, 0, data.n_files - 1, data.list_position, i, &data.list_position, 1, SLIDER_VERTICAL);
		
		check_event(event, &pos, pick_file_action, (void*)i, 0, 0);
		
		update_rect(&content, &pos);
	}
	
	SDL_SetClipRect(mused.console->surface, NULL);
}


void field_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content;
	copy_rect(&content, area);
	adjust_rect(&content, 1);
	bevel(area, mused.slider_bevel, BEV_FIELD);
	font_write(&mused.largefont, mused.console->surface, &content, data.field);
	
	if (check_event(event, area, NULL, 0, 0, 0)) data.focus = FOCUS_FIELD;
}


static void free_files()
{
	if (data.files) 
	{
		for (int i = 0 ; i < data.n_files ; ++i)
			free(data.files[i].name);
		free(data.files);
	}
	
	data.files = NULL;
	data.n_files = 0;
	data.list_position = 0;
}


static int file_sorter(const void *_left, const void *_right)
{
	// directories come before files, otherwise case-insensitive name sorting

	const File *left = _left;
	const File *right = _right;
	
	if (left->type == right->type)
	{
		return stricmp(left->name, right->name);
	}
	else
	{
		return left->type > right->type ? 1 : -1;
	}
}


static int populate_files(const char *dirname)
{
	debug("Opening directory %s", dirname);

	chdir(dirname);
	DIR * dir = opendir(".");
	
	if (!dir)
	{
		msgbox("Could not open directory", MB_OK);
		return 0;
	}
	
	struct dirent *de = NULL;
	
	free_files();
	
	slider_set_params(&data.scrollbar, 0, 0, 0, 0, &data.list_position, 1, SLIDER_VERTICAL);
		
	while ((de = readdir(dir)) != NULL)
	{
		struct stat attribute;
		
		if (stat(de->d_name, &attribute) != -1)
		{		
			if (de->d_name[0] != '.' || strcmp(de->d_name, "..") == 0)
			{
				const int block_size = 256;
			
				if ((data.n_files & (block_size - 1)) == 0)
					data.files = realloc(data.files, sizeof(*data.files) * (data.n_files + block_size));
			
				data.files[data.n_files].type = ( attribute.st_mode & S_IFDIR ) ? FB_DIRECTORY : FB_FILE;
				data.files[data.n_files].name = strdup(de->d_name);
			
				++data.n_files;
			}
		}
	}
	
	closedir(dir);
	
	debug("Got %d files", data.n_files);
	
	data.selected_file = 0;
	data.list_position = 0;
	
	qsort(data.files, data.n_files, sizeof(*data.files), file_sorter);
	
	return 1;
}


int filebox(const char *title, int mode, char *buffer, size_t buffer_size)
{
	set_repeat_timer(NULL);
	
	memset(&data, 0, sizeof(data));
	data.title = title;
	data.mode = mode;
	data.picked_file = NULL;
	
	if (!populate_files(".")) return FB_CANCEL;
	
	while (1)
	{
		if (data.picked_file)
		{
			if (data.picked_file->type == FB_DIRECTORY && !populate_files(data.picked_file->name)) 
			{
				return FB_CANCEL;
			}
			else if (data.picked_file->type == FB_FILE)
			{
				if (mode == FB_SAVE && msgbox("Overwrite?", MB_YES|MB_NO) == MB_YES)
				{
					strncpy(buffer, data.picked_file->name, buffer_size);
					free_files();
					return FB_OK;
				}
			}
		}
	
		data.picked_file = NULL;
		
		SDL_Event e = { 0 };
		int got_event = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
				
				SDL_PushEvent(&e);
				free_files();
				return FB_CANCEL;
				
				break;
				
				case SDL_KEYDOWN:
				{
					if (data.focus == FOCUS_LIST)
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_ESCAPE:
							
							free_files();
							return FB_CANCEL;
							
							break;
							
							case SDLK_RETURN:
							data.picked_file = &data.files[data.selected_file];
							break;
							
							case SDLK_DOWN:
							move_position(&data.selected_file, &data.list_position, &data.scrollbar, 1, data.n_files);
							break;
							
							case SDLK_UP:
							move_position(&data.selected_file, &data.list_position, &data.scrollbar, -1, data.n_files);
							break;
							
							case SDLK_TAB:
							data.focus = FOCUS_FIELD;
							break;
							
							default: break;
						}
					}
					else
					{
						switch (e.key.keysym.sym)
						{
							case SDLK_TAB:
							data.focus = FOCUS_LIST;
							break;
						
							default:
							{
								int r = generic_edit_text(&e, data.field, sizeof(data.field) - 1, &data.editpos);
								if (r == 1)
								{
									struct stat attribute;
				
									int s = stat(data.field, &attribute);
									
									if (s != -1)
									{
										if (mode == FB_SAVE)
										{
											if (msgbox("Overwrite?", MB_YES|MB_NO) == MB_YES)
											{
												strncpy(buffer, data.field, buffer_size);
												free_files();
												return FB_OK;
											}
										}
										else
										{
											if (attribute.st_mode & S_IFDIR)
												populate_files(data.field);
											else
											{
												strncpy(buffer, data.field, buffer_size);
												free_files();
												return FB_OK;
											}
										}
									}
									else 
									{
										if (mode == FB_SAVE)
										{
											strncpy(buffer, data.field, buffer_size);
											free_files();
											return FB_OK;
										}
									}
								}
								else if (r == -1)
								{
									free_files();
									return FB_CANCEL;
								}
							}
							break;
						}
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
