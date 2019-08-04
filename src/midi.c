#include "midi.h"

#ifdef MIDI

#include "mused.h"
#include "mymsg.h"

#define MAX_MIDI_DEVICES 32

extern Menu prefsmenu[];
Menu midi_menu[];

Menu midi_device_menu[MAX_MIDI_DEVICES + 1];
Menu midi_channel_menu[] =
{
	{ 0, midi_menu, "1", NULL, midi_set_channel, 0, 0, 0 },
	{ 0, midi_menu, "2", NULL, midi_set_channel, MAKEPTR(1), 0, 0 },
	{ 0, midi_menu, "3", NULL, midi_set_channel, MAKEPTR(2), 0, 0 },
	{ 0, midi_menu, "4", NULL, midi_set_channel, MAKEPTR(3), 0, 0 },
	{ 0, midi_menu, "5", NULL, midi_set_channel, MAKEPTR(4), 0, 0 },
	{ 0, midi_menu, "6", NULL, midi_set_channel, MAKEPTR(5), 0, 0 },
	{ 0, midi_menu, "7", NULL, midi_set_channel, MAKEPTR(6), 0, 0 },
	{ 0, midi_menu, "8", NULL, midi_set_channel, MAKEPTR(7), 0, 0 },
	{ 0, midi_menu, "9", NULL, midi_set_channel, MAKEPTR(8), 0, 0 },
	{ 0, midi_menu, "10", NULL, midi_set_channel, MAKEPTR(9), 0, 0 },
	{ 0, midi_menu, "11", NULL, midi_set_channel, MAKEPTR(10), 0, 0 },
	{ 0, midi_menu, "12", NULL, midi_set_channel, MAKEPTR(11), 0, 0 },
	{ 0, midi_menu, "13", NULL, midi_set_channel, MAKEPTR(12), 0, 0 },
	{ 0, midi_menu, "14", NULL, midi_set_channel, MAKEPTR(13), 0, 0 },
	{ 0, midi_menu, "15", NULL, midi_set_channel, MAKEPTR(14), 0, 0 },
	{ 0, midi_menu, "16", NULL, midi_set_channel, MAKEPTR(15), 0, 0 },
	{ 0, NULL, NULL }
};

Menu midi_menu[] =
{
	{ 0, prefsmenu, "MIDI sync", NULL, MENU_CHECK, &mused.flags, (void*)MIDI_SYNC, 0 },
	{ 0, prefsmenu, "Device", midi_device_menu, NULL, 0, 0, 0 },
	{ 0, prefsmenu, "Channel", midi_channel_menu, NULL, 0, 0, 0 },
	{ 0, NULL, NULL }
};

static char midi_device_names[MAX_MIDI_DEVICES][100];


static void midi_clock(Uint32 ms)
{
	if (mused.tick_ctr == 0 && mused.midi_start)
	{
		mused.midi_last_clock = ms;
	}
	else if (mused.tick_ctr == 12)
	{
		mused.tick_ctr = 0;
		
		if (ms - mused.midi_last_clock)
			mused.midi_rate = 1000 / ((ms - mused.midi_last_clock) / 12);
		mused.midi_last_clock = ms;

		if (mused.midi_rate)
		{
			enable_callback(true);
			mused.song.song_rate = mused.midi_rate;
		}
	}

	if (mused.midi_start)
	{
		mused.flags |= SONG_PLAYING;
	}
	
	if (mused.mus.flags & MUS_EXT_SYNC)
		mus_ext_sync(&mused.mus);

	mused.midi_start = false;
	++mused.tick_ctr;
}


static void midi_start()
{
	debug("MIDI start");
	mused.midi_start = true;
	mused.tick_ctr = 0;
	mus_set_song(&mused.mus, &mused.song, 0);
	mused.mus.flags |= MUS_EXT_SYNC;
	mused.mus.ext_sync_ticks = 0;
	
	if (mused.midi_rate)
	{
		enable_callback(true);
		mused.song.song_rate = mused.midi_rate;
	}
}


