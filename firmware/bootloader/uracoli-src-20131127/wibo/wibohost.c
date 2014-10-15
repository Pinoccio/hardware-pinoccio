/* Copyright (c) 2010, 2011, 2012 Daniel Thiele, Axel Wachtler
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
 * @brief Wireless bootloader IEEE802.15.4 communication module
 * Based on radio-layer of uracoli, manage commands located above
 * MAC-Layer
 *
 * For data integrity, a checksum is tracked inside the host as
 * well as on each node taking part in flashing process for every
 * slice of data fed. It can be checked for each node of
 * the network separately.
 *
 *   Polynomial: x^16 + x^12 + x^5 + 1 (0x8408)
 *   This is the CRC used by PPP and IrDA.
 *
 *
 * @author Daniel Thiele
 *         Dietzsch und Thiele PartG
 *         Bautzner Str. 67 01099 Dresden
 *         www.ib-dt.de daniel.thiele@ib-dt.de
 *
 * @ingroup grpAppWiBo
 */

/* avr-libc inclusions */
#include <util/crc16.h>

/* uracoli inclusions */
#include <board.h>
#include <timer.h>
#include <transceiver.h>
#include <radio.h>
#include <p2p_protocol.h>

/* project inclusions */
#include "wibohost.h"

#define APP_VERSION (0x01)
#ifndef APP_NAME
# define APP_NAME    "w-host"
#endif

/* running checksum to calculate over all send data packets (via feed())
 * to be set to zero on start of data stream (reset())
 */
static uint16_t datacrc = 0x0000;

/* collect data to send here */
static uint8_t txbuf[MAX_FRAME_SIZE];

/* frame receive buffer */
static uint8_t rxbuf[MAX_FRAME_SIZE];

static node_config_t nodeconfig;
static volatile timer_hdl_t thdl_ping;
static volatile timer_hdl_t thdl_flashcycle;
static volatile uint8_t last_feed = 0; /* flag if last command was "_feed" */
static volatile uint8_t wait_cmd_ping_cnf = 0;

static const p2p_ping_cnf_t PROGMEM PingReply =
{ .hdr.cmd = P2P_PING_CNF, .hdr.fcf = 0x8841, /* short addressing, frame type: data, no ACK requested */
.version = APP_VERSION, .appname = APP_NAME, .boardname = BOARD_NAME, };

/******************* uracoli callback functions ***************/

/*
 * \brief Timeout for ping request
 */
time_t wibohost_pingtimeout(timer_arg_t t)
{
	cb_wibohost_pingtimeout();
	return 0; /* stop timer */
}

/*
 * \brief Timeout for flash cycle timer
 */
time_t wibohost_flashcycletimeout(timer_arg_t t)
{
	cb_wibohost_flashcycletimeout();
	return 0; /* stop timer */
}

/*
 * \brief Callback of uracoli radio-layer
 */
void usr_radio_error(radio_error_t err)
{
	cb_wibohost_radio_error(err);
}

/*
 * \brief Callback of uracoli radio-layer
 */
void usr_radio_irq(uint8_t cause)
{
	/* empty */
}

/*
 * \brief Callback of uracoli radio-layer
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi,
		int8_t ed, uint8_t crc_fail)
{
	p2p_ping_cnf_t *pr = (p2p_ping_cnf_t*) frm;

	/* decode command code */
	if ( P2P_PING_CNF == pr->hdr.cmd && wait_cmd_ping_cnf)
	{ /* this command is sync */
		timer_stop(thdl_ping);
		cb_wibohost_pingreply(pr);
	}
	else if (P2P_PING_REQ == pr->hdr.cmd) /* this command is async */
	{
		wibohost_ping_reply(pr->hdr.src);
	}
	else if (P2P_JUMP_BOOTL == pr->hdr.cmd)
	{
		timer_stop(thdl_ping);
		timer_stop(thdl_flashcycle);
		cli();
#if defined(SR_RX_SAFE_MODE)
		trx_bit_write(SR_RX_SAFE_MODE, 1);
#endif
		jump_to_bootloader();
	}
	else
	{
		/* unknown command, skip the frame */
	}

#if defined(SR_RX_SAFE_MODE)
	trx_bit_write(SR_RX_SAFE_MODE, 1);
#endif

	return frm;
}

/*
 * \brief Callback of uracoli radio-layer
 */
