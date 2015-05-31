#include "hubbard.h"
#include "edit.h"
#include "mused.h"
#include "event.h"
#include "SDL_endian.h"
#include <math.h>
#include "snd/freqs.h"
#include <string.h>
#include "hubdialog.h"

extern Mused mused;

#define ANY 0xFFFF

typedef struct
{
	char magicID[4];
	unsigned short int version;
	unsigned short int dataOffset;
	unsigned short int loadAddress;
	unsigned short int initAddress;
	unsigned short int playAddress;
	unsigned short int songs;
	unsigned short int startSong;
	unsigned int speed;
	char title[32];
	char author[32];
	char released[32];
	unsigned short int flags;
	unsigned char startPage;
	unsigned char pageLength;
	unsigned short int reserved;
	char data[65536];
} sid_t;

static int match_byte(unsigned char haystack, unsigned short int byte)
{
	return (byte == ANY || haystack == byte);
}


static int find_bytes(const unsigned char *haystack, int haystack_size, const unsigned short int *needle, int needle_size)
{
	for (int i = 0 ; i < haystack_size - needle_size ; ++i)
	{
		for (int si = 0 ; si < needle_size ; ++si) 
		{
			if (!match_byte(haystack[i + si], needle[si]))
				goto next_haystack;
		}
		
		return i;
		
next_haystack:;
	}
	
	return -1;
}


void convert_pattern(hubbard_t *tune, int idx)
{
	tune->pattern[idx].length = 0;
	
	unsigned short int addr = tune->data[tune->addr.patternptrhi + idx] << 8 | tune->data[tune->addr.patternptrlo + idx];
	
	if (!addr)
		return;
	
	unsigned char *ptr = &tune->data[addr];
	
	while (*ptr != 0xff)
	{
		int len = tune->pattern[idx].length;
		
		if ((*ptr & 64))
		{
			if (len > 0)
			{
				tune->pattern[idx].note[len - 1].length += (*ptr & 31) + 1;
			}
			else
			{
				tune->pattern[idx].note[len].note = 0xff;
				tune->pattern[idx].note[len].length = *ptr & 31;
				
				if (tune->pattern[idx].length < 256-1)
					tune->pattern[idx].length++;
			}
		}
		else
		{
			tune->pattern[idx].note[len].length = *ptr & 31;
			tune->pattern[idx].note[len].legato = !!(*ptr & 32);
		
			tune->pattern[idx].note[len].instrument = 0xff;
			tune->pattern[idx].note[len].portamento = 0;
		
			if (*ptr & 128)
			{
				++ptr;
				
				char byte = *ptr;
				
				if (byte < 0)
				{
					if (byte & 1)
						tune->pattern[idx].note[len].portamento = -(byte & 0x07e); 
					else
						tune->pattern[idx].note[len].portamento = (byte & 0x07e); 
				}
				else
				{
					tune->pattern[idx].note[len].instrument = byte; 
					
					if (tune->n_instruments < byte + 1)
						tune->n_instruments = byte + 1;
				}
			}
			
			++ptr;
			
			tune->pattern[idx].note[len].note = *ptr;
			
			if (tune->pattern[idx].length < 256-1)
				tune->pattern[idx].length++;
		}
		
		++ptr;
	}
}


void convert_instrument(hubbard_t *tune, int idx)
{
	unsigned char *ptr = &tune->data[tune->addr.instruments + idx * 8];
		
	tune->instrument[idx].pulse_width = ptr[0] | ptr[1] << 8;
	tune->instrument[idx].waveform = ptr[2] >> 4;
	tune->instrument[idx].a = ptr[3] >> 4;
	tune->instrument[idx].d = ptr[3] & 15;
	tune->instrument[idx].s = ptr[4] >> 4;
	tune->instrument[idx].r = ptr[4] & 15;
	tune->instrument[idx].vibrato_depth = ptr[5];
	tune->instrument[idx].pwm = ptr[6];
	tune->instrument[idx].fx = ptr[7];
}


void convert_track(hubbard_t *tune, int song, int track)
{
	unsigned char *ptr = &tune->data[tune->addr.songs[song][track]];
	
	tune->track[track].length = 0;
	
	while (*ptr < 0xfe)
	{
		tune->track[track].pattern[tune->track[track].length] = *ptr;
		
		if (tune->n_patterns < *ptr + 1)
			tune->n_patterns = *ptr + 1;
		
		++ptr;
		
		if (tune->track[track].length < 255)
			++tune->track[track].length;
	}
}


