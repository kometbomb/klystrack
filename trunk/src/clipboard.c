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

#include "clipboard.h"
#include <string.h>

void cp_copy(Clipboard *cp, int type, void *data, const size_t size)
{
	if (cp->data != NULL) free(cp->data);
	cp->data = malloc(size);
	memcpy(cp->data, data, size);
	cp->size = size;
	cp->type = type;
}

void cp_paste(Clipboard *cp, int target_type, void *dest, const size_t buffer_size)
{
	if (target_type != cp->type || buffer_size == 0) return;
	memcpy(dest, cp->data, (buffer_size == ALL_ITEMS || buffer_size > cp->size) ? cp->size : buffer_size);
}

void cp_paste_items(Clipboard *cp, int target_type, void *dest, const size_t dest_items, const size_t item_size)
{
	cp_paste(cp, target_type, dest, dest_items * item_size);
}

size_t cp_get_item_count(Clipboard *cp, const size_t item_size)
{
	return cp->size / item_size;
}

void cp_copy_items(Clipboard *cp, int type, void *data, const size_t dest_items, const size_t item_size)
{
	cp_copy(cp, type, data, dest_items * item_size);
}
