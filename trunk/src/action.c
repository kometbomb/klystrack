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

#include "action.h"
#include "optimize.h"
#include "mused.h"
#include "gui/toolutil.h"
#include "view.h"
#include "event.h"
#include "gui/msgbox.h"
#include "version.h"
#include "../../klystron/src/version.h"
#include "gfx/gfx.h"
#include "theme.h"
#include "key.h"
#include "gui/menu.h"
#include "export.h"
#include <stdbool.h>
#include "gui/mouse.h"
#include "view/wavetableview.h"
#include "help.h"

extern Mused mused;
extern GfxDomain *domain;
extern const Menu mainmenu[];
extern Menu pixelmenu[];
extern Menu patternlengthmenu[];

bool inside_undo = false;

void select_sequence_position(void *channel, void *position, void* unused)
{
	if ((mused.flags & SONG_PLAYING) && (mused.flags & FOLLOW_PLAY_POSITION))
		return;

	if (CASTPTR(int,channel) != -1)
		mused.current_sequencetrack = CASTPTR(int,channel);
	
	if (CASTPTR(int,position) < mused.song.song_length)
		mused.current_sequencepos = CASTPTR(int,position);
		
	mused.pattern_position = mused.current_patternpos = mused.current_sequencepos;
		
	mused.focus = EDITSEQUENCE;
	
	update_horiz_sliders();
}


void select_pattern_param(void *id, void *position, void *track)
{
	mused.current_patternx = CASTPTR(int,id);
	mused.current_sequencetrack = CASTPTR(int,track);
	mused.pattern_position = mused.current_sequencepos = CASTPTR(int,position);
	
	mused.focus = EDITPATTERN;
	
	update_horiz_sliders();
}


void select_instrument_page(void *page, void *unused1, void *unused2)
{
	mused.instrument_page = CASTPTR(int,page) * 10;
	set_info_message("Selected instrument bank %d-%d", mused.instrument_page, mused.instrument_page + 9);
}



void select_instrument(void *idx, void *relative, void *pagey)
{
	if (pagey)
	{
		mused.current_instrument = mused.instrument_page + CASTPTR(int,idx);
	}
	else
	{
		if (relative)
			mused.current_instrument += CASTPTR(int,idx);
		else
			mused.current_instrument = CASTPTR(int,idx);
	}
			
	if (mused.current_instrument >= NUM_INSTRUMENTS) 
		mused.current_instrument = NUM_INSTRUMENTS-1;
	else if (mused.current_instrument < 0) 
		mused.current_instrument = 0;
	
}


void select_wavetable(void *idx, void *unused1, void *unused2)
{
	mused.selected_wavetable = CASTPTR(int,idx);
			
	if (mused.selected_wavetable >= CYD_WAVE_MAX_ENTRIES) 
		mused.selected_wavetable = CYD_WAVE_MAX_ENTRIES-1;
	else if (mused.selected_wavetable < 0) 
		mused.selected_wavetable = 0;
	
}


void change_octave(void *delta, void *unused1, void *unused2)
{
	mused.octave += CASTPTR(int,delta);
	if (mused.octave > 7) mused.octave = 7;
	else if (mused.octave < 0) mused.octave = 0;
}


void change_song_rate(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) > 0)
	{
		if ((int)mused.song.song_rate + CASTPTR(int,delta) <= 0xff)
			mused.song.song_rate += CASTPTR(int,delta);
		else
			mused.song.song_rate = 0xff;
	}
	else if (CASTPTR(int,delta) < 0)
	{
		if ((int)mused.song.song_rate + CASTPTR(int,delta) >= 0x1)
			mused.song.song_rate += CASTPTR(int,delta);
		else
			mused.song.song_rate = 1;
	}
	
	enable_callback(true);
}


