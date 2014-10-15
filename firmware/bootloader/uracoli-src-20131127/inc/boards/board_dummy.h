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
 * @file
 * @ingroup grpBoard
 * @brief Dummy board definition for (board type) with (CPU type).
 *
 * Take this file as a template for a new board implementation.
 *
 */
/**
 * @addtogroup grpBoard
 * @{
 */

/*%*
 *
 * @defgroup grp<brD> ....
 * @brief .....
 *
 * 
 *
 * The wiring of the radio and the AT.... is shown below:
*
<pre>
     AVR      RF230
     ---      -----
     Pxx  -->  SLPTR
     Pxx  <--  MCLK
     Pxx  <--  IRQ (INT2)
     Pxx  -->  RSTN
     Pxx  -->  SS
     Pxx  -->  MOSI
     Pxx  <--  MISO
     Pxx  -->  SCK

     <other peripherals>
</pre>
 */
/*%*
 * @addtogroup grpBoard
 * @{
 */

/** ID String for this hardware */
#define BOARD_NAME_<brd> "<brd>"

#ifndef BOARD_DUMMY_H
#define BOARD_DUMMY_H

#define BOARD_TYPE (<brd>)              /**< current board type (see const.h)*/
#define BOARD_NAME BOARD_NAME_<brd>     /**< current board name */

/*=== Compile time parameters ========================================*/
#ifndef MAX_FRAME_SIZE
# define MAX_FRAME_SIZE (127) /**< maximum allowed frame size */
#endif

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF230)    /**< used radiio (see const.h)*/
#endif

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDR...          /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORT...         /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(...))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORT...         /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDR...          /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PB...))    /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(INT...)    /**< interrupt mask for GICR */
# define TRX_IRQ_vect    INT..._vect    /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            MCUCSR  |= _BV(ISC...);\
                          } while(0) /** rising edge triggers INT... */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {GICR... &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {GICR... |= (TRX_IRQ);}

/** timestamp register for RX_START event (either TCNTx or ICRx)*/
#define TRX_TSTAMP_REG ...

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDR...)   /**< DDR register for SPI port */
#define PORT_SPI (PORT...)  /**< PORT register for SPI port */

#define SPI_MOSI _BV(P...)  /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(P...)  /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(P...)  /**< PIN mask for SCK pin */
#define SPI_SS   _BV(P...)  /**< PIN mask for SS pin */

#define SPI_DATA_REG SPDR...    /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |= SPI_SCK | SPI_SS;

    SPCR = (_BV(SPE) | _BV(MSTR));

    SPCR &= ~(_BV(SPR1) | _BV(SPR0) );
    SPSR &= ~_BV(SPI2X);

    SPCR |= (spirate & 0x03);
    SPSR |= ((spirate >> 2) & 0x01);

}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()        do { while((SPSR & _BV(SPIF)) == 0);} while(0)

/*=== LED access macros ==============================================*/
#define NO_LEDS       (1)        /**< if defined, no LEDs are connected */
#define LED_PORT      PORT.      /**< PORT register for LEDs */
#define LED_DDR       DDR.       /**< DDR register for LEDs */
#define LED_MASK      (0x..)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (.)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (.)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (.)        /**< number of LEDs for this board */


/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */
#define PORT_KEY      PORT.      /**< PORT register for keys */
#define PIN_KEY       PIN.       /**< PIN register for keys */
#define DDR_KEY       DDR.       /**< DDR register for keys */
#define MASK_KEY      (0x..)     /**< MASK value for keys (msb aligned) */
#define SHIFT_KEY     (.)        /**< SHIFT value for keys */
#define INVERSE_KEYS  (.)        /**< if 1, low level at port means that
                                      KEY is pressed */
#define PULLUP_KEYS   (.)        /**< if 1, port bits are set to enable
                                      internal pullup resistors of the MCU */

/*=== Host Interface ================================================*/
/** Type of the host interface. */
#define HIF_TYPE    HIF_.

/*=== TIMER Interface ===============================================*/
#define NO_TIMER (1)

#define HWTMR_PRESCALE  (1) /**< hardware timer prescale factor */
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU) /**< hardware timer clock period in us
                                      (usually: prescaler / F_CPU) */
#define HWTIMER_TICK_NB (0xFFFFUL) /**< number of hardware timer ticks,
                                        when IRQ routine is called */
#define HWTIMER_REG (TCNT1) /**< name of the register where the clock
                                 ticks can be read */
#define TIMER_TICK (HWTIMER_TICK * HWTIMER_TICK_NB)
                            /**< period in us, when the timer interrupt
                                 routine is called */

#define TIMER_POOL_SIZE (0) /**< number of software timers running at
                                 a time */

#define TIMER_INIT() do{}while(0) /**< macro that initializes the
                                       hardware timer */

/** symbolic name of the timer interrupt routine that is called */
#define TIMER_IRQ_vect   ...


/*=== OSCCAL tuning =================================================*/
#ifndef TUNED_OSCCAL
# define TUNED_OSCCAL (0xbf)  /* default is 0xb1, but @2.9V 0xbf is better */
#endif
#endif /** @} BOARD_DUMMY_H*/
