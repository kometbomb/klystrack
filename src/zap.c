#include "zap.h"
#include "mused.h"
#include "gui/toolutil.h"
#include <string.h>
#include "event.h"
#include "view/wavetableview.h"

void zap_instruments(void* no_confirm, void* b, void* c)
{
	if (!CASTPTR(int, no_confirm) && !confirm(domain, mused.slider_bevel, &mused.largefont, "Zap instruments (no undo)?"))
		return;
	
	debug("Zap instruments");
	
	for (int i = 0 ; i < NUM_INSTRUMENTS ; ++i)
	{
		MusInstrument *inst = &mused.song.instrument[i];
		kt_default_instrument(inst);
	}
}


void zap_sequence(void* no_confirm, void* b, void* c)
{
	if (!CASTPTR(int, no_confirm) && !confirm(domain, mused.slider_bevel, &mused.largefont, "Zap sequence (no undo)?"))
		return;
	
	debug("Zap sequence");
	
	for (int i = 0 ; i < MUS_MAX_CHANNELS ; ++i)
	{
		memset(mused.song.sequence[i], 0, NUM_SEQUENCES * sizeof(*mused.song.sequence));
		mused.song.num_sequences[i] = 0;
		mused.song.default_volume[i] = MAX_VOLUME;
		mused.song.default_panning[i] = 0;
	}
	
	for (int i = 0 ; i < NUM_PATTERNS ; ++i)
	{
		clear_pattern_range(&mused.song.pattern[i], 0, mused.song.pattern[i].num_steps);
	}
	
	mused.sequence_position = 0;
	mused.pattern_position = 0;
	mused.current_sequencepos = 0;
	mused.current_sequencetrack = 0;
	
	mused.song.song_length = 0;
	mused.song.loop_point = 0;
	
	update_position_sliders();
}


void zap_fx(void* no_confirm, void* b, void* c)
{
	if (!CASTPTR(int, no_confirm) && !confirm(domain, mused.slider_bevel, &mused.largefont, "Zap FX (no undo)?"))
		return;
	
	debug("Zap FX");
	
	mused.song.flags = 0;
	
	for (int fx = 0 ; fx < CYD_MAX_FX_CHANNELS ; ++fx)
	{	
		mused.song.fx[fx].flags = 0;
		mused.song.fx[fx].crushex.downsample = 0;
		mused.song.fx[fx].crush.bit_drop = 4;
		mused.song.fx[fx].crushex.gain = 128;
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
	
	mirror_flags();
}


void zap_wavetable(void* no_confirm, void* b, void* c)
{
	if (!CASTPTR(int, no_confirm) && !confirm(domain, mused.slider_bevel, &mused.largefont, "Zap wavetable (no undo)?"))
		return;
	
	debug("Zap wavetable");
	
	if (mused.song.wavetable_names)
	{
		for (int i = 0 ; i < CYD_WAVE_MAX_ENTRIES ; ++i)
			free(mused.song.wavetable_names[i]);
		free(mused.song.wavetable_names);
	}
	
	mused.song.wavetable_names = malloc(CYD_WAVE_MAX_ENTRIES * sizeof(char*));
	
	for (int i = 0 ; i < CYD_WAVE_MAX_ENTRIES ; ++i)
	{
		mused.song.wavetable_names[i] = malloc(MUS_WAVETABLE_NAME_LEN + 1);
		strcpy(mused.song.wavetable_names[i], "");
	}
	
	if (mused.mus.cyd) cyd_reset_wavetable(mused.mus.cyd);
	
	invalidate_wavetable_view();
}

