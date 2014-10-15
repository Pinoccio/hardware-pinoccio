/* Copyright (c) 2008 Axel Wachtler
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
 * @brief Meshnetics Meshbean Development Board WDB-A1281-(P1,A1,E2,Amp)
 *        with ATmega1281 and AT86RF230.
 *
 * See also
 * - <a href="http://www.meshnetics.com/dev-tools/meshbean/"
 *      target="_ext">http://www.meshnetics.com/dev-tools/meshbean/</a>
 * - http://www.mikrocontroller.net/articles/Meshnetics_Zigbee
 *
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
     AVR      RF230
     ---      -----
     PB4  -->  SLPTR
     PE5  <--  IRQ (INT5)
     PA7  -->  RSTN
     PB0  -->  SS
     PB2  -->  MOSI
     PB3  <--  MISO
     PB1  -->  SCK
     XTAL1 <--  MCLK

           Meshbean - AVR
     Key1  - IRQ6   - PE6 (INT6)
     Key2  - IRQ7   - PE7 (INT7)

     SW1   - GPIO3  - PG0
     SW2   - GPIO4  - PG1
     SW3   - GPIO5  - PG1

     LED1  - GPIO0  - PB5
     LED2  - GPIO1  - PB6
     LED3  - GPIO2  - PB7

           Bitbean - AVR

     Key1  - IRQ7   - PE7 (INT7)
     LED1  - GPIO0  - PB5
     LED2  - GPIO1  - PB6
     LED3  - GPIO2  - PB7
             GPIO5  - PG2 (enable HIF_IO)
             GPIO6  - PD6
             GPIO7  - PD7 (enable I2C, high active)
Fuses/Locks:
    uracoli settings:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

    original settings
     LF: 0x62 - ... ? todo
     HF: 0x1d - ... ?
     EF: 0xff

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

 - Host Interface: @ref HIF_TYPE


@par Build Options

 - wdba1281 : 2.4GHz Meshbeand with ZigBit (ATmega1281 + AT86RF230B)
 - mn900    : 900MHz Meshbeand with ZigBit (ATmega1281 + AT86RF212)
 */

#if defined(mnb900)
# define BOARD_TYPE (BOARD_MNB900)
# define BOARD_NAME "mnb900"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(wdba1281)
# define BOARD_TYPE (BOARD_WDBA1281)
# define BOARD_NAME "wdba1281"
# define RADIO_TYPE (RADIO_AT86RF230)
#elif defined(bitbean)
# define BOARD_TYPE (BOARD_BITBEAN)
# define BOARD_NAME "bitbean"
# define RADIO_TYPE (RADIO_AT86RF230)
#endif

#ifndef BOARD_WDBA1281_H
#define BOARD_WDBA1281_H

/*=== Compile time parameters ========================================*/

/*=== radio interface definition =====================================*/
#include "base_zdma1281.h"

/*=== LED access macros ==============================================*/
#define LED_PORT PORTB      /**< PORT register for LED */
#define LED_DDR  DDRB       /**< DDR register for LED */
#define LED_MASK 0xe0       /**< MASK value for LED */
#define LED_SHIFT     (5)   /**< SHIFT value for LED */
#define LEDS_INVERSE  (0)   /**< a 1 at the PORT means the LED is off */
#define LED_NUMBER    (3)   /**< number of LEDs for this board */

/*=== KEY access macros ==============================================*/
#define PORT_KEY PORTE
#define PIN_KEY  PINE
#define DDR_KEY  DDRE


#if defined(bitbean)
# define MASK_KEY     (0x80)
# define SHIFT_KEY    (7)
# define INVERSE_KEYS (1)
# define PULLUP_KEYS  (1)

# define SLEEP_ON_KEY_INIT() do{}while(0)
# define SLEEP_ON_KEY() \
        do{\
            EIMSK |= _BV(INT7);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            EIMSK &= ~_BV(INT7);\
        } while(0)

# define SLEEP_ON_KEY_vect INT7_vect
#else
# define MASK_KEY     (0xc0)
# define SHIFT_KEY    (6)
# define INVERSE_KEYS (1)
# define PULLUP_KEYS  (1)

# define SLEEP_ON_KEY_INIT() do{}while(0)
# define SLEEP_ON_KEY() \
        do{\
            EIMSK |= _BV(INT6);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            EIMSK &= ~_BV(INT6);\
        } while(0)

# define SLEEP_ON_KEY_vect INT6_vect
#endif

/*=== Host Interface ================================================*/
/**
 * On the Meshbean board UART1 is routed either via a
 * FT232BM USB converter to connector CN1 or via a RS232 level
 * shifter to header P2. The selection is done with jumper J3.
 *
 */
#define HIF_TYPE (HIF_UART_1)
#if defined(bitbean)
  /**
   * Enable Level Shifter Cirquit.
   *
   * For BitBean we need to enable three control lines of the
   * MAX3221EUE.
   * - /F_OFF : GPIO3 : PG0 : 1
   * - F_ON   : GPIO4 : PG1 : 0
   * - /EN    : GPIO5 : PG2 : 0

   */
# define HIF_IO_ENABLE() \
   do{ DDRG |= 0x07; PORTG |= 0x01;}while(0);
#endif
/*=== TIMER Interface ===============================================*/
/**
 *  - CS1[2:0]  = 1 : Prescaler = 1
 *  - WGM1[3:0] = 0 : Mode = 4 : CTC operation
 *
 * Timer is clocked at F_CPU, and @ref TIMER_IRQ_vect is called every
 * 65535 ticks.
 */
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL+1)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10));\
        TIMSK1 |= _BV(TOIE1); \
    }while(0)
#define TIMER_IRQ_vect  TIMER1_OVF_vect

/**
 * Enable U4 driver chip for I2C
 */
#define I2C_ENABLE() do{\
        DDRD |= (1<<PD7); \
        PORTD |= (1<<PD7); }while(0)

#define LM73_ADDR (0x49)
#endif /* BOARD_WDBA1281_H*/
