/* Copyright (c) 2009 Axel Wachtler
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
 * @brief ....
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */
#include <string.h>
#include <stdio.h>
#include "ioutil.h"
/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */

buffer_t * buffer_init(void * pmem, uint8_t size, uint8_t start)
{
buffer_t *pbuf;

    pbuf = (buffer_t*) pmem;
    pbuf->next = NULL;
    pbuf->used = 1;
    pbuf->len = size - sizeof(buffer_t);
    pbuf->istart =  pbuf->iend = start;
    return pbuf;
}

int buffer_append_char(buffer_t *b, uint8_t c)
{

int ret = (int)c;
    if (b->iend < b->len)
    {
        b->data[b->iend++] = c;
    }
    else
    {
        ret = EOF;
    }
    return ret;
}

int buffer_prepend_char(buffer_t *b, int c)
{
int ret = c;

    if (b->istart > 0)
    {
        b->istart--;
        b->data[b->istart] = c;
    }
    else
    {
        ret = EOF;
    }
    return ret;
}

int buffer_get_char(buffer_t *b)
{
int ret;

    ret = EOF;
    if (b->istart <= b->iend)
    {
        ret = (int) b->data[b->istart++];
    }
    return ret;
}

uint8_t buffer_append_block(buffer_t *b, void *pdata, uint8_t size)
{

uint8_t bsz;
    bsz = BUFFER_FREE_AT_END(b);
    if (bsz < size)
    {
        size = bsz;
    }
    memcpy(&(b->data[b->iend]),pdata, size);
    b->iend += size;
    return size;
}

uint8_t buffer_prepend_block(buffer_t *b, void *pdata, uint8_t size)
{

uint8_t bsz;
    bsz = BUFFER_FREE_AT_START(b);
    if (bsz < size)
    {
        size = bsz;
    }
    b->istart -= size;
    memcpy(&(b->data[b->istart]),pdata, size);
    return size;
}

uint8_t buffer_get_block(buffer_t *b, void *pdata, uint8_t size)
{

uint8_t bsz;
    bsz = BUFFER_SIZE(b);
    if (bsz < size)
    {
        size = bsz;
    }
    memcpy(pdata, &(b->data[b->istart]), size);
    b->istart += size;
    return size;
}


buffer_pool_t * buffer_pool_init(uint8_t *pmem, size_t memsz, uint8_t bsz)
{
buffer_pool_t *p;
buffer_t *pbuf;
size_t bufsize, bufidx;

    bufsize = (memsz - sizeof(buffer_pool_t));
    p = (buffer_pool_t *) pmem;
    p->elsz = BUFFER_ELSZ(bsz);
    p->nb = 0;
    bufidx = 0;
    while (bufsize >= (bufidx + p->elsz))
    {
        p->nb ++;
        pbuf = (buffer_t*) (&p->pool[bufidx]);
        pbuf->next = NULL;
        pbuf->used = 0;
        pbuf->len = bsz;
        bufidx += p->elsz;
    }

    return p;
}

buffer_t * buffer_alloc(buffer_pool_t *ppool, uint8_t istart)
{
buffer_t *pbuf, *ret = NULL;
uint8_t i;
size_t poolidx;

    poolidx = 0;
    for(i=0;i<ppool->nb;i++)
    {
        pbuf = (buffer_t*)&ppool->pool[poolidx];
        if (pbuf->used == 0)
        {
            pbuf->used = 1;
            pbuf->istart = pbuf->iend = istart;
            ret = pbuf;

            break;
        }
        poolidx += ppool->elsz;
    }
    return ret;
}


void buffer_free(buffer_t * pbuf)
{
    pbuf->istart = pbuf->iend = 0;
    pbuf->used = 0;
    return;
}
