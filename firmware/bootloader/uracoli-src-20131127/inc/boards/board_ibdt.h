/* Copyright (c) 2010 Daniel Thiele
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
 * @brief IBDT Ranging Hardware with ATmega644 + AT86RF231
 *
 * The wiring of the radio and the ATmega is shown below:
 *
 * @todo correct wiring
 *
<pre>
     AVR       RF231
     ---       -----

     PD3:2   <--  KEY
     PD5:4 -->  LED

</pre>


@par Build Options

 */

/** ID String for this hardware */
#ifndef BOARD_IBDT_H
#define BOARD_IBDT_H

/*=== Hardware Components ============================================*/

#if defined(ibdt212)

#define BOARD_TYPE BOARD_IBDT212
#define BOARD_NAME  "ibdt212"
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF212)
#endif

#elif defined(ibdt231)

#define BOARD_TYPE BOARD_IBDT231
#define BOARD_NAME  "ibdt231"
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF231)
#endif

#elif defined(ibdt232)

#define BOARD_TYPE BOARD_IBDT232
#define BOARD_NAME  "ibdt232"
#ifndef RADIO_TYPE
#define RADIO_TYPE (RADIO_AT86RF232)
#endif

#else
#error "Unsupported BOARD_TYPE"
#endif

/*=== Compile time parameters ========================================*/

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRD            /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTD           /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(PD6))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTD           /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRD            /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(PD7))      /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

/*=== IRQ access macros ==============================================*/
# define TRX_IRQ         _BV(INT2)      /**< interrupt mask for GICR */
# define TRX_IRQ_vect    INT2_vect      /**< interrupt vector name */

/** configuration of interrupt handling */
# define TRX_IRQ_INIT()  do{\
                            EICRA = _BV(ISC20) | _BV(ISC21);\
                            EIMSK  |= (TRX_IRQ);\
                          } while(0) /** high level INT0 */

/** disable TRX interrupt */
#define DI_TRX_IRQ() {EIMSK &= (~(TRX_IRQ));}
/** enable TRX interrupt */
#define EI_TRX_IRQ() {EIMSK |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDRB)   /**< DDR register for SPI port */
#define PORT_SPI (PORTB)  /**< PORT register for SPI port */

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
#define LED_PORT      PORTD      /**< PORT register for LEDs */
#define LED_DDR       DDRD       /**< DDR register for LEDs */
#define LED_MASK      (0x30)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (4)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (1)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */


/*=== KEY access macros ==============================================*/
//#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */
#define PORT_KEY      PORTD      /**< PORT register for keys */
#define PIN_KEY       PIND       /**< PIN register for keys */
#define DDR_KEY       DDRD       /**< DDR register for keys */
#define MASK_KEY      (0x0C)     /**< MASK value for keys (msb aligned) */
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
            EIMSK |= _BV(PCIE0);\
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);\
            sleep_mode();\
            EIMSK &= ~_BV(PCIE0);\
        } while(0)

#define SLEEP_ON_KEY_vect PCINT0_vect


/*=== Host Interface ================================================*/
/** Type of the host interface. */
#define HIF_TYPE    HIF_UART_0

/*=== TIMER Interface ===============================================*/
/* setup timer of 1ms */
#define HWTMR_PRESCALE  (64)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU) /**< hardware
                                      timer clock period in us
                                      (usually: prescaler / F_CPU) */
#define HWTIMER_TICK_NB (0.001/HWTIMER_TICK) /**< number of hardware timer ticks,
                                        when IRQ routine is called, set to 1ms */
#define HWTIMER_REG (TCNT0) /**< name of the register where the clock
                                 ticks can be read */
#define TIMER_TICK (HWTIMER_TICK * HWTIMER_TICK_NB)
                            /**< period in us, when the timer interrupt
                                 routine is called */

#define TIMER_POOL_SIZE (8) /**< number of software timers running at
                                 a time */

#define TIMER_INIT() \
    do{\
        TCCR0B = _BV(CS01) | _BV(CS00);\
        TCCR0B |= _BV(WGM02); \
        OCR0A = HWTIMER_TICK_NB; \
        TIMSK0 |= _BV(OCIE0A);\
    }while(0)

/** symbolic name of the timer interrupt routine that is called */
#define TIMER_IRQ_vect  TIMER0_COMPA_vect

#endif /* BOARD_IBDT_H*/
