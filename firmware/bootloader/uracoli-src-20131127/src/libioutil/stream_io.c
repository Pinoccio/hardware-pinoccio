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
/**
 * @file
 * @brief Implementation of the radio stream functions
 */


/* === includes ============================================================ */
#include <stdio.h>
#include "ioutil.h"
/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */
int buffer_stream_init( buffer_stream_t *pbs,
                        void (*incb)(buffer_t *pbuf),
                        void (*outcb)(buffer_t *pbuf))
{
FILE *f;
    f = &(pbs->bstream);
    pbs->pbufin = NULL;
    pbs->pbufout = NULL;
    pbs->incb = incb;
    pbs->outcb = outcb;
    fdev_setup_stream ( f,
                        buffer_stream_putchar,
                        buffer_stream_getchar,
                        _FDEV_SETUP_RW);
    fdev_set_udata(f, pbs);

    return 0;
}

int buffer_stream_putchar(char c, FILE *f)
{
buffer_stream_t *pbs;

    pbs = fdev_get_udata(f);
    buffer_append_char(pbs->pbufout, c);
    if( pbs->outcb != NULL)
    {
        pbs->outcb(pbs->pbufout);
    }
    return EOF;
}


int buffer_stream_getchar(FILE *f)
{
buffer_stream_t *pbs;
int c;
    pbs = fdev_get_udata(f);
    if( pbs->incb != NULL)
    {
        pbs->incb(pbs->pbufin);
    }
    c = buffer_get_char(pbs->pbufin);
    return c;
}

