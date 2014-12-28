#include "timer.h"
#include "gui/bevel.h"
#include "mused.h"
#include "mybevdefs.h"

void timer_view(GfxDomain *dest_surface, const SDL_Rect *dest, const SDL_Event *event, void *param)
{
	//bevel(mused.screen, dest, mused.slider_bevel->surface, BEV_THIN_FRAME);
	
	SDL_Rect field;
	copy_rect(&field, dest);
	adjust_rect(&field, 2);
	
	if (mused.flags & SONG_PLAYING)
	{
		Uint64 t = (mused.play_start_at + mused.time_played) / 1000;
		font_write_args(&mused.smallfont, dest_surface, &field, "%02d:%02d", (int)(t / 60), (int)(t % 60));
	}
	else
		font_write(&mused.smallfont, dest_surface, &field, "00:00");
}