void change_time_signature(void *beat, void *unused1, void *unused2)
{
	if (!beat)
	{
		mused.time_signature = (mused.time_signature & 0x00ff) | (((((mused.time_signature >> 8) + 1) & 0xff) % 17) << 8);
		if ((mused.time_signature & 0xff00) == 0) mused.time_signature |= 0x100;
	}
	else
	{
		mused.time_signature = (mused.time_signature & 0xff00) | ((((mused.time_signature & 0xff) + 1) & 0xff) % 17);
		if ((mused.time_signature & 0xff) == 0) mused.time_signature |= 1;
	}
}


void play(void *from_cursor, void *unused1, void *unused2)
{
	enable_callback(true);
	mus_set_song(&mused.mus, &mused.song, from_cursor ? mused.current_sequencepos : 0);
	mused.flags |= SONG_PLAYING;
	if (mused.flags & STOP_EDIT_ON_PLAY) mused.flags &= ~EDIT_MODE;
}


void play_position(void *unused1, void *unused2, void *unused3)
{
	if (!(mused.flags & LOOP_POSITION))
	{
		mused.flags |= LOOP_POSITION;
		mused.loop_store_length = mused.song.song_length;
		mused.loop_store_loop = mused.song.loop_point;
		
		mused.song.song_length = mused.current_sequencepos + mused.song.pattern[current_pattern(NULL)].num_steps;
		mused.song.loop_point = mused.current_sequencepos;
		
		play(MAKEPTR(1), 0, 0);
	}
}


void stop(void *unused1, void *unused2, void *unused3)
{
	mus_set_song(&mused.mus, NULL, 0);
	if (mused.flags & LOOP_POSITION)
	{
		mused.song.song_length = mused.loop_store_length;
		mused.song.loop_point = mused.loop_store_loop;
	}
	
	mused.flags &= ~(SONG_PLAYING | LOOP_POSITION);
}


void change_song_speed(void *speed, void *delta, void *unused)
{
	if (!speed)
	{
		if ((int)mused.song.song_speed + CASTPTR(int,delta) >= 1 && (int)mused.song.song_speed + CASTPTR(int,delta) <= 255)
			mused.song.song_speed += CASTPTR(int,delta);
	}
	else
	{
		if ((int)mused.song.song_speed2 + CASTPTR(int,delta) >= 1 && (int)mused.song.song_speed2 + CASTPTR(int,delta) <= 255)
		mused.song.song_speed2 += CASTPTR(int,delta);
	}
}


void select_instrument_param(void *idx, void *unused1, void *unused2)
{
	mused.selected_param = CASTPTR(int,idx);
}


void select_program_step(void *idx, void *digit, void *unused2)
{
	mused.current_program_step = CASTPTR(int,idx);
	mused.editpos = CASTPTR(int, digit);
}


void new_song_action(void *unused1, void *unused2, void *unused3)
{
	if (confirm(domain, mused.slider_bevel->surface, &mused.largefont, "Clear song and data?"))
	{
		stop(0,0,0);
		new_song();
	}
}


void kill_instrument(void *unused1, void *unused2, void *unused3)
{
	cyd_lock(&mused.cyd, 1);
	mus_get_default_instrument(&mused.song.instrument[mused.current_instrument]);
	cyd_lock(&mused.cyd, 0);
}


void generic_action(void *func, void *unused1, void *unused2)
{
	mus_set_song(&mused.mus, NULL, 0);
	cyd_lock(&mused.cyd, 1);
	
	((void *(*)(void))func)(); /* I love the smell of C in the morning */
	
	cyd_lock(&mused.cyd, 0);
}


void quit_action(void *unused1, void *unused2, void *unused3)
{
	mused.done = 1;
}


void change_mode_action(void *mode, void *unused1, void *unused2)
{
	change_mode(CASTPTR(int,mode));
}


void enable_channel(void *channel, void *unused1, void *unused2)
{
	debug("Toggle chn %d", CASTPTR(int,channel));
	mused.mus.channel[CASTPTR(int,channel)].flags ^= MUS_CHN_DISABLED;
}


