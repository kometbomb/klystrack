#include "stats.h"
#include "memwriter.h"
#include "diskop.h"
#include "macros.h"
#include "mused.h"
#include "gui/msgbox.h"

static void get_stats(SongStats *stats)
{
	SDL_RWops *rw = create_memwriter(NULL);
	save_song_inner(rw, stats);
	SDL_RWclose(rw);
}

void song_stats(void *unused1, void *unused2, void *unused3)
{
	SongStats stats;
	get_stats(&stats);
	
	debug("Header = %d", stats.size[STATS_HEADER]);
	debug("FX = %d", stats.size[STATS_FX]);
	debug("Def vol. pan = %d", stats.size[STATS_DEFVOLPAN]);
	debug("Instruments = %d", stats.size[STATS_INSTRUMENTS]);
	debug("Sequence = %d", stats.size[STATS_SEQUENCE]);
	debug("Patterns = %d", stats.size[STATS_PATTERNS]);
	debug("Wavetable = %d", stats.size[STATS_WAVETABLE]);
	debug("Total = %d", stats.total_size);
	
	char str[1000];
	snprintf(str, sizeof(str), 
		"Header:      %6d bytes  %2d %%\n"
		"FX:          %6d bytes  %2d %%\n"
		"Def.vol/pan: %6d bytes  %2d %%\n"
		"Instruments: %6d bytes  %2d %%\n"
		"Sequence:    %6d bytes  %2d %%\n"
		"Patterns:    %6d bytes  %2d %%\n"
		"Wavetable:   %6d bytes  %2d %%\n"
		"-------------------------------\n"
		"TOTAL:       %6d bytes",
		stats.size[STATS_HEADER], stats.size[STATS_HEADER] * 100 / stats.total_size,
		stats.size[STATS_FX], stats.size[STATS_FX] * 100 / stats.total_size,
		stats.size[STATS_DEFVOLPAN], stats.size[STATS_DEFVOLPAN] * 100 / stats.total_size,
		stats.size[STATS_INSTRUMENTS], stats.size[STATS_INSTRUMENTS] * 100 / stats.total_size,
		stats.size[STATS_SEQUENCE], stats.size[STATS_SEQUENCE] * 100 / stats.total_size,
		stats.size[STATS_PATTERNS], stats.size[STATS_PATTERNS] * 100 / stats.total_size,
		stats.size[STATS_WAVETABLE], stats.size[STATS_WAVETABLE] * 100 / stats.total_size,
		stats.total_size
	);
	
	msgbox(domain, mused.slider_bevel, &mused.largefont, str, MB_OK);
}
