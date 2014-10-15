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
 * @brief Definition of Atmel Raven Development Kit, AT86RF230 Radio Adapter with Atmega1284.
 *
 * The wiring of the radio and the Atmega1284 is shown below:
 *
<pre>
     AVR          AT86RF230
     ---            ---------
     PB3      -->   SLPTR
     XTAL1    <--   MCLK
     PD6/ICP  <--   IRQ
     PB1      -->   RSTN
     PB4      -->   /SEL
     PB5      -->   MOSI
     PB6      <--   MISO
     PB7      -->   SCLK
     PB0      -->   TST

    LEDs: None
    KEYs: None

Fuses/Locks?????:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Original Fuses/Locks
     LF: 0x62
     HF: 0x18
     EF: 0xff
     LOCK: 0xff

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

@par Build Options

 - ravrf : Raven

 */

#ifndef BOARD_RAVRF_H
#define BOARD_RAVRF_H

#if defined(ravrf230a)
# define BOARD_TYPE (BOARD_RAVRF230A)
# define BOARD_NAME "ravrf230a"
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(ravrf230b)
# define BOARD_TYPE (BOARD_RAVRF230B)
# define BOARD_NAME "ravrf230b"
# define RADIO_TYPE (RADIO_AT86RF230B)
#endif

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF230A)    /**< used radiio (see const.h)*/
#endif

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRB            /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTB           /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(PB1))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTB           /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRB            /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PB3))      /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(ICIE1)     /**< interrupt mask for GICR */
# define TRX_IRQ_vect    TIMER1_CAPT_vect    /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            TCCR1B |= (_BV(ICNC1) | _BV(ICES1));\
                            TIFR1 = _BV(ICF1);\
                          } while(0) /** rising edge triggers INT... */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {TIMSK1 &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {TIMSK1 |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG ICR1

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDRB)    /**< DDR register for SPI port */
#define PORT_SPI (PORTB)   /**< PORT register for SPI port */

#define SPI_MOSI _BV(PB5)  /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(PB6)  /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(PB7)  /**< PIN mask for SCK pin */
#define SPI_SS   _BV(PB4)  /**< PIN mask for SS pin */

#define SPI_DATA_REG SPDR  /**< abstraction for SPI data register */


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

/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */

/*=== Host Interface ================================================*/
/** Type of the host interface. */
#define HIF_TYPE    HIF_UART_0

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

/** symbolic name of the timer interrupt routine that is called */
#define TIMER_IRQ_vect   TIMER1_OVF_vect

#endif /* BOARD_RAVRF_H*/
