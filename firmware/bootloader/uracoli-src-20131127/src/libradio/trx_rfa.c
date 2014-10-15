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
 * @addtogroup grpApp...
 */


/* === includes ========================================== */

#include "board.h"
#include "transceiver.h"
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if defined(TRX_IF_RFA1)

/* === macros ============================================ */

/* === types ============================================= */

/* === globals =========================================== */
volatile uint8_t SHADOW_IRQ_MASK = 0;

/* === prototypes ======================================== */

/* === functions ========================================= */
void trx_io_init (uint8_t spirate)
{
#ifdef TRX_PA_LNA_INIT
    TRX_PA_LNA_INIT();
#endif
}

void trx_set_irq_handler(trx_irq_handler_t irqhandler)
{}

void trx_reg_write(trx_regaddr_t addr, trx_regval_t val)
{
    *(uint8_t*)(TRX_REGISTER_BASEADDR + addr) = val;
}

uint8_t trx_reg_read(trx_regaddr_t addr)
{
    return *(uint8_t*)(TRX_REGISTER_BASEADDR + addr);
}

trx_regval_t trx_bit_read(trx_regaddr_t addr, trx_regval_t mask, uint8_t pos)
{
    return (*(uint8_t*)(TRX_REGISTER_BASEADDR + addr) & mask) >> pos;
}

void trx_bit_write(trx_regaddr_t addr, trx_regval_t mask, uint8_t pos, trx_regval_t value)
{
    trx_regval_t tmp;
    tmp = (*(uint8_t*)(TRX_REGISTER_BASEADDR + addr) & ~mask);
    tmp = tmp | ((value << pos) & mask );
    *(uint8_t*)(TRX_REGISTER_BASEADDR + addr) = tmp;
}

void trx_frame_write(uint8_t length, uint8_t *data)
{
   if (length > 127)
   {
        length = 127;
   }
   TRXFBST = length;
   memcpy( (void*)&TRXFBST+1, data, length);
}

uint8_t trx_frame_read(uint8_t *data, uint8_t datasz, uint8_t *lqi)
{
    uint8_t length;

    length = TST_RX_LENGTH;
    if (length < datasz)
    {
        datasz = length;
    }
    memcpy( data, (void*)&TRXFBST, datasz);
    *lqi = *(&TRXFBST+datasz);
    return length;
}

uint8_t trx_frame_read_crc(uint8_t *data, uint8_t datasz, bool *crc_ok)
{
    if (crc_ok != NULL)
    {
        *crc_ok = (trx_bit_read(SR_RX_CRC_VALID) == 1);
    }
    return trx_frame_read(data, datasz, NULL);
}

uint8_t trx_frame_read_data_crc(uint8_t *data, uint8_t datasz, uint8_t *lqi, bool *crc_ok)
{
    if (crc_ok != NULL)
    {
        *crc_ok = (trx_bit_read(SR_RX_CRC_VALID) == 1);
    }
    return trx_frame_read(data, datasz, lqi);
}

uint8_t trx_frame_get_length(void)
{
    return TST_RX_LENGTH;
}

void trx_sram_write(trx_ramaddr_t addr, uint8_t length, uint8_t *data)
{
    if ((addr + length) > 127)
    {
        length = 127 - addr;
    }
    memcpy( (void*)(&TRXFBST+addr), data, length);
}

void trx_sram_read(trx_ramaddr_t addr, uint8_t length, uint8_t *data)
{
    if ((addr + length) > 127)
    {
        length = 127 - addr;
    }
    memcpy( data, (void*)(&TRXFBST+addr), length);
}
#endif /*if defined(TRX_IF_RFA1) */
/* EOF */
