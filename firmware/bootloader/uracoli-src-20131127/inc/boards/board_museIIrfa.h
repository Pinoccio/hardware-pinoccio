/* Copyright (c) 2009 - 2011 Axel Wachtler
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
 * @brief MuseII RFA
 * 
 *
 *
<pre>

</pre>


@par Build Options

 */

#ifndef BOARD_MUSEIIRFA_H
#define BOARD_MUSEIIRFA_H

# define BOARD_TYPE BOARD_MUSEIIRFA
# define BOARD_NAME "museIIrfa"
/*=== Compile time parameters ========================================*/

/*=== Hardware Components ============================================*/
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

#define LED_PORT      PORTE      /**< PORT register for LEDs */
#define LED_DDR       DDRE       /**< DDR register for LEDs */
#define LED_MASK      (0x18)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (3)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (0)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */

/* special handling for this board to enable light measurement */
#define LED_PIN       PINE       /**< PIN register for LEDs */
#define LED_ANODE_bp   (3)		/**< LED anode bit position */
#define LED_CATHODE_bp (4)		/**< LED cathode bit position */

/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */

/*=== Host Interface ================================================*/
# define HIF_TYPE    HIF_NONE

#define TRX_RESET_LOW()   do { TRXPR &= ~(1<<TRXRST); } while (0)
#define TRX_RESET_HIGH()  do { TRXPR |= (1<<TRXRST); } while (0)

#define TRX_SLPTR_LOW()   do { TRXPR &= ~(1<<SLPTR); } while (0)
#define TRX_SLPTR_HIGH()  do { TRXPR |= (1<<SLPTR); } while (0)

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= ((1<<CS10)); \
        TIMSK1 |= (1<<TOIE1); \
    }while(0)
#define TIMER_IRQ_vect   TIMER1_OVF_vect

#endif /* BOARD_MUSEIIRFA_H */
