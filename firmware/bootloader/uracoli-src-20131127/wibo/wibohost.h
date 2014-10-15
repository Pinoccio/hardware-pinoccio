/* Copyright (c) 2010 Axel Wachtler
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
 * @brief Wireless bootloader application, resides in bootloader section
 *
 * @author Daniel Thiele
 *         Dietzsch und Thiele PartG
 *         Bautzner Str. 67 01099 Dresden
 *         www.ib-dt.de daniel.thiele@ib-dt.de
 *
 * @ingroup grpAppWiBo
 */
#ifndef WIBOHOST_H_
#define WIBOHOST_H_

#include <stdint.h>
#include <board.h>
#include "p2p_protocol.h"

#define PINGTIMEOUT_MS MSEC(50)

/* 
 * Cycle time for page write operations
 *
 * AVR typcially: 
 *  Flash write: min 3.7ms, max 4.5ms
 *  Page erase: min 7.3ms, max 8.9ms
 */
#define FLASHTIMEOUT_MS MSEC(20)

void wibohost_init(void);

void cb_wibohost_radio_error(radio_error_t err);
void cb_wibohost_pingreply(p2p_ping_cnf_t *rp);
void cb_wibohost_flashcycletimeout(void);
void cb_wibohost_pingtimeout();
void cb_wibohost_tx_done(radio_tx_done_t status);

void wibohost_sendcommand(uint16_t dst_addr, uint8_t cmdcode,
		uint8_t *data, uint8_t lendata);
void wibohost_ping(uint16_t short_addr);
void wibohost_addr(uint16_t short_addr, uint32_t flash_addr);
void wibohost_deaf(uint16_t short_addr);
void wibohost_ping_reply(uint16_t pingaddr);
uint8_t wibohost_pingreplied(void);
void wibohost_target(uint16_t short_addr, uint8_t targmem);
void wibohost_feed(uint16_t short_addr, uint8_t *data, uint8_t lendata);
void wibohost_finish(uint16_t short_addr);
void wibohost_reset(void);
void wibohost_bootlup(uint16_t short_addr);
void wibohost_exit(uint16_t short_addr);
uint16_t wibohost_getcrc(void);
void wibohost_jbootl(uint16_t short_addr);
node_config_t *wibohost_getnodecfg(void);

#endif /* WIBOHOST_H_ */
