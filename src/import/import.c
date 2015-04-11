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

#include "mused.h"
#include "event.h"
#include "import.h"
#include "mod.h"
#include "ahx.h"
#include "xm.h"
#include "org.h"
#include "hubbard.h"
#include "gui/toolutil.h"
#include "gui/msgbox.h"
#include "diskop.h"
#include "SDL_endian.h"
#include "action.h"
#include "optimize.h"


extern Mused mused;
extern GfxDomain *domain;

void import_module(void *type, void* unused1, void* unused2)
{
	int r;
	if (mused.modified) r = confirm_ync(domain, mused.slider_bevel, &mused.largefont, "Save song?");
	else r = -1;
				
	if (r == 0) return;
	if (r == 1) 
	{ 
		int r;
				
		open_data(MAKEPTR(OD_T_SONG), MAKEPTR(OD_A_SAVE), &r);
				
		if (!r) return;
	}
	
	static const char *mod_name[] = {"a Protracker", "an AHX", "a FastTracker II", "a Cave Story", "a Rob Hubbard"};
	static const char *mod_ext[] = {"mod", "ahx", "xm", "org", "sid"};
	
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Import %s song", mod_name[CASTPTR(int, type)]);

	FILE * f = open_dialog("rb", buffer, mod_ext[CASTPTR(int, type)], domain, mused.slider_bevel, &mused.largefont, &mused.smallfont, NULL);
	
	if (!f) return;
	
	stop(NULL, NULL, NULL);
	new_song();
	
	switch (CASTPTR(int, type))
	{
		case IMPORT_MOD: r = import_mod(f); break;
		case IMPORT_AHX: r = import_ahx(f); break;
		case IMPORT_XM: r = import_xm(f); break;
		case IMPORT_ORG: r = import_org(f); break;
		case IMPORT_HUBBARD: r = import_hubbard(f); break;
	}
	
	if (CASTPTR(int, type) != IMPORT_HUBBARD)
	{
		if (!r) 
		{
			snprintf(buffer, sizeof(buffer), "Not %s song", mod_name[CASTPTR(int, type)]);
			msgbox(domain, mused.slider_bevel, &mused.largefont, buffer, MB_OK);
		}
		else
		{
			optimize_song(&mused.song);
		}
	}
	else
	{
		optimize_song(&mused.song);
	}
	
	mused.song.num_patterns = NUM_PATTERNS;
	
	fclose(f);
	
	set_channels(mused.song.num_channels);
}

