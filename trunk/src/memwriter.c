#include "memwriter.h"
#include <stdlib.h>
#include <string.h>

static Sint64 mw_size(SDL_RWops *ops)
{
	MemWriter *mem = ops->hidden.unknown.data1;
	return mem->size;
}

static Sint64 mw_seek(SDL_RWops *ops, Sint64 position, int mode)
{
	MemWriter *mem = ops->hidden.unknown.data1;
	switch (mode)
	{
		case RW_SEEK_SET:
			mem->position = position;
			break;
			
		case RW_SEEK_CUR:
			mem->position += position;
			break;
			
		case RW_SEEK_END:
			mem->position = mem->size - position;
			break;
	}
	
	return mem->position;
}


static size_t mw_read(SDL_RWops *ops, void *data, size_t size, size_t num)
{
	return 0;
}	


static size_t mw_write(SDL_RWops *ops, const void *data, size_t size, size_t num)
{
	MemWriter *mem = ops->hidden.unknown.data1;
	if (mem->position + size * num > mem->allocated)
	{
		int chunk = mem->position + size;
		
		if (chunk < 1024)
			chunk = 1024;
		
		mem->allocated = mem->allocated + chunk;
		mem->data = realloc(mem->data, mem->allocated);
	}
	
	memcpy(mem->data + mem->position, data, size * num);
	
	mem->position += size * num;
	
	if (mem->size < mem->position)
		mem->size = mem->position;
		
	return size * num;
}
	
	
static int mw_close(SDL_RWops *ops)
{
	MemWriter *mem = ops->hidden.unknown.data1;
	int r = 0;
	
	if (mem->flush)
	{
		r = fwrite(mem->data, mem->size, 1, mem->flush) == mem->size ? 0 : -1;
	}
	
	free(mem->data);
	SDL_FreeRW(ops);
	
	return r;
}


SDL_RWops * create_memwriter(FILE *flush)
{
	MemWriter *mem = malloc(sizeof(*mem));
	mem->position = 0;
	mem->data = NULL;
	mem->size = 0;
	mem->allocated = 0;
	mem->flush = flush;
	
	SDL_RWops *ops = SDL_AllocRW();
	ops->seek = mw_seek;
	ops->write = mw_write;
	ops->read = mw_read;
	ops->close = mw_close;
	ops->size = mw_size;
	ops->type =0x1234;
	ops->hidden.unknown.data1 = mem;
	
	return ops;
}
