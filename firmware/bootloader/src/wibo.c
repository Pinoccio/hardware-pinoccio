/* Copyright (c) 2010 - 2012 Daniel Thiele, Axel Wachtler
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
 *
 *
 *
 * Description of compile flavours:
 *
 * WIBO_FLAVOUR_KEYPRESS
 *   run bootloader only if key is pressed
 *
 * WIBO_FLAVOUR_KEYPRESS_KEYNB
 *   only valid if WIBO_FLAVOUR_KEYPRESS is defined, the number of the key
 *   that is evaluated if stay in bootloader or jump to application
 *
 *
 *
 * WIBO_FLAVOUR_MAILBOX
 *   flag from application to stay in bootloader, typically a certain register with a secret value
 *
 * WIBO_FLAVOUR_MAILBOX_REGISTER
 *    the register to find secret value
 *    example: #define WIBO_FLAVOUR_MAILBOX_REGISTER (GPRR0)
 * WIBO_FLAVOUR_MAILBOX_CODE
 *    the secret value
 *    example: #define WIBO_FLAVOUR_MAILBOX_CODE (0xB5)
 */

/* avr-libc inclusions */
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <util/crc16.h>
#include <util/delay.h>
#include <string.h>

/* uracoli inclusions */
#include <board.h>
#include <ioutil.h>
#include <transceiver.h>
#include <p2p_protocol.h>

/* project inclusions */
#include "wibo.h"

#ifndef _SW_VERSION_
#error "Symbol _SW_VERSION_ not defined"
#endif

//#define NO_LEDS (1)
#define PROGLED (2) // use the green one

#define WIBO_TIMEOUT 10000	// timeout in milliseconds to exit Wibo

#if defined(_DEBUG_SERIAL_)
#include <avr/interrupt.h>
#include <hif.h>
#define EOL "\n"
#endif

/* incoming frames are collected here */
static union
{
	uint8_t data[MAX_FRAME_SIZE];
	p2p_hdr_t hdr;
	p2p_wibo_data_t wibo_data;
	p2p_wibo_finish_t wibo_finish;
	p2p_wibo_target_t wibo_target;
	p2p_wibo_addr_t wibo_addr;
} rxbuf;

#define PAGEBUFSIZE (SPM_PAGESIZE)

/* the only outgoing command, create in SRAM here
 * the values assigned here never have to changed
 */
static p2p_ping_cnf_t pingrep =
{ .hdr.cmd = P2P_PING_CNF, .hdr.fcf = 0x8841, /* short addressing, frame type: data, no ACK requested */
.version = _SW_VERSION_, .appname = "wibo", .boardname = BOARD_NAME };

/* collect memory page data here, FLASH and EEPROM */
static uint8_t pagebuf[PAGEBUFSIZE];


#if FLASHEND > 0x7FFF
static uint32_t addr = 0;
#else
static uint16_t addr = 0;
#endif

#if SPM_PAGESIZE > 255
static uint16_t pagebufidx = 0;
#else
static uint8_t pagebufidx = 0;
#endif
static uint8_t deaf = 0;


static node_config_t nodeconfig;

/* Target for program data
 'F' : flash memory
 'E' : eeprom memory
 'X' : None, dry run
 */
static uint8_t target = 'F'; /* for backwards compatibility */

/* do not initialize variables to save code space, BSS segment sets all variables to zero
 *
 * compiler throws warning, please ignore
 */
uint8_t *ptr;
uint8_t tmp;
uint16_t datacrc = 0; /* checksum for received data */

/*
 * \brief Support to update WIBO itself
 * Put a little snippet at the end of bootloader that copies code from start
 * of application section to start of bootloader section
 *
 *
 */
#if defined(WIBO_FLAVOUR_BOOTLUP)

void __attribute__ ((section (".bootlup"))) __attribute__((noreturn)) bootlup()
{
	const uint32_t addr_to = (uint32_t)BOOTLOADER_ADDRESS*2; /* byte address of bootloader */
	const uint8_t nbpages=3072/SPM_PAGESIZE; /* maximum of 3k bootloader at the moment */
#if SPM_PAGESIZE > 255
	uint16_t i;
#else
	uint8_t i;
#endif
	for(uint8_t page=0;page<nbpages;page++)
	{
		boot_page_erase(addr_to+page*SPM_PAGESIZE);
		boot_spm_busy_wait ();      // Wait until the memory is erased

		for (i=0; i<SPM_PAGESIZE; i+=2)
		{
			uint16_t w = pgm_read_word(page*SPM_PAGESIZE+i);
			boot_page_fill (addr_to+page*SPM_PAGESIZE+i, w);
		}

		boot_page_write (addr_to+page*SPM_PAGESIZE); // Store buffer in flash page
		boot_spm_busy_wait ();// Wait until the memory is erased
	}

	boot_rww_enable ();

	jump_to_bootloader();
}
#endif /* defined(WIBO_FLAVOUR_BOOTLUP) */

