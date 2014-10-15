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
 * This file is a copy of the file hif_rf230.c, which is part of
 * Atmels software package "IEEE 802.15.4 MAC for AVR Z-Link"
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
 * @brief SRAM access functionds for AT86RF230.
 *
 * This functions are moved to this module, in order to
 * make librf230 more scaleable.
 *
 *
 */


/* === Includes ============================================================ */
#include "board.h"
#include "transceiver.h"
#include <stdlib.h>
#include <avr/io.h>


#if !defined(TRX_IF_RFA1)
/* === Globals ============================================================= */

/* === Prototypes ========================================================== */

/* === Implementation ====================================================== */

/* === internal functions ================================================== */


void trx_sram_write(trx_ramaddr_t addr, uint8_t length, uint8_t *data)
{

    // Select transceiver
    SPI_SELN_LOW();

    SPI_DATA_REG = TRX_CMD_SW;
    SPI_WAITFOR();
    SPI_DATA_REG = addr;
    SPI_WAITFOR();
    do
    {
        SPI_DATA_REG = *data++;
        SPI_WAITFOR();
    }
    while (--length > 0);

    // Deselect Slave
    SPI_SELN_HIGH();
}


void trx_sram_read(trx_ramaddr_t addr, uint8_t length, uint8_t *data)
{

    // Select transceiver
    SPI_SELN_LOW();

    SPI_DATA_REG = TRX_CMD_SR;
    SPI_WAITFOR();
    SPI_DATA_REG = addr;
    SPI_WAITFOR();
    do
    {
        SPI_DATA_REG = 0;   /* dummy out */
        SPI_WAITFOR();
        *data++ = SPI_DATA_REG;
    }
    while (--length > 0);

    // Deselect Slave
    SPI_SELN_HIGH();
}

#endif /* #if !defined(TRX_IF_RFA1) */
/* EOF */
