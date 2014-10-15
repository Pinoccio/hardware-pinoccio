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
#include <string.h>

/* uracoli inclusions */
#include <board.h>
#include <ioutil.h>
#include <transceiver.h>
#include <p2p_protocol.h>

#ifndef _SW_VERSION_
#define _SW_VERSION_ (0x05)
#endif

/* serial debug printout via USART0, using uracoli ioutil/hif module */
#ifndef SERIALDEBUG
//#define SERIALDEBUG (1)
#endif

/* frame receive interrupt based */
#ifndef WIBO_FLAVOUR_INTERRUPT
//#define WIBO_FLAVOUR_INTERRUPT (1)
#endif

#if defined(SERIALDEBUG)
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
 *
 * hdr is assigned after reading nodeconfig
 */
static p2p_ping_cnf_t pingrep =
{	.hdr.cmd = P2P_PING_CNF,
	.hdr.fcf = 0x8841, /* short addressing, frame type: data, no ACK requested */
	.version = _SW_VERSION_,
	.appname = APP_NAME,
	.boardname = BOARD_NAME
};

/* collect memory page data here, FLASH and EEPROM */
static uint8_t pagebuf[PAGEBUFSIZE];

/* IEEE802.15.4 parameters for communication */
static node_config_t nodeconfig;

#if defined(WIBO_FLAVOUR_INTERRUPT)

static volatile uint8_t spm_ready;

/*
 * \brief To distinguish between RX_START and TX_END interrupt on
 * transceivers that multiplex these both to TRX_END interrupt
 *
 */
typedef enum
{
	APPSTATE_RX, APPSTATE_TX,
}appstate_t;
static volatile appstate_t appstate;
#endif

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

#if defined(WIBO_FLAVOUR_INTERRUPT)
ISR(SPM_READY_vect)
{
	spm_ready = 1;
}
#endif /* defined(WIBO_FLAVOUR_INTERRUPT) */

/*
 * \brief Program a page
 * (1) Erase the page containing at the given address
 * (2) Fill the page buffer
 * (3) Write the page buffer
 * (4) Wait for finish
 *
 * @param addr The address containing the page to program
 * @param *buf Pointer to buffer of page content
 */
static inline void boot_program_page(uint32_t addr, uint8_t *buf)
{
#if SPM_PAGESIZE > 255
	uint16_t i;
#else
	uint8_t i;
#endif

#if defined(SERIALDEBUG)
	printf("Write Flash addr=%04lX"EOL, addr);
	/*
	 for (i = 0; i < SPM_PAGESIZE; i++)
	 {
	 if (!(i % 16))
	 {
	 putchar('\n');
	 putchar(' ');
	 putchar(' ');
	 }
	 printf(" %02X", buf[i]);
	 }
	 putchar('\n');
	 */
#else /* defined(SERIALDEBUG) */
	boot_page_erase(addr);
	boot_spm_busy_wait();

	i = SPM_PAGESIZE;
	do
	{
		/* Set up little-endian word */
		uint16_t w = *buf++;
		w += (*buf++) << 8;

		boot_page_fill(addr + SPM_PAGESIZE - i, w);

		i -= 2;
	} while (i);

	boot_page_write(addr);
#if 0
//#if defined(WIBO_FLAVOUR_INTERRUPT)
	spm_ready = 0;
	SPMCSR |= (1<<SPMIE);
	do
	{
		sleep_mode();
	}while(0 == spm_ready);
#else
	boot_spm_busy_wait();
#endif

#endif /* defined(SERIALDEBUG) */
}

#if defined(WIBO_FLAVOUR_INTERRUPT)
static enum
{
	IRQ_CAUSE_UNDEFINED, IRQ_CAUSE_RX_END, IRQ_CAUSE_TX_END,
}irq_cause;

#if defined(TRX_IF_RFA1)
ISR(TRX24_TX_END_vect)
{
	__asm__ volatile ("nop");
	irq_cause = IRQ_CAUSE_TX_END;
}
ISR(TRX24_RX_END_vect)
{
	__asm__ volatile ("nop");
	irq_cause = IRQ_CAUSE_RX_END;
}
#else
void irq_handler(uint8_t cause)
{
	if (cause == 0x08)
	{ /* combined Rx+Tx */
		if (APPSTATE_RX == appstate)
		{
			irq_cause = IRQ_CAUSE_RX_END;
		}
		else if (APPSTATE_TX == appstate)
		{
			irq_cause = IRQ_CAUSE_TX_END;
		}
		else
		{
			irq_cause = IRQ_CAUSE_UNDEFINED;
		}
	}
	else
	{
		irq_cause = IRQ_CAUSE_UNDEFINED;
	}
}
#endif /* defined(TRX_IF_RFA1) */
#endif /* defined WIBO_FLAVOUR_INTERRUPT */