static void midi_stop()
{
	debug("MIDI stop");
	mused.midi_start = false;
	mused.flags &= ~SONG_PLAYING;
	mused.mus.flags &= ~MUS_EXT_SYNC;
	enable_callback(false);
	mus_set_song(&mused.mus, NULL, 0);
}


static void midi_continue()
{
	debug("MIDI continue");
	mused.midi_start = true;
	/*mused.tick_ctr = 0;
	mus_set_song(&mused.mus, &mused.song, mused.mus.song_position);*/
}


static void midi_spp(Uint16 position)
{
	debug("MIDI SPP (%d)", position);
	mus_set_song(&mused.mus, &mused.song, position);
}


void midi_event(SDL_Event *e)
{
	switch (e->type)
	{
		case MSG_CLOCK:
			midi_clock(e->user.code);
			break;

		case MSG_START:
			midi_start();
			break;

		case MSG_CONTINUE:
			midi_continue();
			break;

		case MSG_STOP:
			midi_stop();
			break;

		case MSG_SPP:
			midi_spp(e->user.code);
			break;
	}
}


#ifdef WIN32

#include <windows.h>
#include <mmsystem.h>

static HMIDIIN hMidiIn = 0;


static Uint16 midi_14bit(Uint8 first, Uint8 second)
{
	return ((Uint16)(second & 0x7f) << 7) | ((Uint16)first & 0x7f);
}

static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	if (MIM_DATA == wMsg)
	{
		if ((dwParam1 & 0xF0) != 0xf0)
		{
			Uint8 channel = dwParam1 & 0xf;
			
			if (channel == mused.midi_channel)
			{
				Uint8 command = dwParam1 & 0xf0;
				
				switch (command)
				{
					case 0x90:
					{
						SDL_Event e;
						e.type = MSG_NOTEON;
						e.user.code = (dwParam1 >> 8) & 0xff;
						e.user.data1 = MAKEPTR((dwParam1 >> 16) & 0xff);
						SDL_PushEvent(&e);
					}
					break;
						
					case 0x80:
					{
						SDL_Event e;
						e.type = MSG_NOTEOFF;
						e.user.code = (dwParam1 >> 8) & 0xff;
						e.user.data1 = MAKEPTR((dwParam1 >> 16) & 0xff);
						SDL_PushEvent(&e);
					}
					break;
					
					case 0xC0:
					{
						SDL_Event e;
						e.type = MSG_PROGRAMCHANGE;
						e.user.code = (dwParam1 >> 8) & 0xff;
						SDL_PushEvent(&e);
					}
					break;
				}
			}
		}
		else
		{
			if (MIDI_SYNC & mused.flags)
			{
				switch ((dwParam1 & 0xFF))
				{
					case 0xF8:
						midi_clock(dwParam2);
						break;
				
					case 0xFA:
					{
						midi_start();
					}
					break;
					
					case 0xFB:
					{
						midi_continue();
					}
					break;
					
					case 0xFC:
					{
						midi_stop();
					}
					break;
					
					case 0xF2:
					{
						midi_spp(midi_14bit((dwParam1 >> 8) & 0xff, (dwParam1 >> 16) & 0xff));
					}
					break;
				}
			}
		}
	}
}


void midi_set_device(void *dev, void *unused1, void *unused2)
{
	midi_deinit();
	
	mused.midi_device = my_min(midiInGetNumDevs() - 1, CASTPTR(int, dev));
	
	MMRESULT err = midiInOpen(&hMidiIn, mused.midi_device, (DWORD_PTR)MidiInProc, 0, CALLBACK_FUNCTION);
	
	if (MMSYSERR_NOERROR != err)
	{
		warning("midiInOpen returned %d", err);
	}
	else
	{
		midiInStart(hMidiIn);
		
		for (int i = 0, p = 0 ; p < MAX_MIDI_DEVICES && i < midiInGetNumDevs() ; ++i, ++p)
		{
			if (i == mused.midi_device) 
				midi_device_menu[p].flags |= MENU_BULLET;
			else
				midi_device_menu[p].flags &= ~MENU_BULLET;
		}
	}	
}


