#include "nostalgy.h"

void nos_decrunch(GfxDomain *domain)
{
	static const Uint32 palette[] = 
	{
		0x000000,
		0xFFFFFF,
		0x68372B,
		0x70A4B2,
		0x6F3D86,
		0x588D43,
		0x352879,
		0xB8C76F,
		0x6F4F25,
		0x433900,
		0x9A6759,
		0x444444,
		0x6C6C6C,
		0x9AD284,
		0x6C5EB5,
		0x959595 
	};

	for (int i = 0 ; i < 60 ; ++i)
	{
		for (int y = 0 ; y < domain->screen_h ; )
		{
			int h = rand() & 15;
			{
				SDL_Rect line = {0, y, domain->screen_w, h};
				SDL_FillRect(gfx_domain_get_surface(domain), &line, palette[rand() & 15]);
			}
			
			y += h;
		}
		gfx_domain_flip(domain);
		SDL_Delay(15);
	}
}