int __attribute__((OS_main)) __attribute__((noreturn)) main(void);


int main(void)
{
	typedef void (*func_ptr_t)(void) __attribute__((noreturn));
	const func_ptr_t jmp_app = (func_ptr_t) 0x0000;

	/* do not initialize variables to save code space, BSS segment sets all variables to zero
	 *
	 * compiler throws warning, please ignore
	 */
	uint8_t *ptr;
	uint8_t tmp;
	uint16_t datacrc = 0; /* checksum for received data */
#if FLASHEND > 0x7FFF
	uint32_t addr = 0;
#else
	uint16_t addr = 0;
#endif

#if SPM_PAGESIZE > 255
	uint16_t pagebufidx = 0;
#else
	uint8_t pagebufidx = 0;
#endif
	uint8_t deaf = 0;

	/* Target for program data 
	 'F' : flash memory
	 'E' : eeprom memory
	 'X' : None, dry run
	 */
	uint8_t target = 'F'; /* for backwards compatibility */

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

	LED_INIT();
	LED_SET(0);

#if defined(BOARD_PINOCCIO)
	nodeconfig.channel=eeprom_read_byte((uint8_t *)8179);
	nodeconfig.pan_id=eeprom_read_word((uint16_t *)8180);
	nodeconfig.short_addr=eeprom_read_word((uint16_t *)8182);
#else
	get_node_config(&nodeconfig);
#endif

#if 0
	/** DEBUG **/
	nodeconfig.channel=16;
	nodeconfig.pan_id=0x4567;
	nodeconfig.short_addr=0x2221;
	/***********/
#endif

	trx_io_init(DEFAULT_SPI_RATE);
	TRX_RESET_LOW()
	;
	TRX_SLPTR_LOW()
	;
	TRX_RESET_HIGH()
	;

	trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);

#if defined(WIBO_FLAVOUR_INTERRUPT)
#if defined(TRX_IF_RFA1)
	/* do nothing, ISRs are instantiiated directly here */
#else
	trx_set_irq_handler(irq_handler);
#endif

	/* move vector table to start of boot sector
	 * special write sequence acc. to datasheet
	 */
	MCUCR = (1 << IVCE);
	MCUCR = (1 << IVSEL);
	trx_reg_read(RG_IRQ_STATUS);
#if defined(TRX_IF_RFA1)
	trx_reg_write(RG_IRQ_MASK, TRX_IRQ_RX_END | TRX_IRQ_TX_END);
#else
	trx_reg_write(RG_IRQ_MASK, TRX_IRQ_TRX_END);
#endif
	EI_TRX_IRQ();
	appstate = APPSTATE_RX; /* but physically set below! */
#else
	/* BAD: better way would be to abstract for all boards even if it makes no sense (megaRF variants) */
#if defined(DI_TRX_IRQ)
	DI_TRX_IRQ();
#endif
#endif /* defined WIBO_FLAVOUR_INTERRUPT */

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

#if defined(SERIALDEBUG)
	static FILE usart_stdio = FDEV_SETUP_STREAM(hif_putc, NULL, _FDEV_SETUP_WRITE);

	hif_init(HIF_DEFAULT_BAUDRATE);
	stdout = stderr = &usart_stdio;
	printf("WIBO Bootlapp Serial Debug"EOL);
	printf("PANID=%04X SHORTADDR=%04X CHANNEL=%d"EOL,
			nodeconfig.pan_id, nodeconfig.short_addr, nodeconfig.channel);
#endif

#if defined(WIBO_FLAVOUR_INTERRUPT) || defined(SERIALDEBUG)
	sei();
#endif

	for (;;)
	{
		LED_CLR(0);

#if defined(WIBO_FLAVOUR_INTERRUPT)
		set_sleep_mode(SLEEP_MODE_IDLE);
		sleep_mode();

#else

#if defined(TRX_IF_RFA1)
		while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_RX_END))
			;
		trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_RX_END); /* clear the flag */
#else
		while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TRX_END))
		;
#endif

#endif /* defined WIBO_FLAVOUR_INTERRUPT */

