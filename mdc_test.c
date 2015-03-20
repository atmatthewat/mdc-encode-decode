/*-
 * functiontest.c
 *   Functional test for mdc_decode and mdc_encode
 *
 * 24 May 2011 - Initial implementation
 *
 * Author: Matthew Kaufman (matthew@eeph.com)
 *
 * Copyright (c) 2011  Matthew Kaufman  All rights reserved.
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
#include <stdio.h>

#include "mdc_encode.h"
#include "mdc_decode.h"

void run(mdc_encoder_t *encoder, mdc_decoder_t *decoder, int expect);

void testCallback(int numFrames, unsigned char op, unsigned char arg, unsigned short unitID, unsigned char extra0, unsigned char extra1, unsigned char extra2, unsigned char extra3, void *context);



int main()
{
	mdc_encoder_t *encoder;
	mdc_decoder_t *decoder;
	int rv;

	encoder = mdc_encoder_new(48000);
	decoder = mdc_decoder_new(48000);

	if(!decoder)
	{
		fprintf(stderr,"mdc_decoder_new() failed\n");
		exit(-1);
	}

	if(!encoder)
	{
		fprintf(stderr,"mdc_encoder_new() failed\n");
		exit(-1);
	}

	rv = mdc_encoder_set_packet(encoder, 0x12, 0x34, 0x5678);

	if(rv)
	{
		fprintf(stderr,"mdc_encoder_set_packet() failed\n");
		exit(-1);
	}

	run(encoder, decoder, 1);

	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);

	if(rv)
	{
		fprintf(stderr,"mdc_encoder_set_packet() failed\n");
		exit(-1);
	}

printf("1: ");
	run(encoder, decoder, 2);

	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("2: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("3: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("4: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("5: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("6: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("7: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("8: ");
	run(encoder, decoder, 2);
	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);
printf("9: ");
	run(encoder, decoder, 2);
	/* additional tests for callback mode */

	rv = mdc_decoder_set_callback(decoder, testCallback, (void*) 0x555);

	rv = mdc_encoder_set_packet(encoder, 0x12, 0x34, 0x5678);

	if(rv)
	{
		fprintf(stderr,"mdc_encoder_set_packet() failed\n");
		exit(-1);
	}

	run(encoder, decoder, -1);

	rv = mdc_encoder_set_double_packet(encoder, 0x55, 0x34, 0x5678, 0x0a, 0x0b, 0x0c, 0x0d);

	if(rv)
	{
		fprintf(stderr,"mdc_encoder_set_packet() failed\n");
		exit(-1);
	}

	run(encoder, decoder, -2);




	fprintf(stderr,"mdc functional test overall success\n");
	exit(0);
}

int callbackFound;

#define NUMSAMPLES 1024

