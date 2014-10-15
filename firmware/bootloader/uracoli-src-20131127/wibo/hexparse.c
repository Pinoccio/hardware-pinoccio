/* Copyright (c) 2010 Axel Wachtler
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the authors nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

/* $Id$ */
/**
 * @file
 * @brief Wireless bootloader application, resides in bootloader section
 *
 * @author Daniel Thiele
 *         Dietzsch und Thiele PartG
 *         Bautzner Str. 67 01099 Dresden
 *         www.ib-dt.de daniel.thiele@ib-dt.de
 *
 * @ingroup grpAppWiBo
 */

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "hexparse.h"

typedef enum{
	INVALID,
	PIVOT,
	LENGTH,
	ADDRESS,
	TYPE,
	DATA,
	CHECKSUM,
	VALID
}hexparse_state_t;

/*
 * \brief Convert hex digit to integer
 */
static inline uint8_t htoi(char c)
{
	uint8_t a;
	if (isdigit(c)) {
		a = c - '0';
	} else {
		a = c - 'a' + 10;
	}
	return(a & 0x0F);
}

/*
 * \brief Convert two digits of hex numbers to 8-bit integer
 */
static inline uint8_t hto8Bit(char c1, char c0)
{
	return (htoi(c1) << 4) | (htoi(c0) << 0);
}

/*
 * \brief Convert four digits of hex numbers to 16-bit integer
 */
static inline uint16_t hto16Bit(char c3, char c2, char c1, char c0)
{
	return (htoi(c3) << 12) | (htoi(c2) << 8) | (htoi(c1) << 4) | (htoi(c0) << 0);
}

/*
 * \brief Parse a line from intel hex file
 */
uint8_t parsehexline(uint8_t *ln, hexrec_t *rec)
{
	uint8_t i = 0;
	hexparse_state_t state = PIVOT;
	uint8_t cnt = 0;
	uint8_t tmp[4]; /* maximum number of digits for 16-bit integer */

	while( (cnt < 4) && (0 != (tmp[cnt] = *ln++)) && (INVALID != state) && (VALID != state)){
		cnt++;
		switch(state){
		case PIVOT:
			if((1 == cnt) && (tmp[0] == ':')){
				state = LENGTH;
				cnt = 0;
			}else{
				state = INVALID;
			}
			break;
		case LENGTH:
			if(2 == cnt){
				rec->len = hto8Bit(tmp[0], tmp[1]);
				state = ADDRESS;
				cnt = 0;
			}
			break;
		case ADDRESS:
			if(4 == cnt){
				rec->addr = hto16Bit(tmp[0], tmp[1], tmp[2], tmp[3]);
				state = TYPE;
				cnt = 0;
			}
			break;
		case TYPE:
			if(2 == cnt){
				rec->type = hto8Bit(tmp[0], tmp[1]);
				if(0 < rec->len){
					state = DATA;
				}else{
					state = CHECKSUM;
				}
				cnt = 0;
			}
			break;
		case DATA:
			if(2 == cnt){
				rec->data[i] = hto8Bit(tmp[0], tmp[1]);
				if(++i >= rec->len){
					state = CHECKSUM;
				}
				cnt=0;
			}
			break;
		case CHECKSUM:
			if(1 == cnt){
				rec->checksum = hto8Bit(tmp[0], tmp[1]);
				state = VALID;
			}
			break;
		default:
			state = INVALID;
			break;
		}
	}

	/* TODO: validate checksum */

	return (VALID == state);
}

/* EOF */