void wibo_init(uint8_t channel, uint16_t pan_id, uint16_t short_addr, uint64_t ieee_addr)
{
#if defined(WIBO_FLAVOUR_KEYPRESS) || defined(WIBO_FLAVOUR_MAILBOX)
	uint8_t run_bootloader = 0;
#endif

	/* only stay in bootloader if key is pressed */
#if defined(WIBO_FLAVOUR_KEYPRESS)
#if defined(NO_KEYS)
#error "No Keys defined for WIBO_FLAVOUR_KEYPRESS"
#endif
	KEY_INIT();
	if(KEY_GET() != 0)
	{
		run_bootloader = 1;
	}
#endif /* defined(WIBO_FLAVOUR_KEYPRESS) */

#if defined(WIBO_FLAVOUR_MAILBOX)

#if !defined(WIBO_FLAVOUR_MAILBOX_REGISTER) || !defined(WIBO_FLAVOUR_MAILBOX_CODE)
#error "WIBO_FLAVOUR_MAILBOX not defined correctly"
#endif
	if(WIBO_FLAVOUR_MAILBOX_REGISTER == WIBO_FLAVOUR_MAILBOX_CODE)
	{
		run_bootloader = 1;
	}
	//WIBO_MAILBOX_CLR();
#endif /* defined(WIBO_FLAVOUR_MAILBOX) */

#if defined(WIBO_FLAVOUR_KEYPRESS) || defined(WIBO_FLAVOUR_MAILBOX)
	if(run_bootloader == 0)
	{
		app();
	}
#endif

#if !defined(NO_LEDS)
LED_INIT();
LED_SET(PROGLED);
#endif

	nodeconfig.channel=channel;
	nodeconfig.pan_id=pan_id;
	nodeconfig.short_addr=short_addr;
	nodeconfig.ieee_addr = ieee_addr;

	trx_io_init(DEFAULT_SPI_RATE);
	TRX_RESET_LOW();
	TRX_SLPTR_LOW();
	TRX_RESET_HIGH();

#if defined(DI_TRX_IRQ)
	DI_TRX_IRQ();
#endif
	trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);

#if (RADIO_TYPE == RADIO_AT86RF230A) || (RADIO_TYPE == RADIO_AT86RF230B)
	trx_reg_write(RG_PHY_TX_PWR, 0x80); /* set TX_AUTO_CRC bit, and TX_PWR = max */
#else
	trx_reg_write(RG_TRX_CTRL_1, 0x20); /* set TX_AUTO_CRC bit */
#endif

	/* setup network addresses for auto modes */
	pingrep.hdr.pan = nodeconfig.pan_id;
	pingrep.hdr.src = nodeconfig.short_addr;

	trx_set_panid(nodeconfig.pan_id);
	trx_set_shortaddr(nodeconfig.short_addr);

	/* use register write to save code space, overwrites Bits CCA_REQUEST CCA_MODE[1] CCA_MODE[0]
	 * which is accepted
	 */
	trx_reg_write(RG_PHY_CC_CCA, nodeconfig.channel);

#if RADIO_TYPE == RADIO_AT86RF212

	/* reset value, BPSK-40 */
	/* trx_reg_write(RG_TRX_CTRL_2, 0x24); */

	/* +5dBm acc. to datasheet AT86RF212 table 7-15 */
	trx_reg_write(RG_PHY_TX_PWR, 0x84);
#endif /* RADIO_TYPE == RADIO_AT86RF212 */

	trx_reg_write(RG_CSMA_SEED_0, nodeconfig.short_addr); /* some seeding */
	trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
	trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_RX_END); /* clear the flag */

#if defined(_DEBUG_SERIAL_)
	static FILE usart_stdio = FDEV_SETUP_STREAM(hif_putc, NULL, _FDEV_SETUP_WRITE);

	hif_init(HIF_DEFAULT_BAUDRATE);
	stdout = stderr = &usart_stdio;
	printf("WIBO Bootlapp Serial Debug"EOL);
	printf("PANID=%04X SHORTADDR=%04X CHANNEL=%d"EOL,
			nodeconfig.pan_id, nodeconfig.short_addr, nodeconfig.channel);
#endif
}

#if defined(TRX_IF_RFA1)
uint8_t wibo_available(void)
{
	return (0 != (trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_RX_END));
}
#else
#error "Cannot use this function"
#endif