void determine_version(hubbard_t *tune)
{
	const unsigned short int vibcode[] = { 0x29, 0x07, 0xC9, 0x04, 0x90, 0x02, 0x49, 0x07 };
	
	if (find_bytes(tune->data, 65536, vibcode, sizeof(vibcode) / sizeof(vibcode[0])) != 0xffff)
	{
		tune->vib_type = 0;
	}
	else
	{
		tune->vib_type = 1;
	}
}


void convert_hub(hubbard_t *tune, int subsong)
{
	for (int song = 0 ; song < tune->n_subsongs ; ++song)
	for (int i = 0 ; i < tune->n_tracks ; ++i)
	{
		tune->addr.songs[song][i] = tune->data[tune->addr.songtab + i + tune->n_tracks + tune->n_tracks * song * 2] << 8 | tune->data[tune->addr.songtab + i + tune->n_tracks * song * 2];
	}
	
	determine_version(tune);
	
	for (int i = 0 ; i < tune->n_tracks ; ++i)
		convert_track(tune, subsong, i);
	
	for (int i = 0 ; i < tune->n_patterns ; ++i)
		convert_pattern(tune, i);
	
	for (int i = 0 ; i < tune->n_instruments ; ++i)
		convert_instrument(tune, i);
}


void init_addr(hubbard_t *tune)
{
	const unsigned short int pattern_ptr_lo[] = { 0xa8, 0xb9, ANY, ANY, 0x85, ANY, 0xB9, ANY, ANY, 0x85, ANY, 0xa9, 0x00 };
	const unsigned short int inst_tab[] = { 0xBD, ANY, ANY, 0x99, 0x02, 0xd4 };
	
	int pattablo = find_bytes(tune->data, 65536, pattern_ptr_lo, sizeof(pattern_ptr_lo) / sizeof(pattern_ptr_lo[0]));
	int insttab = find_bytes(tune->data, 65536, inst_tab, sizeof(inst_tab) / sizeof(inst_tab[0]));
	int songtab;
	
	tune->addr.songtab = 0xffff;
	
	const unsigned short int songtab_pattern[] = { 0xaa, 0xbd, ANY, ANY, 0x99, ANY, ANY, 0xe8, 0xc8, 0xc0, 0x06 };
	
	songtab = find_bytes(tune->data, 65536, songtab_pattern, sizeof(songtab_pattern) / sizeof(songtab_pattern[0]));
	tune->addr.songtab = songtab >= 0 ? SDL_SwapLE16(*(Uint16*)&tune->data[songtab + 2]) : 0xffff;
		
	if (tune->addr.songtab == 0xffff)
	{
		const unsigned short int songtab_pattern_single[] = { 0xD0, ANY, 0xbd, ANY, ANY, 0x85, ANY, 0xbd, ANY, ANY, 0x85, ANY, 0xDE };
		
		songtab = find_bytes(tune->data, 65536, songtab_pattern_single, sizeof(songtab_pattern_single) / sizeof(songtab_pattern_single[0]));
		tune->addr.songtab = songtab >= 0 ? SDL_SwapLE16(*(Uint16*)&tune->data[songtab + 3]) : 0xffff;
	}
	
	tune->addr.patternptrlo = pattablo >= 0 ? SDL_SwapLE16(*(Uint16*)&tune->data[pattablo + 2]) : 0xffff;
	tune->addr.patternptrhi = pattablo >= 0 ? SDL_SwapLE16(*(Uint16*)&tune->data[pattablo + 7]) : 0xffff;
	tune->addr.instruments = insttab >= 0 ? SDL_SwapLE16(*(Uint16*)&tune->data[insttab + 1]) : 0xffff;
	
	debug("SONGS: %x", tune->addr.songtab);
	debug("PTNLO: %x", tune->addr.patternptrlo);
	debug("PTNHI: %x", tune->addr.patternptrhi);
	debug("INSTR: %x", tune->addr.instruments);
}

unsigned short int load16(FILE *f)
{
	unsigned short int tmp16 = 0;
	
	fread(&tmp16, 1, sizeof(tmp16), f);
	
	return SDL_SwapBE16(tmp16);
}

