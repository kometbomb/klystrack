#ifndef SONG_STATS_H
#define SONG_STATS_H

enum
{
	STATS_HEADER,
	STATS_FX,
	STATS_DEFVOLPAN,
	STATS_INSTRUMENTS,
	STATS_SEQUENCE,
	STATS_PATTERNS,
	STATS_WAVETABLE,
	N_STATS
};

typedef struct
{
	int size[N_STATS];
	int total_size;
} SongStats;

#endif
