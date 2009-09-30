/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

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

#include "view.h"
#include "event.h"
#include "mused.h"
#include "action.h"
#include "mouse.h"
#include "dialog.h"

#define BG_CURSOR 0xffff4040
#define BG_PLAYERPOS 0xff004000
#define BG_SELECTION 0xff80ff80

#define timesig(i, bar, beat, normal) ((((i)%((mused.time_signature>>8)*(mused.time_signature&0xff))) == 0)?(bar):((((i)%(mused.time_signature&0xff))==0)?(beat):(normal)))

extern Mused mused;

void draw_view(const View* views, const SDL_Event *event)
{
	for (int i = 0 ; views[i].handler ; ++i)
	{
		const View *view = &views[i];

		memcpy(&mused.console->clip, &view->position, sizeof(view->position));
		view->handler(&view->position, event, view->param);
		SDL_UpdateRect(mused.console->surface, view->position.x, view->position.y, view->position.w, view->position.h);
	}
}


static void separator(const char * label)
{
	console_set_color(mused.console,0xff808080,CON_CHARACTER);
	console_write_args(mused.console, "%s\n", label);
}


char * notename(Uint8 note)
{	
	static char buffer[4];
	static const char * notename[] =
	{
		"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
	};
	sprintf(buffer, "%s%d", notename[note % 12], (note - note % 12) / 12);
	return buffer;
}


void sequence_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	char text[200];
	
	console_set_color(mused.console,0,CON_BACKGROUND);
	console_clear(mused.console);
	
	int start = mused.sequence_position;
	
	int p[MUS_CHANNELS] ={ 0 };
	
	int draw_colon[MUS_CHANNELS] = {0};
	int draw_colon_id[MUS_CHANNELS] = {0xff};
	
	console_write(mused.console, "     ");
	
	for (int c = 0 ; c < MUS_CHANNELS ; ++c)
	{
		console_set_color(mused.console, mused.mus.channel[c].flags & MUS_CHN_DISABLED ? 0xff808080 : 0xffffffff, CON_CHARACTER);
		check_event(event, console_write_args(mused.console, "  %02X  ", c), enable_channel, (void*)c, 0, 0);
	}
	
	console_write(mused.console, "\n");
	
	mused.sequence_slider_param.visible_first = start;
	
	for (int i = start, s = 0, y = mused.console->font.h ; y < dest->h ; i += mused.sequenceview_steps, ++s, y += mused.console->font.h)
	{
		console_set_color(mused.console,(mused.current_sequencepos != i)?timesig((start/mused.sequenceview_steps+s), ((i < mused.song.song_length)?0xffffffff:0xff809090),((i < mused.song.song_length)?0xffc0c0e0:0xff806090), ((i < mused.song.song_length)?0xffc0c0c0:0xff606090)):0xff0000ff,CON_CHARACTER);
		console_set_color(mused.console,0,CON_BACKGROUND);
		check_event(event, console_write_args(mused.console, "%04X|", i), select_sequence_position, (void*)-1, (void*)i, 0);
		
		for (int c = 0 ; c < MUS_CHANNELS ; ++c)
		{
			Uint32 bg = 0;
			if (mused.stat_song_position >= i && mused.stat_song_position < i + mused.sequenceview_steps)
			{
				//printf("i %d stat_song_position %d mused.sequenceview_steps %d\n", i, mused.stat_song_position, mused.sequenceview_steps);
				bg = BG_PLAYERPOS;
			}
			
			if (c == mused.current_sequencetrack && i >= mused.selection.start && i < mused.selection.end)
			{
				bg = BG_SELECTION;
			}
		
			console_set_color(mused.console,(mused.current_sequencetrack == c && mused.current_sequencepos == i) ? BG_CURSOR : bg,CON_BACKGROUND);
			
			sprintf(text, "--   ");
 			
			if ((draw_colon[c]) > mused.sequenceview_steps)
			{
				draw_colon[c] -=  mused.sequenceview_steps;
				sprintf(text, "::   ");
			}
			else
			{
				draw_colon_id[c] = 0xff;
			}
			
			for (; p[c] < mused.song.num_sequences[c] ; ++p[c])
			{
				if (mused.song.sequence[c][p[c]].position >= i && mused.song.sequence[c][p[c]].position < i + mused.sequenceview_steps && draw_colon_id[c] != mused.song.sequence[c][p[c]].position)
				{
					if (mused.song.sequence[c][p[c]].position != i )
					{	
						//sprintf(text, "%02x+%02x %+3d ", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].position - i, mused.song.sequence[c][p[c]].note_offset);
						sprintf(text, "%02x+%02x", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].position - i);
					}
					else
					{
						//sprintf(text, "%02x   %+3d  ", mused.song.sequence[c][p[c]].pattern, mused.song.sequence[c][p[c]].note_offset);
						sprintf(text, "%02x   ", mused.song.sequence[c][p[c]].pattern);
					}
					draw_colon[c] = mused.song.pattern[mused.song.sequence[c][p[c]].pattern].num_steps;
					draw_colon_id[c] = mused.song.sequence[c][p[c]].position;
					break;
				}
				if (mused.song.sequence[c][p[c]].position >= i)
				{
					break;
				}
				
			}
			
			check_event(event, console_write(mused.console,text), select_sequence_position, (void*)c, (void*)i, 0);
			console_set_color(mused.console,bg,CON_BACKGROUND);
			console_write(mused.console,"|");
		}
		
		console_write(mused.console,"\n");
		
		if (i < mused.song.song_length)
			slider_set_params(&mused.sequence_slider_param, 0, mused.song.song_length - mused.sequenceview_steps, start, i, &mused.sequence_position, mused.sequenceview_steps, SLIDER_VERTICAL);
	}
}