#if defined(WIBO_FLAVOUR_INTERRUPT)
		if (IRQ_CAUSE_RX_END == irq_cause)
		{
			irq_cause = IRQ_CAUSE_UNDEFINED;
#else
		if (1)
		{
#endif
			trx_frame_read(rxbuf.data,
					sizeof(rxbuf.data) / sizeof(rxbuf.data[0]), &tmp); /* dont use LQI, write into tmp variable */
		}

#if defined(WIBO_FLAVOUR_INTERRUPT)
		if (APPSTATE_RX == appstate)
		{ /* must have been RX_END interrupt */
#else
		if (1)
		{
#endif
			LED_SET(0);
			/* light as long as actions are running */

			switch (rxbuf.hdr.cmd)
			{

			case P2P_PING_REQ:
				if (0 == deaf)
				{
					pingrep.hdr.dst = rxbuf.hdr.src;
					pingrep.hdr.seq++;
					pingrep.crc = datacrc;

					trx_reg_write(RG_TRX_STATE, CMD_TX_ARET_ON);

					/* no need to make block atomic since no IRQs are used */TRX_SLPTR_HIGH()
					;
					TRX_SLPTR_LOW()
					;
					trx_frame_write(
							sizeof(p2p_ping_cnf_t) + sizeof(BOARD_NAME) + 2,
							(uint8_t*) &pingrep);
					/*******************************************************/

#if defined(SERIALDEBUG)
					printf("Pinged by 0x%04X"EOL, rxbuf.hdr.src);
#endif

#if defined(WIBO_FLAVOUR_INTERRUPT)
					appstate = APPSTATE_TX;
#else
#if defined(TRX_IF_RFA1)
					while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TX_END))
						;
					trx_reg_write(RG_IRQ_STATUS, TRX_IRQ_TX_END); /* clear the flag */
#else
					while (!(trx_reg_read(RG_IRQ_STATUS) & TRX_IRQ_TRX_END))
					;
#endif /* defined(TRX_IF_RFA1) */
#endif /* defined(WIBO_FLAVOUR_INTERRUPT) */
					trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
				} /* (0 == deaf) */
				break;

			case P2P_WIBO_TARGET:
				target = rxbuf.wibo_target.targmem;
#if defined(SERIALDEBUG)
				printf("Set Target to %c"EOL, target);
#endif
				break;

			case P2P_WIBO_RESET:
#if defined(SERIALDEBUG)
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
#if defined(SERIALDEBUG)
				printf("Set address: 0x%08lX"EOL, rxbuf.wibo_addr.address);
#endif
				addr = rxbuf.wibo_addr.address;
				pagebufidx = 0;
				break;

			case P2P_WIBO_DATA:
#if defined(SERIALDEBUG)
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
						LED_CLR(0);
						if (target == 'F') /* Flash memory */
						{
							boot_program_page(addr, pagebuf);
						}
						else if (target == 'E')
						{
							eeprom_write_block(pagebuf, (uint8_t*) addr,
									pagebufidx);
							eeprom_busy_wait();
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
				bootlup();
				break;
#endif

			case P2P_WIBO_FINISH:
#if defined(SERIALDEBUG)
				printf("Finish"EOL);
#endif
				if (target == 'F') /* Flash memory */
				{
					boot_program_page(addr, pagebuf);
				}
				else if (target == 'E')
				{
					eeprom_write_block(pagebuf, (uint8_t*) addr, pagebufidx);
					eeprom_busy_wait();
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
#if defined(SERIALDEBUG)
				printf("Exit"EOL);
#endif
				LED_CLR(0);
#if defined(WIBO_FLAVOUR_INTERRUPT)
				cli();
				/* move back vector table to start of application sector
				 * special write sequence acc. to datasheet
				 */
				MCUCR = (1 << IVCE);
				MCUCR = (0 << IVSEL);
#endif
				boot_rww_enable();
#if defined(__AVR_ATmega256RFR2__)
				__asm__ volatile("clr	r30		\n\t"
						"clr	r31		\n\t"
						"ijmp	\n\t"
						);
#else
				jmp_app();
#endif


				break;

			case P2P_WIBO_DEAF:
				deaf = 1;
				break;
			default:
				/* unknown or unhandled command */
				break;
			}; /* switch (rxbuf.hdr.cmd) */
		}
#if defined(WIBO_FLAVOUR_INTERRUPT)
		else
		{ /* TX_END interrupt */
			trx_reg_write(RG_TRX_STATE, CMD_RX_AACK_ON);
			appstate = APPSTATE_RX;
		}
		irq_cause = IRQ_CAUSE_UNDEFINED;
#endif
	}
}

/* EOF */
