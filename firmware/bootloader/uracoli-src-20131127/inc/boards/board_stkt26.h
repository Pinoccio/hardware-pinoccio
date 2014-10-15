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
 * @brief AT86RF230 adapter board wired to STK500 with ATtiny26.
 *
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
     AVR       RF230
     ---       -----
     PB0  -->  MOSI
     PB1  <--  MISO
     PB2  -->  SCK
     PB3  -->  SS
     PB4  -->  RSTN
     PB5  -->  SLPTR
     PB6  <--  IRQ (INT0)
               MCLK (NC)
     PB7  -->  (reset)

    PA0:3 <--  KEYS
    PA4:7 -->  LED


</pre>

@par Build Options

 - stkt26 : AT86RF230 adapter board wired to STK500 with ATtiny26.

 */

/** ID String for this hardware */
#define BOARD_NAME_STKT26 "stkt26"

#ifndef BOARD_STKT26_H
#define BOARD_STKT26_H

#define BOARD_TYPE (BOARD_STKT26) /**< current board type (see const.h)*/
#define BOARD_NAME "stkt26"       /**< current board name */

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#define RADIO_TYPE (RADIO_AT86RF230)    /**< used radiio (see const.h)*/

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRB            /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTB           /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(PB4))      /**< PIN mask for RESET pin */

#define PORT_TRX_SLPTR  PORTB           /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRB            /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PB5))      /**< PIN mask for SLP_TR pin */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(INT0)      /**< interrupt mask for GICR */
# define TRX_IRQ_vect    INT0_vect      /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            GIMSK  |= _BV(INT0);\
                          } while(0) /** rising edge triggers INT... */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {GIMSK &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {GIMSK |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_USI
#define DDR_SPI  (DDRB)     /**< DDR register for SPI port */
#define PORT_SPI (PORTB)    /**< PORT register for SPI port */

#define SPI_MOSI _BV(PB0)   /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(PB1)   /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(PB2)   /**< PIN mask for SCK pin */
#define SPI_SS   _BV(PB3)   /**< PIN mask for SS pin */

#define SPI_DATA_REG USIDR  /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    PORT_SPI |= SPI_SCK | SPI_SS;
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()  \
    do \
    { \
        USISR |= _BV(USIOIF); \
        do \
        { \
            USICR = (_BV(USIWM0)+_BV(USICS1)+_BV(USICLK)+_BV(USITC)); \
        } while ((USISR & _BV(USIOIF)) == 0); \
    } while(0)

/*=== LED access macros ==============================================*/
#define LED_PORT      PORTA
#define LED_DDR       DDRA
#define LED_MASK      (0xf0)
#define LED_SHIFT     (4)
#define LED_NUMBER    (4)
#define INVERSE_KEYS  (1)

/*=== KEY access macros ==============================================*/
#define PORT_KEY     PORTA
#define PIN_KEY      PINA
#define DDR_KEY      DDRA
#define MASK_KEY     (0x0f)
#define SHIFT_KEY    (0)
#define LEDS_INVERSE (0)

/**@}*/

/*=== TIMER Interface ===============================================*/
/**
 * Mode: CTC
 * Prescaler: 1
 * Overflow: 0xFF
 */
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xffUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE  (1)
#define TIMER_INIT() \
    do{\
        TCCR0 |= _BV(CS00);\
        TIMSK |= _BV(TOIE0);\
    }while(0)
#define TIMER_IRQ_vect  TIMER0_OVF0_vect
/*=== Host Interface ================================================*/
#define HIF_TYPE   (HIF_NONE)

s#endif /* BOARD_STKT26_H */
