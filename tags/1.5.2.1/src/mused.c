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
#include "util/bundle.h"
#include "gfx/font.h"
#include "gfx/gfx.h"
#include "action.h"
#include "event.h"
#include "theme.h"
#include "undo.h"
#include "edit.h"

extern Mused mused;
extern Menu editormenu[];

void set_edit_buffer(char *buffer, size_t size)
{ 
	if (mused.edit_backup_buffer) 
		free(mused.edit_backup_buffer); 
		
	mused.edit_backup_buffer = strdup(buffer); 
	mused.edit_buffer = buffer; 
	mused.edit_buffer_size = size; 
	mused.editpos = strlen(mused.edit_buffer);
	change_mode(EDITBUFFER); 
} 


void update_ghost_patterns()
{
	/*for (int e = 0 ; e < mused.song.num_sequences[mused.current_sequencetrack] ; ++e)
			if (mused.song.sequence[mused.current_sequencetrack][e].position == mused.current_sequencepos)
				mused.current_sequencepos = mused.song.sequence[mused.current_sequencetrack][e].position;*/
				
	memset(mused.ghost_pattern, 0, sizeof(mused.ghost_pattern));
				
	int p = 0;				
	for (int i = 0 ; i < mused.song.num_channels ; ++i)
	{
		for (int e = 0 ; e < mused.song.num_sequences[i] ; ++e)
			if (mused.song.sequence[i][e].position == mused.current_sequencepos)
			{
				mused.ghost_pattern[i] = &mused.song.sequence[i][e].pattern;
				if (i == mused.current_sequencetrack) mused.current_pattern = mused.song.sequence[i][e].pattern; 
				++p;
			}
	}
	
	mused.current_patternstep = 0;
	mused.current_patternx = 0;
	mused.single_pattern_edit = (p <= 1);
}


void change_mode(int newmode)
{
	if (newmode < VIRTUAL_MODE && mused.mode != MENU) 
	{
		SDL_FillRect(mused.screen, NULL, 0);
		clear_selection(0,0,0);
	}

	if (mused.mode < VIRTUAL_MODE)
	{
		for (int i = 0 ; editormenu[i].parent ; ++i)
			editormenu[i].flags = (editormenu[i].flags & ~MENU_BULLET) | (mused.mode == CASTPTR(int, editormenu[i].p1) ? MENU_BULLET : 0);
		mused.prev_mode = mused.mode;
		
		mused.cursor.w = mused.cursor.h = mused.cursor_target.w = mused.cursor_target.h = 0;
		
		if (newmode != mused.mode && newmode < VIRTUAL_MODE) snapshot(S_T_MODE);
	}
	
	switch (newmode)
	{
		case EDITFX:
			if (mused.mode == EDITINSTRUMENT)
			{
				mused.fx_bus = mused.song.instrument[mused.current_instrument].fx_bus;
			}
			
			break;
			
		case EDITWAVETABLE:
			if (mused.mode == EDITINSTRUMENT)
			{
				mused.selected_wavetable = mused.song.instrument[mused.current_instrument].wavetable_entry;
			}
			
			break;
	
		case EDITCLASSIC:
		case EDITPATTERN:
			if (mused.mode == EDITBUFFER) break;
			if (mused.mode == EDITSEQUENCE || (mused.mode == MENU && !mused.single_pattern_edit) || newmode == EDITCLASSIC)
			{
				update_ghost_patterns();
				slider_move_position(&mused.current_sequencetrack, &mused.pattern_horiz_position, &mused.pattern_horiz_slider_param, 0, mused.pattern_horiz_slider_param.last - mused.pattern_horiz_slider_param.first + 1);
				slider_move_position(&mused.current_patternstep, &mused.pattern_position, &mused.pattern_slider_param, 0, mused.song.pattern[mused.current_pattern].num_steps);
			}
			else
			{
				mused.single_pattern_edit = 1;
				for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
				{
					mused.ghost_pattern[i] = NULL;
				}
				mused.current_patternstep = 0;
				mused.current_patternx = 0;
			}
			break;
	}

	mused.mode = newmode;
	mused.focus = newmode;
	if (mused.focus == EDITCLASSIC)
		mused.focus = EDITPATTERN;
		
	if (mused.mode == EDITCLASSIC || mused.mode == EDITPATTERN)
	{
		mused.selected_param = 0;
	}
}


