#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdlib.h>


typedef struct
{
	int type;
	void *data;
	size_t size;
	
} Clipboard;

#include "mused.h"


enum
{
	CP_PATTERN=EDITPATTERN,
	CP_SEQUENCE=EDITSEQUENCE,
	CP_INSTRUMENT=EDITINSTRUMENT,
	CP_PROGRAM=EDITPROG,
	CP_PATTERNSEGMENT
};

#define ALL_ITEMS 0xffffffff

void cp_copy(Clipboard *cp, int type, void *data, const size_t size);
void cp_copy_items(Clipboard *cp, int type, void *data, const size_t dest_items, const size_t item_size);
void cp_paste(Clipboard *cp, int dest_type, void *dest, const size_t buffer_size);
void cp_paste_items(Clipboard *cp, int target_type, void *dest, const size_t dest_items, const size_t item_size);
size_t cp_get_item_count(Clipboard *cp, const size_t item_size);

#endif
