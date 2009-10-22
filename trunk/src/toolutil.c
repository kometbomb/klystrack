/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

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

#include "toolutil.h"
#include "msgbox.h"
#include "filebox.h"
#include <string.h>
#include "macros.h"

#ifndef WIN32
#include <sys/types.h>
#include <pwd.h>
#endif


FILE *open_dialog(const char *mode, char *title, char *filter)
{
	char filename[5000];
	if (filebox(title, mode[0] == 'w' ? FB_SAVE : FB_OPEN, filename, sizeof(filename) - 1, filter) == FB_OK)
	{
		FILE * f = fopen(filename, mode);
		if (!f) msgbox("Could not open file", MB_OK);
		return f;
	}
	else
		return NULL;
}


int confirm(const char *msg)
{
	return msgbox(msg, MB_YES|MB_NO) == MB_YES; // MessageBox(0, msg, "Confirm", MB_YESNO) == IDYES;
}

int confirm_ync(const char *msg)
{
	int r = msgbox(msg, MB_YES|MB_NO|MB_CANCEL);
	
	if (r == MB_YES)
	{
		return 1;
	}
	if (r == MB_NO)
	{
		return -1;
	}
	
	return 0;
	
}


char * expand_tilde(const char * path)
{
#ifndef WIN32
	if (path[0] != '~') return NULL;
	
	const char *rest = strchr(path, '/');
	char *name = NULL;
	size_t rest_len = 0;
	
	if (rest != NULL)
	{
		size_t l = (rest - (path + 1)) / sizeof(*name);
		if (l)
		{
			name = calloc(sizeof(*name), l + 1);
			strncpy(name, path + 1, l);
		}
		rest_len = strlen(rest);
	}
	
	const char *homedir = NULL;
	
	if (name) 
	{
		struct passwd *pwd = getpwnam(name);
		
		if (!pwd)
		{
			warning("User %s not found", name);
			return NULL;
		}
		
		homedir = pwd->pw_dir;
		free(name);
	}
	else
	{
		homedir = getenv("HOME");
	}
	
	char * final = malloc(strlen(homedir) + rest_len + 2);
	strcpy(final, homedir);
	if (rest) strcat(final, rest);
	
	return final;
#else
	return NULL;
#endif
}
