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
/* Example that implements HIF echo, usefull to test the HIF troughput */

#include "board.h"
#include "hif.h"
#include "xmpl.h"

int main(void)
{
 int inchar;
 uint32_t br = HIF_DEFAULT_BAUDRATE;

 uint32_t txcnt, rxcnt;
 uint8_t state;

const char volatile INFO[] = "info";

    /* setting up UART and adjusting the baudrate */
    hif_init(br);
    LED_INIT();
    txcnt = 0;
    rxcnt = 0;
    state = 0;
    sei();
#if HIF_TYPE == HIF_AT90USB
    /*
     * Wait for terminal user pressing a key so there is time to
     * attach a terminal emulator after the virtual serial port has
     * been established within the host OS.
     */
    do
    {
        inchar = hif_getc();
    }
    while (EOF == inchar);
#endif

    /* using the basic hif_xxx functions */
    hif_printf(FLASH_STRING("\n\rHIF Echo : %s : %ld bit/s\n\r"),BOARD_NAME,br);
    hif_echo(FLASH_STRING("$Revision$\n\r"));

    while(1)
    {
        inchar = hif_getc();
        rxcnt ++;

        if (EOF != inchar)
        {
            hif_putc(inchar);
            txcnt ++;

            if (inchar == INFO[state])
            {
                state += 1;
            }
            else
            {
                state = 0;
            }
            if (state == 4)
            {
                PRINTF("\n\rECHO rx=%ld tx=%ld\n\r", rxcnt, txcnt);
                state = 0;
                txcnt = 0;
                rxcnt = 0;
            }
        }
    }
}
/* EOF */

