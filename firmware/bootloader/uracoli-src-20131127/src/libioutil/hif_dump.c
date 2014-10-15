/* Copyright (c) 2007 Axel Wachtler
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "hif.h"

/* === macros ============================================ */

/* === types ============================================= */

/* === globals =========================================== */

/* === prototypes ======================================== */

/* === functions ========================================= */
void hif_dump(uint16_t sz, uint8_t *d)
{

    uint16_t i=0, k, j;

    #define BLKSZ (8)

/*
Dump line definition:

-------+-------------------------+---------+
0000 : 55 55 55 55 55 55 55 55 : UUUUUUUU :\0
+------+-------------------------+---------+
0      6                        32        41
*/
    char dbuf[2+1+24+3+8+3+3];
    char *pdbuf, tmp;
    dbuf[sizeof(dbuf)-1] = 0;
    PRINTF("DUMP : p=%p, size=%d\n\r", d, sz);

    i = 0;
    memset(dbuf,0,sizeof(dbuf));
    while(i<sz)
    {
        snprintf(dbuf,sizeof(dbuf),
                 "%04x :"\
                 "                         :"\
                 "          :", i);

        k = i + BLKSZ;
        if (k > sz) k = sz;

        pdbuf = &dbuf[7];
        for(j=i; j<k;j++)
        {
            tmp = (d[j]>>4) + 0x30;
            if(tmp>0x39) tmp += 7; /*map 10...15 to 'A' ... 'F'*/
            *pdbuf++ = tmp;
            tmp = (d[j]&0xf) + 0x30;
            if(tmp>0x39) tmp += 7; /*map 10...15 to 'A' ... 'F'*/
            *pdbuf++ = tmp;
            pdbuf ++ ;
        }

        pdbuf = &dbuf[7+24+2];
        for(j=i; j<k;j++)
        {
            tmp = d[j];
            /* because tmp is signed char, all
               letters from 0x80 (-128) to 0xff (-1) are
               marked as non printable by this check */
            if (tmp < 32)
            {
                tmp = '.';
            }
            *pdbuf++ = tmp;
        }
        hif_puts(dbuf);
        hif_puts_p(FLASH_STRING("\n\r"));
        i += BLKSZ;
    }

    PRINT("");
}
