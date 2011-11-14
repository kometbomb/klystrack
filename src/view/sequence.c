#include "sequence.h"
#include "../mused.h"
#include "gui/bevel.h"
#include "../mybevdefs.h"

void sequence_view2(SDL_Surface *dest_surface, const SDL_Rect *_dest, const SDL_Event *event, void *param)
{
	SDL_Rect dest;
	copy_rect(&dest, _dest);
	
	bevel(mused.screen, _dest, mused.slider_bevel->surface, BEV_SEQUENCE_BORDER);
	
	adjust_rect(&dest, 1);
	
	const int height = 12;
	const int top = mused.sequence_position;
	const int bottom = top + dest.h * mused.sequenceview_steps / height;
	
	slider_set_params(&mused.sequence_slider_param, 0, mused.song.song_length - 1, my_max(0, top), bottom - mused.sequenceview_steps, &mused.sequence_position, mused.sequenceview_steps, SLIDER_VERTICAL, mused.slider_bevel->surface);
	
	const int w = my_max(dest.w / mused.song.num_channels - 1, 99);
	
	slider_set_params(&mused.sequence_horiz_slider_param, 0, mused.song.num_channels - 1, mused.sequence_horiz_position, my_min(mused.song.num_channels - 1, mused.sequence_horiz_position + dest.w / w), &mused.sequence_horiz_position, mused.sequenceview_steps, SLIDER_VERTICAL, mused.slider_bevel->surface);
		
	for (int channel = mused.sequence_horiz_position ; channel < mused.song.num_channels ; ++channel)
	{
		const MusSeqPattern *sp = &mused.song.sequence[channel][0];
		const int x = channel * (w + 1);
		
		for (int i = 0 ; i < mused.song.num_sequences[channel] ; ++i, ++sp)
		{
			if (sp->position >= bottom) break;
			
			int len = mused.song.pattern[sp->pattern].num_steps;
			
			if (sp->position + len <= top) continue;
			
			if (i < mused.song.num_sequences[channel] - 1)
				len = my_min(len, (sp + 1)->position - sp->position);
			
			SDL_Rect pat = { x + dest.x, (sp->position - top) * height / mused.sequenceview_steps + dest.y, w, len * height / mused.sequenceview_steps };
			SDL_Rect text;
			
			copy_rect(&text, &pat);
			clip_rect(&text, &dest);
			
			SDL_SetClipRect(mused.screen, &dest);
			
			bevel(mused.screen, &pat, mused.slider_bevel->surface, BEV_THIN_FRAME);
			
			adjust_rect(&text, 2);
			
			SDL_SetClipRect(mused.screen, &text);
			
			font_write_args(&mused.largefont, mused.screen, &text, "%02X+%d", sp->pattern, sp->note_offset);
		}
	}
	
	if (mused.focus == EDITSEQUENCE)
	{
		SDL_Rect pat = { mused.current_sequencetrack * w + dest.x, (mused.current_sequencepos - top) * height / mused.sequenceview_steps + dest.y, w, height };
			
		clip_rect(&pat, &dest);
		adjust_rect(&pat, -2);
		
		set_cursor(&pat);
	}
	
	SDL_SetClipRect(mused.screen, NULL);
}
