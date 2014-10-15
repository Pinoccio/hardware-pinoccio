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
/* Example use of the radio and ioutil functions for a simple range test */

#include <stdio.h>
#include "board.h"
#include "hif.h"
#include "radio.h"
#include "xmpl.h"


/* === Macros ========================================================== */
#define T_TX_PERIOD MSEC(500)
#define RT_ADDR (0x1234)
#define RT_PANID (0xcafe)
#define GET_CFG_ADDR() (0xfffc)
#define ABSDIFF(a,b) ( (a>b) ? a - b : b - a )
#define FRAME_CTRL (0x8841)
/* === Types =========================================================== */
time_t tmr_transmit(timer_arg_t t);

typedef struct
{
    uint16_t fctl;
    uint8_t  seq;
    uint16_t pan;
    uint16_t dst;
    uint16_t src;
    /* frame payload */
    uint8_t  keycnt;
    uint8_t  data[8];
    uint16_t crc;
} rt_frame_t;

/* === Globals ========================================================= */
rt_frame_t TxFrame = {
        .fctl = FRAME_CTRL,
        .pan = RT_PANID,
        .dst = RT_ADDR,
        .src = 0xffff
};
uint8_t RxFrame[MAX_FRAME_SIZE];
volatile bool do_init = true;
volatile uint8_t keycnt = 0;




/* === Implementation ================================================== */

void app_init(void)
{
    LED_INIT();
    KEY_INIT();
    hif_init(HIF_DEFAULT_BAUDRATE);
    timer_init();
    radio_init(RxFrame, sizeof(RxFrame));
    radio_set_state(STATE_OFF);
    radio_set_param(RP_CHANNEL(CHANNEL));
    radio_set_param(RP_IDLESTATE(STATE_RXAUTO));
    radio_set_param(RP_SHORTADDR(RT_ADDR));
    radio_set_param(RP_PANID(RT_PANID));
#if RADIO_TYPE == RADIO_AT86RF212
    radio_set_param(RP_DATARATE(OQPSK100));
#endif
}


int main(void)
{
node_config_t nc = {.short_addr = 0, .pan_id = 0};

    /* setup hardware */
    app_init();

    /* copy flash settings */
    get_node_config(&nc);
    TxFrame.src = nc.short_addr;
    sei();

    PRINTF("Simple Range Test\n\r This is node: 0x%04x TX-FRAME: %d\n\r",
           nc.short_addr, sizeof(TxFrame));
    PRINTF("PWR: 0x%x CHAN: %d\r\n",
            trx_reg_read(RG_PHY_TX_PWR), trx_bit_read(SR_CHANNEL));

    do_init = true;
    timer_start(tmr_transmit,T_TX_PERIOD,0);

    while(1)
    {
        DELAY_MS(10);
        if( keys_debounced() )
        {
            LED_TOGGLE(0);
            keycnt ++;
        }

    }
}

time_t tmr_transmit(timer_arg_t t)
{
    radio_set_state(STATE_TXAUTO);
    TxFrame.keycnt = keycnt;
    radio_send_frame(sizeof(TxFrame) , (uint8_t *)&TxFrame, 0);
    LED_TOGGLE(0);
    return 0;
}

uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc)
{

rt_frame_t *pfrm;

static struct rxstatus_tag
{
    uint8_t  seq;
    uint16_t lostpckts;
    uint16_t rxdpckts;
    uint16_t src;
    uint8_t  keycnt;
    uint8_t lqi;
    int8_t ed;

} rxstatus;

bool doprint;

    LED_TOGGLE(1);

    pfrm = (rt_frame_t *) frm;
    doprint = false;

    /* verify if one of the variables has changed. */


    if (do_init == true)
    {
        do_init = false;
        rxstatus.seq = pfrm->seq;
        rxstatus.rxdpckts = 0;
        rxstatus.lostpckts = 0;

    }

    rxstatus.rxdpckts ++;

    if(pfrm->seq != rxstatus.seq)
    {
        /* improve lost packet counting with timestamp */
        rxstatus.lostpckts += ABSDIFF(pfrm->seq, rxstatus.seq);
        doprint = true;
    }

    if(pfrm->src != rxstatus.src)
    {
        rxstatus.src = pfrm->src;
        doprint = true;
    }
    if(pfrm->keycnt != rxstatus.keycnt)
    {
        rxstatus.keycnt = pfrm->keycnt;
        doprint = true;
    }
    if(lqi != rxstatus.lqi)
    {
        rxstatus.lqi = lqi;
        doprint = true;
    }
    if(ed != rxstatus.ed)
    {
        rxstatus.ed = ed;
        doprint = true;
    }

    if (doprint == true)
    {
        PRINTF(":sta 0x%04x seq %d key %d lqi %d ed %d rxd %d lost %d \n\r",
                pfrm->src, pfrm->seq, pfrm->keycnt, lqi, ed,
                rxstatus.rxdpckts, rxstatus.lostpckts );

        rxstatus.rxdpckts = 0;
        rxstatus.lostpckts = 0;
    }

    rxstatus.seq = pfrm->seq + 1;

    return frm;
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    LED_TOGGLE(0);
    if (status == TX_OK)
    {
        TxFrame.seq++;
    }
    timer_start(tmr_transmit, T_TX_PERIOD, 0);
}
/* XEOF */
