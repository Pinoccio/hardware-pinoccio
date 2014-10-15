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
/* Example for use of the HIF functions */

#include "board.h"
#include "hif.h"
#include "xmpl.h"

#define PROMPT() PRINTF("\n\ruracoli[%02d]> ",lc++)

int main(void)
{
 int inchar;
 const uint32_t br = HIF_DEFAULT_BAUDRATE;
 uint8_t lc = 0;
 uint8_t msg[] = { 0x57, 0x65, 0x6c, 0x63, 0x6f, 0x6d, 0x65, 0x20,
                   0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x77,
                   0x6f, 0x72, 0x6c, 0x64, 0x20, 0x6f, 0x66, 0x20,
                   0xb5, 0x72, 0x61, 0x63, 0x6f, 0x6c, 0x69, 0x21,
                   '\n','\r',0x00};

    /* setting up UART and adjusting the baudrate */
    hif_init(br);

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
    hif_printf(FLASH_STRING("\n\rHIF Example : %s : %ld bit/s\n\r"),BOARD_NAME,br);
    hif_echo(FLASH_STRING("$Revision$\n\r"));

    /* this macro is equivalent to hif_printf(FLASH_STRING(...),...) */
    PRINTF("File: %s:%d\n\r",__FILE__,__LINE__);

    /* this function outputs an array transparently  */
    hif_put_blk (msg, sizeof(msg));

    /* there is also an optimized hexdump function for byte arrays */
    DUMP(sizeof(msg),msg);

    /* starting the main loop, prompting and counting the line numbers */
    PROMPT();
    while(1)
    {
        inchar = hif_getc();
        if (EOF != inchar)
        {
            if (inchar == '\r' || inchar == '\n')
            {
                PROMPT();
            }
            else
            {
                hif_putc(inchar);
            }
        }
    }
}
/* EOF */