void usr_radio_tx_done(radio_tx_done_t status)
{
	if (last_feed == 1)
	{
		/* start flash timer */
		thdl_flashcycle = timer_start(wibohost_flashcycletimeout,
				FLASHTIMEOUT_MS, 0);
		last_feed = 0;
	}

	cb_wibohost_tx_done(status);
}

/*
 * \brief Initialize communication layer
 * - Read node configuration from NVRAM
 * - Initialize uracoli radio-layer
 */
void wibohost_init(void)
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
	radio_init(rxbuf, MAX_FRAME_SIZE);
	radio_set_param(RP_CHANNEL(nodeconfig.channel));
	radio_set_param(RP_PANID(nodeconfig.pan_id));
	radio_set_param(RP_SHORTADDR(nodeconfig.short_addr));
	radio_set_param(RP_IDLESTATE(STATE_RXAUTO));

#if defined(SR_RX_SAFE_MODE)
	trx_bit_write(SR_RX_SAFE_MODE, 1);
#endif
}

/*
 * \brief Send a command over the air
 *
 * Construct the header that is the same for all commands and
 * pack into IEEE802.15.4 MPDU
 *
 * @param short_addr IEEE short address
 * @param cmdcode Command code as defined in p2p_protocol.h
 * @param *data Pointer to buffer of Tx frame (header is mapped onto it here)
 * @param lendata Overall length of Tx frame
 */
void wibohost_sendcommand(uint16_t dst_addr, uint8_t cmdcode, uint8_t *data,
		uint8_t lendata)
{
	p2p_hdr_t *hdr = (p2p_hdr_t*) data;

	FILL_P2P_HEADER_NOACK(hdr, nodeconfig.pan_id, dst_addr,
			nodeconfig.short_addr, cmdcode);
	radio_set_state(STATE_TXAUTO);

	radio_send_frame(lendata + 2, data, 1); /* +2: add CRC bytes (FCF) */
}

void wibohost_ping_reply(uint16_t pingaddr)
{
	memcpy_P(txbuf, &PingReply, sizeof(p2p_ping_cnf_t) + sizeof(BOARD_NAME));
	wibohost_sendcommand(pingaddr, P2P_PING_CNF, txbuf,
			sizeof(p2p_ping_cnf_t) + sizeof(BOARD_NAME));
}

/*
 * \brief Feed data to a node
 * This data is part of the flash image that shall be programmed to
 * the target device. It can be sliced to any size from outside. The
 * target device collects it and programs its pages as soon as one page
 * size of data is reached.
 *
 * @param short_addr Address of node to feed (or broadcast 0xFFFF)
 * @param *data Pointer to buffer where data is stored
 * @param lendata Length of buffer
 */
void wibohost_feed(uint16_t short_addr, uint8_t *data, uint8_t lendata)
{
	p2p_wibo_data_t *dat = (p2p_wibo_data_t*) txbuf;
	uint8_t i;

	for (i = 0; i < lendata; i++)
	{
		datacrc = _crc_ccitt_update(datacrc, data[i]);
		dat->data[i] = data[i]; /* no use for memcpy() */
	}
	dat->dsize = lendata;

	wibohost_sendcommand(short_addr, P2P_WIBO_DATA, (uint8_t*) dat,
			sizeof(p2p_wibo_data_t) + lendata);

	last_feed = 1;
}

/*
 * \brief Set programming target of a node
 *
 * @param short_addr Address of node to feed (or broadcast 0xFFFF)
 * @param targmem Target memory as character, e.g. 'F':Flash 'E':EEPROM
 *
 */
void wibohost_target(uint16_t short_addr, uint8_t targmem)
{
	p2p_wibo_target_t *dat = (p2p_wibo_target_t*) txbuf;

	dat->targmem = targmem;
	wibohost_sendcommand(short_addr, P2P_WIBO_TARGET, (uint8_t*) dat,
			sizeof(p2p_wibo_target_t));
}

/*
 * \brief Issue command to ping a node
 *
 * @param short_addr The node addressed (0xFFFF for broadcast)
 */
void wibohost_ping(uint16_t short_addr)
{
	wibohost_sendcommand(short_addr, P2P_PING_REQ, txbuf, sizeof(p2p_hdr_t));

	/* start timeout timer */
	thdl_ping = timer_start(wibohost_pingtimeout, PINGTIMEOUT_MS, 0);
	wait_cmd_ping_cnf = 1;
}

/*
 * \brief Issue command to set flash address of a node
 *
 * @param short_addr The node addressed (0xFFFF for broadcast)
 * @param flash_addr The flash address to set
 */
