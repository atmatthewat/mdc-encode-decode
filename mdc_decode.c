/*-
 * mdc_decode.c
 *   Decodes a specific format of 1200 BPS MSK data burst
 *   from input audio samples.
 *
 * 4 October 2010 - fixed for 64-bit
 * 5 October 2010 - added four-point method to C version
 * 7 October 2010 - typedefs for easier porting
 * 9 October 2010 - fixed invert case for four-point decoder
 *
 * Author: Matthew Kaufman (matthew@eeph.com)
 *
 * Copyright (c) 2005, 2010  Matthew Kaufman  All rights reserved.
 * 
 *  This file is part of Matthew Kaufman's MDC Encoder/Decoder Library
 *
 *  The MDC Encoder/Decoder Library is free software; you can
 *  redistribute it and/or modify it under the terms of version 2 of
 *  the GNU General Public License as published by the Free Software
 *  Foundation.
 *
 *  If you cannot comply with the terms of this license, contact
 *  the author for alternative license arrangements or do not use
 *  or redistribute this software.
 *
 *  The MDC Encoder/Decoder Library is distributed in the hope
 *  that it will be useful, but WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 *  USA.
 *
 *  or see http://www.gnu.org/copyleft/gpl.html
 *
-*/

#include <stdlib.h>
#include "mdc_decode.h"
#include "mdc_common.c"

mdc_decoder_t * mdc_decoder_new(int sampleRate)
{
	mdc_decoder_t *decoder;
	mdc_int_t i;

	decoder = (mdc_decoder_t *)malloc(sizeof(mdc_decoder_t));
	if(!decoder)
		return (mdc_decoder_t *) 0L;

	decoder->hyst = 3.0/256.0;
	decoder->incr = (1200.0 * TWOPI) / ((mdc_float_t)sampleRate);
	decoder->good = 0;
	decoder->indouble = 0;
	decoder->level = 0;


	for(i=0; i<MDC_ND; i++)
	{
		decoder->th[i] = 0.0 + ( ((mdc_float_t)i) * (TWOPI/(mdc_float_t)MDC_ND));
		decoder->zc[i] = 0;
		decoder->xorb[i] = 0;
		decoder->invert[i] = 0;
		decoder->shstate[i] = -1;
		decoder->shcount[i] = 0;
		decoder->nlstep[i] = i;
	}

	decoder->callback = (mdc_decoder_callback_t)0L;

	return decoder;
}

static void _clearbits(mdc_decoder_t *decoder, mdc_int_t x)
{
	mdc_int_t i;
	for(i=0; i<112; i++)
		decoder->bits[x][i] = 0;
}




static void _procbits(mdc_decoder_t *decoder, int x)
{
	mdc_int_t lbits[112];
	mdc_int_t lbc = 0;
	mdc_int_t i, j, k;
	mdc_u8_t data[14];
	mdc_u16_t ccrc;
	mdc_u16_t rcrc;

	for(i=0; i<16; i++)
	{
		for(j=0; j<7; j++)
		{
			k = (j*16) + i;
			lbits[lbc] = decoder->bits[x][k];
			++lbc;
		}
	}

	for(i=0; i<14; i++)
	{
		data[i] = 0;
		for(j=0; j<8; j++)
		{
			k = (i*8)+j;

			if(lbits[k])
				data[i] |= 1<<j;
		}
	}


	ccrc = _docrc(data, 4);
	rcrc = data[5] << 8 | data[4];

	if(ccrc == rcrc)
	{

		if(decoder->shstate[x] == 2)
		{
			decoder->extra0 = data[0];
			decoder->extra1 = data[1];
			decoder->extra2 = data[2];
			decoder->extra3 = data[3];

			for(k=0; k<MDC_ND; k++)
				decoder->shstate[k] = -1;

			decoder->good = 2;
			decoder->indouble = 0;

		}
		else
		{
			if(!decoder->indouble)
			{
				decoder->good = 1;
				decoder->op = data[0];
				decoder->arg = data[1];
				decoder->unitID = (data[2] << 8) | data[3];
				decoder->crc = (data[4] << 8) | data[5];
	

				switch(data[0])
				{
				/* list of opcode that mean 'double packet' */
				case 0x35:
				case 0x55:
					decoder->good = 0;
					decoder->indouble = 1;
					decoder->shstate[x] = 2;
					decoder->shcount[x] = 0;
					_clearbits(decoder, x);
					break;
				default:
					for(k=0; k<MDC_ND; k++)
						decoder->shstate[k] = -1;	// only in the single-packet case, double keeps rest going
					break;
				}
			}
			else
			{
				// any subsequent good decoder allowed to attempt second half
				decoder->shstate[x] = 2;
				decoder->shcount[x] = 0;
				_clearbits(decoder, x);
			}
		}

	}
	else
	{
#if 0
		printf("bad: ");
		for(i=0; i<14; i++)
			printf("%02x ",data[i]);
		printf("%x\n",ccrc);
#endif

		decoder->shstate[x] = -1;
	}

	if(decoder->good)
	{
		if(decoder->callback)
		{
			(decoder->callback)( (int)decoder->good,
								(unsigned char)decoder->op,
								(unsigned char)decoder->arg,
								(unsigned short)decoder->unitID,
								(unsigned char)decoder->extra0,
								(unsigned char)decoder->extra1,
								(unsigned char)decoder->extra2,
								(unsigned char)decoder->extra3,
								decoder->callback_context);
			decoder->good = 0;

		}
	}
}


