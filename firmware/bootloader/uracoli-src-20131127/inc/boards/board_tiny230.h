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
 * @brief Minimalistic Board with ATtiny84 / AT86RF230 by DL8DTL.
 *
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
     AVR       RF230
     ---       -----
     PA5  -->  MOSI
     PA6  <--  MISO
     PA4  -->  SCK
     PA7  -->  SS
     PB0  -->  RSTN
     PB1  -->  SLPTR
     PB2  <--  IRQ (INT0)
               MCLK (NC)
     PB7  -->  (reset)

     PA2   <--  KEY
     PA0:1 -->  LED
     PA3   <-- Thermo Sensor

     lfuse: 0x62 (8MHz in Osc., no clock divider)
     lfuse: 0xe2 (8MHz in Osc., w/ clock divider)
     hfuse: 0xDF for ISP Mode, 0x9F for debug wire
     efuse: 0xFF


</pre>

    @image html dl8dtl_tiny230.jpg "Tiny230 Radio Controller Board"
    @image latex dl8dtl_tiny230.jpg "Tiny230 Radio Controller Board"

@par Build Options

  - tiny230 : Radio Controller Board by Joerg Wunsch

 */

/** ID String for this hardware */
#ifndef BOARD_TINY230_H
#define BOARD_TINY230_H

#if defined(tiny230)
# define BOARD_NAME "tiny230"
# define BOARD_TYPE (BOARD_TINY230)       /**< current board type (see const.h)*/
# define RADIO_TYPE (RADIO_AT86RF230B)    /**< used radiio (see const.h)*/
#elif defined(tiny231)
# define BOARD_NAME "tiny231"
# define BOARD_TYPE (BOARD_TINY231)       /**< current board type (see const.h)*/
# define RADIO_TYPE (RADIO_AT86RF231)    /**< used radiio (see const.h)*/
#endif

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF230B)    /**< used radiio (see const.h)*/
#endif

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRB            /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTB           /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(PB0))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTB           /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRB            /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PB1))      /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(INT0)      /**< interrupt mask for GICR */
# define TRX_IRQ_vect    INT0_vect      /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            MCUCR = _BV(ISC00) | _BV(ISC01);\
                            GIMSK  |= _BV(INT0);\
                          } while(0) /** high level INT0 */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {GIMSK &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {GIMSK |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_USI
#define DDR_SPI  (DDRA)   /**< DDR register for SPI port */
#define PORT_SPI (PORTA)  /**< PORT register for SPI port */

#define SPI_MOSI _BV(PA5)  /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(PA6)  /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(PA4)  /**< PIN mask for SCK pin */
#define SPI_SS   _BV(PA7)  /**< PIN mask for SS pin */

#define SPI_DATA_REG USIDR  /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |=  SPI_SS | SPI_MISO;
}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()  \
    do \
    { \
        USISR = _BV(USIOIF); \
        do \
        { \
            USICR = _BV(USIWM0)|_BV(USICS1)|_BV(USICLK)|_BV(USITC); \
        } while ((USISR & _BV(USIOIF)) == 0); \
    } while(0)

/*=== LED access macros ==============================================*/
#define LED_PORT      PORTA      /**< PORT register for LEDs */
#define LED_DDR       DDRA       /**< DDR register for LEDs */
#define LED_MASK      (0x03)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (0)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (0)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */


/*=== KEY access macros ==============================================*/
//#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */
#define PORT_KEY      PORTA      /**< PORT register for keys */
#define PIN_KEY       PINA       /**< PIN register for keys */
#define DDR_KEY       DDRA       /**< DDR register for keys */
#define MASK_KEY      (0x04)     /**< MASK value for keys (msb aligned) */
#define SHIFT_KEY     (2)        /**< SHIFT value for keys */
#define INVERSE_KEYS  (1)        /**< = 1, if low level at port
                                      means KEY pressed */
#define PULLUP_KEYS  (1)

#define SLEEP_ON_KEY_INIT() \
        do{\
            PCMSK0 |= _BV(PCINT2);\
        }while(0)

#define SLEEP_ON_KEY() \
        do{\
            GIMSK |= _BV(PCIE0);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            GIMSK &= ~_BV(PCIE0);\
        } while(0)

#define SLEEP_ON_KEY_vect PCINT0_vect


/*=== Host Interface ================================================*/
/** Type of the host interface. */
#define HIF_TYPE    HIF_NONE

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU) /**< hardware
                                      timer clock period in us
                                      (usually: prescaler / F_CPU) */
#define HWTIMER_TICK_NB (0xffffUL) /**< number of hardware timer ticks,
                                        when IRQ routine is called */
#define HWTIMER_REG (TCNT1) /**< name of the register where the clock
                                 ticks can be read */
#define TIMER_TICK (HWTIMER_TICK * HWTIMER_TICK_NB)
                            /**< period in us, when the timer interrupt
                                 routine is called */

#define TIMER_POOL_SIZE (1) /**< number of software timers running at
                                 a time */

#define TIMER_INIT() \
    do{\
        TCCR1B |= _BV(CS10);\
        TIMSK1 |= _BV(TOIE1);\
    }while(0)

/** symbolic name of the timer interrupt routine that is called */
#define TIMER_IRQ_vect  TIM1_OVF_vect

#endif /* BOARD_TINY230_H*/
