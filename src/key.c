/*
Copyright (c) 2009-2011 Tero Lindeman (kometbomb)

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


#include "key.h"
#include "gui/menu.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "action.h"
#include "mused.h"
#include "keytab.h"
#include <string.h>

extern Mused mused;

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef WIN32

#include "windows.h"

#endif


#if __APPLE__
char *query_resource_directory( void );
#endif


#define MAX_KEYMAPS 64
#define MAX_KEYTRANS 500

Menu keymapmenu[MAX_KEYMAPS + 1];
extern const Menu prefsmenu[];
static KeyTran keytrans[MAX_KEYTRANS];


void translate_key_event(SDL_KeyboardEvent *e)
{
	const int allowed = KMOD_SHIFT|KMOD_CTRL|KMOD_ALT;
	
	//debug("key = %u mod = %u %u", e->keysym.sym, e->keysym.mod & allowed, keytrans[0].from_mod);

	for (int i = 0 ; i < MAX_KEYTRANS && !(keytrans[i].from_key == 0 && keytrans[i].from_mod == 0) ; ++i)
	{
		if (e->keysym.sym == keytrans[i].from_key && ((e->keysym.mod & allowed) == keytrans[i].from_mod))
		{
			e->keysym.sym = keytrans[i].to_key;
			e->keysym.mod = keytrans[i].to_mod;
			break;
		}
	}
}


void enum_keymaps()
{
	memset(keymapmenu, 0, sizeof(keymapmenu));

	// TODO: remove copypastecode and write enum_dir() function that takes a handler
	
#ifdef WIN32
// RES_PATH is relative to klystrack.exe
	char path[1000] = "", fullpath[1000];
	
	if (GetModuleFileName(NULL, path, sizeof(path)))
	{
		// Get the path component (this should be functionized and used by load_theme() and enum_theme()
		
		for (int i = strlen(path) - 1 ; i >= 0 && path[i] != '\\' && path[i] != '/' ; --i)
		{
			path[i] = '\0';
		}
	}
	
	snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/key", path);
	DIR *dir = opendir(fullpath);
	debug("Enumerating keymaps at " "%s" TOSTRING(RES_PATH) "/key", path);
#elif __APPLE__
	char path[1000];
	snprintf(path, sizeof(path) - 1, "%s/key", query_resource_directory());
	DIR *dir = opendir(path);
	debug("Enumerating keymaps at %s", path);
#else
	DIR *dir = opendir(TOSTRING(RES_PATH) "/key");
	debug("Enumerating keymaps at " TOSTRING(RES_PATH) "/key");
#endif
	
	if (!dir)
	{
		warning("Could not enumerate keymaps at " TOSTRING(RES_PATH) "/key");
		return;
	}
	
	struct dirent *de = NULL;
	int maps = 0;
	
	keymapmenu[maps].parent = prefsmenu;
	keymapmenu[maps].text = strdup("Default");
	keymapmenu[maps].action = load_keymap_action;
	keymapmenu[maps].p1 = (void*)keymapmenu[maps].text;
	++maps;
	
	while ((de = readdir(dir)) != NULL)
	{
		char fullpath[1000];
	
#ifdef WIN32
		snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/key/%s", path, de->d_name);
#elif __APPLE__
		snprintf(fullpath, sizeof(fullpath) - 1, "%s/key/%s", query_resource_directory(), de->d_name);
#else
		snprintf(fullpath, sizeof(fullpath) - 1, TOSTRING(RES_PATH) "/key/%s", de->d_name);
#endif
		struct stat attribute;
		
		if (stat(fullpath, &attribute) != -1 && !(attribute.st_mode & S_IFDIR))
		{
			if (maps >= MAX_KEYMAPS)
			{
				warning("Maximum keymaps exceeded");
				break;
			}
			
			keymapmenu[maps].parent = prefsmenu;
			keymapmenu[maps].text = strdup(de->d_name);
			keymapmenu[maps].action = load_keymap_action;
			keymapmenu[maps].p1 = (void*)keymapmenu[maps].text;
			++maps;
		}
	}
	
	debug("Got %d keymaps", maps);
	
	closedir(dir);
}


void update_keymap_menu()
{
	for (int i = 0 ; keymapmenu[i].text ; ++i)
	{
		if (strcmp(mused.keymapname, (char*)keymapmenu[i].p1) == 0)
		{
			keymapmenu[i].flags |= MENU_BULLET;
		}
		else
			keymapmenu[i].flags &= ~MENU_BULLET;
	}
}


int parse_key(const char *keys, int *key, int *mod)
{
	*mod = 0;
	*key = 0;
	
	char *temp = strdup(keys);
	int done = 0;
	char *tok = strtok(temp, " \t");
	
	do
	{
		if (!tok) break;
		int found = 0;
		
		for (int i = 0 ; keydefs[i].name ; ++i)
		{
			if (strcasecmp(tok, keydefs[i].name) == 0)
			{
				if (*key != 0) 
				{
					warning("More than one key (%s, was %d) specified", tok, *key);
					done = 1;
				}
				
				*key = keydefs[i].key;
				found = 1;
				break;
			}
		}
		
		for (int i = 0 ; moddefs[i].name ; ++i)
		{
			if (strcasecmp(tok, moddefs[i].name) == 0)
			{
				*mod |= moddefs[i].key;
				found = 1;
				break;
			}
		}
		
		if (!found && strlen(tok) > 0)
		{
			warning("Unknown token %s", tok);
			break;
		}
		
		tok = strtok(NULL, " \t");
	}
	while (!done);
	
	free(temp);
	
	if (*key == 0)
	{
		warning("No keys specified");
		*mod = 0;
	}
	
	return (*key != 0);
}


int parse_keys(const char *from, const char *to, KeyTran *tran)
{
	if (!parse_key(from, &tran->from_key, &tran->from_mod)) return 0;
	
	if (!parse_key(to, &tran->to_key, &tran->to_mod)) return 0;
	
	return 1;
}


void load_keymap(const char *name)
{
	memset(keytrans, 0, sizeof(keytrans));

	char tmpname[1000];
	strncpy(tmpname, name, sizeof(tmpname));

	if (strcmp(name, "Default") == 0)
	{
		return;
	}	

	char fullpath[1000];
	
#ifdef RESOURCES_IN_BINARY_DIR
	// RES_PATH is relative to klystrack.exe
#ifdef WIN32
	char path[1000] = "";

	if (GetModuleFileName(NULL, path, sizeof(path)))
	{
		// Get the path component
		
		for (int i = strlen(path) - 1 ; i >= 0 && path[i] != '\\' && path[i] != '/' ; --i)
		{
			path[i] = '\0';
		}
	}
	
	snprintf(fullpath, sizeof(fullpath) - 1, "%s" TOSTRING(RES_PATH) "/key/%s", path, tmpname);
#else
	snprintf(fullpath, sizeof(fullpath) - 1, "/proc/self/key/%s", tmpname);
#endif
	
#elif __APPLE__
	snprintf(fullpath, sizeof(fullpath) - 1, "%s/key/%s", query_resource_directory(), tmpname);
#else
	snprintf(fullpath, sizeof(fullpath) - 1, TOSTRING(RES_PATH) "/key/%s", tmpname);
#endif

	strncpy(mused.keymapname, tmpname, sizeof(mused.themename));
	update_keymap_menu();
	
	debug("Loading keymap '%s'", fullpath);
	
	FILE *f = fopen(fullpath, "rt");
	int trans = 0;
	
	if (f)
	{
		int lnr = 1;
		
		do
		{
			char line[100], from[100], to[100];
			if (trans >= MAX_KEYTRANS)
			{
				warning("Max keytrans exceeded\n");
				break;
			}
			
			if (!fgets(line, sizeof(line) - 1, f)) break;
			
			if (sscanf(line, "%[^=]=%[^\r\n]", from, to) == 2 && parse_keys(from, to, &keytrans[trans]))
			{
				++trans;
			}
			else
			{
				warning("Keymap line %d is malformed", lnr);
			}
			
			++lnr;
		}
		while (1);
	
		fclose(f);
	}
	else
	{
		debug("Keymap loading failed");
	}
	
	debug("Got %d keytrans", trans);
}