static int _onebits(mdc_u32_t n)
{
	int i=0;
	while(n)
	{
		++i;
		n &= (n-1);
	}
	return i;
}

static void _shiftin(mdc_decoder_t *decoder, int x)
{
	int bit = decoder->xorb[x];
	int gcount;

	switch(decoder->shstate[x])
	{
	case -1:
		decoder->synchigh[x] = 0;
		decoder->synclow[x] = 0;
		decoder->shstate[x] = 0;
		// deliberately fall through
	case 0:
		decoder->synchigh[x] <<= 1;
		if(decoder->synclow[x] & 0x80000000)
			decoder->synchigh[x] |= 1;
		decoder->synclow[x] <<= 1;
		if(bit)
			decoder->synclow[x] |= 1;

		gcount = _onebits(0x000000ff & (0x00000007 ^ decoder->synchigh[x]));
		gcount += _onebits(0x092a446f ^ decoder->synclow[x]);

		if(gcount <= MDC_GDTHRESH)
		{
 //printf("sync %d  %x %x \n",gcount,decoder->synchigh[x], decoder->synclow[x]);
			decoder->shstate[x] = 1;
			decoder->shcount[x] = 0;
			_clearbits(decoder, x);
		}
		else if(gcount >= (40 - MDC_GDTHRESH))
		{
 //printf("isync %d\n",gcount);
			decoder->shstate[x] = 1;
			decoder->shcount[x] = 0;
			decoder->xorb[x] = !(decoder->xorb[x]);
			decoder->invert[x] = !(decoder->invert[x]);
			_clearbits(decoder, x);
		}
		return;
	case 1:
	case 2:
		decoder->bits[x][decoder->shcount[x]] = bit;
		decoder->shcount[x]++;
		if(decoder->shcount[x] > 111)
		{
			_procbits(decoder, x);
		}
		return;

	default:
		return;
	}
}

static void _zcproc(mdc_decoder_t *decoder, int x)
{
	switch(decoder->zc[x])
	{
	case 2:
	case 4:
		break;
	case 3:
		decoder->xorb[x] = !(decoder->xorb[x]);
		break;
	default:
		return;
	}

	_shiftin(decoder, x);
}

static void _nlproc(mdc_decoder_t *decoder, int x)
{
	mdc_float_t vnow;
	mdc_float_t vpast;

	switch(decoder->nlstep[x])
	{
	case 3:
		vnow = ((-0.60 * decoder->nlevel[x][3]) + (.97 * decoder->nlevel[x][1]));
		vpast = ((-0.60 * decoder->nlevel[x][7]) + (.97 * decoder->nlevel[x][9]));
		break;
	case 8:
		vnow = ((-0.60 * decoder->nlevel[x][8]) + (.97 * decoder->nlevel[x][6]));
		vpast = ((-0.60 * decoder->nlevel[x][2]) + (.97 * decoder->nlevel[x][4]));
		break;
	default:
		return;
	}

	decoder->xorb[x] = (vnow > vpast) ? 1 : 0;
	if(decoder->invert[x])
		decoder->xorb[x] = !(decoder->xorb[x]);
	_shiftin(decoder, x);
}

