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
 * @brief ATZGB ZGB-LINK-{230,231,212} modules.
 *
 * The wiring between radio transceiver and ATmega is shown below:
 *
<pre>
          AVR       AT86RF2xx
          ---       ---------
          PB4  -->  SLPTR
    XTAL1/PD6  <--  CLKM
          PD4  <--  IRQ (ICP1)
          PB5  -->  RSTN
          PB0  -->  /SEL
          PB1  -->  SCK
          PB2  -->  MOSI
          PB3  <--  MISO
          PE7  <--  DIG2 (INT7)


    For the zgb-link modules
    KEY: None
    LEDS: None

    The EVM-HOST has memory mapped Keys.


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

 - zgbl230 : ZGB-LINK-Modul with AT86RF230
 - zgbl231 : ZGB-LINK-Modul with AT86RF231
 - zgbl212 : ZGB-LINK-Modul with AT86RF212

 - zgbh230 : EVM-HOST Plattform with AT86RF230
 - zgbh231 : EVM-HOST Plattform with AT86RF231
 - zgbh212 : EVM-HOST Plattform with AT86RF212

 */

/*
 @image html atmel_stk50x.jpg "STK500 + STK501 + Radio board"
 @image latex atmel_stk50x.jpg "STK500 + STK501 + Radio board"
 */
#if defined(zgbl230)
# define BOARD_TYPE BOARD_ZGBL230
# define BOARD_NAME "zgbl230"
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(zgbl231)
# define BOARD_TYPE  BOARD_ZGBL231
# define BOARD_NAME "zgbl231"
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(zgbl212)
# define BOARD_TYPE  BOARD_ZGBL212
# define BOARD_NAME "zgbl212"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(zgbh230)
# define BOARD_TYPE BOARD_ZGBH230
# define BOARD_NAME "zgbh230"
# define RADIO_TYPE (RADIO_AT86RF230B)
# define ZGBL_EVM_HOST (1)
#elif defined(zgbh231)
# define BOARD_TYPE  BOARD_ZGBH231
# define BOARD_NAME "zgbh231"
# define RADIO_TYPE (RADIO_AT86RF231)
# define ZGBL_EVM_HOST (1)
#elif defined(zgbh212)
# define BOARD_TYPE  BOARD_ZGBH212
# define BOARD_NAME "zgbh212"
# define RADIO_TYPE (RADIO_AT86RF212)
# define ZGBL_EVM_HOST (1)
#endif

#ifndef BOARD_ZGBL_H
#define BOARD_ZGBL_H


/*=== Compile time parameters ========================================*/

/*=== radio interface definition =====================================*/
#include "base_rdk2xx.h"

static inline void xmem_init(void)
{
    XMCRA |= _BV(SRE);
    XMCRB |= _BV(XMBK);
}


/*=== LED access macros ==============================================*/
#ifndef ZGBL_EVM_HOST
# define NO_KEYS       (1)
#else
/* EVM HOST memory mapped keys */
# define KEY_IO_AD     (0x4000)
# define PIN_KEY       (*(volatile uint8_t*)(KEY_IO_AD))
# define xDDR_KEY       DDRE
# define MASK_KEY      (0x1)
# define SHIFT_KEY     (0)
# define INVERSE_KEYS  (0)
# define PULLUP_KEYS   (0)
# define KEY_INIT      xmem_init
#endif
/*=== KEY access macros ==============================================*/
#ifndef ZGBL_EVM_HOST
# define NO_LEDS       (1)        /**< if defined, no LEDs are connected */
#else

/*
 * The EVM_HOST has the LEDs scattered around quite a bit, so the
 * standard methods in ioutil.h cannot be applied.
 * LED 0 (B5) is attached to IO_PB6
 * LED 1 (D7) is attached to PD7
 */

#define MASK_LED0 (_BV(6))
#define PORT_LED0 PORTB
#define DDR_LED0  DDRB
#define MASK_LED1 (_BV(7))
#define PORT_LED1 PORTD
#define DDR_LED1  DDRD

#define LED_INIT() \
  do { \
  DDR_LED0 |= MASK_LED0; \
  DDR_LED1 |= MASK_LED1; \
  PORT_LED0 &= ~MASK_LED0; \
  PORT_LED1 |= MASK_LED1;\
  } while (0)

#define LED_SET(x) \
  switch (x) { \
  case 0: PORT_LED0 &= ~MASK_LED0; break; \
  case 1: PORT_LED1 &= ~MASK_LED1; break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 0: PORT_LED0 |= MASK_LED0; break; \
  case 1: PORT_LED1 |= MASK_LED1; break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) PORT_LED0 &= ~MASK_LED0; else PORT_LED0 |= MASK_LED0; \
  if (x & 2) PORT_LED1 &= ~MASK_LED1; else PORT_LED1 |= MASK_LED1; \
  } while (0)

#define LED_GET_VALUE() ( \
  ((PORT_LED0 & MASK_LED0)? 0: 1) | \
  ((PORT_LED1 & MASK_LED1)? 0: 2) \
			  )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: PORT_LED0 ^= MASK_LED0; break; \
  case 1: PORT_LED1 ^= MASK_LED1; break; \
  }

#define LED_NUMBER (2)

#endif
/*
#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() \
        do{\
            EIMSK |= _BV(INT5);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            EIMSK &= ~_BV(INT5);\
        } while(0)

#define SLEEP_ON_KEY_vect INT5_vect
*/

#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() do{}while(0)

/*=== Host Interface ================================================*/
#define HIF_TYPE (HIF_UART_1)

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

#endif /* BOARD_ZGBL_H */
