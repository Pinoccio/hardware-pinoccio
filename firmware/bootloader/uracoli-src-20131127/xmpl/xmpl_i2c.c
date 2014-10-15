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
/* Example for use of the I2C functions */
#include <stdlib.h>
#include "board.h"
#include "ioutil.h"
#include "i2c.h"
#include "hif.h"
#include "xmpl.h"


/* === macros =============================================================== */
#define PROMPT() PRINT("\n\ri2c>>> ")
#define LINE_SIZE (80)
#define MAX_ARGS  (16)

/* === globals ============================================================== */
char *argv[MAX_ARGS];
int argc;

/* === prototypes =========================================================== */
bool process_input(int c);
void process_commands(char **argv, int argc);

/* === implementation ======================================================= */
int main(void)
{
 const uint32_t br = HIF_DEFAULT_BAUDRATE;
 uint8_t addr, ret;
 int chr;
    /* setting up UART and adjusting the baudrate */
    hif_init(br);
    LED_INIT();
    LED_SET(0);
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

    i2c_init( 4000000UL );

    /* using the basic hif_xxx functions */
    PRINTF("\n\rI2C Example : %s : %ld bit/s\n\r", BOARD_NAME, br);

    PRINT("\ni2c bus scan:\n");
    for (addr=0; addr<128; addr++)
    {
        ret = i2c_probe(addr);
        if (ret)
        {
            PRINTF(" a=0x%02x, rv=0x%02x, OK\n", addr, ret);
            LED_TOGGLE(1);
        }
        WAIT_MS(1);
        LED_TOGGLE(0);
    }



    PROMPT();
    do
    {
        chr = hif_getc();
        if (process_input(chr))
        {
            if (argc)
            {
                process_commands(argv, argc);
            }
            PROMPT();
        }
    }
    while (1);

}

bool process_input(int c)
{
    static char line[LINE_SIZE];
    static int idx;

    bool rv = false;
    if (c != EOF)
    {
        if (c == '\n' || c == '\r')
        {
            line[idx] = 0;
            idx = 0;
            hif_putc('\n');
            argc = hif_split_args(line, MAX_ARGS, argv);
            rv = true;
        }
        else if (idx < sizeof(line))
        {
            line[idx++] = c;
            hif_putc(c);
        }
        else
        {
            /* buffer full, throw away */
            idx = 0;
        }
    }

    return rv;
}

void process_commands(char **argv, int argc)
{
    uint8_t wbuf[32], rbuf[32], rlen, wlen, i;

    for (i = 1; i < argc; i++)
    {
        wbuf[i-1] = (uint8_t) strtol(argv[i], NULL, 16);
    }
    hif_dump(argc-1, wbuf);
    switch (argv[0][0])
    {
        case 'r':
            PRINTF("READ rlen=%d\n", wbuf[1]);
            i2c_master_writeread(wbuf[0], NULL, 0, rbuf, wbuf[1]);
            hif_dump(wbuf[1], rbuf);
            break;
        case 'w':
            PRINT("WRITE\n");
            i2c_master_writeread(wbuf[0], wbuf+1, argc-2, NULL, 0);
            break;

        case 'x':
            PRINTF("WRITE/READ wlen=%d, rlen=%d\n", argc-2, wbuf[argc-2]);
            i2c_master_writeread(wbuf[0], wbuf+1, argc-3, rbuf, wbuf[argc-1]);
            hif_dump(wbuf[argc-2], rbuf);
            break;

        default:
            PRINTF("Invalid command %s \n", argv[0]);
            break;

    }
}

/* E_O_F */