sid_t * load_sid(FILE *f)
{
	sid_t *sid = malloc(sizeof(sid_t));
	
	fread(&sid->magicID, 1, sizeof(sid->magicID), f);
	
	sid->version = load16(f);
	sid->dataOffset = load16(f);
	sid->loadAddress = load16(f);
	sid->initAddress = load16(f);
	sid->playAddress = load16(f);
	sid->songs = load16(f);
	sid->startSong = load16(f);
	
	fread(&sid->speed, 1, sizeof(sid->speed), f);
	
	fread(&sid->title, 1, sizeof(sid->title), f);
	fread(&sid->author, 1, sizeof(sid->author), f);
	fread(&sid->released, 1, sizeof(sid->released), f);
	
	if (sid->version == 2)
	{
		fread(&sid->flags, 1, sizeof(sid->flags), f);
		fread(&sid->startPage, 1, sizeof(sid->startPage), f);
		fread(&sid->pageLength, 1, sizeof(sid->pageLength), f);
		fread(&sid->reserved, 1, sizeof(sid->reserved), f);
	}
			
	fread(&sid->data[0], 1, 65536, f);
	
	return sid;
}


hubbard_t * load_hubbard(sid_t *sid)
{
	hubbard_t * hub = malloc(sizeof(hubbard_t));
	
	memset(hub->data, 0, 65536);
	
	if (sid->loadAddress == 0)
	{
		memcpy(&hub->data[*(unsigned short int*)&sid->data[0]], &sid->data[2], 65536 - *(unsigned short int*)&sid->data[0]);
		hub->org = *(unsigned short int*)&sid->data[0];
	}
	else
	{
		memcpy(&hub->data[sid->loadAddress], &sid->data[0], 65536 - sid->loadAddress);
		hub->org = sid->loadAddress;
	}
	
	hub->n_patterns = 0;
	hub->n_subsongs = sid->songs;
	hub->n_instruments = 0;
	hub->n_tracks = 3;
	
	init_addr(hub);
	
	return hub;
}