void pattern_view_inner(const SDL_Rect *dest, const SDL_Event *event, int current_pattern, int channel)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_clip(mused.console, dest);
	console_reset_cursor(mused.console);
			
	int start = mused.pattern_position;
	
	console_set_color(mused.console,0xff808080,CON_CHARACTER);
	
	if (channel != -1) 
	{
		check_event(event, console_write_args(mused.console, "--Ch:%x-Pat:%2x--\n", channel, current_pattern), enable_channel, (void*)channel, 0, 0);
	}
	else
	{
		console_write_args(mused.console, "----Pat:%2x-----\n", current_pattern);
	}
	
	for (int i = start, s = 0 ; i < mused.song.pattern[current_pattern].num_steps && s < (int)dest->h/mused.console->font.h; ++i, ++s)
	{
		console_set_color(mused.console,(mused.current_patternstep != i)?(timesig(i, 0xffffffff,0xffffffc0,0xffc0c0c0)):0xff0000ff,CON_CHARACTER);
		
		Uint32 bg = 0;
		
		if (channel != -1)
		{
			if (i == mused.stat_pattern_position[channel] && mused.mus.song_track[channel].pattern == &mused.song.pattern[current_pattern])
			{
				bg = BG_PLAYERPOS;
			}
		}
		
		if (current_pattern == mused.current_pattern && i >= mused.selection.start && i < mused.selection.end)
		{
			bg = BG_SELECTION;
		}
		
		console_set_color(mused.console,bg,CON_BACKGROUND);
		console_write_args(mused.console,"%02X ", i);
		
		console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == PED_NOTE)?BG_CURSOR:bg,CON_BACKGROUND);
		
		const SDL_Rect *r;
		
		if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_RELEASE)
			r = console_write(mused.console, "--- ");
		else if (mused.song.pattern[current_pattern].step[i].note == MUS_NOTE_NONE)
			r = console_write(mused.console, "... ");
		else
			r = console_write_args(mused.console, "%s ", notename(mused.song.pattern[current_pattern].step[i].note));
			
		check_event(event, r, select_pattern_param, (void*)PED_NOTE, (void*)i, (void*)current_pattern);
		
		console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == PED_INSTRUMENT1)?BG_CURSOR:bg,CON_BACKGROUND);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%x", mused.song.pattern[current_pattern].step[i].instrument >> 4);
		else
			r = console_write(mused.console, ".");
			
		check_event(event, r, select_pattern_param, (void*)PED_INSTRUMENT1, (void*)i, (void*)current_pattern);
		
		console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == PED_INSTRUMENT2)?BG_CURSOR:bg,CON_BACKGROUND);
		
		if (mused.song.pattern[current_pattern].step[i].instrument != MUS_NOTE_NO_INSTRUMENT)
			r = console_write_args(mused.console, "%x", mused.song.pattern[current_pattern].step[i].instrument & 0xf);
		else
			r = console_write(mused.console, ".");
			
		check_event(event, r, select_pattern_param, (void*)PED_INSTRUMENT2, (void*)i, (void*)current_pattern);
		
		for (int p = PED_CTRL ; p < PED_COMMAND1 ; ++p)
		{
			char *bitname = "LSV";
		
			console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == p)?BG_CURSOR:bg,CON_BACKGROUND);
			check_event(event, console_write_args(mused.console, " %c", mused.song.pattern[current_pattern].step[i].ctrl & (MUS_CTRL_BIT << (p - PED_CTRL)) ? bitname[p - PED_CTRL] : '.'), 
				select_pattern_param, (void*)p, (void*)i, (void*)current_pattern);
		}
		
		console_set_color(mused.console,bg,CON_BACKGROUND);
		console_write(mused.console," ");
		
		for (int p = 0 ; p < 4 ; ++p)
		{
			console_set_color(mused.console,(mused.current_pattern == current_pattern && mused.current_patternstep == i && mused.current_patternx == (PED_COMMAND1+p))?BG_CURSOR:bg,CON_BACKGROUND);
			check_event(event, console_write_args(mused.console, "%x", (mused.song.pattern[current_pattern].step[i].command >> ((3-p)*4)) & 0xf), 
				select_pattern_param, (void*)(PED_COMMAND1 + p), (void*)i, (void*)current_pattern);
		}
		
		console_write(mused.console,"\n");
		
		if (current_pattern == mused.current_pattern)
			slider_set_params(&mused.pattern_slider_param, 0, mused.song.pattern[current_pattern].num_steps - 1, start, i, &mused.pattern_position, 1, SLIDER_VERTICAL);
	}
}