void run(mdc_encoder_t *encoder, mdc_decoder_t *decoder, int expect)
{
	int rv, rv2, rv3;
	int cont = 10;
	int msgFound = 0;

	callbackFound = 0;

	while(cont)
	{
		mdc_sample_t buffer[NUMSAMPLES];

		rv = mdc_encoder_get_samples(encoder, buffer, NUMSAMPLES);

		if(rv < 0)
		{
			fprintf(stderr,"mdc_encoder_get_samples() failed\n");
			exit(-1);
		}
		else if(rv == 0)
		{
			int i;
			--cont;
			for(i = 0; i<NUMSAMPLES; i++)
				buffer[i] = 0;
			rv = NUMSAMPLES;
		}

		rv2 = mdc_decoder_process_samples(decoder, buffer, rv);

		if(rv2 < 0)
		{
			fprintf(stderr,"mdc_decoder_process_samples() failed\n");
			exit(-1);
		}
		else if(rv2 == 0)
		{
			if(callbackFound != 0)
			{
				if(callbackFound == expect)
				{
					return;
				}
				else
				{
					fprintf(stderr,"callback found %d but expected %d\n", callbackFound, expect);
					exit(-1);
				}
			}
			// else continue
		}
		else if(rv2 == 1 || rv2 == 2)
		{
			unsigned char op;
			unsigned char arg;
			unsigned short unitID;
			unsigned char extra0, extra1, extra2, extra3;

			msgFound = 1;

			if(rv2 != expect)
			{
				fprintf(stderr,"process samples returned %d but expected %d\n",rv2,expect);
				exit(-1);
			}

			if(rv2 == 1)
				rv3 = mdc_decoder_get_packet(decoder, &op, &arg, &unitID);
			else
				rv3 = mdc_decoder_get_double_packet(decoder, &op, &arg, &unitID, &extra0, &extra1, &extra2, &extra3);

			if(rv3 < 0)
			{
				fprintf(stderr,"%s failed\n", rv2 == 1 ? "mdc_decoder_get_packet()" : "mdc_decoder_get_double_packet()");
				exit(-1);
			}

			if(op != (expect == 1 ? 0x12 : 0x55))
			{
				fprintf(stderr,"op doesn't match\n");
				exit(-1);
			}

			if(arg != 0x34)
			{
				fprintf(stderr,"arg doesn't match\n");
				exit(-1);
			}

			if(unitID != 0x5678)
			{
				fprintf(stderr,"unitID doesn't match\n");
				exit(-1);
			}

			if(rv2 == 1)
				printf("single decode success\n");
			else
			{
				if(extra0 != 0x0a)
				{
					fprintf(stderr,"extra0 doesn't match\n");
					exit(-1);
				}
				if(extra1 != 0x0b)
				{
					fprintf(stderr,"extra1 doesn't match\n");
					exit(-1);
				}
				if(extra2 != 0x0c)
				{
					fprintf(stderr,"extra2 doesn't match\n");
					exit(-1);
				}
				if(extra3 != 0x0d)
				{
					fprintf(stderr,"extra3 doesn't match\n");
					exit(-1);
				}

				printf("double decode success\n");
			}
		}
		else
		{
			fprintf(stderr,"mdc_decoder_process_samples() invalid return value\n");
			exit(-1);
		}
	} // while

	if(!msgFound && !callbackFound)
	{
		fprintf(stderr,"warning: no packet found\n");
	}
}

void testCallback(int numFrames, unsigned char op, unsigned char arg, unsigned short unitID, unsigned char extra0, unsigned char extra1, unsigned char extra2, unsigned char extra3, void *context)
{
	if(context != (void *)0x555)
	{
		fprintf(stderr,"context invalid\n");
		exit(-1);
	}
	if(numFrames == 1)
	{
		callbackFound = -1;
	}
	else if(numFrames == 2)
	{
		callbackFound = -2;
	}
	else
	{
		fprintf(stderr,"numFrames invalid (callback)\n");
		exit(-1);
	}

	if(op == 0x12 && callbackFound == -1)
	{
		// good
	}
	else if(op == 0x55 && callbackFound == -2)
	{
		// good
	}
	else
	{
		fprintf(stderr,"op doesn't match expected (callback)\n");
		exit(-1);
	}

	if(arg != 0x34)
	{
		fprintf(stderr,"arg doesn't match (callback)\n");
		exit(-1);
	}

	if(unitID != 0x5678)
	{
		fprintf(stderr,"unitID doesn't match (callback)\n");
		exit(-1);
	}

	if(callbackFound == -1)
		printf("single decode success (callback)\n");
	else if(callbackFound == -2)
	{
		if(extra0 != 0x0a)
		{
			fprintf(stderr,"extra0 doesn't match (callback)\n");
			exit(-1);
		}
		if(extra1 != 0x0b)
		{
			fprintf(stderr,"extra1 doesn't match (callback)\n");
			exit(-1);
		}
		if(extra2 != 0x0c)
		{
			fprintf(stderr,"extra2 doesn't match (callback)\n");
			exit(-1);
		}
		if(extra3 != 0x0d)
		{
			fprintf(stderr,"extra3 doesn't match (callback)\n");
			exit(-1);
		}

		printf("double decode success (callback)\n");
	}
	// no else required, would have already failed above
}

