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
/* Example use of the LED macros */

#include "board.h"
#include "ioutil.h"
#include "xmpl.h"

#ifdef NO_LEDS
# error "No LED Interface defined"
#endif

int main(void)
{
    volatile uint8_t c,x;

    LED_INIT();

    /* step 1: test the LED_TOGGLE macro */
    LED_SET_VALUE(0);
    WAIT_MS(500);
    for (c=0; c<LED_NUMBER; c++)
    {
        for (x=0; x<16; x++)
        {
            LED_TOGGLE(c);
            WAIT_MS(100);
        }
    }

    /* step 2: set each individual LED to on */
    LED_SET_VALUE(0);
    WAIT_MS(500);
    for (c=0; c<LED_NUMBER; c++)
    {
        LED_SET(c);
        WAIT_MS(500);
    }

    /* step 3: set each individual LED to off */
    for (c=0; c<LED_NUMBER; c++)
    {
        LED_CLR(c);
        WAIT_MS(500);
    }

    /* step 4: dislay BCD coded numbers from 0 to LED_MAX_VALUE */
    LED_SET_VALUE(0);
    WAIT_MS(500);
    for (c=0; c<=LED_MAX_VALUE;c++)
    {
        LED_SET_VALUE(c);
        WAIT_MS(500);
    }

    /* step 5: terminate in endless blinking loop */
    do
    {
        LED_SET_VALUE(0x55);
        WAIT_MS(500);
        LED_SET_VALUE(0xaa);
        WAIT_MS(500);
    }while(1);
}
/* EOF */