void pattern_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	int pv = 0;
	SDL_Rect pos = { dest->x, dest->y, dest->w, dest->h };
	
	console_clear(mused.console);
	
	for (int i = 0 ; i < MUS_CHANNELS ; ++i)
	{
		if (mused.ghost_pattern[i] != -1)
		{
			SDL_Rect r;
			memcpy(&r, &pos, sizeof(r));
			
			pattern_view_inner(&r, event, mused.ghost_pattern[i], i);
			pos.x += 190;
			pv ++;
		}
	}
	
	if (!pv)
	{
		pattern_view_inner(dest, event, mused.current_pattern, -1);
	}
}


void info_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	char text[200];
	
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	
	console_clear(mused.console);
	
	char speedtext[10];
	if (mused.song.song_speed == mused.song.song_speed2)
	{
		sprintf(speedtext, "%d", mused.song.song_speed);
	}
	else
	{
		sprintf(speedtext, "%d+%d", mused.song.song_speed, mused.song.song_speed2);
	}
	
	
	sprintf(text, "Song length: %04x\n"
	              "Loop point:  %04x\n"
	              "Speed:     %6s\n"
				  "Rate:      %3d hz\n"
				  "Time sig.:  %2d/%2d\n\n"
				  "Sel.instrument:%02x\n"
				  ":%-16s\n"
				  "Sel.pattern:   %02x\n"
				  "Sequence step: %02x\n"
				  "Octave:        %02x\n",
				  
				  
				  
				  mused.song.song_length, mused.song.loop_point, speedtext, mused.song.song_rate, mused.time_signature >> 8, mused.time_signature &0xf,
				  mused.current_instrument, mused.song.instrument[mused.current_instrument].name, mused.current_pattern, mused.sequenceview_steps, mused.octave);
	console_write(mused.console,text);
}


