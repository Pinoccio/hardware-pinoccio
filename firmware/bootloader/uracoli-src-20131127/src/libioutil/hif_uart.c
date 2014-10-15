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
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"
#include "ioutil.h"
#include "hif_uart.h"

#if HIF_TYPE_IS_UART

/* === macros ============================================ */
#ifndef UART_TXBUFSIZE
#define UART_TXBUFSIZE (128)
#endif

#ifndef UART_RXBUFSIZE
#define UART_RXBUFSIZE (128)
#endif


#define TXBUF_MASK (UART_TXBUFSIZE-1)


#define RXBUF_MASK (UART_RXBUFSIZE-1)


/* === globals =========================================== */
/* temporary uart buffers */
static volatile struct{
    uint8_t buf[UART_RXBUFSIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
}rx;

static volatile struct{
    uint8_t buf[UART_TXBUFSIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
}tx;

static volatile uint8_t rxovf = 0;

/* === prototypes ======================================== */

/* === functions ========================================= */
void hif_init(const uint32_t baudrate)
{
    HIF_UART_INIT(baudrate);
    HIF_UART_TXIRQ_DI();
    HIF_IO_ENABLE();
    tx.head = tx.tail = 0;
    rx.head = rx.tail = 0;
}

void hif_puts_p(const char *progmem_s)
{
    register char c;
    int rv;
    while ( (c = pgm_read_byte(progmem_s++)) )
    {
        do
        {
            rv = hif_putc(c);
        }
        while(rv != c);
    }
}


void hif_puts(const char *s )
{
#if 0
uint8_t len, tmp, *p;
#endif

int c, rv;

#if 0
    len = strlen(s);
    p = (uint8_t *)s;
    tmp = 0;
    do
    {

        tmp = hif_put_blk(p, len);
        p += tmp;
        len -=tmp;
        if (len)
        {
            DELAY_US(2000);
            LED_TOGGLE(0);
        }
    }
    while(len > 0);
#else
    while(*s != 0)
    {
        c = *s++;
        do
        {
            rv = hif_putc(c);
        }
        while(rv != c);
    }
#endif
}

/*
 * Circular memcpy to HIF Buffer
 */
uint8_t hif_put_blk(unsigned char *data, uint8_t size)
{
uint8_t free_size, retsize;
uint8_t newhead, currtail, currhead, b1=0, b2=0;

    uint8_t __sreg = SREG; cli();
    /* compute space in uart_tx buffer */
    currtail = tx.tail;
    currhead = tx.head;
    newhead = ((currtail - 1) & TXBUF_MASK);

    if (newhead == currhead)
    {
        /* no space left in circular buffer */
        b1 = 0;
        b2 = 0;
    }
    else if (currhead > newhead)
    {
        b1 = ((UART_TXBUFSIZE-currhead) & TXBUF_MASK);
        b2 = newhead;
    }
    else if (currhead < newhead)
    {
        b1 = ((newhead - currhead) & TXBUF_MASK);
        b2 = 0;
    }
    free_size = b1 + b2;
    retsize =  (size > free_size) ? free_size : size;

    /* handle block 1*/
    b1 = (b1 > size) ? size : b1;
    if (b1 > 0)
    {
        memcpy((void*)&tx.buf[currhead], data, b1);
        data += b1;
        tx.head = ((currhead + b1) & TXBUF_MASK);
        size = ((size - b1)  & TXBUF_MASK);
    }

    /* handle block 2*/
    b2 = (size > b2) ?  b2 : size;
    if(b2 > 0)
    {
        memcpy((void*)&tx.buf[0], data, b2);
        tx.head = (b2);
    }

    HIF_UART_TXIRQ_EI();

    SREG = __sreg;

    return retsize;

}


int hif_putc(int c)
{
uint8_t newhead;
    newhead = ((tx.head + 1) & TXBUF_MASK);

    if (newhead != tx.tail)
    {
        tx.buf[tx.head] = (uint8_t)c;
        tx.head = newhead;
        HIF_UART_TXIRQ_EI();
        return c;
    }
    else
    {
        return EOF;
    }
}


int hif_getc(void)
{
int ret = EOF;
    if (rx.tail != rx.head)
    {
        ret = rx.buf[rx.tail];
        rx.tail = ((rx.tail + 1) & RXBUF_MASK);
    }else{
        ret=EOF;
    }
    return ret;
}


uint8_t hif_get_blk(unsigned char *data, uint8_t max_size)
{

uint8_t used_size, retsize;
/**** not used, please check and remove
uint8_t newtail;
****/
uint8_t currtail, currhead, b1=0, b2=0;

    uint8_t __sreg = SREG; cli();
    /* compute space in uart_tx buffer */
    currtail = rx.tail;
    currhead = rx.head;

    /**** not used, please check and remove
    newtail = ((currhead - 1) & RXBUF_MASK);
     ****/

    if (currtail == currhead)
    {
        /* no bytes in circular buffer */
        b1 = 0;
        b2 = 0;
    }
    else if (currtail > currhead)
    {
        b1 = ((UART_RXBUFSIZE-currtail) & RXBUF_MASK);
        b2 = currhead;
    }
    else if (currtail < currhead)
    {
        b1 = ((currhead - currtail) & RXBUF_MASK);
        b2 = 0;
    }
    used_size = b1 + b2;
    retsize =  (max_size > used_size) ? used_size : max_size;

    /* handle block 1*/
    b1 = (b1 > max_size) ? max_size : b1;
    if (b1 > 0)
    {
        memcpy(data, (void*)&rx.buf[currtail],  b1);
        data += b1;
        rx.tail = ((currtail + b1) & RXBUF_MASK);
        max_size = ((max_size - b1)  & RXBUF_MASK);
    }

    /* handle block 2*/
    b2 = (max_size > b2) ?  b2 : max_size;
    if(b2 > 0)
    {
        memcpy(data, (void*)&rx.buf[0], b2);
        rx.tail = (b2);
    }

    SREG = __sreg;

    return retsize;

}

#if defined(DOXYGEN)
void HIF_UART_RX_vect();
#else
ISR(HIF_UART_RX_vect)
#endif
{
    /** todo handle other uart errors (usr register)*/
    if (rx.head == rx.tail)
    {
        rxovf++;
    }
    rx.buf[rx.head] = HIF_UART_DATA;
    rx.head = ((rx.head + 1) & RXBUF_MASK);
}

#if defined(DOXYGEN)
 void HIF_UART_TX_vect();
#else
ISR(HIF_UART_TX_vect)
#endif
{
    /** @todo handle uart errors */

#if 0
    if (tx.head != tx.tail)
    {
        HIF_UART_DATA = tx.buf[tx.tail];
        tx.buf[tx.tail] = '#';
        tx.tail = ((tx.tail + 1) & TXBUF_MASK);
    }
    else
    {
        HIF_UART_TXIRQ_DI();
    }
#else
    HIF_UART_DATA = tx.buf[tx.tail];
    tx.buf[tx.tail] = '#';
    tx.tail = ((tx.tail + 1) & TXBUF_MASK);
    if (tx.head == tx.tail) /*last byte was send, stop */
    {
        HIF_UART_TXIRQ_DI();
    }


#endif
}
#endif