void midi_set_channel(void *chn, void *unused1, void *unused2)
{
	mused.midi_channel = my_min(CASTPTR(int, chn), 15);
	
	for (int i = 0 ; i < 16 ; ++i)
		if (i == mused.midi_channel) 
			midi_channel_menu[i].flags |= MENU_BULLET;
		else
			midi_channel_menu[i].flags &= ~MENU_BULLET;
}


void midi_init() 
{
	debug("MIDI initializing");
	
	memset(midi_device_menu, 0, sizeof(midi_device_menu));
	
	midi_set_channel(MAKEPTR(mused.midi_channel), 0, 0);
	
	if (midiInGetNumDevs() == 0) 
	{
		midi_device_menu[0].parent = midi_menu;
		debug("No MIDI devices detected");
		return;
	}

	for (int i = 0, p = 0 ; p < MAX_MIDI_DEVICES && i < midiInGetNumDevs() ; ++i)
	{
		MIDIINCAPS caps;
		
		if (MMSYSERR_NOERROR == midiInGetDevCaps(i, &caps, sizeof(caps)))
		{
			midi_device_menu[p].parent = midi_menu;
			strncpy(midi_device_names[p], caps.szPname, sizeof(midi_device_names[p]));
			midi_device_menu[p].text = midi_device_names[p];
			midi_device_menu[p].action = midi_set_device;
			midi_device_menu[p].p1 = MAKEPTR(i);
			++p;
		}
	}
	
	midi_set_device(MAKEPTR(mused.midi_device), 0, 0);
}


void midi_deinit() 
{
	if (hMidiIn)
	{
		midiInClose(hMidiIn);
	}
	
	hMidiIn = 0;
}

#elif defined(__linux__)

#include <alsa/asoundlib.h>

static snd_seq_t *seq_handle = NULL;
static int in_port = -1;
static int src_client = -1;
static int src_port = -1;

static inline int chk(int err, const char *operation)
{
	if (err < 0)
	{
		warning("%s failed: %s", operation, snd_strerror(err));
	}
	return err;
}

static int midi_thread(void *data)
{
	snd_seq_event_t *ev = NULL;
	for ( ; ; )
	{
		snd_seq_event_input(seq_handle, &ev);

		if (ev->data.note.channel == mused.midi_channel)
		{
			switch (ev->type)
			{
				case SND_SEQ_EVENT_NOTEON:
				case SND_SEQ_EVENT_NOTEOFF:
				{
					SDL_Event e;
					e.type = ev->type == SND_SEQ_EVENT_NOTEON ? MSG_NOTEON : MSG_NOTEOFF;
					e.user.code = ev->data.note.note;
					e.user.data1 = MAKEPTR(ev->data.note.velocity);
					SDL_PushEvent(&e);
					break;
				}

				case SND_SEQ_EVENT_PGMCHANGE:
				{
					SDL_Event e;
					e.type = MSG_PROGRAMCHANGE;
					e.user.code = ev->data.control.value;
					SDL_PushEvent(&e);
					break;
				}
			}
		}
		else if (MIDI_SYNC & mused.flags)
		{
			switch (ev->type)
			{
				case SND_SEQ_EVENT_CLOCK:
				{
					SDL_Event e;
					e.type = MSG_START;
					e.user.code = ev->time.tick;
					SDL_PushEvent(&e);
					break;
				}

				case SND_SEQ_EVENT_START:
				{
					SDL_Event e;
					e.type = MSG_START;
					SDL_PushEvent(&e);
					break;
				}

				case SND_SEQ_EVENT_CONTINUE:
				{
					SDL_Event e;
					e.type = MSG_CONTINUE;
					SDL_PushEvent(&e);
					break;
				}

				case SND_SEQ_EVENT_STOP:
				{
					SDL_Event e;
					e.type = MSG_STOP;
					SDL_PushEvent(&e);
					break;
				}

				case SND_SEQ_EVENT_SONGPOS:
				{
					SDL_Event e;
					e.type = MSG_SPP;
					e.user.code = ev->data.control.value;
					SDL_PushEvent(&e);
					break;
				}
			}
		}
	}
	return 0;
}

