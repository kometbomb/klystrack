#include "action.h"
#include "mused.h"

extern Mused mused;

void select_sequence_position(void *channel, void *position, void* unused)
{
	mused.current_sequencepos = (int)position;
	if ((int)channel != -1)
		mused.current_sequencetrack = (int)channel;
}


void select_pattern_param(void *id, void *position, void *pattern)
{
	mused.current_pattern = (int)pattern;
	mused.current_patternstep = (int)position;
	mused.current_patternx = (int)id;
}


void select_instrument(void *idx, void *unused1, void *unused2)
{
	mused.current_instrument = (int)idx;
}