void clear_pattern(MusPattern *pat)
{
	snapshot(S_T_PATTERN);
	clear_pattern_range(pat, 0, pat->num_steps);
}


void clear_pattern_range(MusPattern *pat, int first, int last)
{
	for (int i = first ; i < last ; ++i)
	{
		pat->step[i].note = MUS_NOTE_NONE;
		pat->step[i].instrument = MUS_NOTE_NO_INSTRUMENT;
		pat->step[i].ctrl = 0;
		pat->step[i].command = 0;
		pat->step[i].volume = MUS_NOTE_NO_VOLUME;
	}
}


void new_song()
{
	for (int i = 0 ; i < NUM_INSTRUMENTS ; ++i)
	{
		MusInstrument *inst = &mused.song.instrument[i];
		kt_default_instrument(inst);
	}
	
	mused.song.master_volume = MAX_VOLUME;
	mused.song.num_channels = 4;
	mused.song.num_instruments = NUM_INSTRUMENTS;
	mused.song.num_patterns = NUM_PATTERNS;
	mused.song.song_speed = 6;
	mused.song.song_speed2 = 6;
	mused.song.song_rate = 50;
	mused.song.song_length = 0;
	mused.song.loop_point = 0;
	mused.song.flags = 0;
	mused.current_patternstep = 0;
	mused.current_sequencepos = 0;
	mused.current_sequencetrack = 0;
	update_position_sliders();
	memset(mused.song.title, 0, sizeof(mused.song.title));
	
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		clear_pattern_range(&mused.song.pattern[i], 0, mused.song.pattern[i].num_steps);
	}
	
	for (int fx = 0 ; fx < CYD_MAX_FX_CHANNELS ; ++fx)
	{	
		mused.song.fx[fx].flags = 0;
		mused.song.fx[fx].crushex.downsample = 0;
		mused.song.fx[fx].crush.bit_drop = 4;
		mused.song.fx[fx].chr.min_delay = 0;
		mused.song.fx[fx].chr.rate = 40;
		mused.song.fx[fx].chr.max_delay = 20;
		mused.song.fx[fx].rvb.spread = 0;
		for (int i = 0 ; i < CYDRVB_TAPS ; ++i)
		{
			mused.song.fx[fx].rvb.tap[i].delay = i * 100 + 50;
			mused.song.fx[fx].rvb.tap[i].gain = (i + 1) * -30;
		}
	}
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
	{
		memset(mused.song.sequence[i], 0, NUM_SEQUENCES * sizeof(*mused.song.sequence));
		mused.song.num_sequences[i] = 0;
		mused.song.default_volume[i] = MAX_VOLUME;
		mused.song.default_panning[i] = 0;
	}
	
	memset(mused.ghost_pattern, 0, sizeof(mused.ghost_pattern));
	
	if (mused.mus.cyd) cyd_reset_wavetable(mused.mus.cyd);
	
	mirror_flags();
	
	undo_deinit(&mused.undo);
	undo_init(&mused.undo);
	undo_deinit(&mused.redo);
	undo_init(&mused.redo);
	
	mused.modified = false;
}


void kt_default_instrument(MusInstrument *inst)
{
	mus_get_default_instrument(inst);
}


void resize_pattern(MusPattern * pattern, Uint16 new_size)
{
	if (new_size > pattern->num_steps)
	{
		pattern->step = realloc(pattern->step, sizeof(pattern->step[0]) * (size_t)new_size);
	}
	
	pattern->num_steps = new_size;
	
	if (mused.focus == EDITPATTERN)
	{
		mused.selection.start = my_min(mused.selection.start, pattern->num_steps - 1);
		mused.selection.end = my_min(mused.selection.end, pattern->num_steps - 1);
	}	
}


