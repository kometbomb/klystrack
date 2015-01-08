#include "wavegen.h"
#include "util/rnd.h"
#include <math.h>
#include "macros.h"


float wg_osc(WgOsc *osc, float _phase)
{
	double intpart = 0.0f;
	double phase = pow(modf(_phase * osc->mult + (float)osc->shift / 8, &intpart), osc->exp_c); 
	float output = 0;
	
	switch (osc->osc)
	{
		default:
			
		case WG_OSC_SINE:
			output = sin(phase * M_PI * 2.0f);
			break;
			
		case WG_OSC_SQUARE:
			output = phase >= 0.5f ? -1.0f : 1.0f;
			break;
			
		case WG_OSC_TRIANGLE:
			if (phase < 0.5f)
				output = phase * 4.0f - 1.0f;
			else
				output = 1.0f - (phase - 0.5f) * 4.0f;
			break;
			
		case WG_OSC_SAW:
			output = phase * 2.0f - 1.0f;
			break;
			
		case WG_OSC_NOISE:
			output = rndf() * 2.0f - 1.0f;
	}
	
	if (osc->flags & WG_OSC_FLAG_ABS)
	{
		output = output * 0.5 + 0.5f;
	}
	
	if (osc->flags & WG_OSC_FLAG_NEG)
	{
		output = -output;
	}
	
	return output;
}


float wg_get_sample(WgOsc *chain, int num_oscs, float phase)
{
	float sample = 0;
	WgOpType op = WG_OP_ADD;
	
	for (int i = 0 ; i < num_oscs ; ++i)
	{
		WgOsc *osc = &chain[i];
		float output = wg_osc(osc, phase);
		
		switch (op)
		{
			default:
			case WG_OP_ADD:
				sample += output;
				break;
				
			case WG_OP_MUL:
				sample *= output;
				break;
		}
		
		op = osc->op;
	}
	
	return sample;
}


void wg_init_osc(WgOsc *osc)
{
	osc->exp_c = log(0.5f) / log((float)osc->exp / 100);
}


void wg_gen_waveform(WgOsc *chain, int num_oscs, Sint16 *data, int len)
{
	for (int i = 0 ; i < num_oscs ; ++i)
	{
		wg_init_osc(&chain[i]);
	}

	for (int i = 0 ; i < len ; ++i)
	{
		double s = wg_get_sample(chain, num_oscs, (float)i / len);
		
		if (s > 1.0)
			s = 1.0;
		else if (s < -1.0)
			s = -1.0;
			
		data[i] = 32767 * s;
	}
}
