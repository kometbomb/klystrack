#include "menu.h"
#include "bevel.h"
#include "mused.h"
#include "gfx/font.h"
#include "view.h"

extern Mused mused;

static const Menu mainmenu[];

static const Menu showmenu[] =
{
	{ mainmenu, "Instrument editor" },
	{ NULL, NULL }
};


static const Menu filemenu[] =
{
	{ mainmenu, "Open" },
	{ NULL, NULL }
};


static const Menu mainmenu[] =
{
	{ NULL, "FILE", filemenu },
	{ NULL, "SHOW", showmenu },
	{ NULL, NULL }
};


void open_menu()
{
	change_mode(MENU);
	mused.current_menu = mainmenu;
}


void close_menu()
{
	change_mode(mused.prev_mode);
}


static void draw_submenu(const SDL_Event *event, const Menu *items, const Menu *child, SDL_Rect *child_position)
{
	SDL_Rect area = { 0, 0, mused.console->surface->w, mused.smallfont.h + 4 + 1 };
	SDL_Rect r;
	Font *font = NULL;
	
	/* In short: this first iterates upwards the tree until it finds the main menu (FILE, SHOW etc.)
	Then it goes back level by level 
	*/
	
	if (items)
	{
		if (items[0].parent != NULL)
		{
			draw_submenu(event, items[0].parent, items, &area);
			
			font = &mused.largefont;
			
			area.y += area.h;
			
			area.w = 128 + 8;
			area.h = 100;
			
			bevel(&area, mused.slider_bevel, BEV_MENU);
			
			copy_rect(&r, &area);
			adjust_rect(&r, 4);
						
			r.w = 128;
			r.h = font->h;
		}
		else
		{
			bevel(&area, mused.slider_bevel, BEV_MENUBAR);
			
			copy_rect(&r, &area);
			adjust_rect(&r, 2);
			r.w = 64;
			
			font = &mused.smallfont;
		}
		
		r.h = mused.smallfont.h + 1;
	
		const Menu * item = items;
		
		for (; item->text ; ++item)
		{
			font_write(font, mused.console->surface, &r, item->text);
			
			if ((event->button.x >= r.x) && (event->button.y >= r.y) 
				&& (event->button.x < r.x + r.w) && (event->button.y < r.y + r.h))
			{
				if (item->submenu)
				{
					mused.current_menu = item->submenu;
				}
			}
			
			if (item->submenu == child && child)
			{
				copy_rect(child_position, &r);
			}
			
			update_rect(&area, &r);
		}
	}
}


void draw_menu(const SDL_Event *e)
{
	draw_submenu(e, mused.current_menu, NULL, NULL);
}
