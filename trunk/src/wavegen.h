#pragma once

#include "SDL.h"

#define WG_CHAIN_OSCS 4

typedef enum
{
	WG_OSC_SINE,
	WG_OSC_SQUARE,
	WG_OSC_SAW,
	WG_OSC_TRIANGLE,
	WG_OSC_NOISE,
	WG_NUM_OSCS
} WgOscType;

typedef enum
{
	WG_OP_ADD,
	WG_OP_MUL,
	WG_NUM_OPS
} WgOpType;

typedef struct
{
	WgOscType osc;
	WgOpType op;
	int mult, shift;
	int exp;
	float exp_c;
	Uint32 flags;
} WgOsc;

enum
{
	WG_OSC_FLAG_ABS = 1,
	WG_OSC_FLAG_NEG = 2
};

typedef struct
{
	WgOsc chain[WG_CHAIN_OSCS];
	int num_oscs, length;
} WgSettings;

typedef struct
{
	const char *name;
	WgSettings settings;
} WgPreset;

void wg_gen_waveform(WgOsc *chain, int num_oscs, Sint16 *data, int len);
float wg_osc(WgOsc *osc, float _phase);
void wg_init_osc(WgOsc *osc);
float wg_get_sample(WgOsc *chain, int num_oscs, float phase);