void get_command_desc(char *text, Uint16 inst)
{
	static const struct { Uint16 opcode; char* name; }  instructions[] =
	{
		{MUS_FX_ARPEGGIO, "Arpeggio"},
		{MUS_FX_SET_EXT_ARP, "Set external arpeggio notes"},
		{MUS_FX_PORTA_UP, "Portamento up"},
		{MUS_FX_PORTA_DN, "Portamento down"},
		{MUS_FX_PORTA_UP_SEMI, "Portamento up (semitones)"},
		{MUS_FX_PORTA_DN_SEMI, "Portamento down (semitones)"},
		{MUS_FX_CUTOFF_UP, "Filter cutoff up"},
		{MUS_FX_CUTOFF_DN, "Filter cutoff down"},
		{MUS_FX_CUTOFF_SET, "Set filter cutoff"},
		{MUS_FX_PW_DN, "PW down"},
		{MUS_FX_PW_UP, "PW up"},
		{MUS_FX_PW_SET, "Set PW"},
		{MUS_FX_PORTA_VOLUME_SET, "Set volume"},
		{MUS_FX_PORTA_WAVEFORM_SET, "Set waveform"},
		{MUS_FX_END, "Program end"},
		{MUS_FX_JUMP, "Goto"},
		{MUS_FX_LABEL, "Loop begin"},
		{MUS_FX_LOOP, "Loop end"},
		{MUS_FX_NOP, "No operation"},
		{MUS_FX_TRIGGER_RELEASE, "Trigger release"},
		{0, NULL}
	};
	
	char *name = NULL;
	Uint16 fi = 0;
	for (int i = 0 ; instructions[i].name != NULL ; ++i)
	{
		if (instructions[i].opcode == inst || instructions[i].opcode == (inst & 0x7f00))
		{
			name = instructions[i].name;
			fi = instructions[i].opcode;
			break;
		}
	}

	if ((fi & 0x7f00) == MUS_FX_PORTA_WAVEFORM_SET)
	{
		sprintf(text, "%s (%s%s%s%s)\n", name, (inst & CYD_CHN_ENABLE_NOISE) ? "N" : "", (inst & CYD_CHN_ENABLE_SAW) ? "S" : "", (inst & CYD_CHN_ENABLE_TRIANGLE) ? "T" : "", (inst & CYD_CHN_ENABLE_PULSE) ? "P" : "");
	}
	else if (name == NULL) sprintf(text, "Unknown\n");
	else sprintf(text, "%s\n", name);
}


void info_line(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	
	console_clear(mused.console);
	
	char text[200]="";
	
	switch (mused.mode)
	{
		case EDITINSTRUMENT:
		
		if (mused.selected_param >= P_PARAMS)
		{
			Uint16 inst = mused.song.instrument[mused.current_instrument].program[mused.selected_param-P_PARAMS];
			get_command_desc(text, inst);
		}
		
		break;
		
		case EDITPATTERN:
		
		if (mused.current_patternx >= PED_COMMAND1)
		{
			Uint16 inst = mused.song.pattern[mused.current_pattern].step[mused.current_patternstep].command;
			get_command_desc(text, inst);
		}
		
		break;
	}
	
	
	console_write(mused.console,text);
}


void program_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_clear(mused.console);
	
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];

	int rows = dest->h / mused.console->font.h;
	
	separator("----program-----");
	
	int start = mused.selected_param - P_PARAMS - rows/2;
	
	if (start > MUS_PROG_LEN - rows ) start = MUS_PROG_LEN - rows;
	if (start < 0 ) start = 0;
	
	for (int i = start, s = 0 ; i < MUS_PROG_LEN && s < rows; ++i, ++s)
	{
		console_set_color(mused.console,mused.selected_param == (P_PARAMS+i)?0xff0000ff:0xffffffff,CON_CHARACTER);
		
		if (mused.selection.start - P_PARAMS <= i && mused.selection.end - P_PARAMS > i)
			console_set_color(mused.console,0xff0000ff,CON_BACKGROUND);
		else
			console_set_color(mused.console,0,CON_BACKGROUND);
		
		char box[5];
		
		if (inst->program[i] == MUS_FX_NOP)
		{
			strcpy(box, "....");
		}
		else
		{
			sprintf(box, "%04x", inst->program[i]);
		}
		
		if (mused.mode == EDITPROG && mused.selected_param == (P_PARAMS+i))
		{
			box[mused.editpos] = '?';
		}
		
		check_event(event, console_write_args(mused.console, "%c%02x %s%c\n", (inst->program[i] & 0x8000) ? '(' : ' ', i, box, (inst->program[i] & 0x8000) ? ')' : ' '),
			select_instrument_param, (void*)(P_PARAMS + i), 0, 0);
			
		slider_set_params(&mused.program_slider_param, P_PARAMS, MUS_PROG_LEN - 1 + P_PARAMS, start + P_PARAMS, i + P_PARAMS, &mused.selected_param, 1, SLIDER_VERTICAL);
	}
}


static void inst_flags(const SDL_Event *e, int p, const char *label, Uint32 *flags, Uint32 mask)
{
	console_set_color(mused.console,mused.selected_param == p?0xff0000ff:0xffffffff,CON_CHARACTER);
	if (checkbox(e, label, flags, mask)) mused.selected_param = p;
}


static void inst_text(const SDL_Event *e, int p, const char *label, const char *value, int show_spinner)
{
	console_set_color(mused.console,mused.selected_param == p?0xff0000ff:0xffffffff,CON_CHARACTER);
	check_event(e, console_write_args(mused.console, label, value), select_instrument_param, (void*)p, 0, 0);
	
	if (show_spinner)
	{
		int d = spinner(e, (int)value);
		if (d) mused.selected_param = p;
		if (d < 0) instrument_add_param(-1);
		else if (d >0) instrument_add_param(1);
	}
}