void solo_channel(void *channel, void *unused1, void *unused2)
{
	int c = 0;
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
		if (!(mused.mus.channel[i].flags & MUS_CHN_DISABLED))
			++c;
			
	if (c == 1 && !(mused.mus.channel[CASTPTR(int,channel)].flags & MUS_CHN_DISABLED))
	{
		debug("Unmuted all");
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			mused.mus.channel[i].flags &= ~MUS_CHN_DISABLED;
	}	
	else
	{
		debug("Solo chn %d", CASTPTR(int,channel));
		for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
			mused.mus.channel[i].flags |= MUS_CHN_DISABLED;
			
		mused.mus.channel[CASTPTR(int,channel)].flags &= ~MUS_CHN_DISABLED;
	}
}


void unmute_all_action(void *unused1, void *unused2, void *unused3)
{
	for(int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
		mused.mus.channel[i].flags &= ~MUS_CHN_DISABLED;
}


void select_all(void *unused1, void *unused2, void *unused3)
{
	switch (mused.focus)
	{
		case EDITPATTERN:
			mused.selection.start = 0;
			mused.selection.end = mused.song.pattern[current_pattern(NULL)].num_steps;
			break;
			
		case EDITPROG:
			mused.selection.start = 0;
			mused.selection.end = MUS_PROG_LEN;
			break;
			
		case EDITSEQUENCE:
			mused.selection.start = 0;
			mused.selection.end = mused.song.song_length;
			break;
	}
}


void clear_selection(void *unused1, void *unused2, void *unused3)
{
	mused.selection.start = 0;
	mused.selection.end = 0;
}


void cycle_focus(void *_views, void *_focus, void *_mode)
{
	View **viewlist = _views;
	int *focus = _focus, *mode = _mode;
	View *views = viewlist[*mode];
	
	int i;
	for (i = 0 ; views[i].handler ; ++i)
	{
		if (views[i].focus == *focus) break;
	}
	
	if (!views[i].handler) return;
	
	int next;
	
	for (next = i + 1 ; i != next ; ++next)
	{
		if (views[next].handler == NULL)
		{
			next = -1;
			continue;
		}
		
		if (views[next].focus != -1 && views[next].focus != *focus) 
		{
			*focus = views[next].focus;
			break;
		}
	}
}


void change_song_length(void *delta, void *unused1, void *unused2)
{
	int l = mused.song.song_length;
	l += CASTPTR(int, delta);
	l = l - (l % mused.sequenceview_steps);

	mused.song.song_length = my_max(0, my_min(0xfffe, l));
}


void change_loop_point(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0)
	{
		if (mused.song.loop_point >= -CASTPTR(int,delta))
			mused.song.loop_point += CASTPTR(int,delta);
		else
			mused.song.loop_point = 0;
	}
	else if (CASTPTR(int,delta) > 0)
	{
		mused.song.loop_point += CASTPTR(int,delta);
		if (mused.song.loop_point >= mused.song.song_length)
			mused.song.loop_point = mused.song.song_length;
	}
}


void change_seq_steps(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0)
	{
		if (mused.sequenceview_steps > -CASTPTR(int,delta))
		{
			mused.sequenceview_steps += CASTPTR(int,delta);
		}
		else
			mused.sequenceview_steps = 1;
	}
	else if (CASTPTR(int,delta) > 0)
	{
		if (mused.sequenceview_steps == 1 && CASTPTR(int,delta) > 1)
			mused.sequenceview_steps = 0;
	
		if (mused.sequenceview_steps < 128)
		{
			mused.sequenceview_steps += CASTPTR(int,delta);
		}
		else
			mused.sequenceview_steps = 128;
	}
	
	mused.current_sequencepos = (mused.current_sequencepos/mused.sequenceview_steps) * mused.sequenceview_steps;
	
	if (mused.flags & LOCK_SEQUENCE_STEP_AND_PATTERN_LENGTH)
		change_default_pattern_length(MAKEPTR(mused.sequenceview_steps), NULL, NULL);
}


void show_about_box(void *unused1, void *unused2, void *unused3)
{
	msgbox(domain, mused.slider_bevel->surface, &mused.largefont, VERSION_STRING "\n" KLYSTRON_VERSION_STRING, MB_OK);
}


