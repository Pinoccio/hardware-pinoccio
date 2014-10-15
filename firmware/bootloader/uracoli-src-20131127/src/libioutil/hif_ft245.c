/* Copyright (c) 2007 Axel Wachtler
   Copyright (c) 2009 Joerg Wunsch
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

/* === includes ========================================== */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "board.h"
#include "ioutil.h"
#if HIF_TYPE == HIF_FT245

/* === macros ============================================ */

/* === types ============================================= */

/* === globals =========================================== */

/* === prototypes ======================================== */

/* === functions ========================================= */
void hif_init(const uint32_t baudrate)
{
    HIF_IO_ENABLE();
}

void hif_puts_p(const char *progmem_s)
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) != 0 )
    {
		while(FT245_TX_IS_BLOCKED())
			;
		HIF_USB_WRITE(c);
    }
}

void hif_puts(const char *s)
{
    register char c;

    while ((c = *s++) != 0)
    {
		while(FT245_TX_IS_BLOCKED())
			;
		HIF_USB_WRITE(c);
    }
}

uint8_t hif_put_blk(unsigned char *data, uint8_t size)
{
    uint8_t xfered;

    for (xfered = 0; size != 0; xfered++, size--)
    {
		while(FT245_TX_IS_BLOCKED())
			;
		HIF_USB_WRITE(*data++);
    }
    return xfered;
}

int hif_getc()
{
int ret = EOF;
    if(FT245_RX_HAS_DATA())
    {
        ret = (unsigned int)HIF_USB_READ();
    }
    else
    {
        ret = EOF;
    }
    return ret;
}


uint8_t hif_get_blk(unsigned char *data, uint8_t max_size)
{
    uint8_t cnt = 0;

    while(FT245_RX_HAS_DATA() && (cnt < max_size))
    {
        *data++ = (unsigned char)HIF_USB_READ();
        cnt++;
    }

    return cnt;
}

int hif_putc(int c)
{
	while(FT245_TX_IS_BLOCKED())
		;
	HIF_USB_WRITE((uint8_t)c);
	return c;
}

#endif
