#include "menu.h"
#include "bevel.h"
#include "mused.h"
#include "gfx/font.h"
#include "view.h"
#include "action.h"

extern Mused mused;

static const Menu mainmenu[];

static const Menu showmenu[] =
{
	{ mainmenu, "Instrument editor",  NULL, change_mode_action, (void*)EDITINSTRUMENT },
	{ mainmenu, "Pattern editor",  NULL, change_mode_action, (void*)EDITPATTERN },
	{ mainmenu, "Sequence editor",  NULL, change_mode_action, (void*)EDITSEQUENCE },
	{ mainmenu, "Classic editor",  NULL, change_mode_action, (void*)EDITCLASSIC },
	{ NULL, NULL }
};


static const Menu filemenu[] =
{
	{ mainmenu, "Open", NULL },
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
	mused.current_menu_action = NULL;
}


void close_menu()
{
	if (mused.current_menu_action == NULL)
		change_mode(mused.prev_mode);
	else
		mused.current_menu_action->action(mused.current_menu_action->p1, mused.current_menu_action->p2, mused.current_menu_action->p3);
}


static int get_menu_item_width(const Menu *item)
{
	return strlen(item->text);
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
			area.w = area.h = 0;
			
			const Menu * item = items;
		
			for (; item->text ; ++item)
			{
				area.w = my_max(get_menu_item_width(item), area.w);
				area.h += font->h + 1;
			}
			
			area.w *= font->w;
			area.x += 3;
			area.y += 4;
			
			copy_rect(&r, &area);
			SDL_Rect bev;
			copy_rect(&bev, &area);
			adjust_rect(&bev, -6);
			
			bevel(&bev, mused.slider_bevel, BEV_MENU);
		}
		else
		{
			bevel(&area, mused.slider_bevel, BEV_MENUBAR);
			
			copy_rect(&r, &area);
			adjust_rect(&r, 2);
			r.w = 64;
			
			font = &mused.smallfont;
		}
		
		r.h = font->h + 1;
	
		const Menu * item = items;
		
		for (; item->text ; ++item)
		{
			int bg = 0;
			
			if (event->type == SDL_MOUSEMOTION)
			{
				if ((event->button.x >= r.x) && (event->button.y >= r.y) 
					&& (event->button.x < r.x + r.w) && (event->button.y < r.y + r.h))
				{
					if (item->submenu)
					{
						mused.current_menu = item->submenu;
						mused.current_menu_action = NULL;
						bg = 1;
					}
					else if (item->action)
					{
						mused.current_menu_action = item;
						bg = 1;
					}
				}
				else if (mused.current_menu_action && item == mused.current_menu_action)
				{
					mused.current_menu_action = NULL;
				}
			}
			
			if (item->submenu == child && child)
			{
				copy_rect(child_position, &r);
				bg = 1;
			}
			
			if (bg || (mused.current_menu_action == item && mused.current_menu_action))
			{
				SDL_Rect sel;
				copy_rect(&sel, &r);
				adjust_rect(&sel, 0);
				bevel(&sel, mused.slider_bevel, BEV_MENU_SELECTED);
			}
			
			font_write(font, mused.console->surface, &r, item->text);
			
			update_rect(&area, &r);
		}
	}
}


void draw_menu(const SDL_Event *e)
{
	draw_submenu(e, mused.current_menu, NULL, NULL);
}
