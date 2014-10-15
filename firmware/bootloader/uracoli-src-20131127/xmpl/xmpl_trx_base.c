/* Copyright (c) 2008 Axel Wachtler
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
/* Example for accessing the transceiver */

#include "board.h"
#include "transceiver.h"
#include "ioutil.h"
#include "xmpl.h"

static volatile trx_regval_t irq_cause;

int main(void)
{
static volatile trx_regval_t rval;

    /* This will stop the application before initializing the radio transceiver
     * (ISP issue with MISO pin, see FAQ)
     */
    trap_if_key_pressed();

    /* init peripherals */
    LED_INIT();
    trx_io_init(SPI_RATE_1_2);
    LED_SET_VALUE(LED_MAX_VALUE);
    LED_SET_VALUE(0);

    /* Step 1: reset transceiver */
    DELAY_US(TRX_INIT_TIME_US);
    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    LED_SET_VALUE(1);
    DELAY_MS(10);

    /* Step 2: check if correct transceiver is present
     * verify if the part and version ID's are correct.
     */
    rval = trx_reg_read(RG_PART_NUM);
    ERR_CHECK(RADIO_PART_NUM != rval);

    rval = trx_reg_read(RG_VERSION_NUM);
    ERR_CHECK_DIAG(RADIO_VERSION_NUM !=rval,2);


    /* Step 3: try a write command on an arbitrary RW register.
     */
    trx_reg_write(RG_IEEE_ADDR_0,42);
    rval = trx_reg_read(RG_IEEE_ADDR_0);
    ERR_CHECK_DIAG((42!=rval),3);


    /* Step 4: go to state TRX_OFF
     * bring the transceiver in state TRX_OFF and verify it.
     */
    trx_reg_write(RG_TRX_STATE, CMD_TRX_OFF);
    DELAY_US(TRX_INIT_TIME_US);
    DELAY_MS(1000);
    rval = trx_bit_read(SR_TRX_STATUS);
    ERR_CHECK_DIAG((TRX_OFF!=rval),4);

    /* Step 5: check SRAM access */

    /* Step 6: check IRQ
     * enable PLL-LOCK IRQ and transceiver IRQ at MCU
     * switch to PLL_ON state (which causes lock IRQ)
     * wait until the interrupt happened
     */
    trx_reg_write(RG_IRQ_MASK, TRX_IRQ_PLL_LOCK);
    irq_cause = 0;
    sei();

    trx_reg_write(RG_TRX_STATE,CMD_PLL_ON);
    
    DELAY_US(TRX_PLL_LOCK_TIME_US);
    DELAY_MS(200);
    ERR_CHECK_DIAG((0==irq_cause),6);
    /* done */
    LED_SET_VALUE(0);
    while(1)
    {
        WAIT500MS();
        LED_TOGGLE(0);
    }
}

#if defined(TRX_IF_RFA1)
ISR(TRX24_PLL_LOCK_vect)
{
    irq_cause |= TRX_IRQ_PLL_LOCK;
}
#else
ISR(TRX_IRQ_vect)
{
    /* reading IRQ_STATUS acknowledges the interrupt */
    irq_cause |= trx_reg_read(RG_IRQ_STATUS);
    LED_SET(1);
}
#endif

/* EOF */
