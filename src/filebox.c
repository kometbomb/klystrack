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
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef WIN32
#include <sys/types.h>
#include <pwd.h>
#endif

extern GfxDomain *domain;
extern Mused mused;

#define WIDTH 280
#define HEIGHT 180
#define TOP_LEFT (SCREEN_WIDTH / 2 - WIDTH / 2)
#define TOP_RIGHT (SCREEN_HEIGHT / 2 - HEIGHT / 2)
#define MARGIN 8
#define TITLE 14
#define FIELD 14
#define CLOSE_BUTTON 12
#define PATH 10
#define ELEMWIDTH (WIDTH - MARGIN * 2)
#define LIST_WIDTH (ELEMWIDTH - SCROLLBAR)

enum { FB_DIRECTORY, FB_FILE };

enum { FOCUS_LIST, FOCUS_FIELD };

typedef struct
{
	int type;
	char *name;
	char *display_name;
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
	int quit;
	char path[1024];
} data;

static void file_list_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void title_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void field_view(const SDL_Rect *area, const SDL_Event *event, void *param);
static void path_view(const SDL_Rect *area, const SDL_Event *event, void *param);

static const View filebox_view[] =
{
	{{ TOP_LEFT, TOP_RIGHT, WIDTH, HEIGHT }, bevel_view, (void*)BEV_MENU, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN, ELEMWIDTH, TITLE - 2 }, title_view, &data, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE, ELEMWIDTH, PATH - 2 }, path_view, &data, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE + PATH, ELEMWIDTH, FIELD - 2 }, field_view, &data, -1},
	{{ TOP_LEFT + LIST_WIDTH + SCROLLBAR, TOP_RIGHT + MARGIN + TITLE + FIELD + PATH, SCROLLBAR, HEIGHT - MARGIN * 2 - TITLE - FIELD - PATH }, slider, &data.scrollbar, -1},
	{{ TOP_LEFT + MARGIN, TOP_RIGHT + MARGIN + TITLE + FIELD + PATH, LIST_WIDTH, HEIGHT - MARGIN * 2 - TITLE - FIELD - PATH }, file_list_view, &data, -1},
	{{0, 0, 0, 0}, NULL}
};


static void pick_file_action(void *file, void *unused1, void *unused2)
{
	if (data.focus == FOCUS_LIST && data.selected_file == (int)file) data.picked_file = &data.files[(int)file];
	data.selected_file = (int)file;
	data.focus = FOCUS_LIST;
	strncpy(data.field, data.files[(int)file].name, sizeof(data.field));
	data.editpos = strlen(data.field);
}


void title_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	const char* title = data.title;
	SDL_Rect titlearea, button;
	copy_rect(&titlearea, area);
	titlearea.w -= CLOSE_BUTTON - 4;
	copy_rect(&button, area);
	adjust_rect(&button, titlearea.h - CLOSE_BUTTON);
	button.w = CLOSE_BUTTON;
	button.x = area->w + area->x - CLOSE_BUTTON;
	font_write(&mused.largefont, mused.console->surface, &titlearea, title);
	if (button_event(event, &button, mused.slider_bevel, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_CLOSE, NULL, (void*)1, 0, 0) & 1)
		data.quit = 1;
}


void file_list_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content, pos;
	copy_rect(&content, area);
	adjust_rect(&content, 1);
	copy_rect(&pos, &content);
	pos.h = mused.largefont.h;
	bevel(area, mused.slider_bevel, BEV_FIELD);
	
	SDL_SetClipRect(mused.console->surface, &content);
	
	for (int i = data.list_position ; i < data.n_files && pos.y < content.h + content.y ; ++i)
	{
		if (data.selected_file == i && data.focus == FOCUS_LIST)
		{
			bevel(&pos, mused.slider_bevel, BEV_SELECTED_PATTERN_ROW);
		}
	
		if (data.files[i].type == FB_FILE)
			font_write(&mused.largefont, mused.console->surface, &pos, data.files[i].display_name);
		else
			font_write_args(&mused.largefont, mused.console->surface, &pos, "½%s", data.files[i].display_name);
		
		if (pos.y + pos.h <= content.h + content.y) slider_set_params(&data.scrollbar, 0, data.n_files - 1, data.list_position, i, &data.list_position, 1, SLIDER_VERTICAL);
		
		check_event(event, &pos, pick_file_action, (void*)i, 0, 0);
		
		update_rect(&content, &pos);
	}
	
	SDL_SetClipRect(mused.console->surface, NULL);
	
	check_mouse_wheel_event(event, area, &data.scrollbar);
}


void field_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect content, pos;
	copy_rect(&content, area);
	adjust_rect(&content, 1);
	copy_rect(&pos, &content);
	pos.w = mused.largefont.w;
	pos.h = mused.largefont.h;
	bevel(area, mused.slider_bevel, BEV_FIELD);
	
	if (data.focus == FOCUS_FIELD)
	{
		int i = 0;
		size_t length = strlen(data.field);
		for ( ; data.field[i] && i < length ; ++i)
		{
			font_write_args(&mused.largefont, mused.console->surface, &pos, "%c", data.editpos == i ? '§' : data.field[i]);
			if (check_event(event, &pos, NULL, NULL, NULL, NULL))
				data.editpos = i;
			pos.x += pos.w;
		}
		
		if (data.editpos == i && i < length + 1) 
			font_write(&mused.largefont, mused.console->surface, &pos, "§");
	}
	else
		font_write(&mused.largefont, mused.console->surface, &content, data.field);
	
	if (check_event(event, area, NULL, 0, 0, 0)) data.focus = FOCUS_FIELD;
}


