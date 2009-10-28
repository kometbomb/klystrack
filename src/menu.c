#include "menu.h"
#include "bevel.h"
#include "mused.h"
#include "gfx/font.h"
#include "view.h"
#include "action.h"
#include "shortcuts.h"
#include "msgbox.h"

#define SC_SIZE 64
#define MENU_CHECK (void*)1
#define MENU_CHECK_NOSET (void*)2

enum { ZONE, DRAW };

extern Mused mused;

static const Menu mainmenu[];
static const Menu showmenu[];

extern Menu thememenu[];

static const Menu editormenu[] =
{
	{ 0, showmenu, "Instrument",  NULL, change_mode_action, (void*)EDITINSTRUMENT, 0, 0 },
	{ 0, showmenu, "Pattern",  NULL, change_mode_action, (void*)EDITPATTERN, 0, 0 },
	{ 0, showmenu, "Sequence",  NULL, change_mode_action, (void*)EDITSEQUENCE, 0, 0 },
	{ 0, showmenu, "Classic",  NULL, change_mode_action, (void*)EDITCLASSIC, 0, 0 },
	{ 0, showmenu, "Effects",  NULL, change_mode_action, (void*)EDITREVERB, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu showmenu[] =
{
	{ 0, mainmenu, "Editor", editormenu, NULL },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Compact", NULL, MENU_CHECK, &mused.flags, (void*)COMPACT_VIEW, 0 },
	{ 0, NULL, NULL }
};


const Menu prefsmenu[];

Menu pixelmenu[] =
{
	{ 0, prefsmenu, "1x1", NULL, change_pixel_scale, (void*)1, 0, 0 },
	{ 0, prefsmenu, "2x2", NULL, change_pixel_scale, (void*)2, 0, 0 },
	{ 0, prefsmenu, "3x3", NULL, change_pixel_scale, (void*)3, 0, 0 },
	{ 0, prefsmenu, "4x4", NULL, change_pixel_scale, (void*)4, 0, 0 },
	{ 0, NULL,NULL },
};


const Menu prefsmenu[] =
{
	{ 0, mainmenu, "Theme", thememenu, NULL },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Pixel size", pixelmenu },
	{ 0, mainmenu, "Fullscreen", NULL, MENU_CHECK_NOSET, &mused.flags, (void*)FULLSCREEN, toggle_fullscreen },
	{ 0, NULL, NULL }
};


static const Menu filemenu[] =
{
	{ 0, mainmenu, "New", NULL, new_song_action },
	{ 0, mainmenu, "Open", NULL, open_song_action },
	{ 0, mainmenu, "Save", NULL, save_song_action },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Exit", NULL, quit_action },
	{ 0, NULL, NULL }
};


static const Menu playmenu[] =
{
	{ 0, mainmenu, "Play",  NULL, play, (void*)0, 0, 0 },
	{ 0, mainmenu, "Stop",  NULL, stop, 0, 0, 0 },
	{ 0, mainmenu, "Play from cursor",  NULL, play, (void*)1, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu infomenu[] =
{
	{ 0, mainmenu, "About",  NULL, show_about_box, (void*)0, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu mainmenu[] =
{
	{ 0, NULL, "FILE", filemenu },
	{ 0, NULL, "PLAY", playmenu },
	{ 0, NULL, "SHOW", showmenu },
	{ 0, NULL, "PREFS", prefsmenu },
	{ 0, NULL, "INFO", infomenu },
	{ 0, NULL, NULL }
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
		if (mused.current_menu_action->action == MENU_CHECK || mused.current_menu_action->action == MENU_CHECK_NOSET)
		{
			if (mused.current_menu_action->action == MENU_CHECK) *(int*)(mused.current_menu_action->p1) ^= (int)(mused.current_menu_action->p2);
			
			if (mused.current_menu_action->p3)
				((void *(*)(void*,void*,void*))(mused.current_menu_action->p3))(0,0,0);
		}
		else
		{
			mused.current_menu_action->action(mused.current_menu_action->p1, mused.current_menu_action->p2, mused.current_menu_action->p3);
		}
		
		mused.current_menu = NULL;
		mused.current_menu_action = NULL;
	}
}


static int get_menu_item_width(const Menu *item)
{
	return strlen(item->text) + 1;
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
		if (((item->action == MENU_CHECK || item->action == MENU_CHECK_NOSET) && (void*)shortcuts[i].action == item->p3) ||
			(shortcuts[i].action == item->action &&
			(void*)shortcuts[i].p1 == item->p1 &&
			(void*)shortcuts[i].p3 == item->p2 &&
			(void*)shortcuts[i].p2 == item->p3))
		{
			strcpy(buffer, "");
			
			if (shortcuts[i].mod & KMOD_CTRL)
				strncat(buffer, "ctrl-", sizeof(buffer));
				
			if (shortcuts[i].mod & KMOD_ALT)
				strncat(buffer, "alt-", sizeof(buffer));
				
			if (shortcuts[i].mod & KMOD_SHIFT)
				strncat(buffer, "shift-", sizeof(buffer));
			
			char keyname[4] = { 0 };
			
			strncpy(keyname, SDL_GetKeyName(shortcuts[i].key), 3);
			
			strncat(buffer, keyname, sizeof(buffer) - 1);
			return upcase(buffer);
		}
		else if (item->submenu)
		{
			return "½";
		}
	}

	return NULL;
}


// Below is a two-pass combined event and drawing menu routine. It is two-pass (as opposed to other combined drawing
// handlers otherwhere in the project) so it can handle overlapping zones correctly. Otherwhere in the app there simply
// are no overlapping zones, menus however can overlap because of the cascading submenus etc.

static void draw_submenu(const SDL_Event *event, const Menu *items, const Menu *child, SDL_Rect *child_position, int pass)
{
	SDL_Rect area = { 0, 0, mused.screen->w, mused.smallfont.h + 4 + 1 };
	SDL_Rect r;
	Font *font = NULL;
	int horiz = 0;
	
	/* In short: this first iterates upwards the tree until it finds the main menu (FILE, SHOW etc.)
	Then it goes back level by level updating the collision/draw zones
	*/
	
	if (items)
	{
		if (items[0].parent != NULL)
		{
			draw_submenu(event, items[0].parent, items, &area, pass);
			
			font = &mused.largefont;
			
			area.w = area.h = 0;
			
			const Menu * item = items;
		
			for (; item->text ; ++item)
			{
				area.w = my_max(get_menu_item_width(item), area.w);
				if (item->text[0])
					area.h += font->h + 1;
				else
					area.h += SEPARATOR_HEIGHT;
			}
			
			area.w = area.w * font->w;
			area.x += 3;
			area.y += 4;
			
			area.w += SC_SIZE;
			
			if (area.w + area.x > mused.screen->w)
				area.x -= area.w + area.x - mused.screen->w + 2;
			
			copy_rect(&r, &area);
			
			SDL_Rect bev;
			copy_rect(&bev, &area);
			adjust_rect(&bev, -6);
			
			if (pass == DRAW) bevel(&bev, mused.slider_bevel, BEV_MENU);
			
			r.h = font->h + 1;
		}
		else
		{
			if (pass == DRAW) bevel(&area, mused.slider_bevel, BEV_MENUBAR);
			
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
				
				if (horiz) r.w = font->w * get_menu_item_width(item) + 8;
				
				if (event->type == SDL_MOUSEMOTION && pass == ZONE)
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
							mused.current_menu = items;
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
					if (horiz) child_position->y += r.h;
					else { child_position->x += r.w + 4; child_position->y -= 4; } 
					bg = 1;
				}
				
				if ((pass == DRAW) && (bg || (mused.current_menu_action == item && mused.current_menu_action)))
				{
					SDL_Rect bar;
					copy_rect(&bar, &r);
					adjust_rect(&bar, -1);
					bar.h --;
					bevel(&bar, mused.slider_bevel, BEV_MENU_SELECTED);
				}
				
				if (pass == DRAW) 
				{
					SDL_Rect text;
					copy_rect(&text, &r);
					text.x += font->w;
					text.w -= font->w;
					font_write(font, mused.screen, &text, item->text);
					
					char tick_char[2] = { 0 };
					
					if ((item->action == MENU_CHECK || item->action == MENU_CHECK_NOSET) && (*(int*)item->p1 & (int)item->p2))
						*tick_char = '§';
					else if (item->flags & MENU_BULLET)
						*tick_char = '^';
					
					if (tick_char[0] != 0)
					{
						SDL_Rect tick;
						copy_rect(&tick, &r);
						tick.y = r.h / 2 + r.y - mused.smallfont.h / 2;
						font_write(&mused.smallfont, mused.screen, &tick, tick_char);
					}
				}
				
				if (pass == DRAW && !horiz && sc_text) 
				{
					r.x += r.w;
					int tmpw = r.w, tmpx = r.x, tmpy = r.y;
					r.w = SC_SIZE;
					r.x -= strlen(sc_text) * mused.smallfont.w;
					r.y = r.h / 2 + r.y - mused.smallfont.h / 2;
					font_write(&mused.smallfont, mused.screen, &r, sc_text);
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
	draw_submenu(e, mused.current_menu, NULL, NULL, ZONE);
	draw_submenu(e, mused.current_menu, NULL, NULL, DRAW);
}
