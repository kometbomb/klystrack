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
