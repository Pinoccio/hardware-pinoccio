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
 * @brief Definition 2.4G Module from In-Circquit with AT86RF230 and Atmega1281 (V1.0)
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
 PB0/PB4    --> SEL        (you need to drive both pins)
     PB5    --> SLP_TR
     PB6    --> TRX_RESET
INT4/PE4    --> IRQ
    XTAL2   <-- CLKM

    PD0     --> KEY Interface
    PD6:7   --> LED Interface

    Fuses:
      LF: 0xe2 - 8MHz internal RC Osc.
      HF: 0x11 - without boot loader
      HF: 0x10 - with boot loader
      EF: 0xff
      LOCK: 0xef - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>


@par Build Options

 - ict230 : Incirquit Module

 */

#ifndef BOARD_ICT230_H
#define BOARD_ICT230_H
#define BOARD_TYPE  (BOARD_ICT230)
#define BOARD_NAME  "ict230"

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

#ifndef PB0
# define PB0 (0)
# define PB1 (1)
# define PB2 (2)
# define PB3 (3)
# define PB4 (4)
# define PB5 (5)
# define PB6 (6)
# define PB7 (7)
#endif

/*=== Hardware Components ============================================*/
#define RADIO_TYPE (RADIO_AT86RF230)    /**< used radiio (see const.h)*/

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRB            /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTB           /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(PB6))      /**< PIN mask for RESET pin */
#define CUSTOM_RESET_TIME_MS (10)       /**< additional delay needed by hardware */

#define PORT_TRX_SLPTR  PORTB           /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRB            /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PB5))    /**< PIN mask for SLP_TR pin */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(INT4)    /**< interrupt mask for GICR */
# define TRX_IRQ_vect    INT4_vect    /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            EICRB  |= (_BV(ISC41)|_BV(ISC40));\
                          } while(0) /** rising edge triggers INT4 */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {EIMSK &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {EIMSK |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define DDR_SPI  (DDRB)     /**< DDR register for SPI port */
#define PORT_SPI (PORTB)    /**< PORT register for SPI port */

#define SPI_MOSI _BV(PB2)   /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(PB3)   /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(PB1)   /**< PIN mask for SCK pin */
#define SPI_SS   _BV(PB4)   /**< PIN mask for SS pin */

#define SPI_DATA_REG SPDR   /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |=  SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  |= _BV(PB0); /* need to make /SS pin to output in
                             in order to run safely in SPI
                             Master mode */
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |= SPI_SCK | SPI_SS;

    SPCR = (_BV(SPE) | _BV(MSTR));

    /* set spi rate */
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

/* stick settings */
#define LED_PORT      PORTD
#define LED_DDR       DDRD
#define LED_MASK      (0xc0)
#define LED_SHIFT     (6)
#define LEDS_INVERSE  (1)
#define LED_NUMBER    (2)

/*=== KEY access macros ==============================================*/
#define PORT_KEY     PORTD
#define PIN_KEY      PIND
#define DDR_KEY      DDRD
#define MASK_KEY     (0x01)
#define SHIFT_KEY    (0)
#define INVERSE_KEYS (1)
#define PULLUP_KEYS  (1)

/*=== Host Interface ================================================*/
#define HIF_TYPE    HIF_UART_1


/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (8)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (1000UL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
/**
 * @brief Initialisation of TIMER1 (16bit timer)
 *
 *  - CSxxx=b010: Prescaler = 8
 *  - WGMxxxx = b0100: Mode = 4 (CTC Mode)
 *  - OCR1A = 1000
 * Prescaler = 8 / Mode 4, CTC mit OCR1A = 1000
 * @ F_CPU = 8MHz this results in 1ms timer tick IRQ's.
 */
# define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS11) | _BV(WGM12)); \
        TIMSK1 |= _BV(OCIE1A); \
        OCR1A = 1000; \
    }while(0)

# define TIMER_IRQ_vect   TIMER1_COMPA_vect

#endif /* BOARD_ICT230_H */
