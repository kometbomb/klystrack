#include "midi.h"

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
	{ 0, prefsmenu, "Device", midi_device_menu, NULL, 0, 0, 0 },
	{ 0, prefsmenu, "Channel", midi_channel_menu, NULL, 0, 0, 0 },
	{ 0, prefsmenu, "MIDI sync", NULL, MENU_CHECK, &mused.flags, (void*)MIDI_SYNC, 0 },
	{ 0, NULL, NULL }
};

static char midi_device_names[MAX_MIDI_DEVICES][100];

#ifdef MIDI
#ifdef WIN32

#include <windows.h>
#include <mmsystem.h>

static HMIDIIN hMidiIn = 0;

static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
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
			}
		}
	}
	else
	{
		switch ((dwParam1 & 0xFF))
		{
			case 0xF8:
				break;
		
			case 0xFA:
			{
				SDL_Event e;
				e.type = MSG_PLAYSTART;
				SDL_PushEvent(&e);
			}
			break;
			
			case 0xFB:
			{
				SDL_Event e;
				e.type = MSG_PLAYCONTINUE;
				SDL_PushEvent(&e);
			}
			break;
			
			case 0xFC:
			{
				SDL_Event e;
				e.type = MSG_PLAYSTOP;
				SDL_PushEvent(&e);
			}
			break;
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
		warning("No MIDI devices detected");
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
		midiInClose(hMidiIn);
	
	hMidiIn = 0;
}

#else

void midi_init(Uint32 uDeviceID) {}
void midi_deinit() {}

#endif // WIN32
#endif // MIDI
