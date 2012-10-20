#ifndef __WAVE_ACTION_H
#define __WAVE_ACTION_H

void wavetable_drop_lowest_bit(void *unused1, void *unused2, void *unused3);
void wavetable_halve_samplerate(void *unused1, void *unused2, void *unused3);
void wavetable_normalize(void *unused1, void *unused2, void *unused3);
void wavetable_cut_tail(void *unused1, void *unused2, void *unused3);
void wavetable_cut_head(void *unused1, void *unused2, void *unused3);
void wavetable_chord(void *transpose, void *unused2, void *unused3);

#endif
