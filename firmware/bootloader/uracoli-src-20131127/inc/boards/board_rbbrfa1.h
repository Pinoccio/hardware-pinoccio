/* Copyright (c) 2009 Axel Wachtler
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
 *
 * @file
 * @brief Definition of Radio Controller Breakout Board
 * (light and normal) from Dresden Elektronik, ATmega128RFA1 version.
 *
 * The Breakout board is a carrier board for the radio
 * controller board family.
 *
<pre>

Peripherals:
    KEY: PE5
    LEDS PE2:PE4

Fuses/Locks:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

@par Build Options

  - rbb128rfa1 : BB + RCB128RFA1
  - derftorcbrfa1: deRFtoRCB  + derfa1
 */

#if defined(rbb128rfa1)
# define BOARD_TYPE  BOARD_RBB128RFA1
# define BOARD_NAME "rbb128rfa1"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(derftorcbrfa1)
# define BOARD_TYPE  BOARD_DERFTORCBRFA1
# define BOARD_NAME "derftorcbrfa1"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#endif

#ifndef BOARD_RBBRFA1_H
#define BOARD_RBBRFA1_H

/*=== Compile time parameters ========================================*/
#ifndef MAX_FRAME_SIZE
# define MAX_FRAME_SIZE (127) /**< maximum allowed frame size */
#endif

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== LED access macros ==============================================*/
#define LED_PORT     PORTE
#define LED_DDR      DDRE
#define LED_MASK     (0x1c)
#define LED_SHIFT    (2)
#define LEDS_INVERSE (1)
#define LED_NUMBER   (3)

/*=== KEY access macros ==============================================*/
#define PORT_KEY     PORTE
#define PIN_KEY      PINE
#define DDR_KEY      DDRE
#define MASK_KEY     (0x20)
#define SHIFT_KEY    (5)
#define INVERSE_KEYS (1)
#define PULLUP_KEYS  (1)

#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() \
        do{\
            EIMSK |= _BV(INT5);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            EIMSK &= ~_BV(INT5);\
        } while(0)

#define SLEEP_ON_KEY_vect INT5_vect

/*=== Host Interface ================================================*/
#if BOARD_TYPE == BOARD_DERFTORCBRFA1
# define HIF_TYPE    HIF_UART_0
#else
# define HIF_TYPE    HIF_UART_1
#endif
#define HIF_IO_ENABLE() \
    do{ DDRD |= 0xD0; PORTD |= 0xC0;}while(0);

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

/*=== OSCCAL tuning =================================================*/

#endif /* BOARD_RBBRFA1_H*/
