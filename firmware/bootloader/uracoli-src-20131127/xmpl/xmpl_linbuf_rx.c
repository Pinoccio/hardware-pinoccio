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
/* Example use of the buffer functions for receive operation
 * This example expects frames to receive from xmpl_linbuf_tx.c
 */
#include <stdio.h>
#include "board.h"
#include "hif.h"
#include "radio.h"
#include "xmpl.h"

#define XMPL_FRAME_SIZE (40)

uint8_t rxbuf[XMPL_FRAME_SIZE + 2];
uint8_t buf[sizeof(buffer_t) + XMPL_FRAME_SIZE];
buffer_t *pbuf;


int main(void)
{
uint8_t frame_header[] = {0x01, 0x80, 0, 0x11,0x22,0x33,0x44};


    /* Init ressources */
    LED_INIT();
#if HIF_TYPE != HIF_NONE
    hif_init(HIF_DEFAULT_BAUDRATE);
#endif

    radio_init(rxbuf,sizeof(rxbuf));
    sei();
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_state(STATE_RX);

    /* Initialize buffer structure */
    pbuf = buffer_init(buf, sizeof(buf), 0);

    while(1)
    {
        if (BUFFER_IS_LOCKED(pbuf) == true)
        {
            uint8_t c, i, seq;
            /* do a header compare to ensure that it "our" frame */
            for(i=0; i<sizeof(frame_header);i++)
            {
                c = buffer_get_char(pbuf);
                if ( (i != 2) && (c != frame_header[i]))
                {
                    break;
                }
                if(i==2)
                {
                    seq = c;
                }
            }

            if (i==7)
            {
                /* if this frame belongs to us,
                   blink the LED and if available
                   send the payload to the UART */
                LED_TOGGLE(0);
                /* output at hif */
            #if HIF_TYPE != HIF_NONE
                //PRINTF("\n\r %03d: ", seq);
                hif_put_blk (BUFFER_PDATA(pbuf), BUFFER_SIZE(pbuf));
            #endif
            }
            else
            {
                LED_TOGGLE(1);
            }
            BUFFER_RESET(pbuf,0);
            BUFFER_SET_UNLOCK(pbuf);
        }
        /* wait after this run */
        WAIT_MS(500);
    }
}


uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{
    if ( BUFFER_IS_LOCKED(pbuf) == false && crc == 0)
    {
        /* copy the payload, reduced by the CRC bytes */
        buffer_append_block(pbuf, frm, len-2);
        BUFFER_SET_LOCK(pbuf);
    }
    return frm;
}

/* XEOF */