void midi_set_device(void *dev, void *client, void *port)
{
	mused.midi_device = my_min(CASTPTR(int, dev), MAX_MIDI_DEVICES - 1);

	if (src_client != -1 && src_port != -1)
	{
		chk(snd_seq_disconnect_from(seq_handle, in_port, src_client, src_port),
				"snd_seq_disconnect_from");
	}
	src_client = CASTPTR(int, client);
	src_port = CASTPTR(int, port);
	int err = snd_seq_connect_from(seq_handle, in_port, src_client, src_port);

	if (chk(err, "snd_seq_connect_from") == 0)
	{
		for (int p = 0 ; p < MAX_MIDI_DEVICES ; ++p)
		{
			if (midi_device_menu[p].parent == NULL)
				break;
			if (p == mused.midi_device)
				midi_device_menu[p].flags |= MENU_BULLET;
			else
				midi_device_menu[p].flags &= ~MENU_BULLET;
		}
	}
}

void midi_set_channel(void *chn, void *unused1, void *unused2)
{
	mused.midi_channel = my_min(CASTPTR(int, chn), 15);

	for (int i = 0 ; i < 16 ; ++i)
		if (i == mused.midi_channel)
			midi_channel_menu[i].flags |= MENU_BULLET;
		else
			midi_channel_menu[i].flags &= ~MENU_BULLET;
}

void midi_init(Uint32 uDeviceID)
{
	debug("MIDI initializing");

	memset(midi_device_menu, 0, sizeof(midi_device_menu));

	midi_set_channel(MAKEPTR(mused.midi_channel), 0, 0);

	int err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
	if (chk(err, "snd_seq_open") < 0)
		return;

	chk(snd_seq_set_client_name(seq_handle, "klystrack"),
			"snd_seq_set_client_name");

	in_port = chk(snd_seq_create_simple_port(seq_handle, "midi in",
					 SND_SEQ_PORT_CAP_WRITE |
					 SND_SEQ_PORT_CAP_SUBS_WRITE,
					 SND_SEQ_PORT_TYPE_MIDI_GENERIC |
					 SND_SEQ_PORT_TYPE_APPLICATION), "snd_seq_create_simple_port");

	if (in_port < 0)
		return;

	SDL_Thread *thread = SDL_CreateThread(midi_thread, "ALSA MIDI", NULL);
	if (thread == NULL)
	{
		warning("SDL_CreateThread failed: %s", SDL_GetError());
		return;
	}
	SDL_DetachThread(thread);

	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;

	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);

	int p = 0;
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq_handle, cinfo) >= 0)
	{
		int client = snd_seq_client_info_get_client(cinfo);

		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
			if ((snd_seq_port_info_get_capability(pinfo)
			     & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
			    != (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ))
				continue;

			int port = snd_seq_port_info_get_port(pinfo);
			const char *name = snd_seq_port_info_get_name(pinfo);

			midi_device_menu[p].parent = midi_menu;
			strncpy(midi_device_names[p], name, sizeof(midi_device_names[p]));
			midi_device_menu[p].text = midi_device_names[p];
			midi_device_menu[p].action = midi_set_device;
			midi_device_menu[p].p1 = MAKEPTR(p);
			midi_device_menu[p].p2 = MAKEPTR(client);
			midi_device_menu[p].p3 = MAKEPTR(port);

			if (p == mused.midi_device)
				midi_set_device(MAKEPTR(p), MAKEPTR(client), MAKEPTR(port));
			if (++p >= MAX_MIDI_DEVICES)
				goto done;
		}
	}

done:
	if (p == 0)
		midi_device_menu[0].parent = midi_menu;
	else if (mused.midi_device >= p)
		midi_set_device(MAKEPTR(0), MAKEPTR(midi_device_menu[0].p2), MAKEPTR(midi_device_menu[0].p3));
}

void midi_deinit()
{
	if (seq_handle != NULL)
	{
		chk(snd_seq_close(seq_handle), "snd_seq_close");
	}
	seq_handle = NULL;
}

#else

void midi_set_channel(void *chn, void *unused1, void *unused2) {}
void midi_init(Uint32 uDeviceID) {}
void midi_deinit() {}

#endif
#endif // MIDI