static void inst_hex8(const SDL_Event *e, int p, const char *label, Uint8 *value)
{
	console_set_color(mused.console,mused.selected_param == p?0xff0000ff:0xffffffff,CON_CHARACTER);
	check_event(e, console_write_args(mused.console, label, *value), select_instrument_param, (void*)p, 0, 0);
	int d = spinner(e, (int)value);
	
	if (d) mused.selected_param = p;
	if (d < 0) instrument_add_param(-1);
	else if (d >0) instrument_add_param(1);
}


static void inst_hex16(const SDL_Event *e, int p, const char *label, Uint16 *value)
{
	console_set_color(mused.console,mused.selected_param == p?0xff0000ff:0xffffffff,CON_CHARACTER);
	check_event(e, console_write_args(mused.console, label, *value), select_instrument_param, (void*)p, 0, 0);
	int d = spinner(e, (int)value);
	
	if (d) mused.selected_param = p;
	if (d < 0) instrument_add_param(-1);
	else if (d >0) instrument_add_param(1);
}


static void cr()
{
	// Fixes bug with text bouding rect starting from prev line
	console_write(mused.console, "\n");
}


void instrument_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_clear(mused.console);
	
	MusInstrument *inst = &mused.song.instrument[mused.current_instrument];
	
	separator("------name------");
	inst_text(event, P_NAME, "%-16s\n", inst->name, 0);
	
	separator("---attributes---");
	
	inst_text(event, P_BASENOTE, "Base: %s", notename(inst->base_note), 1);
	inst_flags(event, P_LOCKNOTE, "Lock\n", &inst->flags, MUS_INST_LOCK_NOTE);
	inst_flags(event, P_DRUM, "Drum ", &inst->flags, MUS_INST_DRUM);
	inst_flags(event, P_METAL, "Metal\n", &inst->cydflags, CYD_CHN_ENABLE_METAL);
	inst_flags(event, P_KEYSYNC, "KSync ", &inst->cydflags, CYD_CHN_ENABLE_KEY_SYNC);
	inst_flags(event, P_INVVIB, "Vib\n", &inst->flags, MUS_INST_INVERT_VIBRATO_BIT);
	inst_flags(event, P_SETPW, "Set PW\n", &inst->flags, MUS_INST_SET_PW);
	inst_flags(event, P_SETCUTOFF, "Set cutoff\n", &inst->flags, MUS_INST_SET_CUTOFF);
	inst_flags(event, P_REVERB, "Rvb\n", &inst->cydflags, CYD_CHN_ENABLE_REVERB);
	
	separator("------sync------");
	inst_flags(event, P_SYNC, "Enable", &inst->cydflags, CYD_CHN_ENABLE_SYNC);
	cr();
	inst_hex8(event, P_SYNCSRC, "Src: %x", &inst->sync_source);
	
	separator("\n----ring mod----");
	inst_flags(event, P_RINGMOD, "Enable", &inst->cydflags, CYD_CHN_ENABLE_RING_MODULATION);
	cr();
	inst_hex8(event, P_RINGMODSRC, "Src: %x", &inst->ring_mod);
	
	
	separator("\n----waveform----");
	inst_flags(event, P_PULSE, "Pul ", &inst->cydflags, CYD_CHN_ENABLE_PULSE);
	inst_flags(event, P_SAW, "Saw\n", &inst->cydflags, CYD_CHN_ENABLE_SAW);
	inst_flags(event, P_TRIANGLE, "Tri ", &inst->cydflags, CYD_CHN_ENABLE_TRIANGLE);
	inst_flags(event, P_NOISE, "Noi\n", &inst->cydflags, CYD_CHN_ENABLE_NOISE);
	
	separator("----envelope----");
	inst_hex8(event, P_ATTACK,  "Atk: %02x", &inst->adsr.a);
	cr();
	inst_hex8(event, P_DECAY,   "Dec: %02x", &inst->adsr.d);
	cr();
	inst_hex8(event, P_SUSTAIN, "Sus: %02x", &inst->adsr.s);
	cr();
	inst_hex8(event, P_RELEASE, "Rel: %02x", &inst->adsr.r);
	
	separator("\n------misc------");
	inst_hex16(event, P_PW,        "PW:         %03x", &inst->pw);
	cr();
	inst_hex8(event, P_VOLUME,     "Volume:      %02x", &inst->volume);
	cr();
	inst_hex8(event, P_SLIDESPEED, "Slide speed: %02x", &inst->slide_speed);
	cr();
	inst_hex8(event, P_VIBSPEED,   "Vib. speed:  %02x", &inst->vibrato_speed);
	cr();
	inst_hex8(event, P_VIBDEPTH,   "Vib. depth:  %02x", &inst->vibrato_depth);
	cr();
	inst_hex8(event, P_PWMSPEED,   "PWM speed:   %02x", &inst->pwm_speed);
	cr();
	inst_hex8(event, P_PWMDEPTH,   "PWM depth:   %02x", &inst->pwm_depth);
	cr();
	inst_hex8(event, P_PROGPERIOD, "Prg. period: %02x", &inst->prog_period);
	
	separator("\n-----filter-----");
	inst_flags(event, P_FILTER, "Enabled\n", &inst->cydflags, CYD_CHN_ENABLE_FILTER);
	
	static const char* flttype[] = {"LP", "HP", "BP"};
	
	inst_text(event, P_FLTTYPE, "Type: %s", flttype[inst->flttype], 1);
	cr();
	inst_hex16(event, P_CUTOFF, "Cutoff: %03x", &inst->cutoff);
	cr();
	inst_hex8(event, P_RESONANCE, "Res: %1x", &inst->resonance);
}


