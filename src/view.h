#ifndef VIEW_H
#define VIEW_H

#include "SDL.h"

typedef struct
{
	const SDL_Rect position;
	
	/*
	This is a combined drawing and mouse event handler.
	*/
	
	void (*handler)(const SDL_Rect *dest, const SDL_Event *event);
} View;


void draw_view(const View* view, const SDL_Event *event);

/* 
"Controls"
*/

void info_line(const SDL_Rect *dest, const SDL_Event *event);
void sequence_view(const SDL_Rect *dest, const SDL_Event *event);
void pattern_view(const SDL_Rect *dest, const SDL_Event *event);
void info_view(const SDL_Rect *dest, const SDL_Event *event);
void instrument_view(const SDL_Rect *dest, const SDL_Event *event);
void instrument_list(const SDL_Rect *dest, const SDL_Event *event);

#endif
