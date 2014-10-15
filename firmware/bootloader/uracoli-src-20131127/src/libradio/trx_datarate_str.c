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
 * @brief Module for providing string translations of datarates.
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */
#include "board.h"
#include "transceiver.h"
#include <avr/pgmspace.h>

/* === macros ============================================================== */
#define FLASHSTR_RATE(x) const char C_##x##_STR [] PROGMEM = x##_STR

/* this is the clumsy stuff, found no elegant way with less typing
   effort */
#ifdef TRX_BPSK20
    FLASHSTR_RATE(BPSK20);
#endif
#ifdef TRX_BPSK40
    FLASHSTR_RATE(BPSK40);
#endif
#ifdef TRX_OQPSK100
    FLASHSTR_RATE(OQPSK100);
#endif
#ifdef TRX_OQPSK200
    FLASHSTR_RATE(OQPSK200);
#endif
#ifdef TRX_OQPSK250
    FLASHSTR_RATE(OQPSK250);
#endif
#ifdef TRX_OQPSK400
    FLASHSTR_RATE(OQPSK400);
#endif
#ifdef TRX_OQPSK500
    FLASHSTR_RATE(OQPSK500);
#endif
#ifdef TRX_OQPSK1000
    FLASHSTR_RATE(OQPSK1000);
#endif
#ifdef TRX_OQPSK2000
    FLASHSTR_RATE(OQPSK2000);
#endif

/**
 * table with data rate hash values
 * the hash value maps a string, e.g. "OQPSK250" to 8 bit
 * value, in this case 0x33. The hash value is generated
 * from the script Tools/cmdhash.py or from the function
 * get_cmd_hash()
 */
const uint8_t rate_hshtable[] PROGMEM =
{
# ifdef TRX_BPSK20
    BPSK20,
# endif
# ifdef TRX_BPSK40
    BPSK40,
# endif
# ifdef TRX_OQPSK100
    OQPSK100,
# endif
# ifdef TRX_OQPSK200
    OQPSK200,
# endif
# ifdef TRX_OQPSK250
    OQPSK250,
# endif
# ifdef TRX_OQPSK400
    OQPSK400,
# endif
# ifdef TRX_OQPSK500
    OQPSK500,
# endif
# ifdef TRX_OQPSK1000
    OQPSK1000,
# endif
# ifdef TRX_OQPSK2000
    OQPSK2000,
# endif
};



PGM_P const rate_strtable[] PROGMEM =
{
# ifdef TRX_BPSK20
    C_BPSK20_STR,
# endif
# ifdef TRX_BPSK40
    C_BPSK40_STR,
# endif
# ifdef TRX_OQPSK100
    C_OQPSK100_STR,
# endif
# ifdef TRX_OQPSK200
    C_OQPSK200_STR,
# endif
# ifdef TRX_OQPSK250
    C_OQPSK250_STR,
# endif
# ifdef TRX_OQPSK400
    C_OQPSK400_STR,
# endif
# ifdef TRX_OQPSK500
    C_OQPSK500_STR,
# endif
# ifdef TRX_OQPSK1000
    C_OQPSK1000_STR,
# endif
# ifdef TRX_OQPSK2000
    C_OQPSK2000_STR,
# endif

};

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */
uint8_t trx_decode_datarate(uint8_t rhash, char * rstr, uint8_t nlen)
{
uint8_t ret, i,tmp;

    ret = 255;
    if(rstr != NULL)
    {
        for(i=0;i<sizeof(rate_hshtable)/sizeof(uint8_t);i++)
        {
             tmp = pgm_read_byte(rate_hshtable+i);
             if (tmp == rhash)
             {
                  ret = 0;
                  break;
             }
        }

        trx_get_datarate_str(i, rstr, nlen);
    }
    return ret;
}


void * trx_decode_datarate_p(uint8_t rhash)
{
    uint8_t i,tmp;
    for(i=0;i<sizeof(rate_hshtable)/sizeof(uint8_t);i++)
    {
         tmp = pgm_read_byte(rate_hshtable+i);
         if (tmp == rhash)
         {
              break;
         }
    }

    return trx_get_datarate_str_p(i);
}


uint8_t trx_get_number_datarates(void)
{
    return sizeof(rate_hshtable)/sizeof(uint8_t);
}


void *  trx_get_datarate_str_p(uint8_t idx)
{
    PGM_P p = NULL;
    if (idx <= sizeof(rate_hshtable)/sizeof(uint8_t))
    {
        memcpy_P(&p, &rate_strtable[idx], sizeof(PGM_P));
    }
    return (void*)p;
}

uint8_t trx_get_datarate_str(uint8_t idx, char * rstr, uint8_t nlen)
{
    uint8_t ret;
    PGM_P p;
    ret = 255;
    if (idx <= sizeof(rate_hshtable)/sizeof(uint8_t))
    {
        memcpy_P(&p, &rate_strtable[idx], sizeof(PGM_P));
        strncpy_P(rstr,p,nlen);
        ret = 0;
    }
    return ret;
}
