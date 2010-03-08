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

#include "mused.h"
#include "event.h"
#include "import.h"
#include "gui/toolutil.h"
#include "gui/msgbox.h"
#include "diskop.h"
#include "SDL_endian.h"

extern Mused mused;
extern GfxDomain *domain;

static Uint16 find_command_pt(Uint16 command)
{
	if ((command & 0xff00) == 0x0c00)
		command = 0x0c00 | ((command & 0xff) * 2);
	else if ((command & 0xff00) == 0x0a00)
		command = 0x0a00 | (my_min(0xf, (command & 0x0f) * 2)) | (my_min(0xf, ((command & 0xf0) >> 4) * 2) << 4);
	else if ((command & 0xfff0) == 0x0ea0 || (command & 0xfff0) == 0x0eb0)
		command = (command & 0xfff0) | (my_min(0xf, (command & 0x0f) * 2));
	else if ((command & 0xff00) == 0x0f00 && (command & 0xff) < 32)
		command = 0x0f00 | (command & 0xff);
	else if ((command & 0xff00) == 0x0100 || (command & 0xff00) == 0x0200 || (command & 0xff00) == 0x0300) 
		command = (command & 0xff00) | my_min(0xff, (command & 0xff) * 8);
	else if ((command & 0xff00) != 0x0400 && (command & 0xff00) != 0x0000) 
		command = 0;	
	
	return command;
}


static Uint16 find_command_ahx(Uint8 command, Uint8 data, Uint8 *ctrl)
{
	if (command == 0xc)
	{
		if (data <= 0x40)
			return 0xC00 | (data * 2);
	}
	else if (command == 0xa)
	{
		return 0xa00 | data;
	}
	else if (command == 0x3)
	{
		return 0x300|(data * 4);
	}
	else if (command == 0x5)
	{
		*ctrl |= MUS_CTRL_SLIDE|MUS_CTRL_LEGATO;
		return 0xa00 | data;
	}
	else if (command == 0x1)
	{
		return 0x100 | (data * 4);
	}
	else if (command == 0x2)
	{
		return 0x200 | (data * 4);
	}
	else if (command == 0xf)
	{
		return 0xf00 | (data & 0xf);
	}
	else if (command == 0xe)
	{
		if ((data & 0xf0) == 0xc0)
		{
			return 0xec0|(data&0xf);
		}
		else if ((data & 0xf0) == 0xd0)
		{
			return 0xed0|(data&0xf);
		}
		else if ((data & 0xf0) == 0x40)
		{
			return 0x400|(data&0xf);
		}
		else if ((data & 0xf0) == 0x10)
		{
			return 0xe10|((data&0xf));
		}
		else if ((data & 0xf0) == 0x20)
		{
			return 0xe20|((data&0xf));
		}
		else if ((data & 0xf0) == 0xa0)
		{
			return 0xeb0|((data&0xf));
		}
		else if ((data & 0xf0) == 0xb0)
		{
			return 0xea0|((data&0xf));
		}
	}
	
	return 0;
}

static Uint8 find_note(Uint16 period)
{
	static const Uint16 periods[] = 
	{
		856,808,762,720,678,640,604,570,538,508,480,453,
		428,404,381,360,339,320,302,285,269,254,240,226,
		214,202,190,180,170,160,151,143,135,127,120,113,
		0
	};
	
	if (period == 0) return MUS_NOTE_NONE;
	
	for (int i = 0 ; periods[i] ; ++i)
		if (periods[i] == period) return i + MIDDLE_C - 12;
		
	return MUS_NOTE_NONE;
}

static int import_mod(FILE *f)
{
	char ver[4];
	
	fseek(f, 1080, SEEK_SET);
	fread(ver, 1, sizeof(ver), f);
	
	int channels = 0, instruments = 0;
	
	static const struct { int chn, inst; char *sig; } specs[] =
	{
		{ 4, 31, "M.K." },
		{ 4, 31, "M!K!" },
		{ 4, 31, "FLT4" },
		{ 8, 31, "FLT8" },
		{ 4, 31, "4CHN" },
		{ 6, 31, "6CHN" },
		{ 8, 31, "8CHN" },
		{ 0 }
	};
	
	for (int i = 0 ; specs[i].chn ; ++i)
	{
		if (strncmp(specs[i].sig, ver, 4) == 0)
		{
			channels = specs[i].chn;
			instruments = specs[i].inst;
			break;
		}
	}
	
	if (channels == 0) return 0;
	
	fseek(f, 0, SEEK_SET);
	fread(mused.song.title, 20, sizeof(char), f);
	
	for (int i = 0 ; i < instruments ; ++i)
	{
		char name[23] = { 0 };
		fread(name, 1, 22, f);
		name[15] = '\0';
		strcpy(mused.song.instrument[i].name, name);
		
		fseek(f, 3, SEEK_CUR); // skip sample length + finetune
		fread(&mused.song.instrument[i].volume, 1, 1, f);
		mused.song.instrument[i].volume *= 2;
		fseek(f, 4, SEEK_CUR); // skip loop length (use for setting base note?)	
	}
	
	Uint8 temp;
	
	fread(&temp, 1, sizeof(temp), f);
	mused.song.song_length = (Uint16)temp * 64;
	fread(&temp, 1, sizeof(temp), f);
	mused.song.loop_point = 0; //(Uint16)temp * 64;
	
	Uint8 sequence[128];
	fread(sequence, 1, 128, f);
	
	fseek(f, 4, SEEK_CUR); // skip id sig
	
	int pat = 0;
	int patterns = 0;
	
	for (int i = 0 ; i * 64 < mused.song.song_length ; ++i)
	{
		patterns = my_max(patterns, sequence[i]);
		for (int c = 0 ; c < channels ; ++c)
			add_sequence(c, i * 64, sequence[i] * channels + c, 0);
	}
	
	for (Uint8 i = 0 ; i <= patterns ; ++i)
	{
		for (int c = 0 ; c < channels ; ++c)
		{
			pat = i * channels + c;
			resize_pattern(&mused.song.pattern[pat], 64);
			memset(mused.song.pattern[pat].step, 0, sizeof(mused.song.pattern[pat].step[0]) * 64);
		}
		
		for (int s = 0 ; s < 64 ; ++s)
		{
			pat = i * channels;
		
			for (int c = 0 ; c < channels ; ++c)
			{
				Uint16 period;
				fread(&period, 1, sizeof(period), f);
				Uint8 inst, param;
				fread(&inst, 1, sizeof(inst), f);
				fread(&param, 1, sizeof(param), f);
				
				mused.song.pattern[pat].step[s].note = find_note(SDL_SwapBE16(period) & 0xfff);
				mused.song.pattern[pat].step[s].instrument = ((inst >> 4) | ((SDL_SwapBE16(period) & 0xf000) >> 8)) - 1;
				mused.song.pattern[pat].step[s].command = find_command_pt(param | ((inst & 0xf) << 8));
				++pat;
			}
		
		
		}
	}
	
	mused.sequenceview_steps = 64;
	
	return 1;
}


