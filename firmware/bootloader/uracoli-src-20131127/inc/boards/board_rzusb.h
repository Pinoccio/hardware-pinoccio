/* Copyright (c) 2007 Axel Wachtler
   Copyright (c) 2008 Joerg Wunsch
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
 * @brief Atmel Raven Development Kit, USB Stick, AT86RF230 Radio Adapter with Atmega1287.
 *
 *
 * This board wiring fits the Atmel radio development kit
 * hardware.
 *
 * The wiring of the radio and the AT90USB1287 is shown below:
 *
<pre>
     AVR            RF230
     ---            -----
     PB4      -->  SLPTR
     PD6/T1   <--  MCLK
     PD4/ICP1 <--  IRQ
     PB5      -->  RSTN
     PB0      -->  SEL
     PB2      -->  MOSI
     PB3      <--  MISO
     PB1      -->  SCLK
     PB7      -->  TST

     Schematic   AVR  Index
     ---------   ---  -----
     LED2:       PD5  (#0) red
     LED3:       PE7  (#1) green
     LED4:       PE6  (#2) yellow
     LED1:       PD7  (#3) blue (VBUS, USB supply)

Fuses/Locks?????:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Original Fuses/Locks
     LF: 0xde
     HF: 0x91
     EF: 0xfb
     LOCK: 0xff

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

@par Build Options

  - rzusb : Raven USB Stick

 */

#if defined(rzusb)
# define BOARD_TYPE  BOARD_RZUSB
# define BOARD_NAME  "rzusb230"
# define RADIO_TYPE  (RADIO_AT86RF230B)
#endif

#ifndef BOARD_RDK230_H
#define BOARD_RDK230_H

/*=== Compile time parameters ========================================*/

/*=== radio interface definition =====================================*/
/* the tranceiver wiring fits the RCB/RDK settings */
#include "base_rdk230.h"

/*=== LED access macros ==============================================*/
/*
 * The RZUSBSTICK has the LEDs scattered around quite a bit, so the
 * standard methods in ioutil.h cannot be applied.  LED 1 is attached
 * to PD7 using positive logic, and powered by Vbus.  For this reason,
 * it is not counted as a standard LED, and gets number 3 assigned.
 * LED 2 is attached to PD5, LED 3 to PE7, and LED 4 to PE6, all using
 * negative logic.  They get assigned numbers 0, 1, and 2,
 * respectively.
 */

#define LED_INIT() \
  do { \
  DDRD |= _BV(7) | _BV(5); DDRE |= _BV(7) | _BV(6); \
  PORTD &= ~_BV(7); PORTD |= _BV(5); PORTE |= _BV(7) | _BV(6); \
  } while (0)

#define LED_SET(x) \
  switch (x) { \
  case 3: PORTD |= _BV(7); break; \
  case 0: PORTD &= ~_BV(5); break; \
  case 1: PORTE &= ~_BV(7); break; \
  case 2: PORTE &= ~_BV(6); break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 3: PORTD &= ~_BV(7); break; \
  case 0: PORTD |= _BV(5); break; \
  case 1: PORTE |= _BV(7); break; \
  case 2: PORTE |= _BV(6); break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) PORTD &= ~_BV(5); else PORTD |= _BV(5); \
  if (x & 2) PORTE &= ~_BV(7); else PORTE |= _BV(7); \
  if (x & 4) PORTE &= ~_BV(6); else PORTE |= _BV(6); \
  } while (0)

#define LED_GET_VALUE() ( \
  ((PORTD & _BV(5))? 0: 1) | \
  ((PORTE & _BV(7))? 0: 2) | \
  ((PORTE & _BV(6))? 0: 4) \
			  )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 3: PORTD ^= _BV(7); break; \
  case 0: PORTD ^= _BV(5); break; \
  case 1: PORTE ^= _BV(7); break; \
  case 2: PORTE ^= _BV(6); break; \
  }

#define LED_SHIFT (0) /* not useful */
#define LED_NUMBER (3)

#define LED_ACTIVITY (0)
#define LED_USB_CONFIGURED (3)


/*=== KEY access macros ==============================================*/
#define NO_KEYS

/*=== Host Interface ================================================*/
#define HIF_TYPE (HIF_AT90USB)

//#define USB_VID  0x3eb          /* Atmel */
#define USB_VID             URACOLI_USB_VID
#define USB_PID             URACOLI_USB_PID
#define USB_BCD_RELEASE     URACOLI_USB_BCD_RELEASE
#define USB_VENDOR_NAME     URACOLI_USB_VENDOR_NAME
#define USB_PRODUCT_NAME    URACOLI_USB_PRODUCT_NAME

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

#endif /* BOARD_RDK230_H*/
