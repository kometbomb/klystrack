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

#include "wave.h"
#include "macros.h"

typedef struct
{
	char ckID[4]; 
	Uint32 cksize;
} Chunk;

Wave * wave_load(FILE *f)
{
	struct { 
		Chunk c;
		char WAVEID[4];
	}  __attribute__((__packed__)) RIFF;
	
	if (fread(&RIFF, 1 , sizeof(RIFF), f) != sizeof(RIFF) || strncmp(RIFF.c.ckID, "RIFF", 4) != 0 || strncmp(RIFF.WAVEID, "WAVE", 4) != 0)
	{
		fatal("Not a RIFF wave file");
		return NULL;
	}
	
	struct
	{
		Chunk c;
		Uint16 wFormatTag;
		Uint16 nChannels;
		Uint32 nSamplesPerSec;
		Uint32 nAvgBytesPerSec;
		Uint16 nBlockAlign;
		Uint16 wBitsPerSample;
		/*----*/
		Uint16 cbSize;
		/*----*/
		Uint16 wValidBitsPerSample;
		Uint32 dwChannelMask;
		char SubFormat[16];
	} __attribute__((__packed__)) WAVE;
	
	size_t beginning_of_WAVE = ftell(f);
	
	if (fread(&WAVE, 1 , sizeof(WAVE), f) < 16 || strncmp(WAVE.c.ckID, "fmt ", 4) != 0) 
	{
		fatal("No 'fmt ' chunk found");
		return NULL;
	}
	
	if (WAVE.wFormatTag != WAVE_FORMAT_PCM/* && WAVE.wFormatTag != WAVE_FORMAT_FLOAT*/)
	{
		//fatal("Only PCM and float supported");
		fatal("Only PCM supported");
		return NULL;
	}
	
	fseek(f, beginning_of_WAVE + WAVE.c.cksize + 8, SEEK_SET);
	
	Chunk peek = { "", 0 };
	
	fread(&peek, 1, sizeof(peek) ,f);
	
	if (strncmp(peek.ckID, "fact", 4) == 0)
	{
		fseek(f, sizeof(Uint32), SEEK_CUR);
	}
	else
	{
		fseek(f, beginning_of_WAVE + WAVE.c.cksize + 8, SEEK_SET);
	}
	
	fread(&peek, 1, sizeof(peek) ,f);
	
	if (strncmp(peek.ckID, "data", 4) != 0)
	{
		fatal("No 'data' chunk found");
		return NULL;
	}
	
	Wave *w = malloc(sizeof(*w));
	
	w->format = WAVE.wFormatTag;
	w->channels = WAVE.nChannels;
	w->sample_rate = WAVE.nSamplesPerSec;
	w->length = peek.cksize / (WAVE.wBitsPerSample / 8) / WAVE.nChannels;
	w->bits_per_sample = WAVE.wBitsPerSample;
	
	w->data = malloc(peek.cksize);
	
	debug("Reading %d bytes (chn = %d, bits = %d)", peek.cksize, w->channels, w->bits_per_sample);
	
	fread(w->data, 1, peek.cksize, f);
		
	return w;
}


void wave_destroy(Wave *wave)
{
	if (wave)
	{
		free(wave->data);
		free(wave);
	}
}
