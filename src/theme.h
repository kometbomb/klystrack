#ifndef THEME_H
#define THEME_H

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

enum
{
	COLOR_NORMAL,
	COLOR_SEQUENCE_SELECTED ,
	COLOR_SEQUENCE_BAR,
	COLOR_SEQUENCE_BEAT,
	COLOR_SEQUENCE_NORMAL ,
	COLOR_SEQUENCE_DISABLED ,
	COLOR_PATTERN_SELECTED ,
	COLOR_PATTERN_BAR,
	COLOR_PATTERN_BEAT,
	COLOR_PATTERN_INSTRUMENT,
	COLOR_PATTERN_INSTRUMENT_BAR,
	COLOR_PATTERN_INSTRUMENT_BEAT,
	COLOR_PATTERN_NORMAL,
	COLOR_PATTERN_DISABLED,
	COLOR_PROGRAM_SELECTED,
	COLOR_PROGRAM_EVEN,
	COLOR_PROGRAM_ODD,
	COLOR_INSTRUMENT_SELECTED,
	COLOR_INSTRUMENT_NORMAL,
	COLOR_MENU_NORMAL,
	COLOR_MENU_SELECTED,
	COLOR_MENU_HEADER,
	COLOR_MENU_HEADER_SELECTED,
	COLOR_MENU_SHORTCUT,
	COLOR_MENU_SHORTCUT_SELECTED,
	/*-------------*/
	NUM_COLORS
};

#include "SDL.h"

extern Uint32 colors[NUM_COLORS];

void load_theme(const char *name);
void enum_themes();
void free_themes();
void update_theme_menu();

#endif
