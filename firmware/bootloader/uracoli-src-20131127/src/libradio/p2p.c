/* Copyright (c) 2012 Axel Wachtler
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
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */

/* === send flags =========================================================== */
#include "board.h"
#include "transceiver.h"
#include "radio.h"
#include "p2p.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

static uint8_t rxbuf[MAX_FRAME_SIZE];

/* IEEE802.15.4 parameters for communication */
const node_config_t PROGMEM nc_flash =
{
	.short_addr = 0xFECA,
	.pan_id = 0x3412,
	.channel = 17
};
static node_config_t NodeConfig;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

char p2p_init()
{
	char cfg_location = '?';

	/* === read node configuration data ===================================== */
	/* 1st trial: read from EEPROM */
	if (get_node_config_eeprom(&NodeConfig, 0) == 0)
	{
		/* using EEPROM config */;
		cfg_location = 'E';
	}
	/* 2nd trial: read from FLASHEND */
	else if (get_node_config(&NodeConfig) == 0)
	{
		/* using FLASHEND config */;
		cfg_location = 'F';
	}
	/* 3rd trial: read default values compiled into the application */
	else
	{
		/* using application default config */;
		memcpy_P(&NodeConfig, (PGM_VOID_P) & nc_flash, sizeof(node_config_t));
		cfg_location = 'D';
	}

	radio_init(rxbuf, sizeof(rxbuf)/sizeof(rxbuf[0]));
	radio_set_state(STATE_OFF);

	radio_set_param(RP_IDLESTATE(STATE_OFF));
    radio_set_param(RP_CHANNEL(NodeConfig.channel));
    radio_set_param(RP_SHORTADDR(NodeConfig.short_addr));
    radio_set_param(RP_PANID(NodeConfig.pan_id));

    return cfg_location;
}

node_config_t* p2p_get_config(void)
{
	return &NodeConfig;
}

#if 0
/* will come soon */
void p2p_task(void)
{

}
#endif

void p2p_send(uint16_t dst, uint8_t cmd, uint8_t flags,
              uint8_t *data, uint8_t lendata)
{
    p2p_hdr_t *hdr = (p2p_hdr_t*) data;

    __FILL_P2P_HEADER__(hdr, ((flags & P2P_ACK) ? 0x8861 : 0x8841),
                    NodeConfig.pan_id, dst, NodeConfig.short_addr, cmd);
    radio_set_state(STATE_TX);
    radio_set_state(STATE_TXAUTO);
    radio_send_frame(lendata + 2, data, 1); /* +2: add CRC bytes (FCF) */
}


#if 0
/* will come soon */
uint8_t *usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc_fail)
{

}
#endif
