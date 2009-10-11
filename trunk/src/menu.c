#include "menu.h"
#include "bevel.h"
#include "mused.h"
#include "gfx/font.h"
#include "view.h"
#include "action.h"
#include "shortcuts.h"

#define SC_SIZE 64

extern Mused mused;

static const Menu mainmenu[];

static const Menu showmenu[] =
{
	{ mainmenu, "Instrument editor",  NULL, change_mode_action, (void*)EDITINSTRUMENT, 0, 0 },
	{ mainmenu, "Pattern editor",  NULL, change_mode_action, (void*)EDITPATTERN, 0, 0 },
	{ mainmenu, "Sequence editor",  NULL, change_mode_action, (void*)EDITSEQUENCE, 0, 0 },
	{ mainmenu, "Classic editor",  NULL, change_mode_action, (void*)EDITCLASSIC, 0, 0 },
	{ mainmenu, "Reverb",  NULL, change_mode_action, (void*)EDITREVERB, 0, 0 },
	{ NULL, NULL }
};


static const Menu filemenu[] =
{
	{ mainmenu, "New", NULL, new_song_action },
	{ mainmenu, "Open", NULL, open_song_action },
	{ mainmenu, "Save", NULL, save_song_action },
	{ mainmenu, "", NULL, NULL },
	{ mainmenu, "Exit", NULL, quit_action },
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
	{
		change_mode(mused.prev_mode);
		mused.current_menu_action->action(mused.current_menu_action->p1, mused.current_menu_action->p2, mused.current_menu_action->p3);
		mused.current_menu = NULL;
		mused.current_menu_action = NULL;
	}
}


static int get_menu_item_width(const Menu *item)
{
	return strlen(item->text);
}


const char *upcase(char *str)
{
	for (char *c = str ; *c ; ++c)
		*c = toupper(*c);
		
	return str;
}


static const char * get_shortcut_key(const Menu *item)
{
	static char buffer[100];
	
	for (int i = 0 ; shortcuts[i].action ; ++i)
	{
		if (shortcuts[i].action == item->action &&
			(void*)shortcuts[i].p1 == item->p1 &&
			(void*)shortcuts[i].p3 == item->p2 &&
			(void*)shortcuts[i].p2 == item->p3)
		{
			strcpy(buffer, "");
			
			if (shortcuts[i].mod & KMOD_CTRL)
				strncat(buffer, "ctrl-", sizeof(buffer));
				
			if (shortcuts[i].mod & KMOD_ALT)
				strncat(buffer, "alt-", sizeof(buffer));
				
			if (shortcuts[i].mod & KMOD_SHIFT)
				strncat(buffer, "shift-", sizeof(buffer));
			
			strncat(buffer, SDL_GetKeyName(shortcuts[i].key), sizeof(buffer));
			return upcase(buffer);
		}
	}

	return NULL;
}


static void draw_submenu(const SDL_Event *event, const Menu *items, const Menu *child, SDL_Rect *child_position)
{
	SDL_Rect area = { 0, 0, mused.console->surface->w, mused.smallfont.h + 4 + 1 };
	SDL_Rect r;
	Font *font = NULL;
	int horiz = 0;
	
	/* In short: this first iterates upwards the tree until it finds the main menu (FILE, SHOW etc.)
	Then it goes back level by level 
	*/
	
	if (items)
	{
		if (items[0].parent != NULL)
		{
			draw_submenu(event, items[0].parent, items, &area);
			
			font = &mused.largefont;
			
			area.w = area.h = 0;
			
			const Menu * item = items;
		
			for (; item->text ; ++item)
			{
				area.w = my_max(get_menu_item_width(item), area.w);
				if (item->text[0])
					area.h += font->h + 1;
				else
					area.h += SEPARATOR_HEIGHT + 1;
			}
			
			area.w = area.w * font->w;
			area.x += 3;
			area.y += 4;
			
			area.w += SC_SIZE;
			
			if (area.w + area.x > mused.console->surface->w)
				area.x -= area.w + area.x - mused.console->surface->w;
			
			copy_rect(&r, &area);
			
			SDL_Rect bev;
			copy_rect(&bev, &area);
			adjust_rect(&bev, -6);
			
			bevel(&bev, mused.slider_bevel, BEV_MENU);
			
			r.h = font->h + 1;
		}
		else
		{
			bevel(&area, mused.slider_bevel, BEV_MENUBAR);
			
			copy_rect(&r, &area);
			adjust_rect(&r, 2);
			
			font = &mused.smallfont;
			
			horiz = 1;
			
			r.h = font->h;
		}
		
		const Menu * item = items;
		
		for (; item->text ; ++item)
		{
			if (item->text[0])
			{
				const char * sc_text = get_shortcut_key(item);
				int bg = 0;
				
				if (horiz) r.w = font->w * get_menu_item_width(item) + 16;
				
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
					child_position->y += r.h;
					bg = 1;
				}
				
				if (bg || (mused.current_menu_action == item && mused.current_menu_action))
				{
					SDL_Rect bar;
					copy_rect(&bar, &r);
					adjust_rect(&bar, -1);
					bar.h --;
					bevel(&bar, mused.slider_bevel, BEV_MENU_SELECTED);
				}
				
				font_write(font, mused.console->surface, &r, item->text);
				
				if (!horiz && sc_text) 
				{
					r.x += r.w;
					int tmpw = r.w, tmpx = r.x, tmpy = r.y;
					r.w = SC_SIZE;
					r.x -= strlen(sc_text) * mused.smallfont.w;
					r.y = r.h / 2 + r.y - mused.smallfont.h / 2;
					font_write(&mused.smallfont, mused.console->surface, &r, sc_text);
					r.x = tmpx;
					r.y = tmpy;
					update_rect(&area, &r);
					r.w = tmpw;
				}
				else update_rect(&area, &r);
			}
			else
			{
				separator(&area, &r);
			}
		}
	}
}


void draw_menu(const SDL_Event *e)
{
	draw_submenu(e, mused.current_menu, NULL, NULL);
}