void wibohost_addr(uint16_t short_addr, uint32_t flash_addr)
{
	p2p_wibo_addr_t *addr = (p2p_wibo_addr_t*) txbuf;
	addr->address = flash_addr;
	wibohost_sendcommand(short_addr, P2P_WIBO_ADDR, (uint8_t*) addr,
			sizeof(p2p_wibo_addr_t));
}

/*
 * \brief Issue command to set node to deaf
 *
 * @param short_addr The node addressed (0xFFFF for broadcast)
 */
void wibohost_deaf(uint16_t short_addr)
{
	wibohost_sendcommand(short_addr, P2P_WIBO_DEAF, txbuf, sizeof(p2p_hdr_t));
}

/*
 * \brief Issue command to force write pagebuffer that is filled partially
 *
 * @param short_addr The node addressed (or broadcast 0xFFFF)
 */
void wibohost_finish(uint16_t short_addr)
{
	p2p_wibo_finish_t *dat = (p2p_wibo_finish_t*) txbuf;

	wibohost_sendcommand(short_addr, P2P_WIBO_FINISH, (uint8_t*) dat,
			sizeof(p2p_wibo_finish_t));
}

/*
 * \brief Reset the running checksum for image data
 * Reset our own checksum and all nodes in the network, therefore
 * send a broadcast. Otherwise a separate CRC for each node
 * would have to be hold here
 *
 */
void wibohost_reset(void)
{
	p2p_wibo_reset_t *dat = (p2p_wibo_reset_t*) txbuf;

	/* no data fields to fill */

	wibohost_sendcommand(0xFFFF, P2P_WIBO_RESET, (uint8_t*) dat,
			sizeof(p2p_wibo_reset_t));

	/* reset checksum */
	datacrc = 0x0000;
}

/*
 * \brief Deliver the checksum of the data fed since last wibohost_reset()
 *
 * @return CRC-16 checksum
 */
uint16_t wibohost_getcrc(void)
{
	return datacrc;
}

/*
 * \brief Issue command to exit bootloader and jump into
 * application (Vector 0x0000)
 *
 * @param short_addr The node addressed (or broadcast 0xFFFF)
 */
void wibohost_exit(uint16_t short_addr)
{
	p2p_wibo_exit_t *dat = (p2p_wibo_exit_t*) txbuf;

	/* no data fields to fill */

	wibohost_sendcommand(short_addr, P2P_WIBO_EXIT, (uint8_t*) dat,
			sizeof(p2p_wibo_exit_t));
}

/*
 * \brief Issue command to jump into bootloader
 * This is used for example application that demonstrates the
 * ability to jump between application and bootloader by air
 * commands entirely.
 * Usually the application itself is responsible for jumping
 * into bootloader, not the bootloader host application. The
 * command here is for demonstration purposes only.
 *
 * @param short_addr The node addressed (or broadcast 0xFFFF)
 */
void wibohost_jbootl(uint16_t short_addr)
{
	p2p_jump_bootl_t *dat = (p2p_jump_bootl_t*) txbuf;

	wibohost_sendcommand(short_addr, P2P_JUMP_BOOTL, (uint8_t*) dat,
			sizeof(p2p_jump_bootl_t));
}

/*
 * \brief Update bootloader
 *
 * @param short_addr The node addressed (or broadcast 0xFFFF)
 */
void wibohost_bootlup(uint16_t short_addr)
{
	p2p_jump_bootl_t *dat = (p2p_jump_bootl_t*) txbuf;

	wibohost_sendcommand(short_addr, P2P_WIBO_BOOTLUP, (uint8_t*) dat,
			sizeof(p2p_wibo_bootlup_t));
}

/*
 * \brief Deliver node configuration, IEEE802.15.4 parameters
 *
 * @return Pointer to node configuration structure as defined in uracoli
 */
node_config_t* wibohost_getnodecfg(void)
{
	return &nodeconfig;
}

uint8_t wibohost_setchannel(uint8_t channel)
{
	if ((TRX_MIN_CHANNEL <= channel) && (TRX_MAX_CHANNEL >= channel))
	{
		nodeconfig.channel = channel;
		radio_set_param(RP_CHANNEL(channel));
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t wibohost_setpanid(uint16_t pan_id)
{

	/* every value allowed */
	nodeconfig.pan_id = pan_id;
	radio_set_param(RP_PANID(pan_id));
	return 1;
}

/* EOF */
