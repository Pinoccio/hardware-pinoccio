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
/* Example for key event processing with a single  key */

#include "ioutil.h"
#include "timer.h"
#include "xmpl.h"

#define SHORT_PRESS_TICKS  (MSEC(100))
#define MEDIUM_PRESS_TICKS (MSEC(300))
#define LONG_PRESS_TICKS   (MSEC(800))

#define NONE_PRESS   (0)
#define SHORT_PRESS  (1)
#define MEDIUM_PRESS (2)
#define LONG_PRESS   (3)
#define CONT_PRESS   (4)

volatile uint8_t KeyEvent = 0;

static uint8_t debounce_key0(void)
{
uint8_t ret, tmp;
static uint16_t ocnt=0, ccnt=0;

    ret = NONE_PRESS;
    tmp = (KEY_GET() & 1);
    if (tmp != 0)
    {
        ocnt ++;
        if(ocnt >= LONG_PRESS_TICKS)
        {
            ocnt = LONG_PRESS_TICKS;
            ccnt++;
            if (ccnt >= MEDIUM_PRESS_TICKS)
            {
                ccnt = 0;
                ret = CONT_PRESS;
            }
        }
    }
    else
    {
        if(ocnt >= LONG_PRESS_TICKS)
        {
            ret = LONG_PRESS;
        }
        else if(ocnt >= MEDIUM_PRESS_TICKS)
        {
            ret = MEDIUM_PRESS;
        }
        else if(ocnt >= SHORT_PRESS_TICKS)
        {
            ret = SHORT_PRESS;
        }
        ccnt = 0;
        ocnt = 0;
    }

    return ret;
}

int main(void)
{
    uint8_t tmp;

    KEY_INIT();
    LED_INIT();
    TIMER_INIT();
    sei();
    while(1)
    {
        if (KeyEvent == SHORT_PRESS)
        {
            LED_TOGGLE(0);
        }

        if (KeyEvent == MEDIUM_PRESS)
        {
            LED_TOGGLE(1);
        }

        if (KeyEvent == LONG_PRESS)
        {
            LED_SET_VALUE(0);
        }
        if (KeyEvent == CONT_PRESS)
        {
            tmp = LED_GET_VALUE();
            tmp ++;
            LED_SET_VALUE(tmp);
        }
        KeyEvent = NONE_PRESS;
        SLEEP_ON_IDLE();
    }
}

ISR(TIMER_IRQ_vect)
{
    if (KeyEvent == NONE_PRESS)
    {
        KeyEvent = debounce_key0();
    }
}
/* EOF */
