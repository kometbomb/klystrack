#ifndef EVENT_H
#define EVENT_H

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

#include "SDL.h"
#include "gui/slider.h"

void edit_instrument_event(SDL_Event *e);
void sequence_event(SDL_Event *e);
void pattern_event(SDL_Event *e);
void edit_program_event(SDL_Event *e);
void edit_text(SDL_Event *e);
int generic_edit_text(SDL_Event *e, char *edit_buffer, size_t edit_buffer_size, int *editpos);
void fx_event(SDL_Event *e);
void fx_add_param(int d);
void instrument_add_param(int a);
void del_sequence(int first,int last,int track);
void add_sequence(int channel, int position, int pattern, int offset);
void set_room_size(int fx, int size, int vol, int dec);
void update_position_sliders();

enum
{
	PED_NOTE,
	PED_INSTRUMENT1,
	PED_INSTRUMENT2,
	PED_LEGATO,
	PED_SLIDE,
	PED_VIB,
	PED_COMMAND1,
	PED_COMMAND2,
	PED_COMMAND3,
	PED_COMMAND4,
	PED_PARAMS
};

#define PED_CTRL PED_LEGATO

enum
{
	P_INSTRUMENT,
	P_NAME,
	P_BASENOTE,
	P_LOCKNOTE,
	P_DRUM,
	P_KEYSYNC,
	P_INVVIB,
	P_SETPW,
	P_SETCUTOFF,
	P_FX,
	P_PULSE,
	P_PW,
	P_SAW,
	P_TRIANGLE,
	P_NOISE,
	P_METAL,
	P_VOLUME,
	P_RELVOL,
	P_ATTACK,
	P_DECAY,
	P_SUSTAIN,
	P_RELEASE,
	P_BUZZ,
	P_BUZZ_SEMI,
	P_BUZZ_FINE,
	P_BUZZ_SHAPE,
	P_SYNC,
	P_SYNCSRC,
	P_RINGMOD,
	P_RINGMODSRC,
	P_FILTER,
	P_FLTTYPE,
	P_CUTOFF,
	P_RESONANCE,
	P_SLIDESPEED,
	P_PROGPERIOD,
	P_VIBSPEED,
	P_VIBDEPTH,
	P_VIBSHAPE,
	P_VIBDELAY,
	P_PWMSPEED,
	P_PWMDEPTH,
	P_PWMSHAPE,
	P_FXBUS,
	P_PARAMS
};

enum
{
	R_MULTIPLEX,
	R_MULTIPLEX_PERIOD,
	R_FX_BUS,
	R_CRUSH,
	R_CRUSHBITS,
	R_CHORUS,
	R_MINDELAY,
	R_MAXDELAY,
	R_SEPARATION,
	R_RATE,
	R_ENABLE,
	R_ROOMSIZE,
	R_ROOMVOL,
	R_ROOMDECAY,
	R_SPREAD,
	R_DELAY,
	R_GAIN
};

#endif
