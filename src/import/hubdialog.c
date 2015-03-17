#include "hubdialog.h"
#include "mused.h"
#include "view.h"
#include "gui/msgbox.h"
#include "gui/bevel.h"
#include "gui/bevdefs.h"
#include "gui/dialog.h"
#include "gfx/gfx.h"
#include "gui/view.h"
#include "gui/mouse.h"
#include "gui/toolutil.h"
#include <string.h>

#define TOP_LEFT 0
#define TOP_RIGHT 0
#define MARGIN 8
#define SCREENMARGIN 64
#define DIALOG_WIDTH 150
#define DIALOG_HEIGHT 120
#define TITLE 14
#define FIELD 14
#define CLOSE_BUTTON 12
#define ELEMWIDTH data.elemwidth
#define LIST_WIDTH data.list_width
#define BUTTONS 16

static struct
{
	int subsong;
	int selected_subsong;
	const char *title;
	int quit;
	const Font *largefont, *smallfont;
	GfxSurface *gfx;
	hubbard_t *hub;
} data;

extern const KeyShortcut shortcuts[];

static void title_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void window_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void buttons_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);
static void parameters_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param);

static const View filebox_view[] =
{
	{{ SCREENMARGIN, SCREENMARGIN, DIALOG_WIDTH, DIALOG_HEIGHT-MARGIN }, window_view, &data, -1},
	{{ MARGIN+SCREENMARGIN, SCREENMARGIN+MARGIN, DIALOG_WIDTH-MARGIN*2, TITLE - 2 }, title_view, &data, -1},
	{{ SCREENMARGIN+MARGIN, SCREENMARGIN+MARGIN+TITLE+2, DIALOG_WIDTH-MARGIN, DIALOG_HEIGHT-BUTTONS-2-MARGIN }, parameters_view, &data, -1},
	{{ SCREENMARGIN+MARGIN, DIALOG_HEIGHT-BUTTONS+2+SCREENMARGIN-MARGIN-8, DIALOG_WIDTH-MARGIN*2, BUTTONS-2 }, buttons_view, &data, -1},
	{{0, 0, 0, 0}, NULL}
};


static void ok_action(void *unused0, void *unused1, void *unused2)
{
	data.selected_subsong = data.subsong;
}


static void parameters_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect button;
	
	copy_rect(&button, area);
	
	button.w = 128;
	button.h = 10;
	
	data.subsong += generic_field(event, &button, -1, -1, "SUBSONG", "%02d", MAKEPTR(data.subsong), 2);
	button.y += button.h;
	
	if (data.subsong < 0)
		data.subsong = 0;
	
	if (data.subsong >= data.hub->n_subsongs)
		data.subsong = data.hub->n_subsongs - 1;
	
	data.hub->n_tracks += generic_field(event, &button, -1, -1, "TRACKS", "%d", MAKEPTR(data.hub->n_tracks), 1);
	button.y += button.h;
	
	if (data.hub->n_tracks < 1)
		data.hub->n_tracks = 1;
	
	if (data.hub->n_tracks > 3)
		data.subsong = 3;
	
	data.hub->addr.patternptrlo += generic_field(event, &button, -1, -1, "PAT LO", "%04X", MAKEPTR(data.hub->addr.patternptrlo), 4);
	button.y += button.h;
	
	data.hub->addr.patternptrhi += generic_field(event, &button, -1, -1, "PAT HI", "%04X", MAKEPTR(data.hub->addr.patternptrhi), 4);
	button.y += button.h;
	
	data.hub->addr.songtab += generic_field(event, &button, -1, -1, "SONGS", "%04X", MAKEPTR(data.hub->addr.songtab), 4);
	button.y += button.h;
	
	data.hub->addr.instruments += generic_field(event, &button, -1, -1, "INSTRUMENTS", "%04X", MAKEPTR(data.hub->addr.instruments), 4);
	button.y += button.h;
}


static void buttons_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect button;
	
	copy_rect(&button, area);
	
	/*button_text_event(dest_surface, event, &button, data.gfx, data.smallfont, data.mode == 0 ? BEV_BUTTON_ACTIVE : BEV_BUTTON, BEV_BUTTON_ACTIVE, "Commands", init_lines, 0, 0, 0);
	button.x += button.w + 1;
	*/
	button.w = strlen("OK") * data.largefont->w + 24;
	button.x = area->w + area->x - button.w;
	button_text_event(dest_surface, event, &button, data.gfx, data.largefont, BEV_BUTTON, BEV_BUTTON_ACTIVE, "OK", ok_action, 0, 0, 0);
	button.x += button.w + 1;
}


void window_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	bevel(dest_surface, area, data.gfx, BEV_MENU);
}


void title_view(GfxDomain *dest_surface, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	const char* title = data.title;
	SDL_Rect titlearea, button;
	copy_rect(&titlearea, area);
	titlearea.w -= CLOSE_BUTTON - 4;
	copy_rect(&button, area);
	adjust_rect(&button, titlearea.h - CLOSE_BUTTON);
	button.w = CLOSE_BUTTON;
	button.x = area->w + area->x - CLOSE_BUTTON;
	font_write(data.largefont, dest_surface, &titlearea, title);
	if (button_event(dest_surface, event, &button, data.gfx, BEV_BUTTON, BEV_BUTTON_ACTIVE, DECAL_CLOSE, NULL, MAKEPTR(1), 0, 0) & 1)
		data.quit = 1;
}


int hub_view(hubbard_t *hub)
{
	set_repeat_timer(NULL);
	
	memset(&data, 0, sizeof(data));
	data.title = "Import .SID";
	data.largefont = &mused.largefont;
	data.smallfont = &mused.smallfont;
	data.selected_subsong = -1;
	data.gfx = mused.slider_bevel;
	data.hub = hub;
	
	while (!data.quit)
	{
		SDL_Event e = { 0 };
		int got_event = 0;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
				case SDL_QUIT:
				
				set_repeat_timer(NULL);
				SDL_PushEvent(&e);
				return -1;
				
				break;
				
				case SDL_KEYDOWN:
				{
					switch (e.key.keysym.sym)
					{
						case SDLK_ESCAPE:
						
						set_repeat_timer(NULL);
						return -1;
						
						break;
						
						/*case SDLK_KP_ENTER:
						case SDLK_RETURN:
						if (data.selected_file != -1) data.picked_file = &data.files[data.selected_file];
						else goto enter_pressed;
						break;*/
												
						default: break;
					}
				
				
				}
				break;
			
				case SDL_USEREVENT:
					e.type = SDL_MOUSEBUTTONDOWN;
				break;
				
				case SDL_MOUSEMOTION:
					if (domain)
					{
						e.motion.xrel /= domain->scale;
						e.motion.yrel /= domain->scale;
						e.button.x /= domain->scale;
						e.button.y /= domain->scale;
					}
				break;
				
				case SDL_MOUSEBUTTONDOWN:
					if (domain)
					{
						e.button.x /= domain->scale;
						e.button.y /= domain->scale;
					}
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
			draw_view(domain, filebox_view, &e);
			gfx_domain_flip(domain);
		}
		else
			SDL_Delay(5);
		
		if (data.selected_subsong != -1)
			break;
	}
	
	return data.selected_subsong;
}

