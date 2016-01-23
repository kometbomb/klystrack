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
#include <string.h>
#include "mused.h"
#include "view.h"

static const InstructionDesc instruction_desc[] =
{
	{MUS_FX_END, 0xffff, "Program end", "PrgEnd", 0, 0},
	{MUS_FX_NOP, 0xffff, "No operation", "Nop", 0, 0},
	{MUS_FX_JUMP, 0xff00, "Goto", NULL, -1, -1},
	{MUS_FX_LABEL, 0xff00, "Loop begin", "Begin", 0, 0},
	{MUS_FX_LOOP, 0xff00, "Loop end", "Loop", -1, -1},	
	{MUS_FX_ARPEGGIO, 0x7f00, "Set arpeggio note", "Arp", -1, -1},
	{MUS_FX_ARPEGGIO_ABS, 0x7f00, "Set absolute arpeggio note", "AbsArp", 0, FREQ_TAB_SIZE - 1},
	{MUS_FX_SET_EXT_ARP, 0x7f00, "Set external arpeggio notes", "ExtArp", -1, -1},
	{MUS_FX_PORTA_UP, 0x7f00, "Portamento up", "PortUp", -1, -1},
	{MUS_FX_PORTA_DN, 0x7f00, "Portamento down", "PortDn", -1, -1},
	{MUS_FX_PORTA_UP_LOG, 0x7f00, "Portamento up (curve)", "PortUpC", -1, -1},
	{MUS_FX_PORTA_DN_LOG, 0x7f00, "Portamento down (curve)", "PortDnC", -1, -1},
	{MUS_FX_EXT_NOTE_DELAY, 0x7ff0, "Note delay", "Delay", -1, -1},
	{MUS_FX_VIBRATO, 0x7f00, "Vibrato", NULL, -1, -1},
	{MUS_FX_SLIDE, 0x7f00, "Slide", NULL, -1, -1},
	{MUS_FX_PORTA_UP_SEMI, 0x7f00, "Portamento up (semitones)", "PortUpST", -1, -1},
	{MUS_FX_PORTA_DN_SEMI, 0x7f00, "Portamento down (semitones)", "PortDnST", -1, -1},
	{MUS_FX_CUTOFF_UP, 0x7f00, "Filter cutoff up", "CutoffUp", -1, -1},
	{MUS_FX_CUTOFF_DN, 0x7f00, "Filter cutoff down", "CutoffDn", -1, -1},
	{MUS_FX_CUTOFF_SET, 0x7f00, "Set filter cutoff", "Cutoff", 0, 0xff},
	{MUS_FX_CUTOFF_SET_COMBINED, 0x7f00, "Set combined cutoff", "CutoffAHX", 0, 0xff},
	{MUS_FX_RESONANCE_SET, 0x7f00, "Set filter resonance", "Resonance", 0, 3},
	{MUS_FX_FILTER_TYPE, 0x7f00, "Set filter type", "FltType", 0, 2},
	{MUS_FX_PW_DN, 0x7f00, "PW down", "PWDn", -1, -1},
	{MUS_FX_PW_UP, 0x7f00, "PW up", "PWUp", -1, -1},
	{MUS_FX_PW_SET, 0x7f00, "Set PW", "PW", -1, -1},
	{MUS_FX_SET_VOLUME, 0x7f00, "Set volume", "Volume", 0, 0xff},
	{MUS_FX_FADE_GLOBAL_VOLUME, 0x7f00, "Global volume fade", "GlobFade", -1, -1},
	{MUS_FX_SET_GLOBAL_VOLUME, 0x7f00, "Set global volume", "GlobVol", 0, MAX_VOLUME},
	{MUS_FX_SET_CHANNEL_VOLUME, 0x7f00, "Set channel volume", "ChnVol", 0, MAX_VOLUME},
	{MUS_FX_SET_WAVEFORM, 0x7f00, "Set waveform", "Waveform", 0, 0xff},
	{MUS_FX_SET_WAVETABLE_ITEM, 0x7f00, "Set wavetable item", "Wavetable", 0, CYD_WAVE_MAX_ENTRIES - 1},
	{MUS_FX_SET_FXBUS, 0x7f00, "Set FX bus", "SetFxBus", 0, CYD_MAX_FX_CHANNELS - 1},
	{MUS_FX_SET_DOWNSAMPLE, 0x7f00, "Set downsample", "SetDnSmp", 0, 0xff},
	{MUS_FX_SET_SPEED, 0x7f00, "Set speed", "Speed", -1, -1},
	{MUS_FX_SET_RATE, 0x7f00, "Set rate", "Rate", -1, -1},
	{MUS_FX_LOOP_PATTERN, 0x7f00, "Loop pattern", "PatLoop", -1, -1},
	{MUS_FX_SKIP_PATTERN, 0x7f00, "Skip pattern", "PatSkip", -1, -1},
	{MUS_FX_TRIGGER_RELEASE, 0x7f00, "Trigger release", "Release", 0, 0xff},
	{MUS_FX_RESTART_PROGRAM, 0x7f00, "Restart program", "Restart", 0, 0},
	{MUS_FX_FADE_VOLUME, 0x7f00, "Fade volume", "VolFade", -1, -1},
	{MUS_FX_EXT_FADE_VOLUME_UP, 0x7ff0, "Fine fade volume in", "VolUpFine", 0, 0xf},
	{MUS_FX_EXT_FADE_VOLUME_DN, 0x7ff0, "Fine fade volume out", "VolDnFine", 0, 0xf},
	{MUS_FX_EXT_PORTA_UP, 0x7ff0, "Fine portamento up", "PortUpFine", 0, 0xf},
	{MUS_FX_EXT_PORTA_DN, 0x7ff0, "Fine portamento down", "PortDnFine", 0, 0xf},
	{MUS_FX_EXT_NOTE_CUT, 0x7ff0, "Note cut", "NoteCut", 0, 0xf},
	{MUS_FX_EXT_RETRIGGER, 0x7ff0, "Retrigger", "Retrig", 0, 0xf},
	{MUS_FX_WAVETABLE_OFFSET, 0x7000, "Wavetable offset", "WaveOffs", 0, 0xfff},
	{MUS_FX_SET_PANNING, 0x7f00, "Set panning", "SetPan", -1, -1},
	{MUS_FX_PAN_LEFT, 0x7f00, "Pan left", "PanLeft", -1, -1},
	{MUS_FX_PAN_RIGHT, 0x7f00, "Pan right", "PanRight", -1, -1},
	{MUS_FX_BUZZ_UP, 0x7f00, "Tune buzz up", "BuzzUp", -1, -1},
	{MUS_FX_BUZZ_DN, 0x7f00, "Tune buzz down", "BuzzDn", -1, -1},
	{MUS_FX_BUZZ_SHAPE, 0x7f00, "Set buzz shape", "BuzzShape", 0, 3},
	{MUS_FX_BUZZ_SET, 0x7f00, "Set buzz finetune", "BuzzFine", -1, -1},
	{MUS_FX_CUTOFF_FINE_SET, 0x7000, "Set filter cutoff (fine)", "CutFine", 0, CYD_CUTOFF_MAX - 1},
	{MUS_FX_BUZZ_SET_SEMI, 0x7f00, "Set buzz semitone", "BuzzSemi", -1, -1},
	{MUS_FX_FM_SET_MODULATION, 0x7f00, "Set FM modulation", "FMMod", 0, 0x7f},
	{MUS_FX_FM_SET_FEEDBACK, 0x7ff0, "Set FM feedback", "FMFB", 0, 7},
	{MUS_FX_FM_SET_HARMONIC, 0x7f00, "Set FM multiplier", "Mult", 0, 255},
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

	
void get_command_desc(char *text, size_t buffer_size, Uint16 inst)
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
		if (inst & 0xff)
			snprintf(text, buffer_size, "%s (%s%s%s%s%s%s)\n", name, (inst & MUS_FX_WAVE_NOISE) ? "N" : "", (inst & MUS_FX_WAVE_SAW) ? "S" : "", (inst & MUS_FX_WAVE_TRIANGLE) ? "T" : "", 
				(inst & MUS_FX_WAVE_PULSE) ? "P" : "", (inst & MUS_FX_WAVE_LFSR) ? "L" : "", (inst & MUS_FX_WAVE_WAVE) ? "W" : "");
		else
			snprintf(text, buffer_size, "%s (None)\n", name);
	}
	else if ((fi & 0x7f00) == MUS_FX_FILTER_TYPE)
	{
		static const char *fn[FLT_TYPES] = {"LP", "HP", "BP"};
		snprintf(text, buffer_size, "%s (%s)\n", name, fn[(inst & 0xf) % FLT_TYPES]);
	}
	else if ((fi & 0x7f00) == MUS_FX_BUZZ_SHAPE)
	{
		snprintf(text, buffer_size, "%s (%c)\n", name, ((inst & 0xf) % 4) + 0xf0);
	}
	else if ((fi & 0x7f00) == MUS_FX_SET_FXBUS)
	{
		snprintf(text, buffer_size, "%s (%s)\n", name, mused.song.fx[(inst & 0xf) % CYD_MAX_FX_CHANNELS].name);
	}
	else if ((fi & 0x7f00) == MUS_FX_SET_WAVETABLE_ITEM)
	{
		snprintf(text, buffer_size, "%s (%s)\n", name, mused.song.wavetable_names[(inst & 0xff)]);
	}
	else if ((fi & 0x7f00) == MUS_FX_SET_VOLUME)
	{
		snprintf(text, buffer_size, "%s (%+.1f dB)\n", name, percent_to_dB((float)(inst & 0xff) / MAX_VOLUME));
	}
	else snprintf(text, buffer_size, "%s\n", name);
}


Uint16 validate_command(Uint16 command)
{
	const InstructionDesc *i = get_instruction_desc(command);
	
	if (i)
	{
		if (i->maxv != -1 && i->minv != -1)
		{
			Uint16 v = ((~i->mask) & command) & ~0x8000;
			Uint16 c = command & 0x8000;
			v = my_min(i->maxv, my_max(i->minv, v));
			command = (command & i->mask) | v | c;
		}
	}
	
	return command;
}


const InstructionDesc* list_all_commands()
{
	return instruction_desc;
}