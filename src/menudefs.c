/*
Copyright (c) 2009-2010 Tero Lindeman (kometbomb)

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

#include "gui/menu.h"
#include "action.h"
#include "diskop.h"
#include "edit.h"
#include "mused.h"
#include "import/import.h"

extern Mused mused;

const Menu mainmenu[];
static const Menu showmenu[];
static const Menu filemenu[];

extern Menu thememenu[];

Menu editormenu[] =
{
	{ 0, showmenu, "Instrument",  NULL, change_mode_action, (void*)EDITINSTRUMENT, 0, 0 },
	{ 0, showmenu, "Pattern",  NULL, change_mode_action, (void*)EDITPATTERN, 0, 0 },
	{ 0, showmenu, "Sequence",  NULL, change_mode_action, (void*)EDITSEQUENCE, 0, 0 },
	{ 0, showmenu, "Classic",  NULL, change_mode_action, (void*)EDITCLASSIC, 0, 0 },
	{ 0, showmenu, "Effects",  NULL, change_mode_action, (void*)EDITFX, 0, 0 },
	{ 0, showmenu, "Wavetable",  NULL, change_mode_action, (void*)EDITWAVETABLE, 0, 0 },
	{ 0, NULL, NULL }
};


static const Menu columnsmenu[] =
{
	{ 0, showmenu, "Instrument", NULL, MENU_CHECK, &mused.visible_columns, (void*)VC_INSTRUMENT, 0 },
	{ 0, showmenu, "Volume", NULL, MENU_CHECK, &mused.visible_columns, (void*)VC_VOLUME, 0 },
	{ 0, showmenu, "Control bits", NULL, MENU_CHECK, &mused.visible_columns, (void*)VC_CTRL, 0 },
	{ 0, showmenu, "Command", NULL, MENU_CHECK, &mused.visible_columns, (void*)VC_COMMAND, 0 },
	{ 0, NULL, NULL }
};


static const Menu showmenu[] =
{
	{ 0, mainmenu, "Editor", editormenu, NULL },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Compact", NULL, MENU_CHECK, &mused.flags, (void*)COMPACT_VIEW, 0 },
	{ 0, mainmenu, "Visible columns", columnsmenu, NULL },
	{ 0, mainmenu, "Show position offset", NULL, MENU_CHECK, &mused.flags, (void*)SHOW_PATTERN_POS_OFFSET, 0 },
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
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Keyjazz", NULL, MENU_CHECK, &mused.flags, (void*)MULTICHANNEL_PREVIEW, 0 },
	{ 0, mainmenu, "Follow song position", NULL, MENU_CHECK, &mused.flags, (void*)FOLLOW_PLAY_POSITION, 0 },
	{ 0, mainmenu, "Animate cursor", NULL, MENU_CHECK, &mused.flags, (void*)ANIMATE_CURSOR, 0 },
	{ 0, mainmenu, "Hide zeros", NULL, MENU_CHECK, &mused.flags, (void*)HIDE_ZEROS, 0 },
	{ 0, mainmenu, "Protracker style delete", NULL, MENU_CHECK, &mused.flags, (void*)DELETE_EMPTIES, 0 },
	{ 0, mainmenu, "Center pattern editor", NULL, MENU_CHECK, &mused.flags, (void*)CENTER_PATTERN_EDITOR, 0 },
	{ 0, NULL, NULL }
};


static const Menu importmenu[] =
{
	{ 0, filemenu, "Import .MOD", NULL, import_module, MAKEPTR(IMPORT_MOD) },
	{ 0, filemenu, "Import .AHX", NULL, import_module, MAKEPTR(IMPORT_AHX) },
	{ 0, NULL, NULL }
};


static const Menu instmenu[] =
{
	{ 0, filemenu, "Kill instrument", NULL, kill_instrument },
	{ 0, filemenu, "Open instrument", NULL, open_data, MAKEPTR(OD_T_INSTRUMENT), MAKEPTR(OD_A_OPEN) },
	{ 0, filemenu, "Save instrument", NULL, open_data, MAKEPTR(OD_T_INSTRUMENT), MAKEPTR(OD_A_SAVE) },
	{ 0, NULL, NULL }
};


static const Menu wavetablemenu[] =
{
	{ 0, filemenu, "Open .WAV", NULL, open_data, MAKEPTR(OD_T_WAVETABLE), MAKEPTR(OD_A_OPEN) },
	{ 0, NULL, NULL }
};


static const Menu filemenu[] =
{
	{ 0, mainmenu, "New song", NULL, new_song_action },
	{ 0, mainmenu, "Open song", NULL, open_data, MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_OPEN) },
	{ 0, mainmenu, "Save song", NULL, open_data, MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_SAVE) },
	{ 0, mainmenu, "Export .WAV", NULL, export_wav_action },
	{ 0, mainmenu, "Import", importmenu },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Instrument", instmenu },
	{ 0, mainmenu, "Wavetable", wavetablemenu },
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
	{ 0, mainmenu, "Select all", NULL, select_all, 0, 0, 0 },
	{ 0, mainmenu, "Deselect", NULL, clear_selection, 0, 0, 0 },
	{ 0, mainmenu, "", NULL, NULL },
	{ 0, mainmenu, "Pattern", editpatternmenu, NULL },
	{ 0, mainmenu, "",  NULL },
	{ 0, mainmenu, "Interpolate", NULL, interpolate, 0, 0, 0 },
	{ 0, mainmenu, "",  NULL },
	{ 0, mainmenu, "Edit mode", NULL, MENU_CHECK, &mused.flags, (void*)EDIT_MODE, 0 },
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
