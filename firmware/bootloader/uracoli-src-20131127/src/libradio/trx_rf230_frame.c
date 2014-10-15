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

/*
 * ===========================================================================
 * This file contains refactored code from hif_rf230.c,
 * which is part of Atmels software package "IEEE 802.15.4 MAC for AVR Z-Link"
 * ===========================================================================
 */
/*
 * Copyright (c) 2006, Atmel Corporation All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of ATMEL may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


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
#include <avr/io.h>
#include <avr/interrupt.h>

#if !defined(TRX_IF_RFA1)

/* === macros ============================================ */

/* === types ============================================= */

/* === globals =========================================== */

/* === prototypes ======================================== */

/* === functions ========================================= */

void trx_frame_write(uint8_t length, uint8_t *data)
{

    // Select transceiver
    SPI_SELN_LOW();

    SPI_DATA_REG = TRX_CMD_FW;
    SPI_WAITFOR();
    SPI_DATA_REG = length;

    do
    {
        SPI_WAITFOR();
        SPI_DATA_REG = *data++;
    }
    while (--length > 0);

    SPI_WAITFOR(); /* wait here until last byte is out -
                    * otherwise underrun irq */

    // Deselect Slave
    SPI_SELN_HIGH();
}

uint8_t trx_frame_read(uint8_t *data, uint8_t datasz, uint8_t *lqi)
{
    uint8_t length = 0;
    uint8_t i;

    /* Select transceiver */
    SPI_SELN_LOW();

    /* start frame read */
    SPI_DATA_REG = TRX_CMD_FR;
    SPI_WAITFOR();

    /* read length */
    SPI_DATA_REG = 0;
    SPI_WAITFOR();
    length = SPI_DATA_REG;

    if (length <= datasz)
    {
        i = length;
        do
        {
            SPI_DATA_REG = 0;   /* dummy out */
            SPI_WAITFOR();
            *data++ = SPI_DATA_REG;
        }
        while(--i);

        if (lqi!= NULL)
        {
            SPI_DATA_REG = 0;   /* dummy out */
            SPI_WAITFOR();
            *lqi = SPI_DATA_REG;
        }
    }
    else
    {
        /* we drop the frame */
        length = 0x80 | length;
    }
    SPI_SELN_HIGH();
    return length;
}

#endif /* #if !defined(TRX_IF_RFA1) */
/* EOF */