void change_channels(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0 && mused.song.num_channels > 1)
	{
		--mused.song.num_channels;
	}
	else if (CASTPTR(int,delta) > 0 && mused.song.num_channels < MUS_MAX_CHANNELS)
	{
		++mused.song.num_channels;
	}
}


void change_master_volume(void *delta, void *unused1, void *unused2)
{
	if (CASTPTR(int,delta) < 0 && mused.song.master_volume > 0)
	{
		mused.mus.volume = --mused.song.master_volume;
	}
	else if (CASTPTR(int,delta) > 0 && mused.song.master_volume < MAX_VOLUME)
	{
		mused.mus.volume = ++mused.song.master_volume;
	}
}


void begin_selection_action(void *unused1, void *unused2, void *unused3)
{
	switch (mused.focus)
	{
		case EDITPATTERN:
		begin_selection(current_patternstep());
		break;
		
		case EDITSEQUENCE:
		begin_selection(mused.current_sequencepos);
		break;
		
		case EDITPROG:
		begin_selection(mused.current_program_step);
		break;
	}
}


void end_selection_action(void *unused1, void *unused2, void *unused3)
{
	switch (mused.focus)
	{
		case EDITPATTERN:
		select_range(current_patternstep());
		break;
		
		case EDITSEQUENCE:
		select_range(mused.current_sequencepos);
		break;
		
		case EDITPROG:
		select_range(mused.current_program_step);
		break;
	}
}


void toggle_pixel_scale(void *a, void*b, void*c)
{
	change_pixel_scale(MAKEPTR(((domain->scale) & 3) + 1), 0, 0);
}


void change_pixel_scale(void *scale, void*b, void*c)
{	
	mused.pixel_scale = CASTPTR(int,scale);
	domain->screen_w = my_max(320, mused.window_w / mused.pixel_scale);
	domain->screen_h = my_max(240, mused.window_h / mused.pixel_scale);
	domain->scale = mused.pixel_scale;
	gfx_domain_update(domain);
	mused.screen = gfx_domain_get_surface(domain);
	
	for (int i = 0 ; i < 4; ++i)
	{
		if (pixelmenu[i].p1 == scale)
			pixelmenu[i].flags |= MENU_BULLET;
		else
			pixelmenu[i].flags &= ~MENU_BULLET;
	}
}


void toggle_fullscreen(void *a, void*b, void*c)
{
	SDL_Event e;
	e.button.button = SDL_BUTTON_LEFT;
	mouse_released(&e);
	mused.flags ^= FULLSCREEN;
	change_fullscreen(0,0,0);
}


void change_fullscreen(void *a, void*b, void*c)
{
	domain->fullscreen = (mused.flags & FULLSCREEN) != 0;
	gfx_domain_update(domain);
	mused.screen = gfx_domain_get_surface(domain);
}


void load_theme_action(void *name, void*b, void*c)
{
	load_theme((char*)name);
}


void load_keymap_action(void *name, void*b, void*c)
{
	load_keymap((char*)name);
}


void change_timesig(void *delta, void *b, void *c)
{
	// http://en.wikipedia.org/wiki/Time_signature says the following signatures are common. 
	// I'm a 4/4 or 3/4 man myself so I'll trust the article :)
	
	static const Uint16 sigs[] = { 0x0404, 0x0304, 0x0308, 0x0608, 0x0908, 0x0c08 };
	int i;
	for (i = 0 ; i < sizeof(sigs) / sizeof(sigs[0]) ; ++i)
	{
		if (sigs[i] == mused.time_signature)
			break;
	}
	
	i += CASTPTR(int,delta);
	
	if (i >= (int)(sizeof(sigs) / sizeof(sigs[0]))) i = 0;
	if (i < 0) i = sizeof(sigs) / sizeof(sigs[0]) - 1;
	mused.time_signature = sigs[i];
}


