#include "gui/menu.h"
#include "action.h"
#include "edit.h"
#include "mused.h"
#include "import.h"

extern Mused mused;

const Menu mainmenu[];
static const Menu showmenu[];

extern Menu thememenu[];

Menu editormenu[] =
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
	{ 0, mainmenu, "Import .MOD", NULL, import_module },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Exit", NULL, quit_action },
	{ 0, NULL, NULL }
};


static const Menu playmenu[] =
{
	{ 0, mainmenu, "Play",  NULL, play, (void*)0, 0, 0 },
	{ 0, mainmenu, "Stop",  NULL, stop, 0, 0, 0 },
	{ 0, mainmenu, "Play from cursor",  NULL, play, (void*)1, 0, 0 },
	{ 0, mainmenu, "",  NULL },
	{ 0, mainmenu, "Unmute all channels",  NULL, unmute_all_action, 0, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu infomenu[] =
{
	{ 0, mainmenu, "About",  NULL, show_about_box, (void*)0, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu editmenu[];


static const Menu editpatternmenu[] =
{
	{ 0, editmenu, "Clone",  NULL, clone_pattern, 0, 0, 0 },
	{ 0, editmenu, "Find empty",  NULL, get_unused_pattern, 0, 0, 0 },
	{ 0, editmenu, "",  NULL },
	{ 0, editmenu, "Expand 2X",  NULL, expand_pattern, MAKEPTR(2), 0, 0 },
	{ 0, editmenu, "Shrink 2X",  NULL, shrink_pattern, MAKEPTR(2), 0, 0 },
	{ 0, editmenu, "Expand 3X",  NULL, expand_pattern, MAKEPTR(3), 0, 0 },
	{ 0, editmenu, "Shrink 3X",  NULL, shrink_pattern, MAKEPTR(3), 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu editmenu[] =
{
	{ 0, mainmenu, "Copy", NULL, generic_action, copy, 0, 0 },
	{ 0, mainmenu, "Paste", NULL, generic_action, paste, 0, 0 },
	{ 0, mainmenu, "Cut", NULL, generic_action, cut, 0, 0 },
	{ 0, mainmenu, "Delete", NULL, generic_action, delete, 0, 0 },
	{ 0, mainmenu, "Deselect", NULL, clear_selection, 0, 0, 0 },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Pattern", editpatternmenu, NULL },
	{ 0, mainmenu, "",  NULL },
	{ 0, mainmenu, "Interpolate", NULL, interpolate, 0, 0, 0 },
	{ 0, NULL, NULL }
};


const Menu mainmenu[] =
{
	{ 0, NULL, "FILE", filemenu },
	{ 0, NULL, "PLAY", playmenu },
	{ 0, NULL, "EDIT", editmenu },
	{ 0, NULL, "SHOW", showmenu },
	{ 0, NULL, "PREFS", prefsmenu },
	{ 0, NULL, "INFO", infomenu },
	{ 0, NULL, NULL }
};
