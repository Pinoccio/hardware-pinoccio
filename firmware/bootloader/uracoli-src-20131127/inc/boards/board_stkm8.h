/* Copyright (c) 2007 - 2009 Axel Wachtler
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
 * @brief AT86RF230 adapter board wired to STK500 with Atmega8(L)/Atmega88(L).
 * 
 *
 *
 * The wiring of the radio and the ATmega is shown below:
<pre>
     AVR         RF230
     ---         -----
     PB0    <--  INT
     PB1    -->  TXCW
     PB2    -->  /SEL
     PB3    -->  MOSI
     PB4    <--  MISO
     PB5    -->  SCK
     PB6 intentionally unused for XTAL1
     PB7 intentionally unused for XTAL2

     PC0:6


    (T1) PD5   <--  CLKM
         PD6   --> /RST
         PD7   --> SLPTR

    USART:
        PD0 RXD
        PD1 TXD

    LED:  PC0:1
    KEY:  PD2:3

    Fuses:
     LF: 0xe1
     HF: 0xd9

</pre>

  @par Build Options

 - stkm8 : AT86RF230 adapter board wired to STK500 with Atmega8(L)/Atmega88(L).

 */


#ifndef BOARD_STK_M8_H
#define BOARD_STK_M8_H

#define BOARD_TYPE  (BOARD_STKM8)
#define BOARD_NAME "stkm8"

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#define RADIO_TYPE (RADIO_AT86RF230B)

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRD
#define PORT_TRX_RESET  PORTD
#define MASK_TRX_RESET  (_BV(PD6))
#define TRX_RESET_INIT() DDR_TRX_RESET |= MASK_TRX_RESET
#define TRX_RESET_HIGH() PORT_TRX_RESET |= MASK_TRX_RESET
#define TRX_RESET_LOW()  PORT_TRX_RESET &= ~MASK_TRX_RESET

#define PORT_TRX_SLPTR  PORTD
#define DDR_TRX_SLPTR   DDRD
#define MASK_TRX_SLPTR  (_BV(PD7))

#define TRX_SLPTR_INIT() DDR_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_HIGH() PORT_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_LOW()  PORT_TRX_SLPTR &= ~MASK_TRX_SLPTR


/*=== IRQ access macros ==============================================*/
# define TRX_IRQ_vect    TIMER1_CAPT_vect    /**< interrupt vector name */

/** init interrupt handling
 *  - rising edge triggers ICP1 (ICES1),
 *  - timer capture is enabled (ICF1)
 */
# define TRX_IRQ_INIT()  do{\
                            /* TCCR1B |= (_BV(ICNC1) | _BV(ICES1) | _BV(CS12) | _BV(CS10)); */\
                            TCCR1B |= (_BV(ICNC1) | _BV(ICES1));\
                            TIFR = _BV(ICF1);\
                          } while(0)

/** disable TRX interrupt */
#define DI_TRX_IRQ() {TIMSK &= ~_BV(TICIE1);}

/** enable TRX interrupt */
#define EI_TRX_IRQ() {TIMSK |= _BV(TICIE1);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1
/*=== SPI access macros ==============================================*/
#define DDR_SPI  (DDRB)
#define PORT_SPI (PORTB)

#define SPI_MOSI _BV(PB3)
#define SPI_MISO _BV(PB4)
#define SPI_SCK  _BV(PB5)
#define SPI_SS   _BV(PB2)

#define SPI_DATA_REG SPDR

static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    PORT_SPI |= SPI_SCK | SPI_SS;
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;

    SPCR = (_BV(SPE) | _BV(MSTR));

    SPCR &= ~(_BV(SPR1) | _BV(SPR0) );
    SPSR &= ~_BV(SPI2X);

    SPCR |= (spirate & 0x03);
    SPSR |= ((spirate >> 2) & 0x01);

}

#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
#define SPI_WAITFOR()        do { while((SPSR & _BV(SPIF)) == 0);} while(0)

/*=== LED access macros ==============================================*/
#define LED_PORT      PORTC
#define LED_DDR       DDRC
#define LED_MASK      (0x03)
#define LED_SHIFT     (0)
#define LED_NUMBER    (2)
#define LEDS_INVERSE  (1)

/*=== KEY access macros ==============================================*/
#define PORT_KEY      PORTD
#define PIN_KEY       PIND
#define DDR_KEY       DDRD
#define MASK_KEY     (0x0c)
#define SHIFT_KEY    (2)
#define INVERSE_KEYS (1)
#define PULLUP_KEYS  (1)

/*=== Host Interface ================================================*/
#define HIF_TYPE    HIF_UART_0


/*=== TIMER Interface ===============================================*/
/**
 * Mode: normal
 * Prescaler: 1
 * Overflow: 0xFFFFUL
 */
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10)); \
        TIMSK |= _BV(TOIE1); \
    }while(0)
# define TIMER_IRQ_vect   TIMER1_OVF_vect

#endif /* BOARD_STK_M8_H */
