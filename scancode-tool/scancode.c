#include "SDL.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, 0);
		
	printf("Make sure the empty window is focused and hit any key to see info for that key!\n\n");
	
	while (1)
	{
		SDL_Event e;
		
		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				break;
			
			if (e.type == SDL_KEYDOWN)
			{
				printf("scancode = %x\tsym = %x\tmod = %x\n", e.key.keysym.scancode, e.key.keysym.sym, e.key.keysym.mod);
			}
		}
		else
			SDL_Delay(10);
	}
	
	SDL_Quit();
	
	return 0;
}
