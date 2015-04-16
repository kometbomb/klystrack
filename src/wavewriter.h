#ifndef WAVEWRITER_H
#define WAVEWRITER_H

#include <stdio.h>
#include "SDL.h"

typedef struct
{
	FILE *file;
	int channels;
	size_t chunksize_pos, riffsize_pos;
} WaveWriter;

/* Create WaveWriter with sample rate of rate/16-bits and start writing to file */
WaveWriter * ww_create(FILE * file, int sample_rate, int channels);
/* Write channels * samples Sint16's */
void ww_write(WaveWriter *ww, Sint16 * buffer, int samples);
/* Close file and free WaveWriter */
void ww_finish(WaveWriter *ww);

#endif
