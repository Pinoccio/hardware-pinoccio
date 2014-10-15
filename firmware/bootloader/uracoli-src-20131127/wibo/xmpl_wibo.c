/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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
 * @brief Wireless bootloader example application
 * It blinks a LED so that you can see the success of your
 * wireless bootloading process. Additionally it receives the
 * command to jump back into bootloader.
 *
 * @author Daniel Thiele
 *         Dietzsch und Thiele PartG
 *         Bautzner Str. 67 01099 Dresden
 *         www.ib-dt.de daniel.thiele@ib-dt.de
 *
 * @ingroup grpAppWiBo
 */

 /* avr-libc inclusions */
#include <avr/io.h>
#include <util/delay.h>

/* uracoli inclusions */
#include <board.h>
#include <ioutil.h>
#include <radio.h>
#include <timer.h>
#include <p2p_protocol.h>


#define _SW_VERSION_ (0x10)
#ifndef BLINKPERIOD_MS
#define BLINKPERIOD_MS (250)
#endif

/* IEEE802.15.4 parameters for communication */
static node_config_t nodeconfig;
static uint8_t rxbuf[128];
static volatile timer_hdl_t thdl;

static p2p_ping_cnf_t pingrep = {
    .hdr.cmd= P2P_PING_CNF,
    .hdr.fcf = 0x8841, 				/* short addressing, frame type: data, no ACK requested */
    .version = _SW_VERSION_,
    .appname = APP_NAME,
    .boardname = BOARD_NAME,
};

void usr_radio_error(radio_error_t err)
{
}

void usr_radio_irq(uint8_t cause)
{
}

uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
        int8_t ed, uint8_t crc_fail)
{
    switch( ((p2p_hdr_t*)frm)->cmd)
    {

    case P2P_JUMP_BOOTL:
        LED_CLR(0);
        LED_CLR(1);
        timer_stop(thdl);
        jump_to_bootloader();
        break;

    case P2P_XMPL_LED:
        if(((p2p_xmpl_led_t*)frm)->state){
            LED_SET(((p2p_xmpl_led_t*)frm)->led);
        }else{
            LED_CLR(((p2p_xmpl_led_t*)frm)->led);
        }
        break;

    case P2P_PING_REQ:
        pingrep.hdr.dst = ((p2p_hdr_t*) frm)->src;
        radio_set_state(STATE_TXAUTO);
        radio_send_frame(sizeof(p2p_ping_cnf_t) + sizeof(BOARD_NAME)+ 2,
                         (uint8_t*)(&pingrep), 1);
        LED_TOGGLE(1);
        break;

    default :
        /* unknown or unhandled command */
        break;
    };
    return frm;
}

void usr_radio_tx_done(radio_tx_done_t status)
{
    if (TX_OK == status) {
    } else {
        /* TODO handle error */
    }
}

/*
 * \brief
 */
time_t led_toggle(timer_arg_t t)
{
	LED_TOGGLE(0);
	return MSEC(BLINKPERIOD_MS);
}

int main()
{
#if defined(NODECONFIG_STATIC)
	/* for development purposes */
	nodeconfig.channel = 11;
	nodeconfig.pan_id = 0x01;
	nodeconfig.short_addr = 0x05;
#elif BOARD_TYPE == BOARD_PINOCCIO
	nodeconfig.channel = eeprom_read_byte((uint8_t *)8179);
	nodeconfig.pan_id = eeprom_read_word((uint16_t *)8180);
	nodeconfig.short_addr = eeprom_read_word((uint16_t *)8182);
	nodeconfig.ieee_addr = 0;
#else
	get_node_config(&nodeconfig);
#endif

    LED_INIT();
    hif_init(HIF_DEFAULT_BAUDRATE);

    pingrep.hdr.pan = nodeconfig.pan_id;
    pingrep.hdr.src = nodeconfig.short_addr;

    radio_init(rxbuf, 128);

    radio_set_param(RP_CHANNEL(nodeconfig.channel));
    radio_set_param(RP_PANID(nodeconfig.pan_id));
    radio_set_param(RP_SHORTADDR(nodeconfig.short_addr));
    radio_set_param(RP_IDLESTATE(STATE_RXAUTO));

    sei();
    PRINTF("xmpl_wibo_%s.hex channel: %d\n\r", BOARD_NAME, nodeconfig.channel);

    timer_init();
    thdl = timer_start(led_toggle, MSEC(BLINKPERIOD_MS), 0);

    set_sleep_mode(SLEEP_MODE_IDLE);
    for(;;)
    {
        sleep_mode();
    }
}

/* EOF */
