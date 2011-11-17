#ifndef MYBEVDEFS_H
#define MYBEVDEFS_H

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

#include "gui/bevdefs.h"

enum
{
	BEV_SELECTED_PATTERN_ROW = BEV_SELECTED_ROW,
	BEV_SEQUENCE_PLAY_POS = BEV_USER,
	BEV_BACKGROUND,
	BEV_SELECTED_SEQUENCE_ROW,
	BEV_SEQUENCE_BORDER,
	BEV_SEQUENCE_LOOP,
	BEV_THIN_FRAME,
	BEV_SELECTION,
	BEV_EDIT_CURSOR,
	BEV_PATTERN
};

enum
{
	DECAL_AUDIO_ENABLED = DECAL_USER,
	DECAL_AUDIO_DISABLED,
	DECAL_MODE_PATTERN,
	DECAL_MODE_SEQUENCE,
	DECAL_MODE_CLASSIC,
	DECAL_MODE_INSTRUMENT,
	DECAL_MODE_FX,
	DECAL_MODE_WAVETABLE,
	DECAL_MODE_PATTERN_SELECTED,
	DECAL_MODE_SEQUENCE_SELECTED,
	DECAL_MODE_CLASSIC_SELECTED,
	DECAL_MODE_INSTRUMENT_SELECTED,
	DECAL_MODE_FX_SELECTED,
	DECAL_MODE_WAVETABLE_SELECTED,
	DECAL_TOOLBAR_VISUALIZATIONS,
	DECAL_TOOLBAR_FULLSCREEN,
	DECAL_TOOLBAR_QUIT,
	DECAL_COMPACT,
	DECAL_COMPACT_SELETED
};

#endif
