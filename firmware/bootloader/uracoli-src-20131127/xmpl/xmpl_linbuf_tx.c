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
/* Example use of the buffer functions */
#include <stdio.h>
#include "board.h"
#include "hif.h"
#include "radio.h"
#include "xmpl.h"



int main(void)
{

/* buffer is smaller than a limmerick line */
#define XMPL_FRAME_SIZE (40)
/*two byte more for the CRC */
uint8_t txbuf[sizeof(buffer_t) + XMPL_FRAME_SIZE + 2];

uint8_t frame_header[] = {0x01, 0x80, 0, 0x11,0x22,0x33,0x44};
buffer_t *pbuf;
//                           1         2         3         4
//                  1234567890123456789012345678901234567890123456789
char limmerick[] = "A wonderful bird is the pelican,\n\r"
                   "His bill will hold more than his belican,\n\r"
                   "He can take in his beak\n\r"
                   "Enough food for a week\n\r"
                   "But I'm damned if I see how the helican!\n\r\n\r";

char *plim;

    /* Prerequisite: Init radio */
    LED_INIT();
    radio_init(NULL, 0);
    sei();
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_state(STATE_TX);

    /* Initialize the buffer structure */
    pbuf = buffer_init(txbuf, sizeof(txbuf)-2, sizeof(frame_header));
    plim = limmerick;
    while(1)
    {
        /* fill buffer until '\n' or '\0' or EOF is reached */
        do
        {
            if (buffer_append_char(pbuf, *plim) == EOF)
            {
                break;
            }
            plim++;
        }
        while (*plim != 0 && *plim != '\n');

        /* finalize the buffer and transmit it */
        buffer_prepend_block(pbuf, frame_header, sizeof(frame_header));
        radio_set_state(STATE_TX);
        radio_send_frame(BUFFER_SIZE(pbuf)+ 2, BUFFER_PDATA(pbuf), 0);

        /* wait after this run */
        LED_TOGGLE(0);
        WAIT_MS(500);

        /* prepare next run */
        BUFFER_RESET(pbuf,sizeof(frame_header));
        frame_header[2]++;
        if (*plim == 0) plim = limmerick;
    }
}
/* XEOF */
