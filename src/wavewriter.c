#include "wavewriter.h"
#include <stdlib.h>
#include "SDL.h"

WaveWriter * ww_create(FILE * file, int sample_rate, int channels)
{
	WaveWriter * ww = malloc(sizeof(WaveWriter));
	
	ww->file = file;
	ww->channels = channels;
	
	Uint16 tmp16 = 0;
	Uint32 tmp32 = 0;
	
	fwrite("RIFF", 4, 1, ww->file);
	
	ww->riffsize_pos = ftell(ww->file);
	
	fwrite(&tmp32, 4, 1, ww->file);
	fwrite("WAVE", 4, 1, ww->file);
	fwrite("fmt ", 4, 1, ww->file);
	
	tmp32 = SDL_SwapLE32(16); // size of format data
	
	fwrite(&tmp32, 4, 1, ww->file);
	
	tmp16 = SDL_SwapLE16(1); // data type = PCM
	
	fwrite(&tmp16, 2, 1, ww->file);
	
	tmp16 = SDL_SwapLE16(channels);
	
	fwrite(&tmp16, 2, 1, ww->file);
	
	tmp32 = SDL_SwapLE32(sample_rate);
	
	fwrite(&tmp32, 4, 1, ww->file);
	
	tmp32 = SDL_SwapLE32(sample_rate * channels * sizeof(Sint16));
	
	fwrite(&tmp32, 4, 1, ww->file);
	
	tmp16 = SDL_SwapLE16(channels * sizeof(Sint16));
	
	fwrite(&tmp16, 2, 1, ww->file);
	
	tmp16 = SDL_SwapLE16(16); // bits per sample
	
	fwrite(&tmp16, 2, 1, ww->file);
	
	fwrite("data", 4, 1, ww->file);
	
	ww->chunksize_pos = ftell(ww->file);
	
	tmp32 = 0;
	fwrite(&tmp32, 4, 1, ww->file);
	
	return ww;
}


void ww_write(WaveWriter *ww, Sint16 * buffer, int samples)
{
	fwrite(buffer, samples * ww->channels * sizeof(Sint16), 1, ww->file);
}


void ww_finish(WaveWriter *ww)
{
	Uint32 sz = ftell(ww->file) - 8;
	Uint32 tmp32 = SDL_SwapLE32(sz);
	
	fseek(ww->file, ww->riffsize_pos, SEEK_SET);
	fwrite(&tmp32, sizeof(tmp32), 1, ww->file);
	
	tmp32 = SDL_SwapLE32(sz + 8 - (ww->chunksize_pos + 4));
	
	fseek(ww->file, ww->chunksize_pos, SEEK_SET);
	fwrite(&tmp32, sizeof(tmp32), 1, ww->file);

	fclose(ww->file);
	free(ww);
}
