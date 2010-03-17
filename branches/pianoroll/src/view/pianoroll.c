#include "pianoroll.h"
#include "../mused.h"
#include "gui/view.h"
#include "snd/freqs.h"

extern Mused mused;

void pianoroll_view(SDL_Surface *screen, const SDL_Rect *area, const SDL_Event *event, void *param)
{
	SDL_Rect r;
	copy_rect(&r, area);
	SDL_FillRect(screen, &r, 0);
	
	static const Uint32 pal[6] = { 0xff0000, 0x00ff00, 0x0000ff, 0xffff00, 0xff00c0, 0x808080 };

	for (int c = 0 ; c < mused.song.num_channels ; ++c)
	{
		struct { SDL_Rect area; Uint8 inst; } note;
	
		for (int s = 0; s < mused.song.num_sequences[c] ; ++s)
		{	
			const MusPattern *pat = &mused.song.pattern[mused.song.sequence[c][s].pattern];
			int inst = MUS_NOTE_NO_INSTRUMENT;
			for (int ps = 0; ps < pat->num_steps ; ++ps)
			{
				if (pat->step[ps].instrument != MUS_NOTE_NO_INSTRUMENT) inst = pat->step[ps].instrument;
			
				if (pat->step[ps].note < 0xf0 && inst != MUS_NOTE_NO_INSTRUMENT)
				{
					if (note.area.w > 0)
						SDL_FillRect(screen, &note.area, pal[note.inst % 6]);
						
					note.area.x = (mused.song.sequence[c][s].position + ps) * 2 - mused.pianoroll_x_position * 2;
					note.area.y = (mused.song.sequence[c][s].note_offset + pat->step[ps].note + mused.song.instrument[inst].base_note) * 2 - mused.pianoroll_y_position * 2;
					note.area.w = 2;
					note.area.h = 2;
					note.inst = inst;
				}
				else if (pat->step[ps].note == MUS_NOTE_RELEASE)
				{
					if (note.area.w > 0)
						SDL_FillRect(screen, &note.area, pal[note.inst % 6]);
						
					note.area.w = 0;
				}
				else if (note.area.w > 0)
					note.area.w += 2;
			}
		}
		
		if (note.area.w > 0)
			SDL_FillRect(screen, &note.area, pal[note.inst % 6]);
	}
	
	slider_set_params(&mused.pianoroll_x_param, 0, my_max(0, (int)mused.song.song_length - 1), mused.pianoroll_x_position, mused.pianoroll_x_position + area->w / 2 - 1, &mused.pianoroll_x_position, 1, SLIDER_HORIZONTAL, mused.slider_bevel);
	slider_set_params(&mused.pianoroll_y_param, 0, area->h / 2, mused.pianoroll_y_position, mused.pianoroll_y_position + area->h / 2 - 1, &mused.pianoroll_y_position, 1, SLIDER_VERTICAL, mused.slider_bevel);
	
}