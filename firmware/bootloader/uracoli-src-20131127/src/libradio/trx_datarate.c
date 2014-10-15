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
 * @brief Chip dependent implementation of functions for setting and getting
 * the data rate of the transceiver.
 * @_addtogroup grpApp
 */


/* === includes ============================================================ */
#include "board.h"
#include "transceiver.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */

/* === functions =========================================================== */
#if RADIO_TYPE ==  RADIO_AT86RF212
uint8_t trx_set_datarate(uint8_t rate_type)
{
trx_regval_t regval = TRX_NONE;

    switch(rate_type)
    {
        case BPSK20: regval = TRX_BPSK20; break;
        case BPSK40: regval = TRX_BPSK40; break;
        case OQPSK100: regval = TRX_OQPSK100; break;
        case OQPSK200: regval = TRX_OQPSK200; break;
        case OQPSK250: regval = TRX_OQPSK250; break;
        case OQPSK400: regval = TRX_OQPSK400; break;
        case OQPSK500: regval = TRX_OQPSK500; break;
        case OQPSK1000: regval = TRX_OQPSK1000; break;
        default: break;
    }
    if (regval != TRX_NONE)
    {
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        regval |=  trx_reg_read(RG_TRX_CTRL_2) & 0xf0 ;
        trx_reg_write(RG_TRX_CTRL_2, regval);

    }
    else
    {
        rate_type = TRX_NONE;
    }
    return rate_type;
}

uint8_t trx_get_datarate(void)
{
uint8_t rate_type, rate_code;

    rate_code = trx_reg_read(RG_TRX_CTRL_2) & 0x0f;

    switch(rate_code)
    {
        case TRX_BPSK20: rate_type = BPSK20; break;
        case TRX_BPSK40: rate_type = BPSK40; break;
        case TRX_OQPSK100: rate_type = OQPSK100; break;
        case TRX_OQPSK200: rate_type = OQPSK200; break;
        case TRX_OQPSK250: rate_type = OQPSK250; break;
        case TRX_OQPSK400: /* fall through */
        case TRX_OQPSK400_1: rate_type = OQPSK400; break;
        case TRX_OQPSK500: rate_type = OQPSK500; break;
        case TRX_OQPSK1000: /* fall through */
        case TRX_OQPSK1000_1: rate_type = OQPSK1000; break;
        default: rate_type = RATE_NONE; break;
    }
    return rate_type;
}

#elif RADIO_TYPE == RADIO_AT86RF231

uint8_t trx_set_datarate(uint8_t rate_type)
{
trx_regval_t regval = TRX_NONE;

    switch(rate_type)
    {
        case OQPSK250: regval = TRX_OQPSK250; break;
        case OQPSK500: regval = TRX_OQPSK500; break;
        case OQPSK1000: regval = TRX_OQPSK1000; break;
        case OQPSK2000: regval = TRX_OQPSK2000; break;
        default: break;
    }
    if (regval != TRX_NONE)
    {
        trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
        regval |=  trx_reg_read(RG_TRX_CTRL_2) & 0xfc ;
        trx_reg_write(RG_TRX_CTRL_2, regval);
    }
    else
    {
        rate_type = TRX_NONE;
    }
    return rate_type;
}

uint8_t trx_get_datarate(void)
{
uint8_t rate_type, rate_code;

    rate_code = trx_reg_read(RG_TRX_CTRL_2) & 0x03;

    switch(rate_code)
    {
        case TRX_OQPSK250: rate_type = OQPSK250; break;
        case TRX_OQPSK500: rate_type = OQPSK500; break;
        case TRX_OQPSK1000: rate_type = OQPSK1000; break;
        case TRX_OQPSK2000: rate_type = OQPSK2000; break;
        default: rate_type = RATE_NONE; break;
    }
    return rate_type;
}

#else

uint8_t trx_set_datarate(uint8_t rate_type)
{

    if(rate_type != OQPSK250)
    {
        rate_type = TRX_NONE;
    }
    return rate_type;
}

uint8_t trx_get_datarate(void)
{
    return OQPSK250;
}


#endif /* RADIO_TYPE == AT86RF212 */
