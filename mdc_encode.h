/*-
 * mdc_encode.h
 *  header for mdc_encode.c
 *
 * 9 October 2010 - typedefs for easier porting
 *
 * Author: Matthew Kaufman (matthew@eeph.com)
 *
 * Copyright (c) 2005  Matthew Kaufman  All rights reserved.
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

#ifndef _MDC_ENCODE_H_
#define _MDC_ENCODE_H_

#include "mdc_types.h"

//#define FILL_FINAL	// fills the end of the last block with zeros, rather than returning fewer samples than requested

//#define MDC_ENCODE_FULL_AMPLITUDE	// encode at 100% amplitude (default is 68% amplitude for recommended deviation)

typedef struct {
	mdc_int_t loaded;
	mdc_int_t bpos;
	mdc_int_t ipos;
	mdc_int_t preamble_set;
	mdc_int_t preamble_count;
	mdc_u32_t thu;
	mdc_u32_t tthu;
	mdc_u32_t incru;
	mdc_u32_t incru18;
	mdc_int_t state;
	mdc_int_t lb;
	mdc_int_t xorb;
	mdc_u8_t data[14+14+5+7];
} mdc_encoder_t;
	

/*
 mdc_encoder_new
 create a new mdc_encoder object

  parameters: int sampleRate - the sampling rate in Hz

  returns: an mdc_encoder object or null if failure

*/
mdc_encoder_t * mdc_encoder_new(int sampleRate);

/*
 mdc_encoder_set_preamble(mdc_encoder_t *encoder,
                          int preambleLength)

 parameters: mdc_encoder_t *encoder  - pointer to the encoder object
             int preambleLength - length of additional preamble (in bytes) - preamble time is 6.66 msec * preambleLength

 returns: -1 for error, 0 otherwise
*/

int mdc_encoder_set_preamble(mdc_encoder_t *encoder,
                          int preambleLength);


/*
 mdc_encoder_set_packet
 set up a normal-length MDC packet for transmission

 parameters: mdc_encoder_t *encoder  - pointer to the encoder object
	     unsigned char op        - the "opcode"
	     unsigned char arg       - the "argument"
	     unsigned short unitID   - the "unit ID"

 returns: -1 for error, 0 otherwise

*/
int mdc_encoder_set_packet(mdc_encoder_t *encoder,
                           unsigned char op,      
                           unsigned char arg,
                           unsigned short unitID);

/*
 mdc_encoder_set_double_packet
 set up a double-length MDC packet for transmission

 parameters: mdc_encoder_t *encoder  - pointer to the encoder object
	     unsigned char op        - the "opcode"
	     unsigned char arg       - the "argument"
	     unsigned short unitID   - the "unit ID
	     unsigned char extra0    - the 1st "extra byte"
	     unsigned char extra1    - the 2nd "extra byte"
	     unsigned char extra2    - the 3rd "extra byte"
	     unsigned char extra3    - the 4th "extra byte"

 returns: -1 for error, 0 otherwise

*/
int mdc_encoder_set_double_packet(mdc_encoder_t *encoder,
                                  unsigned char op,
                                  unsigned char arg,
                                  unsigned short unitID,
                                  unsigned char extra0,
                                  unsigned char extra1,
                                  unsigned char extra2,
                                  unsigned char extra3);


/*
 mdc_encoder_get_samples
 get generated output audio samples from encoder

 parameters: mdc_encoder_t *encoder - the pointer to the encoder object
	     mdc_sample_t *buffer  - the sample buffer to write into
	     int bufferSize         - the size (in samples) of the sample buffer

 returns: -1 for error, otherwise returns the number of samples written
	  into the buffer (will be equal to bufferSize unless the end has
	  been reached, in which case the last block may be less than
	  bufferSize and all subsequent calls will return zero, until
	  a new packet is loaded for transmission

*/
int mdc_encoder_get_samples(mdc_encoder_t *encoder,
                            mdc_sample_t *buffer,
                            int bufferSize);

#endif

