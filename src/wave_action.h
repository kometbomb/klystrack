#ifndef __WAVE_ACTION_H
#define __WAVE_ACTION_H

void wavetable_drop_lowest_bit(void *unused1, void *unused2, void *unused3);
void wavetable_halve_samplerate(void *unused1, void *unused2, void *unused3);
void wavetable_normalize(void *vol, void *unused2, void *unused3);
void wavetable_cut_tail(void *unused1, void *unused2, void *unused3);
void wavetable_cut_head(void *unused1, void *unused2, void *unused3);
void wavetable_chord(void *transpose, void *unused2, void *unused3);
void wavetable_create_one_cycle(void *unused1, void *unused2, void *unused3);
void wavetable_randomize_and_create_one_cycle(void *unused1, void *unused2, void *unused3);
void wavegen_randomize(void *unused1, void *unused2, void *unused3);
void wavegen_preset(void *_preset, void *_settings, void *unused3);
void wavetable_draw(float x, float y, float w);
void wavetable_amp(void *amp, void *unused2, void *unused3);
void wavetable_remove_dc(void *unused1, void *unused2, void *unused3);
void wavetable_filter(void *filter_type, void *unused2, void *unused3);

#endif
