#include "wavegen.h"
#include "util/rnd.h"
#include <math.h>
#include "macros.h"


float wg_osc(WgOsc *osc, float _phase)
{
	double intpart = 0.0f;
	double phase = pow(modf(_phase * osc->mult + (float)osc->shift / 8, &intpart), osc->exp); 
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


float wg_get_sample(WgOsc *chain, float phase)
{
	float sample = 0;
	WgOpType op = WG_OP_EQ;
	
	for (int i = 0 ; i < WG_CHAIN_OSCS ; ++i)
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
		
			case WG_OP_EQ:
				sample = output;
				break;
		}
		
		op = osc->op;
		++osc;
		
		if (op == WG_OP_EQ)
			break;
	}
	
	return sample;
}


void wg_gen_waveform(WgOsc *chain, Sint16 *data, int len)
{
	for (int i = 0 ; i < len ; ++i)
		data[i] = my_min(32767, my_max(-32768, 32767 * wg_get_sample(chain, (float)i / len)));
}
