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

#ifndef KEYTAB_H
#define KEYTAB_H

#include "SDL.h"

typedef enum { KEYSYM, SCANCODE } KeyTranType;

typedef struct
{
	KeyTranType type;
	int focus;
	int from_mod;
	int from_key;
	int from_scancode;
	int to_mod;
	int to_key;
	int to_scancode;
} KeyTran;

typedef struct
{
	const char *name;
	int key;
} KeyDef;

#define KEYDEF(key) {"K_"#key, SDLK_##key}
#define MODDEF(key) {"M_"#key, KMOD_##key}

extern const KeyDef keydefs[];
extern const KeyDef moddefs[];

#endif