static int import_ahx(FILE *f)
{
	char sig[4];
	
	fread(sig, 1, sizeof(sig), f);
	
	int ver = 0;
	
	if (memcmp("THX\0", sig, 4) == 0)
	{
		ver = 0;
	}
	else if (memcmp("THX\1", sig, 4) == 0)
	{
		ver = 1;
	}
	else return 0;
	
	Uint8 byte;
	Uint16 word;
	
	fread(&word, 1, sizeof(word), f); // title offset
	fread(&word, 1, sizeof(word), f); 
	
	word = SDL_SwapBE16(word);
	
	debug("word = %x", word);
	
	int track0 = (word & 0x8000) != 0;
	
	mused.song.song_rate = (1 + ((word & 0x7000) >> 12)) * 50;
	
	int LEN = word & 0xfff;
	
	fread(&word, 1, sizeof(word), f); 
	
	mused.song.loop_point = SDL_SwapBE16(word);
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int TRL = mused.sequenceview_steps = byte;
	
	mused.song.loop_point *= TRL;
	mused.song.song_length = LEN * TRL;
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int TRK = byte;
	
	fread(&byte, 1, sizeof(byte), f);  
	
	int SMP = byte;
	
	fread(&byte, 1, sizeof(byte), f); 
	
	int SS = byte;
	
	fseek(f, SS * 2, SEEK_CUR); // we don't need the subsong positions
	
	debug("LEN = %d TRL = %d TRK = %d SMP = %d SS = %d", LEN, TRL, TRK, SMP, SS);
	
	for (int i = 0 ; i < LEN ; ++i)
	{
		for (int c = 0 ; c < 4 ; ++c)
		{
			Uint8 pat;
			Sint8 trans;
			fread(&pat, 1, 1, f);
			fread(&trans, 1, 1, f);
			
			if (!track0 && pat == 0)
				continue;
			
			add_sequence(c, i * TRL, pat - !track0, trans);
		}	
	}
	
	for (int pat = 0 ; pat < TRK ; ++pat)
	{
		resize_pattern(&mused.song.pattern[pat], TRL);
		
		for (int s = 0 ; s < TRL ; ++s)
		{
			Uint32 step = 0;
			fread((Uint8*)&step + 1, 3, 1, f);
			
			step = SDL_SwapBE32(step);
			
			Uint8 note = (step >> 18) & 0x3f;
			Uint8 instrument = (step >> 12) & 0x3f;
			Uint8 command = (step >> 8) & 0xf;
			Uint8 data = step & 0xff;
			
			mused.song.pattern[pat].step[s].note = note ? (note - 1) + 12 : MUS_NOTE_NONE;
			mused.song.pattern[pat].step[s].instrument = instrument ? instrument - 1 : MUS_NOTE_NO_INSTRUMENT;
			mused.song.pattern[pat].step[s].ctrl = 0;
			mused.song.pattern[pat].step[s].command = find_command_ahx(command, data, &mused.song.pattern[pat].step[s].ctrl);
		}
	}
	
	return 1;
}


void import_module(void *type, void* unused1, void* unused2)
{
	int r = confirm_ync(domain, mused.slider_bevel, &mused.largefont, "Save song?");
				
	if (r == 0) return;
	if (r == 1) { change_mode(EDITSEQUENCE); if (!save_data()) return; }
	
	static const char *mod_name[] = {"a Protracker", "an AHX"};
	static const char *mod_ext[] = {"mod", "ahx"};
	
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Import %s module", mod_name[(int)type]);

	FILE * f = open_dialog("rb", buffer, mod_ext[(int)type], domain, mused.slider_bevel, &mused.largefont, &mused.smallfont);
	
	if (!f) return;
	
	new_song();
	
	switch ((int)type)
	{
		case IMPORT_MOD: r = import_mod(f); break;
		case IMPORT_AHX: r = import_ahx(f); break;
	}
	
	if (!r) 
	{
		snprintf(buffer, sizeof(buffer), "Not %s module", mod_name[(int)type]);
		msgbox(domain, mused.slider_bevel, &mused.largefont, buffer, MB_OK);
	}
	
	fclose(f);
}