void export_wav_action(void *a, void*b, void*c)
{
	char def[1000];
	snprintf(def, sizeof(def), "%s.wav", mused.song.title);
	FILE * f = open_dialog("wb", "Export .WAV", "wav", domain, mused.slider_bevel->surface, &mused.largefont, &mused.smallfont, def);
	if (f)
	{
		export_wav(&mused.song, mused.mus.cyd->wavetable_entries, f);
		fclose(f);
	}
}


void do_undo(void *a, void*b, void*c)
{
	UndoFrame *frame = a ? undo(&mused.redo) : undo(&mused.undo);
	
	debug("%s frame %p", a ? "Redo" : "Undo", frame);
	
	if (!frame) return;
	
	

	if (!a)
	{
		UndoStack tmp = mused.redo;
		mused.redo = mused.undo;
		mused.undo = tmp;
	}
	
	switch (frame->type)
	{
		case UNDO_PATTERN:
			undo_store_pattern(&mused.undo, frame->event.pattern.idx, &mused.song.pattern[frame->event.pattern.idx], mused.modified);
			
			resize_pattern(&mused.song.pattern[frame->event.pattern.idx], frame->event.pattern.n_steps);
			memcpy(mused.song.pattern[frame->event.pattern.idx].step, frame->event.pattern.step, frame->event.pattern.n_steps * sizeof(frame->event.pattern.step[0]));
			break;
			
		case UNDO_SEQUENCE:
			mused.current_sequencetrack = frame->event.sequence.channel;
			
			undo_store_sequence(&mused.undo, mused.current_sequencetrack, mused.song.sequence[mused.current_sequencetrack], mused.song.num_sequences[mused.current_sequencetrack], mused.modified);
			
			mused.song.num_sequences[mused.current_sequencetrack] = frame->event.sequence.n_seq;
			
			memcpy(mused.song.sequence[mused.current_sequencetrack], frame->event.sequence.seq, frame->event.sequence.n_seq * sizeof(frame->event.sequence.seq[0]));
			break;
			
		case UNDO_MODE:
			change_mode(frame->event.mode.old_mode);
			mused.focus = frame->event.mode.focus;
			break;
		
		case UNDO_INSTRUMENT:
			mused.current_instrument = frame->event.instrument.idx;
			
			undo_store_instrument(&mused.undo, mused.current_instrument, &mused.song.instrument[mused.current_instrument], mused.modified);
			
			memcpy(&mused.song.instrument[mused.current_instrument], &frame->event.instrument.instrument, sizeof(frame->event.instrument.instrument));
			
			break;
			
		case UNDO_FX:
			mused.fx_bus = frame->event.fx.idx;
			
			undo_store_fx(&mused.undo, mused.fx_bus, &mused.song.fx[mused.fx_bus], mused.song.multiplex_period, mused.modified);
			
			memcpy(&mused.song.fx[mused.fx_bus], &frame->event.fx.fx, sizeof(frame->event.fx.fx));
			mused.song.multiplex_period = frame->event.fx.multiplex_period;
			mus_set_fx(&mused.mus, &mused.song);
			break;
			
		case UNDO_SONGINFO:
		{
			undo_store_songinfo(&mused.undo, &mused.song, mused.modified);
		
			MusSong *song = &mused.song;
			song->song_length = frame->event.songinfo.song_length;  
			mused.sequenceview_steps = song->sequence_step = frame->event.songinfo.sequence_step;  
			song->loop_point = frame->event.songinfo.loop_point;
			song->song_speed = frame->event.songinfo.song_speed;
			song->song_speed2 = frame->event.songinfo.song_speed2; 
			song->song_rate = frame->event.songinfo.song_rate;
			song->time_signature = frame->event.songinfo.time_signature;
			song->flags = frame->event.songinfo.flags;
			song->num_channels = frame->event.songinfo.num_channels;
			strcpy(song->title, frame->event.songinfo.title);
			song->master_volume = frame->event.songinfo.master_volume;
			memcpy(song->default_volume, frame->event.songinfo.default_volume, sizeof(frame->event.songinfo.default_volume));
			memcpy(song->default_panning, frame->event.songinfo.default_panning, sizeof(frame->event.songinfo.default_panning));
		}
		break;
		
		case UNDO_WAVE_PARAM:
		{
			mused.selected_wavetable = frame->event.wave_param.idx;
			
			undo_store_wave_param(&mused.undo, mused.selected_wavetable, &mused.mus.cyd->wavetable_entries[mused.selected_wavetable], mused.modified);
			
			CydWavetableEntry *entry = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
			entry->flags = frame->event.wave_param.flags;
			entry->sample_rate = frame->event.wave_param.sample_rate;
			entry->samples = frame->event.wave_param.samples; 
			entry->loop_begin = frame->event.wave_param.loop_begin;
			entry->loop_end = frame->event.wave_param.loop_end;
			entry->base_note = frame->event.wave_param.base_note;
		}
		break;
		
		case UNDO_WAVE_DATA:
		{
			mused.selected_wavetable = frame->event.wave_param.idx;
			
			undo_store_wave_data(&mused.undo, mused.selected_wavetable, &mused.mus.cyd->wavetable_entries[mused.selected_wavetable], mused.modified);
			
			CydWavetableEntry *entry = &mused.mus.cyd->wavetable_entries[mused.selected_wavetable];
			entry->data = realloc(entry->data, frame->event.wave_data.length * sizeof(entry->data[0]));
			memcpy(entry->data, frame->event.wave_data.data, frame->event.wave_data.length * sizeof(entry->data[0]));
			entry->samples = frame->event.wave_data.length;
			entry->sample_rate = frame->event.wave_data.sample_rate;
			entry->samples = frame->event.wave_data.samples; 
			entry->loop_begin = frame->event.wave_data.loop_begin;
			entry->loop_end = frame->event.wave_data.loop_end;
			entry->flags = frame->event.wave_data.flags;
			entry->base_note = frame->event.wave_data.base_note;
			
			invalidate_wavetable_view();
		}
		break;
		
		default: warning("Undo type %d not handled", frame->type); break;
	}
	
	mused.modified = frame->modified;
	
	if (!a)
	{
		UndoStack tmp = mused.redo;
		mused.redo = mused.undo;
		mused.undo = tmp;
	}
	
	
	mused.last_snapshot_a = -1;
	
/*#ifdef DEBUG
	undo_show_stack(&mused.undo);
	undo_show_stack(&mused.redo);
#endif*/
	
	undo_destroy_frame(frame);
}


