#ifndef MEMWRITER_H
#define MEMWRITER_H

#include <stdio.h>

typedef struct
{
	void *data;
	size_t allocated, size;
	size_t position;
	FILE *flush;
} MemWriter;

#include "SDL_rwops.h"

SDL_RWops * create_memwriter();

#endif
