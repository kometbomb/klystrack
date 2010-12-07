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

#include "command.h"
#include "snd/music.h"
#include "snd/freqs.h"
#include "macros.h"

static const InstructionDesc instruction_desc[] =
{
	{MUS_FX_END, 0xffff, "Program end", 0, 0},
	{MUS_FX_NOP, 0xffff, "No operation", 0, 0},
	{MUS_FX_JUMP, 0xff00, "Goto", -1, -1},
	{MUS_FX_LABEL, 0xff00, "Loop begin", 0, 0},
	{MUS_FX_LOOP, 0xff00, "Loop end", -1, -1},	
	{MUS_FX_ARPEGGIO, 0x7f00, "Set arpeggio note", -1, -1},
	{MUS_FX_ARPEGGIO_ABS, 0x7f00, "Set absolute arpeggio note", 0, FREQ_TAB_SIZE - 1},
	{MUS_FX_SET_EXT_ARP, 0x7f00, "Set external arpeggio notes", -1, -1},
	{MUS_FX_PORTA_UP, 0x7f00, "Portamento up", -1, -1},
	{MUS_FX_PORTA_DN, 0x7f00, "Portamento down", -1, -1},
	{MUS_FX_VIBRATO, 0x7f00, "Vibrato", -1, -1},
	{MUS_FX_SLIDE, 0x7f00, "Slide", -1, -1},
	{MUS_FX_PORTA_UP_SEMI, 0x7f00, "Portamento up (semitones)", -1, -1},
	{MUS_FX_PORTA_DN_SEMI, 0x7f00, "Portamento down (semitones)", -1, -1},
	{MUS_FX_CUTOFF_UP, 0x7f00, "Filter cutoff up", -1, -1},
	{MUS_FX_CUTOFF_DN, 0x7f00, "Filter cutoff down", -1, -1},
	{MUS_FX_CUTOFF_SET, 0x7f00, "Set filter cutoff", 0, 0xff},
	{MUS_FX_RESONANCE_SET, 0x7f00, "Set filter resonance", 0, 3},
	{MUS_FX_FILTER_TYPE, 0x7f00, "Set filter type", 0, 2},
	{MUS_FX_PW_DN, 0x7f00, "PW down", -1, -1},
	{MUS_FX_PW_UP, 0x7f00, "PW up", -1, -1},
	{MUS_FX_PW_SET, 0x7f00, "Set PW", -1, -1},
	{MUS_FX_SET_VOLUME, 0x7f00, "Set volume", 0, MAX_VOLUME},
	{MUS_FX_FADE_GLOBAL_VOLUME, 0x7f00, "Global volume fade", -1, -1},
	{MUS_FX_SET_GLOBAL_VOLUME, 0x7f00, "Set global volume", 0, MAX_VOLUME},
	{MUS_FX_SET_CHANNEL_VOLUME, 0x7f00, "Set channel volume", 0, MAX_VOLUME},
	{MUS_FX_SET_WAVEFORM, 0x7f00, "Set waveform", 0, 0xf},
	{MUS_FX_SET_SPEED, 0x7f00, "Set speed", -1, -1},
	{MUS_FX_SET_RATE, 0x7f00, "Set rate", -1, -1},
	{MUS_FX_LOOP_PATTERN, 0x7f00, "Loop pattern", -1, -1},
	{MUS_FX_TRIGGER_RELEASE, 0x7f00, "Trigger release", -1, -1},
	{MUS_FX_RESTART_PROGRAM, 0x7f00, "Restart program", 0, 0},
	{MUS_FX_FADE_VOLUME, 0x7f00, "Fade volume", -1, -1},
	{MUS_FX_EXT_FADE_VOLUME_UP, 0x7ff0, "Fine fade volume in", 0, 0xf},
	{MUS_FX_EXT_FADE_VOLUME_DN, 0x7ff0, "Fine fade volume out", 0, 0xf},
	{MUS_FX_EXT_PORTA_UP, 0x7ff0, "Fine portamento up", 0, 0xf},
	{MUS_FX_EXT_PORTA_DN, 0x7ff0, "Fine portamento down", 0, 0xf},
	{MUS_FX_EXT_NOTE_CUT, 0x7ff0, "Note cut", 0, 0xf},
	{MUS_FX_EXT_RETRIGGER, 0x7ff0, "Retrigger", 0, 0xf},
	{MUS_FX_WAVETABLE_OFFSET, 0x7000, "Wavetable offset", 0, 0xfff},
	{MUS_FX_SET_PANNING, 0x7f00, "Set panning", -1, -1},
	{MUS_FX_PAN_LEFT, 0x7f00, "Pan left", -1, -1},
	{MUS_FX_PAN_RIGHT, 0x7f00, "Pan right", -1, -1},
	{MUS_FX_BUZZ_UP, 0x7f00, "Tune buzz up", -1, -1},
	{MUS_FX_BUZZ_DN, 0x7f00, "Tune buzz down", -1, -1},
	{MUS_FX_BUZZ_SHAPE, 0x7f00, "Set buzz shape", 0, 3},
	{MUS_FX_BUZZ_SET, 0x7f00, "Set buzz finetune", -1, -1},
	{MUS_FX_CUTOFF_FINE_SET, 0x7000, "Set filter cutoff (fine)", 0, CYD_CUTOFF_MAX - 1},
	{MUS_FX_BUZZ_SET_SEMI, 0x7f00, "Set buzz semitone", -1, -1},
	{0, 0, NULL}
};