uint8_t wibo_run(void)
{
	uint8_t isLeave=0;
	uint8_t isStay=0;
	unsigned long timeout = WIBO_TIMEOUT;
	
	while(!isLeave) {
#if !defined(NO_LEDS)
		LED_CLR(PROGLED);
#endif
		if (!(isStay))
		{
			while(!(wibo_available()) && (timeout--))  _delay_ms(1);	// minimum frame time @ 250kbps ~ 2ms.
		
			if (!(wibo_available()))	// no packets received, bye bye!
			{
				isLeave=1;
				return isLeave;
			}			
		}
		else
		{
			while(!(wibo_available()));	// wait for next packet
		}

		trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_RX_END); /* clear the flag */

		trx_frame_read(rxbuf.data, sizeof(rxbuf.data) / sizeof(rxbuf.data[0]),
				&tmp); /* dont use LQI, write into tmp variable */

#if !defined(NO_LEDS)
		LED_SET(PROGLED);
		/* light as long as actions are running */
#endif

		switch (rxbuf.hdr.cmd)
		{

		case P2P_PING_REQ:
			isStay=1;
			if (0 == deaf)
			{
				pingrep.hdr.dst = rxbuf.hdr.src;
				pingrep.hdr.seq++;
				pingrep.crc = datacrc;

				trx_reg_write(RG_TRX_STATE, CMD_TX_ARET_ON);

				/* no need to make block atomic since no IRQs are used */
				TRX_SLPTR_HIGH()
				;
				TRX_SLPTR_LOW()
				;
				trx_frame_write(sizeof(p2p_ping_cnf_t) + sizeof(BOARD_NAME) + 2,
						(uint8_t*) &pingrep);
				/*******************************************************/

#if defined(_DEBUG_SERIAL_)
				printf("Pinged by 0x%04X"EOL, rxbuf.hdr.src);
#endif

#if defined(TRX_IF_RFA1)
				while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TX_END))
					;
				trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_TX_END); /* clear the flag */
#else
				while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TRX_END))
				;
#endif /* defined(TRX_IF_RFA1) */
				trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
			} /* (0 == deaf) */
			break;

		case P2P_WIBO_TARGET:
			isStay=1;
			target = rxbuf.wibo_target.targmem;
#if defined(_DEBUG_SERIAL_)
			printf("Set Target to %c"EOL, target);
#endif
			break;

		case P2P_WIBO_RESET:
			isStay=1;
#if defined(_DEBUG_SERIAL_)
			printf("Reset"EOL);
#endif

			addr = SPM_PAGESIZE; /* misuse as counter */
			ptr = pagebuf;
			do
			{
				*ptr++ = 0xFF;
			} while (--addr);

			addr = 0;
			datacrc = 0;
			pagebufidx = 0;
			deaf = 0;
			break;

		case P2P_WIBO_ADDR:
			isStay=1;
#if defined(_DEBUG_SERIAL_)
			printf("Set address: 0x%08lX"EOL, rxbuf.wibo_addr.address);
#endif
			addr = rxbuf.wibo_addr.address;
			pagebufidx = 0;
			break;

		case P2P_WIBO_DATA:
			isStay=1;
#if defined(_DEBUG_SERIAL_)
			printf("Data[%d]", rxbuf.wibo_data.dsize);
			for(uint8_t j=0;j<rxbuf.wibo_data.dsize;j++)
			{
				printf(" %02X", rxbuf.wibo_data.data[j]);
			}
			printf(EOL);
#endif
			tmp = rxbuf.wibo_data.dsize;
			ptr = rxbuf.wibo_data.data;
			do
			{
				datacrc = _crc_ccitt_update(datacrc, *ptr);
				pagebuf[pagebufidx++] = *ptr;
				if (pagebufidx >= PAGEBUFSIZE)
				{
					/* LED off to save current and avoid flash corruption
					 *  because of possible voltage drops
					 */
#if !defined(NO_LEDS)
					LED_CLR(PROGLED);
#endif

					if (target == 'F') /* Flash memory */
					{
						boot_program_page(addr, pagebuf);
					}
					else if (target == 'E')
					{
						/* not implemented */
					}
					else
					{
						/* unknown target, dry run */
					}

					/* also for dry run! */
					addr += SPM_PAGESIZE;
					pagebufidx = 0;
				}
				ptr++;
			} while (--tmp);
			break;
#if defined(WIBO_FLAVOUR_BOOTLUP)
		case P2P_WIBO_BOOTLUP:
			isStay=1;
			bootlup();
		break;
#endif

		case P2P_WIBO_FINISH:
			isStay=1;
#if defined(_DEBUG_SERIAL_)
			printf("Finish"EOL);
#endif
			if (target == 'F') /* Flash memory */
			{
				boot_program_page(addr, pagebuf);
			}
			else if (target == 'E')
			{
				/* not implemented */
			}
			else
			{
				/* unknown target, dry run */
			}

			/* also for dry run! */
			addr += SPM_PAGESIZE;
			pagebufidx = 0;

			break;

		case P2P_WIBO_EXIT:
#if defined(_DEBUG_SERIAL_)
			printf("Exit"EOL);
#endif
#if !defined(NO_LEDS)
			LED_CLR(PROGLED);
#endif
			isLeave=1;
			break;

		case P2P_WIBO_DEAF:
			isStay=1;
			deaf = 1;
			break;
		default:
			/* unknown or unhandled command */
			if (!(isStay)) {
				if (!(timeout--)) {
					isLeave = 1;
				}
			}
			break;
		}; /* switch (rxbuf.hdr.cmd) */
	}

	return isLeave;
}

/* EOF */

