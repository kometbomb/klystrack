#ifndef ACTION_H
#define ACTION_H

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

void select_sequence_position(void *channel, void *position, void *);
void select_pattern_param(void *id, void *position, void *pattern);
void select_instrument_param(void *idx, void *, void *);
void select_instrument(void *idx, void *relative, void *);
void select_program_step(void *idx, void *, void *);
void change_octave(void *delta, void *, void *);
void change_song_rate(void *delta, void *, void *);
void change_time_signature(void *beat, void *, void *);
void play(void *from_cursor, void*,void*);
void stop(void*,void*,void*);
void change_song_speed(void *speed, void *delta, void *);
void new_song_action(void *, void *, void *);
void save_song_action(void *, void *, void *);
void open_song_action(void *, void *, void *);
void generic_action(void *func, void *, void *);
void quit_action(void *, void *, void *);
void change_mode_action(void *mode, void *, void *);
void enable_channel(void *channel, void *, void *);
void enable_reverb(void *unused1, void *unused2, void *unused3);
void clear_selection(void *, void *, void*);
void cycle_focus(void *views, void *focus, void *mode);
void change_song_length(void *delta, void *, void *);
void change_loop_point(void *delta, void *, void *);
void change_seq_steps(void *delta, void *, void *);
void show_about_box(void *unused1, void *unused2, void *unused3);
void change_channels(void *delta, void *unused1, void *unused2);
void begin_selection_action(void *unused1, void *unused2, void *unused3);
void end_selection_action(void *unused1, void *unused2, void *unused3);
void toggle_pixel_scale(void *a, void*b, void*c);
void change_pixel_scale(void *a, void*b, void*c);
void toggle_fullscreen(void *a, void*b, void*c);
void change_fullscreen(void *a, void*b, void*c);

#endif
