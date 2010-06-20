#include "tfmx.h"
#include "SDL_endian.h"
#include "macros.h"

int import_tfmx(FILE *f)
{
	struct
	{
		char sig[10];
		Uint16 dummy;
		Uint32 flags;
		char textarea[240];
		Uint16 song_start[32], song_end[32], song_tempo[32];
		Uint8 padding[16];
		Uint32 trackstep, patterns, macros;
	} __attribute__((__packed__)) header;
	
	if (fread(&header, 1, sizeof(header), f) != sizeof(header) || strncmp(header.sig, "TFMX-SONG ", 9) != 0)
	{
		fatal("Not a TFMX module");
		return 0;
	}
	
	int max_pos = 0;
	
	for (int i = 0 ; i < 32 ; ++i)
	{
		FIX_ENDIAN(song_start[i]);
		FIX_ENDIAN(song_end[i]);
		FIX_ENDIAN(song_tempo[i]);
		
		if (song_end[i] <= 128)
			max_pos = my_max(max_pos, song_end[i] + 1);
	}
	
	if (header.trackstep == 0 && header.patterns == 0 && header.macros == 0)
	{
		debug("Not a packed module");
		
		header.trackstep = 0x600;
		header.patterns = 0x200;
		header.macros = 0x400;
	}
	else
	{
		FIX_ENDIAN(header.trackstep);
		FIX_ENDIAN(header.patterns);
		FIX_ENDIAN(header.macros);
		
		debug("A packed module (offsets = %p, %p, %p)", header.trackstep, header.patterns, header.macros);
	}
	
	header.trackstep += 0x200;
	header.patterns += 0x200;
	header.macros += 0x200;
	
	Uint32 *pattern_data = NULL;
	Uint32 pattern_offsets[128] = { 0 };
	
	{
		fseek(f, header.patterns, SEEK_SET);
		
		if (fread(&pattern_offsets, sizeof(Uint32), 128, f) != 128)
		{
			fatal("Pattern offsets not loaded");
			return 0;
		}
		
		Uint32 last_pat = 0;
		
		for (int i = 0 ; i < 128 ; ++i)
		{
			FIX_ENDIAN(pattern_offsets[i]);
			last_pat = my_max(pattern_offsets[i], last_pat);
		}
		
		/* find the length of the last pattern */
		
		Uint32 step = 0, end = last_pat;
		fseek(f, last_pat, SEEK_SET);
		
		do
		{
			if (fread(&step, 1, sizeof(step), f) != sizeof(step))
			{
				fatal("Faulty pattern data");
				return 0;
			}
			
			FIX_ENDIAN(step);
			end += sizeof(step);
		}
		while (step != 0xf0000000);
			
		pattern_data = malloc(end - pattern_offsets[0]);
		
		fseek(f, pattern_offsets[0], SEEK_SET);
		
		if (fread(pattern_data, sizeof(Uint32), end - pattern_offsets[0]) != end - pattern_offsets[0])
		{
			fatal("Faulty pattern data");
			free(pattern_data);
			return 0;
		}
		
		for (int i = 1 ; i < 128 ; ++i)
		{
			pattern_offsets[i] -= pattern_offsets[0]:
		}
		
		end -= pattern_offsets[0];
		
		pattern_offsets[0] = 0;
		
		for (int i = 0 ; i < end ; ++i)
		{
			FIX_ENDIAN(pattern_data[i]);
		}
	}
	
	Uint16 *trackstep = malloc(max_pos * 8 * sizeof(Uint16));
	
	{
		fseek(f, header.patterns, SEEK_SET);
		
		for (int t = 0 ; t < max_pos ; ++t)
		{
			if (fread(&trackstep[t * 8], 1, sizeof(trackstep[0]) * 8, f) != sizeof(sizeof(trackstep[0]) * 8))
			{
				fatal("Trackstep not loaded");
				free(trackstep);
				free(pattern_data);
				return 0;
			}
			
			for (int i = 0 ; i < 8 ; ++i)
				FIX_ENDIAN(trackstep[t * 8 + i]);
		}
	}
	
	for (int pattern = 0 ; pattern < 128 ; ++pattern)
	{
		resize_pattern(&mused.song.pattern[pattern], 0);
		
		for (int step = 0 ; pattern_data[step + pattern_offsets[pattern]] != 0xf0000000 ; ++step)
		{
			
		}
	}
	
	free(trackstep);
	free(pattern_data);

	return 1;
}