static void path_view(const SDL_Rect *area, const SDL_Event *event, void *param)
{
	font_write(&mused.smallfont, mused.console->surface, area, data.path);
}


static void free_files()
{
	if (data.files) 
	{
		for (int i = 0 ; i < data.n_files ; ++i)
		{
			free(data.files[i].name);
			free(data.files[i].display_name);
		}
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
		return strcasecmp(left->name, right->name);
	}
	else
	{
		return left->type > right->type ? 1 : -1;
	}
}


static int checkext(const char * filename, const char *extension)
{
	int i = strlen(filename);
	while (i > 0)
	{
		if (filename[i] == '.') break;
		--i;
	}
	
	if (i < 0) return 0;
	
	return strcasecmp(&filename[i + 1], extension) == 0;
}


static char * expand_tilde(const char * path)
{
#ifndef WIN32
	if (path[0] != '~') return NULL;
	
	const char *rest = strchr(path, '/');
	char *name = NULL;
	
	if (rest != NULL)
	{
		size_t l = (rest - (path + 1)) / sizeof(*name);
		if (l)
		{
			name = calloc(sizeof(*name), l + 1);
			strncpy(name, path + 1, l);
		}
	}
	
	const char *homedir = NULL;
	
	if (name) 
	{
		struct passwd *pwd = getpwdnam(name);
		homedir = pwd->pw_dir;
		free(name);
	}
	else
	{
		homedir = getenv("HOME");
	}
	
	char * final = malloc(strlen(homedir) + strlen(rest) + 2);
	sprintf(final, "%s%s", homedir, rest);
	
	return final;
#else
	return NULL;
#endif
}


static int populate_files(const char *dirname, const char *extension)
{
	debug("Opening directory %s", dirname);

	char * expanded = expand_tilde(dirname);
	
	int r = chdir(expanded == NULL ? dirname : expanded);
	
	if (expanded) free(expanded);
	
	if (r)
	{
		warning("chdir failed");
		return 0;
	}
	
	getcwd(data.path, sizeof(data.path) - 1);
	
	size_t l;
	if ((l = strlen(data.path)) > ELEMWIDTH / mused.smallfont.w)
	{
		memmove(&data.path[3], &data.path[l - ELEMWIDTH / mused.smallfont.w + 3], l + 1); 
		memcpy(data.path, "...", 3);
	}
	
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
				if ((attribute.st_mode & S_IFDIR) || checkext(de->d_name, extension))
				{
					const int block_size = 256;
				
					if ((data.n_files & (block_size - 1)) == 0)
					{
						data.files = realloc(data.files, sizeof(*data.files) * (data.n_files + block_size));
					}
					data.files[data.n_files].type = ( attribute.st_mode & S_IFDIR ) ? FB_DIRECTORY : FB_FILE;
					data.files[data.n_files].name = strdup(de->d_name);
					data.files[data.n_files].display_name = strdup(de->d_name);
					if (strlen(data.files[data.n_files].display_name) > LIST_WIDTH / mused.largefont.w - 4)
					{
						data.files[data.n_files].display_name[LIST_WIDTH / mused.largefont.w - 4] = '\0';
					}
					
					++data.n_files;
				}
			}
		}
	}
	
	closedir(dir);
	
	debug("Got %d files", data.n_files);
	
	data.selected_file = 0;
	data.list_position = 0;
	data.editpos = 0;
	strcpy(data.field, "");
	
	qsort(data.files, data.n_files, sizeof(*data.files), file_sorter);
	
	return 1;
}


int filebox(const char *title, int mode, char *buffer, size_t buffer_size, const char *extension)
{
	set_repeat_timer(NULL);
	
	memset(&data, 0, sizeof(data));
	data.title = title;
	data.mode = mode;
	data.picked_file = NULL;
	
	if (!populate_files(".", extension)) return FB_CANCEL;
	
	while (!data.quit)
	{
		if (data.picked_file)
		{
			set_repeat_timer(NULL);
			if (data.picked_file->type == FB_FILE)
			{
				if (mode == FB_OPEN || (mode == FB_SAVE && msgbox("Overwrite?", MB_YES|MB_NO) == MB_YES))
				{
					set_repeat_timer(NULL);
					strncpy(buffer, data.picked_file->name, buffer_size);
					free_files();
					return FB_OK;
				}
				
				// note that after the populate_files() picked_file will point to some other file!
				// thus we need to check this before the FB_DIRECTORY handling below
			}
			else if (data.picked_file->type == FB_DIRECTORY && !populate_files(data.picked_file->name, extension)) 
			{
				
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
				
				set_repeat_timer(NULL);
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
							
							set_repeat_timer(NULL);
							free_files();
							return FB_CANCEL;
							
							break;
							
							case SDLK_RETURN:
							data.picked_file = &data.files[data.selected_file];
							break;
							
							case SDLK_DOWN:
							move_position(&data.selected_file, &data.list_position, &data.scrollbar, 1, data.n_files);
							strncpy(data.field, data.files[data.selected_file].name, sizeof(data.field));
							data.editpos = strlen(data.field);
							break;
							
							case SDLK_UP:
							move_position(&data.selected_file, &data.list_position, &data.scrollbar, -1, data.n_files);
							strncpy(data.field, data.files[data.selected_file].name, sizeof(data.field));
							data.editpos = strlen(data.field);
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
												set_repeat_timer(NULL);
												strncpy(buffer, data.field, buffer_size);
												free_files();
												return FB_OK;
											}
										}
										else
										{
											if (attribute.st_mode & S_IFDIR)
												populate_files(data.field, extension);
											else
											{
												set_repeat_timer(NULL);
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
											set_repeat_timer(NULL);
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
	
	free_files();
	return FB_CANCEL;
}