const InstructionDesc * get_instruction_desc(Uint16 command)
{
	for (int i = 0 ; instruction_desc[i].name != NULL ; ++i)
	{
		if (instruction_desc[i].opcode == (command & instruction_desc[i].mask))
		{
			return &instruction_desc[i];
		}
	}
	
	return false;
}


bool is_valid_command(Uint16 command)
{
	return get_instruction_desc(command) != NULL;
}

	
void get_command_desc(char *text, Uint16 inst)
{
	const InstructionDesc *i = get_instruction_desc(inst);

	if (i == NULL) 
	{
		strcpy(text, "Unknown\n");
		return;
	}

	const char *name = i->name;
	const Uint16 fi = i->opcode;
	
	if ((fi & 0x7f00) == MUS_FX_SET_WAVEFORM)
	{
		if (inst & 0xf)
			sprintf(text, "%s (%s%s%s%s)\n", name, (inst & CYD_CHN_ENABLE_NOISE) ? "N" : "", (inst & CYD_CHN_ENABLE_SAW) ? "S" : "", (inst & CYD_CHN_ENABLE_TRIANGLE) ? "T" : "", (inst & CYD_CHN_ENABLE_PULSE) ? "P" : "");
		else
			sprintf(text, "%s (None)\n", name);
	}
	else if ((fi & 0x7f00) == MUS_FX_FILTER_TYPE)
	{
		static const char *fn[FLT_TYPES] = {"LP", "HP", "BP"};
		sprintf(text, "%s (%s)\n", name, fn[(fi & 0xf) % FLT_TYPES]);
	}
	else if ((fi & 0x7f00) == MUS_FX_BUZZ_SHAPE)
	{
		sprintf(text, "%s (%c)\n", name, ((fi & 0xf) % 4) + 0xf0);
	}
	else sprintf(text, "%s\n", name);
}


Uint16 validate_command(Uint16 command)
{
	const InstructionDesc *i = get_instruction_desc(command);
	
	if (i)
	{
		if (i->maxv != -1 && i->minv != -1)
		{
			Uint16 v = (~i->mask) & command;
			v = my_min(i->maxv, my_max(i->minv, v));
			command = (command & i->mask) | v;
		}
	}
	
	return command;
}
