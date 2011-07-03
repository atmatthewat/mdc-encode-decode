/*-
 * mdc_decode.h
 *  header for mdc_decode.c
 *
 * 4 October 2010 - fixed for 64-bit
 * 5 October 2010 - added four-point method to C version
 * 7 October 2010 - typedefs for easier porting
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

#ifndef _MDC_DECODE_H_
#define _MDC_DECODE_H_

#include "mdc_types.h"

#ifndef TWOPI
#define TWOPI (2.0 * 3.1415926535)
#endif

#define MDC_ND 5	// number of decoders (5 is a good choice for new way, 4 is a good choice for zerocrossing
#define MDC_GDTHRESH 5  // "good bits" threshold

#define DIFFERENTIATOR

// #define ZEROCROSSING /* turn off for better 4-point method */


typedef void (*mdc_decoder_callback_t)(	int frameCount, // 1 or 2 - if 2 then extra0-3 are valid
										unsigned char op,
										unsigned char arg,
										unsigned short unitID,
										unsigned char extra0,
										unsigned char extra1,
										unsigned char extra2,
										unsigned char extra3);

typedef struct {
	mdc_float_t hyst;
	mdc_float_t incr;
	mdc_float_t th[MDC_ND];
	mdc_int_t level;
	mdc_float_t lastvalue;
	mdc_int_t zc[MDC_ND];
	mdc_int_t xorb[MDC_ND];
	mdc_int_t invert[MDC_ND];
	mdc_int_t nlstep[MDC_ND];
	mdc_float_t nlevel[MDC_ND][10];
	mdc_u32_t synclow[MDC_ND];
	mdc_u32_t synchigh[MDC_ND];
	mdc_int_t shstate[MDC_ND];
	mdc_int_t shcount[MDC_ND];
	mdc_int_t bits[MDC_ND][112];
	mdc_int_t good;
	mdc_u8_t op;
	mdc_u8_t arg;
	mdc_u16_t unitID;
	mdc_u8_t crc;
	mdc_u8_t extra0;
	mdc_u8_t extra1;
	mdc_u8_t extra2;
	mdc_u8_t extra3;
	mdc_decoder_callback_t callback;
} mdc_decoder_t;
	


/*
 mdc_decoder_new
 create a new mdc_decoder object

  parameters: int sampleRate - the sampling rate in Hz

  returns: an mdc_decoder object or null if failure

*/
mdc_decoder_t * mdc_decoder_new(int sampleRate);

/*
 mdc_decoder_process_samples
 process incoming samples using an mdc_decoder object
 (currently limited to 8-bit unsigned samples)

 parameters: mdc_decoder_t *decoder - pointer to the decoder object
             mdc_sample_t *samples - pointer to samples (in format set in mdc_types.h)
             int numSamples - count of the number of samples in buffer

 returns: 0 if more samples are needed
         -1 if an error occurs
          1 if a decoded single packet is available to read
          2 if a decoded double packet is available to read
*/
 
int mdc_decoder_process_samples(mdc_decoder_t *decoder,
                                mdc_sample_t *samples,
                                int numSamples);


/*
 mdc_decoder_get_packet
 retrieve last successfully decoded data packet from decoder object

 parameters: mdc_decoder_t *decoder - pointer to the decoder object
             unsigned char *op      - pointer to where to store "opcode"
             unsigned char *arg     - pointer to where to store "argument"
             unsigned short *unitID - pointer to where to store "unit ID"

 returns: -1 if error, 0 otherwise
*/

int mdc_decoder_get_packet(mdc_decoder_t *decoder, 
                           unsigned char *op,
			   unsigned char *arg,
			   unsigned short *unitID);

/*
 mdc_decoder_get_double_packet
 retrieve last successfully decoded double-length packet from decoder object

 parameters: mdc_decoder_t *decoder - pointer to the decoder object
             unsigned char *op      - pointer to where to store "opcode"
             unsigned char *arg     - pointer to where to store "argument"
             unsigned short *unitID - pointer to where to store "unit ID"
             unsigned char *extra0  - pointer to where to store 1st extra byte
             unsigned char *extra1  - pointer to where to store 2nd extra byte
             unsigned char *extra2  - pointer to where to store 3rd extra byte
             unsigned char *extra3  - pointer to where to store 4th extra byte

 returns: -1 if error, 0 otherwise
*/

int mdc_decoder_get_double_packet(mdc_decoder_t *decoder, 
                           unsigned char *op,
			   unsigned char *arg,
			   unsigned short *unitID,
                           unsigned char *extra0,
                           unsigned char *extra1,
                           unsigned char *extra2,
                           unsigned char *extra3);


/*
 mdc_decoder_set_callback
 set a callback function to be called upon successful decode
 if this is set, the functions mdc_decoder_get_packet and mdc_decoder_get_double_packet
 will no longer be functional, instead the callback function is called immediately when
 a successful decode happens (from within the context of mdc_decoder_process_samples)

 returns: -1 if error, 0 otherwise
 */

int mdc_decoder_set_callback(mdc_decoder_t *decoder, mdc_decoder_callback_t callbackFunction);

#endif