void instrument_list(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_clear(mused.console);
	int y = mused.console->font.h;
	
	separator("----instruments----");
	
	int start = mused.instrument_list_position;
	
	/*if (start > NUM_INSTRUMENTS - rows ) start = NUM_INSTRUMENTS - rows;
	if (start < 0 ) start = 0;*/
	
	for (int i = start ; i < NUM_INSTRUMENTS && y < dest->h ; ++i, y += mused.console->font.h)
	{
		console_set_color(mused.console, i == mused.current_instrument ? 0xffff0000 : 0x0, CON_BACKGROUND);
		console_set_color(mused.console, 0xffffffff, CON_CHARACTER);
		check_event(event, console_write_args(mused.console, "%02x %-16s\n", i + 1, mused.song.instrument[i].name), select_instrument, (void*)i, 0, 0);
		
		slider_set_params(&mused.instrument_list_slider_param, 0, NUM_INSTRUMENTS - 1, start, i, &mused.instrument_list_position, 1, SLIDER_VERTICAL);
	}
}


void reverb_view(const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	console_set_color(mused.console,0x00000000,CON_BACKGROUND);
	console_set_color(mused.console,0xffffffff,CON_CHARACTER);
	console_clear(mused.console);
	
	separator("----reverb----");
	
	console_set_color(mused.console, mused.edit_reverb_param == R_ENABLE ? 0xff0000ff:0xffffffff,CON_CHARACTER);
	
	if (checkbox(event, "Enabled\n", &mused.song.flags, MUS_ENABLE_REVERB)) mused.edit_reverb_param = R_ENABLE;
	
	// We need to mirror the reverb flag to the corresponding Cyd flag
	
	if (mused.song.flags & MUS_ENABLE_REVERB)
		mused.cyd.flags |= CYD_ENABLE_REVERB;
	else
		mused.cyd.flags &= ~CYD_ENABLE_REVERB;
	
	int p = R_DELAY;
	
	for (int i = 0 ; i < CYDRVB_TAPS ; ++i)
	{
		if ((i % 3) == 0 && i > 0) console_write(mused.console, "\n");
		
		console_set_color(mused.console,0xffffffff,CON_CHARACTER);
		console_write_args(mused.console, "Tap %x:", i);
	
		console_set_color(mused.console,mused.edit_reverb_param == p ? 0xff0000ff:0xffffffff,CON_CHARACTER);
		console_write_args(mused.console, " %4d ms ", mused.song.rvbtap[i].delay);
		
		++p;
		
		console_set_color(mused.console,mused.edit_reverb_param == p ? 0xff0000ff:0xffffffff,CON_CHARACTER);
		
		if (mused.song.rvbtap[i].gain <= CYDRVB_LOW_LIMIT)
			console_write(mused.console, "- INF dB");
		else
			console_write_args(mused.console, "%+5.1f dB", (double)mused.song.rvbtap[i].gain * 0.1);
		
		++p;
	}
}
