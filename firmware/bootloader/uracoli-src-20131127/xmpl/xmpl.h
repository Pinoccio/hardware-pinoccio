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
/**
 * @file
 * @brief Example Include File.
 */
#ifndef XMPL_H
#define XMPL_H

/* === includes ============================================================ */
#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "ioutil.h"
#include "radio.h"
#include "transceiver.h"
/* === macros ============================================================== */
#ifndef CHANNEL
/** radio channel */
# if defined(TRX_SUPPORTS_BAND_800)
#  define CHANNEL    (0)
# elif defined(TRX_SUPPORTS_BAND_900) && defined(REGION_USA)
#  define CHANNEL    (5)
# elif defined(TRX_SUPPORTS_BAND_2400)
#  define CHANNEL    (17)
# else
#  error "No supported frequency band found"
# endif
#endif

/** Default PAN ID. */
#define PANID      (0xcafe)
/** Default short address. */
#define SHORT_ADDR (0xbabe)

#define ERR_CHECK(x) do{}while(x)

/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif


static inline void WAIT_MS(uint16_t t)
{
    while (t--) DELAY_MS(1);
}

#define WAIT500MS() WAIT_MS(500)


static inline void ERR_CHECK_DIAG(bool test, char code)
{
uint8_t i;
    if(test)
    {
        do
        {
            LED_SET_VALUE(0);
            WAIT_MS(500);

            for(i=0;i<code;i++)
            {
                LED_SET(0);
                WAIT_MS(150);

                LED_CLR(0);
                WAIT_MS(400);
            }
            LED_SET_VALUE(0);
            WAIT_MS(500);
        }
        while(1);
    }
    LED_SET_VALUE(code);
}

#endif  /* #ifndef XMPL_H */