void kill_wavetable_entry(void *a, void*b, void*c)
{
	snapshot(S_T_WAVE_DATA);
	cyd_wave_entry_init(&mused.mus.cyd->wavetable_entries[mused.selected_wavetable], NULL, 0, 0, 0, 0, 0);
}


void open_menu_action(void*a,void*b,void*c)
{
	my_open_menu(mainmenu, NULL);
}


void flip_bit_action(void *a, void *b, void *c)
{
	*(Uint32*)a ^= CASTPTR(Uint32,b);
}


void set_note_jump(void *steps, void *unused1, void *unused2)
{
	mused.note_jump = CASTPTR(int, steps);
	
	set_info_message("Note jump set to %d", mused.note_jump);
}


void change_default_pattern_length(void *length, void *unused1, void *unused2)
{
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		if (mused.song.pattern[i].num_steps == mused.default_pattern_length && is_pattern_empty(&mused.song.pattern[i]))
		{
			resize_pattern(&mused.song.pattern[i], CASTPTR(int,length));
		}
	}
	
	mused.sequenceview_steps = mused.default_pattern_length = CASTPTR(int,length);
	
	for (Menu *m = patternlengthmenu ; m->text ; ++m)
	{
		if (CASTPTR(int, m->p1) == mused.default_pattern_length)
			m->flags |= MENU_BULLET;
		else
			m->flags &= ~MENU_BULLET;
	}
}


void change_visualizer_action(void *vis, void *unused1, void *unused2)
{
	change_visualizer(CASTPTR(int,vis));
}


void open_help(void *unused0, void *unused1, void *unused2)
{
	helpbox("Help", domain, mused.slider_bevel->surface, &mused.largefont, &mused.smallfont);
}