int import_hubbard(FILE *f)
{
	sid_t *sid = load_sid(f);
	
	if (!sid)
		return 0;
	
	hubbard_t *hub = load_hubbard(sid);
	
	if (!hub)
	{
		free(sid);
		return 0;
	}
	
	int subtune = hub_view(hub);
	
	if (subtune < 0)
	{
		free(sid);
		free(hub);
		return 0;
	}
	
	debug("subtune = %d", subtune);
	
	convert_hub(hub, subtune);
	
	strncpy(mused.song.title, sid->title, 32);
	
	for (int i = 0 ; i < hub->n_patterns ; ++i)
	{
		if (!hub->pattern[i].length)
			continue;
		
		int total_length = 0;
		
		for (int s = 0 ; s < hub->pattern[i].length ; ++s)
			total_length += 1 + hub->pattern[i].note[s].length;
		
		resize_pattern(&mused.song.pattern[i], total_length);
		
		for (int s = 0 ; s < total_length ; ++s)
		{
			zero_step(&mused.song.pattern[i].step[s]);
		}
		
		int pos = 0;
		
		for (int s = 0 ; s < hub->pattern[i].length ; ++s)
		{
			mused.song.pattern[i].step[pos].note = hub->pattern[i].note[s].note;
			
			if (hub->pattern[i].note[s].instrument != 0xff)
				mused.song.pattern[i].step[pos].instrument = hub->pattern[i].note[s].instrument;
			
			if (hub->pattern[i].note[s].legato)
				mused.song.pattern[i].step[pos].ctrl |= MUS_CTRL_LEGATO;
			
			if (hub->pattern[i].note[s].portamento != 0)
			{
				for (int c = 0 ; c <= hub->pattern[i].note[s].length ; ++c)
				{
					if (hub->pattern[i].note[s].portamento < 0)
						mused.song.pattern[i].step[pos].command = MUS_FX_PORTA_DN_LOG | (((unsigned char)hub->pattern[i].note[s].portamento) & 0x7f);
					else
						mused.song.pattern[i].step[pos].command = MUS_FX_PORTA_UP_LOG | hub->pattern[i].note[s].portamento;
					
					pos++;
				}
			}
			else
				pos += 1 + hub->pattern[i].note[s].length;
		}
	}
	
	int max_pos = 0;
	
	for (int track = 0 ; track < hub->n_tracks ; ++track)
	{
		int pos = 0;
		
		for (int i = 0 ; i < hub->track[track].length ; ++i)
		{
			int total_length = 0;
			for (int s = 0 ; s < hub->pattern[hub->track[track].pattern[i]].length ; ++s)
				total_length += 1 + hub->pattern[hub->track[track].pattern[i]].note[s].length;
			
			pos += total_length;
		}
		
		max_pos = my_max(max_pos, pos);
	}
	
	for (int track = 0 ; track < hub->n_tracks ; ++track)
	{
		int pos = 0;
		
		for ( ; pos < max_pos ; )
		{
			for (int i = 0 ; i < hub->track[track].length ; ++i)
			{
				int total_length = 0;
				for (int s = 0 ; s < hub->pattern[hub->track[track].pattern[i]].length ; ++s)
					total_length += 1 + hub->pattern[hub->track[track].pattern[i]].note[s].length;
			
				add_sequence(track, pos, hub->track[track].pattern[i], 0);
				
				pos += total_length;
			}
		}
	}
	
	mused.song.num_channels = hub->n_tracks;
	mused.song.song_speed = 2;
	mused.song.song_speed2 = 2;
	mused.song.song_length = max_pos;
	
	for (int i = 0 ; i < hub->n_instruments ; ++i)
	{
		int waveforms = 0;
		mused.song.instrument[i].flags &= ~MUS_INST_DRUM;
		mused.song.instrument[i].flags |= MUS_INST_INVERT_VIBRATO_BIT;
		mused.song.instrument[i].cydflags &= ~WAVEFORMS;
		mused.song.instrument[i].cydflags |= CYD_CHN_ENABLE_KEY_SYNC;
		
		if (hub->instrument[i].waveform & 1)
			waveforms |= CYD_CHN_ENABLE_TRIANGLE;
		
		if (hub->instrument[i].waveform & 2)
			waveforms |= CYD_CHN_ENABLE_SAW;
		
		if (hub->instrument[i].waveform & 4)
			waveforms |= CYD_CHN_ENABLE_PULSE;
		
		if (hub->instrument[i].waveform & 8)
		{
			waveforms |= CYD_CHN_ENABLE_NOISE;
			mused.song.instrument[i].base_note = MIDDLE_C + 12;
		}
		
		mused.song.instrument[i].cydflags |= waveforms;
		
		int pw = hub->instrument[i].pulse_width;
		
		if (pw >= 0x800)
			pw = 0x800 - (pw - 0x800);
		
		mused.song.instrument[i].pw = pw;
		
		const int attack_tab[] = {1, 2, 2, 3, 3, 4, 5, 5, 6, 9, 13, 16, 18, 32, 41, 52};
		const int decay_tab[] = {1, 3, 4, 5, 6, 7, 8, 9, 10, 16, 22, 28, 32, 55, 63, 63};
		const int sustain_tab[] = {0, 3, 6, 8, 0xa, 0xc, 0x10, 0x14, 0x1c, 0x1d, 0x1e, 0x1f, 0x1f};
		
		mused.song.instrument[i].adsr.a = attack_tab[hub->instrument[i].a];
		mused.song.instrument[i].adsr.d = decay_tab[hub->instrument[i].d];
		mused.song.instrument[i].adsr.s = sustain_tab[hub->instrument[i].s];
		mused.song.instrument[i].adsr.r = decay_tab[hub->instrument[i].r];
		mused.song.instrument[i].prog_period = 1;
		mused.song.instrument[i].pwm_depth = 0x30;
		mused.song.instrument[i].pwm_speed = hub->instrument[i].pwm / 8;
		mused.song.instrument[i].vibrato_speed = 0x20;
		mused.song.instrument[i].vib_delay = 0;
		mused.song.instrument[i].vibrato_depth = hub->vib_type == 1 ? hub->instrument[i].vibrato_depth * 0x0c : hub->instrument[i].vibrato_depth;
		
		if (hub->instrument[i].fx & 1)
		{
			mused.song.instrument[i].flags |= MUS_INST_DRUM;
			mused.song.instrument[i].program[0] = 0x0208;
			mused.song.instrument[i].program[1] = 0xFF00;
		}
		
		if (hub->instrument[i].fx & 2)
		{
			mused.song.instrument[i].program[0] = 0x0204;
			mused.song.instrument[i].program[1] = 0xFF00;
		}
		
		if (hub->instrument[i].fx & 4)
		{
			mused.song.instrument[i].program[0] = 0x0000;
			mused.song.instrument[i].program[1] = 0x000C;
			mused.song.instrument[i].program[2] = 0xFF00;
		}
	}
	
	free(hub);
	free(sid);
	
	return 1;
}
