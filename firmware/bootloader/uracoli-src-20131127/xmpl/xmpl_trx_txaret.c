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
/* Example for transmitting frames in tx_aret mode */

#include "board.h"
#include "transceiver.h"
#include "ioutil.h"
#include "xmpl.h"

static volatile bool tx_in_progress;
static volatile uint8_t tx_cnt, fail_cnt;

#define SEQ_OFFSET     (2)
#define TX_FAIL_OFFSET (7)
#define TX_SRAM_OFFSET (1)

int main(void)
{
trx_regval_t rval;
uint8_t txfrm[] = {0x21,0x08,                /* IEEE 802.15.4 FCF: 
                                                data frame with ack request */
                   42,                       /* sequence counter */
                   (PANID & 0xff), (PANID >> 8), /* destination PAN_ID */
                   (SHORT_ADDR & 0xff), (SHORT_ADDR >> 8), /* dest. short address */
                   42,                       /* TX fail counter */
                   'h','e','l','l','o',' ','µ','r','a','c','o','l','i','!', /* data */
                   'X','X'    /* these bytes are overwritten from transceivers
                               * CRC generator directly before sent. */
                  };
    /* This will stop the application before initializing the radio transceiver
     * (ISP issue with MISO pin, see FAQ)
     */
    trap_if_key_pressed();

    /* Step 0: init MCU peripherals */
    LED_INIT();
    trx_io_init(SPI_RATE_1_2);
    LED_SET_VALUE(LED_MAX_VALUE);
    LED_SET_VALUE(0);

    /* Step 1: initialize the transceiver */
    TRX_RESET_LOW();
    TRX_SLPTR_LOW();
    DELAY_US(TRX_RESET_TIME_US);
    TRX_RESET_HIGH();
    trx_reg_write(RG_TRX_STATE,CMD_TRX_OFF);
    DELAY_MS(TRX_INIT_TIME_US);
    rval = trx_bit_read(SR_TRX_STATUS);
    ERR_CHECK(TRX_OFF!=rval);
    LED_SET_VALUE(1);

    /* Step 2: setup transmitter
     * - configure radio channel
     * - enable transmitters automatic crc16 generation
     * - go into TX state,
     * - enable "transmit end" IRQ
     */
    trx_bit_write(SR_CHANNEL,CHANNEL);
    trx_bit_write(SR_TX_AUTO_CRC_ON,1);
    trx_reg_write(RG_TRX_STATE,CMD_TX_ARET_ON);
#if defined(TRX_IRQ_TRX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_TRX_END);
#elif defined(TRX_IRQ_TX_END)
    trx_reg_write(RG_IRQ_MASK,TRX_IRQ_TX_END);
#else
#  error "Unknown IRQ bits"
#endif
    sei();
    LED_SET_VALUE(2);

    /* Step 3: send a frame each 500ms */
    tx_cnt = 0;
    tx_in_progress = false;
    LED_SET_VALUE(0);

    while(1)
    {
        WAIT500MS();
        if (tx_in_progress == false)
        {
            txfrm[SEQ_OFFSET] = tx_cnt;
            txfrm[TX_FAIL_OFFSET] = fail_cnt;
            trx_frame_write (sizeof(txfrm), txfrm);
            tx_in_progress = true;
            TRX_SLPTR_HIGH();
            TRX_SLPTR_LOW();
            LED_SET(1);
            LED_TOGGLE(0);
        }
    }
}

#if defined(TRX_IF_RFA1)
ISR(TRX24_TX_END_vect)
{
static volatile trx_regval_t trac_status;

    trac_status = trx_bit_read(SR_TRAC_STATUS);

    tx_in_progress = false;
    if (trac_status != TRAC_SUCCESS)
    {
        fail_cnt++;
    }
    else
    {
        tx_cnt ++;
        LED_CLR(1);
    }
}
#else  /* !RFA1 */
ISR(TRX_IRQ_vect)
{
static volatile trx_regval_t irq_cause;
static volatile trx_regval_t trac_status;

    irq_cause = trx_reg_read(RG_IRQ_STATUS);
    trac_status = trx_bit_read(SR_TRAC_STATUS);

    if (irq_cause & TRX_IRQ_TRX_END)
    {
        tx_in_progress = false;
        if (trac_status != TRAC_SUCCESS)
        {
            fail_cnt++;
        }
        else
        {
            tx_cnt ++;
            LED_CLR(1);
        }
    }
}
#endif  /* RFA1 */

/* EOF */
