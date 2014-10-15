/* Copyright (c) 2007 Axel Wachtler
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
 * (light and normal) from Dresden Elektronik.
 *
 * The Breakout board is a carrier board for the radio
 * controller board family.
 * The transceiver wiring fits the common RCBs.
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
          AVR      AT86RF230
          ---      ---------
          PB4  -->  SLPTR
    XTAL1/PD6  <--  CLKM
          PD4  <--  IRQ (ICP1)
          PB5  -->  RSTN
          PB0  -->  /SEL
          PB2  -->  MOSI
          PB3  <--  MISO
          PB1  -->  SCK

    KEY: PE5
    LEDS PE2:PE4

    DBG: PA0 - connector J3:1 (ground J3:10)

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

  - rbb230  : BB + RCB230 prior V3.2 / V3.3.1 (AT86RF230A)
  - rbb230b : BB + RCB230 V3.2 / RCB230 V3.3.1 (AT86RF230B)
  - rbb231  : BB + RCB231 V4.0.2 / RCB231ED V4.1.1
  - rbb212  : BB + RCB212SMA V5.3.2

 */

#if defined(rbb230)
# define BOARD_TYPE  BOARD_RBB230
# define BOARD_NAME "rbb230"
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(rbb230b)
# define BOARD_TYPE  BOARD_RBB230B
# define BOARD_NAME "rbb230b"
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(rbb231)
# define BOARD_TYPE  BOARD_RBB231
# define BOARD_NAME  "rbb231"
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(rbb212)
# define BOARD_TYPE  BOARD_RBB212
# define BOARD_NAME  "rbb212"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(rbb232)
# define BOARD_TYPE  BOARD_RBB232
# define BOARD_NAME  "rbb232"
# define RADIO_TYPE (RADIO_AT86RF232)
#elif defined(rbb233)
# define BOARD_TYPE  BOARD_RBB233
# define BOARD_NAME  "rbb233"
# define RADIO_TYPE (RADIO_AT86RF233)
#endif

#ifndef BOARD_RBB_H
#define BOARD_RBB_H

/*=== Compile time parameters ========================================*/

/*=== radio interface definition =====================================*/
#if BOARD_TYPE == BOARD_RBB230 || BOARD_TYPE == BOARD_RBB230B
# include "base_rdk230.h"
#else
# include "base_rdk2xx.h"
#endif

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
#define HIF_TYPE (HIF_UART_1)
#define HIF_IO_ENABLE() \
   do{ DDRC |= 0xD0; PORTC |= 0xC0;}while(0);

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

/*=== DBG interface =================================================*/
# define DBG_PORT PORTA
# define DBG_DDR DDRA
# define DBG_PIN (1<<PA0)

#endif /* BOARD_RBB_H*/
