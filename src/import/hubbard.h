#ifndef HUBBARD_H
#define HUBBARD_H

#include <stdio.h>

typedef struct
{
	unsigned short int org;
	unsigned char data[65536];
	struct
	{
		unsigned short int songtab;
		unsigned short int patternptrhi, patternptrlo;
		unsigned short int songs[16][3];
		unsigned short int instruments;
	} addr;
	
	// decoded data
	
	struct { 
		unsigned short int pulse_width;
		unsigned char waveform;
		unsigned char a, d, s, r;
		unsigned char vibrato_depth;
		unsigned char pwm;
		unsigned char fx;
	} instrument[256];
	
	struct {
		struct {
			unsigned char note;
			unsigned char length;
			unsigned char legato;
			unsigned char instrument;
			char portamento;
		} note[256];
		int length;
	} pattern[256];
	
	int n_patterns;
	
	struct
	{
		unsigned char pattern[256];
		int length;
	} track[3];
	
	int n_tracks;
	int n_subsongs, n_instruments;
	int vib_type;
} hubbard_t;

int import_hubbard(FILE *f);

#endif
