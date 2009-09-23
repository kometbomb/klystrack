/*
Copyright (c) 2009 Tero Lindeman (kometbomb)

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
