/* Copyright (c) 2009-2011, Bjoern Riemer, Karl Fessel, Axel Wachtler
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
 * @brief Definition 2.4G Module from In-Circquit with
 *        AT86RF230B and Atmega1281 (V1.1)
 *
 *
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
  ATmega1281    AT86RF230
  ----------    ---------
    PB1    --> SCK
    PB2    <-- MOSI
    PB3    --> MISO
    PB0    --> SEL
    PB4    --> SLP_TR
    PB5    --> TRX_RESET
    PD4/ICP1  --> IRQ
    PD6/T1   <-- CLKM

    PG5   --> LED Interface

    Fuses:
      LF: 0xe2 - 8MHz internal RC Osc.
      HF: 0x11 - without boot loader
      HF: 0x10 - with boot loader
      EF: 0xff
      LOCK: 0xef - protection of boot section

    Original Fuse settings (Stick):
       LF: 0xe2
       HF: 0x1a
       EF: 0xfd

    Original Fuse settings (Module):
       LF: 0x62
       HF: 0x93
       EF: 0xff

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>


@par Build Options
 - In-Cirquit radio stick
   - ics230_11  : version 1.1 (RF230 RevB)
   - ics230_12  : version 1.2 (RF230 RevB; AtMega128)
 - In-Cirquit radio module
   - icm230_11  : version 1.1 (RF230 RevB)
   - icm230_12a : version 1.2 (RF230 RevA) [shiny finish]
   - icm230_12b : version 1.2 (RF230 RevB) [tarnished finish]
   - icm230_12c : version 1.2 (RF230 RevB) [tarnished finish & AtMega128]
 */
/** ID String for this hardware */
#if defined(ics230_11)
# define BOARD_NAME "icstick230"
# define BOARD_TYPE (BOARD_ICS230_11)
# define RADIO_TYPE (RADIO_AT86RF230B)

#elif defined(icm230_11)
# define BOARD_NAME "icmodule230"
# define BOARD_TYPE (BOARD_ICM230_11)
# define RADIO_TYPE (RADIO_AT86RF230B)

#elif defined(icm230_12a)
# define BOARD_NAME "icmodule230a"
# define BOARD_TYPE (BOARD_ICM230_12A)
# define RADIO_TYPE (RADIO_AT86RF230A)

#elif defined(icm230_12b)
# define BOARD_NAME "icmodule230b"
# define BOARD_TYPE (BOARD_ICM230_12B)
# define RADIO_TYPE (RADIO_AT86RF230B)

#elif defined(icm230_12c)
# define BOARD_NAME "icmodule230c"
# define BOARD_TYPE (BOARD_ICM230_12C)
# define RADIO_TYPE (RADIO_AT86RF230B)

#elif defined(ics230_12)
# define BOARD_NAME "icstick230_12"
# define BOARD_TYPE (BOARD_ICS230_12)
# define RADIO_TYPE (RADIO_AT86RF230B)
#endif

#ifndef BOARD_ICT230_11_H
#define BOARD_ICT230_11_H

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_4)
#endif

/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/
#if defined(__AVR_ATmega1281__)
# include "base_rdk230.h"
#elif defined(__AVR_ATmega128__)
# include "base_rdk230_m128.h"
#endif

#define CUSTOM_RESET_TIME_MS (10)       /**< additional delay needed by hardware */

/*=== LED access macros ==============================================*/

/* stick settings */
#if (BOARD_TYPE == BOARD_ICS230_11) || (BOARD_TYPE == BOARD_ICS230_12)
# define LED_PORT      PORTG
# define LED_DDR       DDRG
# define LED_MASK      (0x20)
# define LED_SHIFT     (5)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (1)
#elif BOARD_TYPE == BOARD_ICM230_11
# define LED_PORT      PORTD
# define LED_DDR       DDRD
# define LED_MASK      (0xc0)
# define LED_SHIFT     (6)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (2)
#elif (BOARD_TYPE == BOARD_ICM230_12A) ||\
      (BOARD_TYPE == BOARD_ICM230_12B) ||\
      (BOARD_TYPE == BOARD_ICM230_12C)
# define LED_PORT      PORTA
# define LED_DDR       DDRA
# define LED_MASK      (_BV(4)|_BV(5)|_BV(6)|_BV(7))
# define LED_SHIFT     (4)
# define LEDS_INVERSE  (0)
# define LED_NUMBER    (4)
#endif


/*=== KEY access macros ==============================================*/
#if (BOARD_TYPE == BOARD_ICS230_11) || (BOARD_TYPE == BOARD_ICS230_12)
# define NO_KEYS (1)
#elif (BOARD_TYPE == BOARD_ICM230_11) ||\
      (BOARD_TYPE == BOARD_ICM230_12A) ||\
      (BOARD_TYPE == BOARD_ICM230_12B) ||\
      (BOARD_TYPE == BOARD_ICM230_12C)
# define PORT_KEY     PORTD
# define PIN_KEY      PIND
# define DDR_KEY      DDRD
# define MASK_KEY     (0x01)
# define SHIFT_KEY    (0)
# define INVERSE_KEYS (1)
# define PULLUP_KEYS  (1)
#endif

/*=== Host Interface ================================================*/
# define HIF_TYPE    HIF_UART_1

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)

#if defined(__AVR_ATmega1281__)
# define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10)); \
        TIMSK1 |= _BV(TOIE1); \
    }while(0)

# define TIMER_IRQ_vect   TIMER1_OVF_vect
#elif defined(__AVR_ATmega128__)
# define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10)); \
        TIMSK |= _BV(TOIE1); \
    }while(0)

# define TIMER_IRQ_vect   TIMER1_OVF_vect
#endif

#endif /* BOARD_ICT230_11_H */
