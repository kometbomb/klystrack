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
	
	const int w = my_max(dest.w / mused.song.num_channels - 1, 64);
	
	slider_set_params(&mused.sequence_horiz_slider_param, 0, mused.song.num_channels - 1, mused.sequence_horiz_position, my_min(mused.song.num_channels - 1, mused.sequence_horiz_position + (dest.w - (w - 1)) / (w + 1)), 
		&mused.sequence_horiz_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel->surface);
		
	for (int channel = mused.sequence_horiz_position ; channel < mused.song.num_channels ; ++channel)
	{
		const MusSeqPattern *sp = &mused.song.sequence[channel][0];
		const int x = (channel - mused.sequence_horiz_position) * (w + 1);
		
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
			
			bevel(mused.screen, &pat, mused.slider_bevel->surface, BEV_PATTERN);
			
			adjust_rect(&text, 2);
			
			SDL_SetClipRect(mused.screen, &text);
			
			font_write_args(&mused.largefont, mused.screen, &text, "%02X+%d", sp->pattern, sp->note_offset);
		}
	}
	
	SDL_SetClipRect(mused.screen, &dest);
	
	SDL_Rect loop = { dest.x, (mused.song.loop_point - top) * height / mused.sequenceview_steps + dest.y, dest.w, (mused.song.song_length - mused.song.loop_point) * height / mused.sequenceview_steps };
	bevel(mused.screen, &loop, mused.slider_bevel->surface, BEV_SEQUENCE_LOOP);
	
	if (mused.focus == EDITSEQUENCE)
	{
		SDL_Rect pat = { (mused.current_sequencetrack - mused.sequence_horiz_position) * (w + 1) + dest.x, (mused.current_sequencepos - top) * height / mused.sequenceview_steps + dest.y, w, height };
			
		clip_rect(&pat, &dest);
		adjust_rect(&pat, -2);
		
		set_cursor(&pat);
		
		if (mused.selection.start != mused.selection.end)
		{
			if (mused.selection.start <= bottom && mused.selection.end >= top)
			{
				SDL_Rect selection = { dest.x + (w + 1) * (mused.current_sequencetrack - mused.sequence_horiz_position), 
					dest.y + height * (mused.selection.start - mused.sequence_position) / mused.sequenceview_steps, w, height * (mused.selection.end - mused.selection.start) / mused.sequenceview_steps};
					
				adjust_rect(&selection, -3);
				bevel(mused.screen, &selection, mused.slider_bevel->surface, BEV_SELECTION);
			}
		}
	}
	
	SDL_SetClipRect(mused.screen, NULL);
	
	SDL_Rect scrollbar = { _dest->x, _dest->y + _dest->h - SCROLLBAR, _dest->w, SCROLLBAR };
	
	slider(dest_surface, &scrollbar, event, &mused.sequence_horiz_slider_param); 
}
