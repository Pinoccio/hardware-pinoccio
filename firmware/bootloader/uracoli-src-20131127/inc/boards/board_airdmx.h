/* Copyright (c) 2013 Axel Wachtler, Daniel Thiele, Charles Goyard
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
 * @brief AirDMX boards for wireless DMX control
 *
 *
 *
<pre>

Fuses/Locks:
     LF: 0xC6 - 16Mhz transceiver Osc. 258CLK+4.1ms
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

radiofaro:
  LEDS: PG1, PG2

</pre>


@par Build Options

  - dracula : the DMX master board
  - bat     : the DMX slave board
 */

#ifndef BOARD_AIRDMX_H
#define BOARD_AIRDMX_H

#if defined(dracula)
# define BOARD_TYPE BOARD_DRACULA
# define BOARD_NAME "dracula"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#elif defined(bat)
# define BOARD_TYPE BOARD_BAT
# define BOARD_NAME "bat"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#else
#error "Undefined board"
#endif
/*=== Compile time parameters ========================================*/

/*=== Hardware Components ============================================*/

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1
#if BOARD_TYPE == BOARD_DRACULA || \
    BOARD_TYPE == BOARD_BAT
# define LED_PORT      PORTG
# define LED_DDR       DDRG
# define LED_MASK      (0x06)
# define LED_SHIFT     (1)
# define LEDS_INVERSE  (0)
# define LED_NUMBER    (2)
#endif

#define NO_KEYS        (1)

#if defined(dracula)
#define UNUSED_PINS_INIT() do{}while(0)
#elif defined(bat)
#define UNUSED_PINS_INIT() do{      \
	DDRD &= ~0xFC;  \
	PORTD |= 0xFC;  \
	DDRE &= ~0xF0;  \
	PORTE |= 0xF0;  \
	DDRG &= ~0x10;  \
	PORTG |= 0x10;  \
}while(0)
#else
#error "Undefined board"
#endif

#define DMX_DRIVER_INIT()       do{ PORTE &= ~(1<<PE2); DDRE |= (1<<PE2); }while(0)
#define DMX_DRIVER_TX_ENABLE()  do{ PORTE |= (1<<PE2);  }while(0)
#define DMX_DRIVER_TX_DISABLE() do{ PORTE &= ~(1<<PE2); }while(0)

#define DMX_ADDR_MASK_B (0xEF)
#define DMX_ADDR_MASK_D (0x03)

/*
 * \brief DIP-switch 9-bit
 *
 */
static inline uint16_t dmx_get_address(void)
{
    uint16_t rv = 0;
    uint8_t tmppd, tmppb;
    // on 22C00 tmp = pd0 pd1 pb3 pb2 pb1 pb0 pb4 pb5 pb6 pb7 
    //                sw9 sw8 sw7 sw6 sw5 sw4 sw3 sw2 sw1 who 

    // that is:       pd1 pd0 pb7 pb6 pb5 pb4 pb3 pb2 pb1 pb0
    //                sw8 sw9 who sw1 sw2 sw3 sw7 sw6 sw5 sw4

	/* Address decoding */
	DDRB &= ~(DMX_ADDR_MASK_B);
	DDRD &= ~(DMX_ADDR_MASK_D);
	PORTB |= (DMX_ADDR_MASK_B); /* enable pullups */
	PORTD |= (DMX_ADDR_MASK_D); /* enable pullups */
	_delay_ms(10); /* charge pins */
    
    tmppb = PINB;
    tmppd = PIND;
    //                                __w______987654321
    rv  |= (tmppb & (1 << PB0)) ? 0 : 0b0000000001000000;
    rv  |= (tmppb & (1 << PB1)) ? 0 : 0b0000000000100000;
    rv  |= (tmppb & (1 << PB2)) ? 0 : 0b0000000000010000;
    rv  |= (tmppb & (1 << PB3)) ? 0 : 0b0000000000001000;
    rv  |= (tmppb & (1 << PB4)) ? 0 : 0b0000000000000100;
    rv  |= (tmppb & (1 << PB5)) ? 0 : 0b0000000000000010;
    rv  |= (tmppb & (1 << PB6)) ? 0 : 0b0000000000000001;
    rv  |= (tmppd & (1 << PD0)) ? 0 : 0b0000000100000000;
    rv  |= (tmppd & (1 << PD1)) ? 0 : 0b0000000010000000;

    return rv;
}

/*=== Host Interface ================================================*/
#if BOARD_TYPE == BOARD_DRACULA || \
    BOARD_TYPE == BOARD_BAT
# define HIF_TYPE    HIF_UART_1
#endif

#define TRX_RESET_LOW()   do { TRXPR &= ~_BV(TRXRST); } while (0)
#define TRX_RESET_HIGH()  do { TRXPR |= _BV(TRXRST); } while (0)
#define TRX_SLPTR_LOW()   do { TRXPR &= ~_BV(SLPTR); } while (0)
#define TRX_SLPTR_HIGH()  do { TRXPR |= _BV(SLPTR); } while (0)

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10)); \
        TIMSK1 |= _BV(TOIE1); \
    }while(0)
#define TIMER_IRQ_vect   TIMER1_OVF_vect

#endif /* BOARD_AIRDMX_H */
