/* Copyright (c) 2008 Axel Wachtler
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
/* Example use of the radio stream functions */
#include <stdio.h>
#include "board.h"
#include "hif.h"
#include "radio.h"
#include "xmpl.h"

#define PROMPT() PRINTF("\n\ruracoli[%02d]> ",lc++)

void incb(buffer_t *pbuf);
void outcb(buffer_t *pbuf);

buffer_stream_t Rstream;
uint8_t frame_header[] = {0x01, 0x80, 0, 0x11,0x22,0x33,0x44};
#define XMPL_FRAME_SIZE (40)
uint8_t ibuf[sizeof(buffer_t) + XMPL_FRAME_SIZE + 2];
uint8_t obuf[sizeof(buffer_t) + XMPL_FRAME_SIZE + 2];

//volatile buffer_t *pibuf;
int main(void)
{
uint8_t rxbuf[MAX_FRAME_SIZE];
uint8_t cnt = 0;


    /* setup buffers */
    buffer_init(ibuf, sizeof(ibuf)-2, 0);
    buffer_init(obuf, sizeof(obuf)-2, sizeof(frame_header));

    /* setup buffer stream structure and stdio */
    buffer_stream_init(&Rstream, &incb, &outcb);
    /* todo add buffer assignment as parameters to buffer_stream_init! */
    Rstream.pbufin  = (buffer_t *)ibuf;
    Rstream.pbufout = (buffer_t *)obuf;
    stdout = stdin = &Rstream.bstream;

    /* setup hardware */
    LED_INIT();
    radio_init(rxbuf, MAX_FRAME_SIZE);
    radio_set_state(STATE_OFF);
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_param(RP_IDLESTATE(STATE_RX));
    sei();
    printf_P(PSTR("Hello World %d\n\r"),cnt++);

    while(1)
    {
        WAIT_MS(10);

        int c;
        c = getchar();
        if ( ('a' <= c) && (c <= 'z'))        
        {
            printf_P(PSTR(":%c:\n\r"),c);
        }
    }
}


void outcb(buffer_t *pbuf)
{
static uint8_t frame_header[] = {0x01, 0x80, 0, 0x11,0x22,0x33,0x44};
char lastchar;
    //LED_TOGGLE(0);

    lastchar = BUFFER_LAST_CHAR(pbuf);
    if ((BUFFER_FREE_AT_END(pbuf) < 1) || (lastchar == '\r'))
    {
        /* prepare send */
        buffer_prepend_block(pbuf, frame_header, sizeof(frame_header));
        radio_set_state(STATE_TX);
        radio_send_frame(BUFFER_SIZE(pbuf)+ 2, BUFFER_PDATA(pbuf), 0);
        /* clean buffer */
        BUFFER_RESET(pbuf, sizeof(frame_header));
        frame_header[2]++;
        /* blink LED if done. */
        //LED_TOGGLE(0);
    }
}


void incb(buffer_t *pbuf)
{
uint8_t sz;
    sz = BUFFER_SIZE(pbuf);
    if (sz < 1)
    {
        /* buffer is now empty, free it. */
cli();
        BUFFER_RESET(pbuf,0);
        BUFFER_SET_UNLOCK(pbuf);
sei();        
    }
}


uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
uint16_t fctl;
uint8_t hlength;
    LED_TOGGLE(1);
    

    if ( BUFFER_IS_LOCKED(Rstream.pbufin) == false && crc == 0)
    {
        fctl = *(uint16_t*)frm;
        /* copy the payload, reduced by the CRC bytes */
        buffer_append_block(Rstream.pbufin, frm, len-2);
        hlength = 3;
        hlength += ((fctl & FCTL_DST_MASK) == FCTL_DST_LONG) ? 10:0;
        hlength += ((fctl & FCTL_DST_MASK) == FCTL_DST_SHORT) ? 4:0;
        hlength += ((fctl & FCTL_SRC_MASK) == FCTL_SRC_LONG) ? 10:0;
        hlength += ((fctl & FCTL_SRC_MASK) == FCTL_SRC_SHORT) ? 4:0;
        if (fctl & FCTL_IPAN_MASK)
        {
            hlength -= (fctl & FCTL_SRC_MASK) ? 2:0;
        }
        BUFFER_ADVANCE(Rstream.pbufin,hlength-1);
        printf("rx=%d\n\r", BUFFER_SIZE(Rstream.pbufin));
        BUFFER_SET_LOCK(Rstream.pbufin);
    }
    return frm;
}
/* XEOF */