void init(MusInstrument *instrument, MusPattern *pattern, MusSeqPattern sequence[MUS_MAX_CHANNELS][NUM_SEQUENCES], MusChannel *channel, SDL_Surface *screen)
{
	debug("init");
	
	memset(&mused, 0, sizeof(mused));
	
	mused.flags = MULTICHANNEL_PREVIEW|ANIMATE_CURSOR|EDIT_MODE|SHOW_LOGO|CENTER_PATTERN_EDITOR|FOLLOW_PLAY_POSITION;
	mused.visible_columns = VC_INSTRUMENT | VC_COMMAND;
	mused.screen = screen;
	mused.done = 0;
	mused.octave = 4;
	mused.note_jump = 1;
	mused.current_instrument = 0;
	mused.selected_param = 0;
	mused.editpos = 0;
	mused.mode = EDITINSTRUMENT;
	mused.current_patternstep = 0;
	mused.current_pattern = 0;
	mused.current_patternx = 0;
	mused.current_sequencepos = 0;
	mused.sequenceview_steps = 16;
	mused.current_sequencetrack = 0;
	mused.time_signature = 0x0404;
	mused.prev_mode = 0;
	mused.edit_backup_buffer = NULL;
	mused.pixel_scale = 1;
	mused.mix_rate = 44100;
	mused.mix_buffer = 2048;
	mused.window_w = 640;
	mused.window_h = 480;
	mused.fx_room_size = 16;
	mused.fx_room_vol = 8;
	mused.fx_room_dec = 8;
		
	strcpy(mused.themename, "Default");
	
	memset(&mused.cp, 0, sizeof(mused.cp));
	memset(&mused.song, 0, sizeof(mused.song));
	mused.song.instrument = instrument;
	mused.song.pattern = pattern;
	mused.channel = channel;

	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
	{
		mused.song.sequence[i] = sequence[i];	
	}
	
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		mused.song.pattern[i].step = NULL; 
		mused.song.pattern[i].num_steps = 0;
		resize_pattern(&mused.song.pattern[i], 16);
	}
	
	undo_init(&mused.undo);
	undo_init(&mused.redo);
	
	new_song();
	
	enum_themes();
	
	//change_mode(EDITCLASSIC);
	mused.mode = EDITCLASSIC;
	mused.focus = EDITPATTERN;
	mused.single_pattern_edit = 1;
		
	debug("undo = %p redo = %p", mused.undo, mused.redo);
	
	debug("init done");
}


void init_scrollbars()
{
	slider_set_params(&mused.sequence_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	slider_set_params(&mused.pattern_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	slider_set_params(&mused.instrument_list_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
	slider_set_params(&mused.pattern_horiz_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
	slider_set_params(&mused.sequence_horiz_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
	slider_set_params(&mused.program_slider_param, 0, 0, 0, 0, &mused.sequence_position, 1, SLIDER_VERTICAL, mused.slider_bevel->surface);
}


void deinit()
{
	undo_deinit(&mused.undo);
	undo_deinit(&mused.redo);

	console_destroy(mused.console);
	if (mused.slider_bevel) gfx_free_surface(mused.slider_bevel);
	if (mused.vu_meter) gfx_free_surface(mused.vu_meter);
	if (mused.analyzer) gfx_free_surface(mused.analyzer);
	if (mused.logo) gfx_free_surface(mused.logo);
	font_destroy(&mused.smallfont);
	font_destroy(&mused.largefont);
	font_destroy(&mused.menufont);
	font_destroy(&mused.menufont_selected);
	font_destroy(&mused.shortcutfont);
	font_destroy(&mused.shortcutfont_selected);
	font_destroy(&mused.headerfont);
	font_destroy(&mused.headerfont_selected);
	free_themes();
}


void mirror_flags()
{
	// We need to mirror the flags to the corresponding Cyd flags
	for (int fx = 0 ; fx < CYD_MAX_FX_CHANNELS ; ++fx)
	{
		mused.cyd.fx[0].flags = mused.song.fx[0].flags;
	}
	
	mused.mus.volume = mused.song.master_volume;
}


int viscol(int col)
{
	return !(mused.flags & COMPACT_VIEW) || (mused.visible_columns & col);
}