int mdc_decoder_process_samples(mdc_decoder_t *decoder,
                                mdc_sample_t *samples,
                                int numSamples)
{
	mdc_int_t i, j, k;
	mdc_sample_t sample;
	mdc_float_t value;
	mdc_float_t delta;

	if(!decoder)
		return -1;

	for(i = 0; i<numSamples; i++)
	{
		sample = samples[i];

#if defined(MDC_SAMPLE_FORMAT_U8)
		value = (((mdc_float_t)sample) - 128.0)/256;
#elif defined(MDC_SAMPLE_FORMAT_U16)
		value = (((mdc_float_t)sample) - 32768.0)/65536.0;
#elif defined(MDC_SAMPLE_FORMAT_S16)
		value = ((mdc_float_t)sample) / 65536.0;
#elif defined(MDC_SAMPLE_FORMAT_FLOAT)
		value = sample;
#else
#error "no known sample format set"
#endif

#ifdef ZEROCROSSING

#ifdef DIFFERENTIATOR
		delta = value - decoder->lastvalue;
		decoder->lastvalue = value;

		if(decoder->level == 0)
		{
			if(delta > decoder->hyst)
			{
				for(k=0; k<MDC_ND; k++)
					decoder->zc[k]++;
				decoder->level = 1;
			}
		}
		else
		{
			if(delta < (-1 * decoder->hyst))
			{
				for(k=0; k<MDC_ND; k++)
					decoder->zc[k]++;
				decoder->level = 0;
			}
		}
#else	/* DIFFERENTIATOR */
		if(decoder->level == 0)
		{
			if(s > decoder->hyst)
			{
				for(k=0; k<MDC_ND; k++)
					decoder->zc[k]++;
				decoder->level = 1;
			}
		}
		else
		{
			if(s < (-1.0 * decoder->hyst))
			{
				for(k=0; k<MDC_ND; k++)
					decoder->zc[k]++;
				decoder->level = 0;
			}
		}
#endif	/* DIFFERENTIATOR */
		

		for(j=0; j<MDC_ND; j++)
		{
			decoder->th[j] += decoder->incr;
			if(decoder->th[j] >= TWOPI)
			{
				_zcproc(decoder, j);
				decoder->th[j] -= TWOPI;
				decoder->zc[j] = 0;
			}
		}
#else	/* ZEROCROSSING */


		for(j=0; j<MDC_ND; j++)
		{
			decoder->th[j] += (5.0 * decoder->incr);
			if(decoder->th[j] >= TWOPI)
			{
				decoder->nlstep[j]++;
				if(decoder->nlstep[j] > 9)
					decoder->nlstep[j] = 0;
				decoder->nlevel[j][decoder->nlstep[j]] = value;	

				_nlproc(decoder, j);

				decoder->th[j] -= TWOPI;
			}
		}
#endif
	}

	

	if(decoder->good)
		return decoder->good;

	return 0;
}

int mdc_decoder_get_packet(mdc_decoder_t *decoder, 
                           unsigned char *op,
			   unsigned char *arg,
			   unsigned short *unitID)
{
	if(!decoder)
		return -1;

	if(decoder->good != 1)
		return -1;

	if(op)
		*op = decoder->op;

	if(arg)
		*arg = decoder->arg;

	if(unitID)
		*unitID = decoder->unitID;

	decoder->good = 0;

	return 0;
}

int mdc_decoder_get_double_packet(mdc_decoder_t *decoder, 
                           unsigned char *op,
			   unsigned char *arg,
			   unsigned short *unitID,
                           unsigned char *extra0,
                           unsigned char *extra1,
                           unsigned char *extra2,
                           unsigned char *extra3)
{
	if(!decoder)
		return -1;

	if(decoder->good != 2)
		return -1;

	if(op)
		*op = decoder->op;

	if(arg)
		*arg = decoder->arg;

	if(unitID)
		*unitID = decoder->unitID;

	if(extra0)
		*extra0 = decoder->extra0;
	if(extra1)
		*extra1 = decoder->extra1;
	if(extra2)
		*extra2 = decoder->extra2;
	if(extra3)
		*extra3 = decoder->extra3;

	decoder->good = 0;

	return 0;
}

int mdc_decoder_set_callback(mdc_decoder_t *decoder, mdc_decoder_callback_t callbackFunction, void *context)
{
	if(!decoder)
		return -1;

	decoder->callback = callbackFunction;
	decoder->callback_context = context;

	return 0;
}
