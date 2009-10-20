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

#include "diskop.h"
#include "toolutil.h"
#include "mused.h"
#include "macros.h"
#include "msgbox.h"

extern Mused mused;

	
void save_instrument(FILE *f, MusInstrument *inst)
{
	_VER_WRITE(&inst->flags, 0);
	_VER_WRITE(&inst->cydflags, 0);
	_VER_WRITE(&inst->adsr, 0);
	_VER_WRITE(&inst->sync_source, 0);
	_VER_WRITE(&inst->ring_mod, 0); 
	_VER_WRITE(&inst->pw, 0);
	_VER_WRITE(&inst->volume, 0);
	Uint8 progsteps = 0;
	for (int i = 0 ; i < MUS_PROG_LEN ; ++i)
		if (inst->program[i] != MUS_FX_NOP) progsteps = i+1;
	_VER_WRITE(&progsteps, 0);
	if (progsteps)
		_VER_WRITE(&inst->program, (int)(progsteps)*sizeof(inst->program[0]));
	_VER_WRITE(&inst->prog_period, 0); 
	_VER_WRITE(&inst->vibrato_speed, 0); 
	_VER_WRITE(&inst->vibrato_depth, 0); 
	_VER_WRITE(&inst->pwm_speed, 0); 
	_VER_WRITE(&inst->pwm_depth, 0); 
	_VER_WRITE(&inst->slide_speed, 0);
	_VER_WRITE(&inst->base_note, 0);
	_VER_WRITE(inst->name, sizeof(inst->name));
	_VER_WRITE(&inst->cutoff, 0);
	_VER_WRITE(&inst->resonance, 0);
	_VER_WRITE(&inst->flttype, 0);
	
}

int save_data()
{
	switch (mused.mode)
	{
		case EDITPROG:
		case EDITINSTRUMENT:
		{
			FILE * f = open_dialog("wb", "Save instrument", "Instrument\0*.ins\0\0");
			
			if (f)
			{
				const Uint8 version = MUS_VERSION;
				
				fwrite(MUS_INST_SIG, strlen(MUS_INST_SIG), sizeof(MUS_INST_SIG[0]), f);
				
				fwrite(&version, 1, sizeof(version), f);
				
				save_instrument(f, &mused.song.instrument[mused.current_instrument]);
			
				fclose(f);
			}
			
			return f != NULL;
		}
		break;
		
		default:
		{
			FILE * f = open_dialog("wb", "Save song", "Song\0*.sng\0\0");
			
			if (f)
			{
				if (!confirm("Keep unused patterns?"))
				{
					int maxpat = -1;
					for (int c = 0 ; c < mused.song.num_channels ; ++c)
					{
						for (int i = 0 ; i < mused.song.num_sequences[c] ; ++i)
							 if (maxpat < mused.song.sequence[c][i].pattern)
								maxpat = mused.song.sequence[c][i].pattern;
					}
					
					mused.song.num_patterns = maxpat + 1;
				}
			
				fwrite(MUS_SONG_SIG, strlen(MUS_SONG_SIG), sizeof(MUS_SONG_SIG[0]), f);
				
				const Uint8 version = MUS_VERSION;
				
				fwrite(&version, 1, sizeof(version), f);
				
				fwrite(&mused.song.num_channels, 1, sizeof(mused.song.num_channels), f);
				fwrite(&mused.song.time_signature, 1, sizeof(mused.song.time_signature), f);
				fwrite(&mused.song.num_instruments, 1, sizeof(mused.song.num_instruments), f);
				fwrite(&mused.song.num_patterns, 1, sizeof(mused.song.num_patterns), f);
				fwrite(mused.song.num_sequences, 1, sizeof(mused.song.num_sequences[0]) * (int)mused.song.num_channels, f);
				fwrite(&mused.song.song_length, 1, sizeof(mused.song.song_length), f);
				fwrite(&mused.song.loop_point, 1, sizeof(mused.song.loop_point), f);
				fwrite(&mused.song.song_speed, 1, sizeof(mused.song.song_speed), f);
				fwrite(&mused.song.song_speed2, 1, sizeof(mused.song.song_speed2), f);
				fwrite(&mused.song.song_rate, 1, sizeof(mused.song.song_rate), f);
				fwrite(&mused.song.flags, 1, sizeof(mused.song.flags), f);
				fwrite(mused.song.title, 1, MUS_TITLE_LEN + 1, f);
				
				if (mused.song.flags & MUS_ENABLE_REVERB)
				{
					for (int i = 0 ; i < CYDRVB_TAPS ; ++i)	
					{
						fwrite(&mused.song.rvbtap[i].gain, 1, sizeof(mused.song.rvbtap[i].gain), f);
						fwrite(&mused.song.rvbtap[i].delay, 1, sizeof(mused.song.rvbtap[i].delay), f);
					}
				}
				
				for (int i = 0 ; i < mused.song.num_instruments; ++i)
				{
					save_instrument(f, &mused.song.instrument[i]);
				}
				
				for (int i = 0 ; i < mused.song.num_channels; ++i)
				{
					fwrite(mused.song.sequence[i], mused.song.num_sequences[i], sizeof(mused.song.sequence[i][0]), f);
				}
				
				for (int i = 0 ; i < mused.song.num_patterns; ++i)
				{
					fwrite(&mused.song.pattern[i].num_steps, 1, sizeof(mused.song.pattern[i].num_steps), f);
					fwrite(mused.song.pattern[i].step, mused.song.pattern[i].num_steps, sizeof(mused.song.pattern[i].step[0]), f);
				}
				
				fclose(f);
				
				mused.song.num_patterns = NUM_PATTERNS;
				mused.song.num_instruments = NUM_INSTRUMENTS;
			}		
			
			return f != NULL;
		}
		break;
	}
}


void open_data()
{
	switch (mused.mode)
	{
		case EDITINSTRUMENT:
		{
			FILE * f = open_dialog("rb", "Load instrument", "Instrument\0*.ins\0\0");
			
			if (f)
			{
				if (!mus_load_instrument_file2(f, &mused.song.instrument[mused.current_instrument])) msgbox("Could not load instrument", MB_OK);
				
				fclose(f);
			}
		}
		break;
		
		default:
		{
			new_song();
		
			FILE * f = open_dialog("rb", "Load song", "Song\0*.sng\0\0");
			
			if (f)
			{
				if (!mus_load_song_file(f, &mused.song)) msgbox("Could not load song", MB_OK);
				
				fclose(f);
				
				mused.song.num_patterns = NUM_PATTERNS;
				mused.song.num_instruments = NUM_INSTRUMENTS;
				
				// Use kewlkool heuristics to determine sequence spacing
				
				mused.sequenceview_steps = 1000;
				
				for (int c = 0 ; c < MUS_MAX_CHANNELS ; ++c)
					for (int s = 1 ; s < mused.song.num_sequences[c] ; ++s)
						if (mused.sequenceview_steps > mused.song.sequence[c][s].position - mused.song.sequence[c][s-1].position)
						{
							mused.sequenceview_steps = mused.song.sequence[c][s].position - mused.song.sequence[c][s-1].position;
						}
				
				if (mused.sequenceview_steps == 1000) mused.sequenceview_steps = 16;
				
				mus_set_reverb(&mused.mus, &mused.song);
				cyd_set_callback(&mused.cyd, mus_advance_tick, &mused.mus, mused.song.song_rate);
				mirror_flags();
			}		
		}
		break;
	}
}