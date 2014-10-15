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
 * @brief AT86RF230 adapter board wired to STK500 with Atmega16(L).
 *
 *
 *
 * The wiring of the radio and the ATmega is shown below:
<pre>
     AVR      RF230
     ---      -----
     PB0 -->  RSTN
     PB1 <--  MCLK
     PB2 <--  IRQ (INT2)
     PB3 -->  SLPTR
     PB4 -->  SS
     PB5 -->  MOSI
     PB6 <--  MISO
     PB7 -->  SCK

     PA0:3 --> KEY0:3
     PA4:7 --> LED0:3

Fuses/Locks:
     LF: 0xe4 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     LOCK: 0xef - protection of boot section

Bootloader:
     Start at byte=0x3800, address=0x1c0, size = 1024 instructions/2048 bytes

</pre>

  @par Build Options

 - stkm16 : AT86RF230 adapter board wired to STK500 with Atmega16(L).

 */


#ifndef BOARD_STK_M16_H
#define BOARD_STK_M16_H (1)

/** ID String for this hardware */
#define BOARD_NAME "stkm16"
/** Board Type */
#define BOARD_TYPE  (BOARD_STKM16)

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/
#define RADIO_TYPE (RADIO_AT86RF230)

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRB
#define PORT_TRX_RESET  PORTB
#define MASK_TRX_RESET  (_BV(PB0))
#define TRX_RESET_INIT() DDR_TRX_RESET |= MASK_TRX_RESET
#define TRX_RESET_HIGH() PORT_TRX_RESET |= MASK_TRX_RESET
#define TRX_RESET_LOW()  PORT_TRX_RESET &= ~MASK_TRX_RESET

#define PORT_TRX_SLPTR  PORTB
#define DDR_TRX_SLPTR   DDRB
#define MASK_TRX_SLPTR  (_BV(PB3))

#define TRX_SLPTR_INIT() DDR_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_HIGH() PORT_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_LOW()  PORT_TRX_SLPTR &= ~MASK_TRX_SLPTR


/*=== IRQ access macros ==============================================*/
#if 1
# define TRX_IRQ         _BV(INT2)
# define TRX_IRQ_vect    INT2_vect
# define TRX_IRQ_INIT()  do{\
                            MCUCSR  |= _BV(ISC2);\
                          } while(0) /** rising edge triggers INT2 */
#else
# define TRX_IRQ         _BV(INT0)
# define TRX_IRQ_vect    INT0_vect
# define TRX_IRQ_INIT()  do{\
                            MCUCR  |= _BV(ISC01) | _BV(ISC00);\
                          } while(0) /** rising edge triggers INT0 */
#endif

#define DI_TRX_IRQ() {GICR &= (~(TRX_IRQ));}
#define EI_TRX_IRQ() {GICR |= (TRX_IRQ);}

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define DDR_SPI  (DDRB)
#define PORT_SPI (PORTB)

#define SPI_MOSI _BV(PB5)
#define SPI_MISO _BV(PB6)
#define SPI_SCK  _BV(PB7)
#define SPI_SS   _BV(PB4)

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
#define LED_PORT      PORTA
#define LED_DDR       DDRA
#define LED_MASK      (0xf0)
#define LED_SHIFT     (4)
#define LEDS_INVERSE  (1)

/*=== KEY access macros ==============================================*/
#define PORT_KEY       PORTA
#define PIN_KEY        PINA
#define DDR_KEY        DDRA
#define MASK_KEY       (0x0f)
#define SHIFT_KEY      (0x00)
#define INVERSE_KEYS   (0)
#define LED_NUMBER     (4)
/*=== Host Interface ================================================*/
#define HIF_TYPE    HIF_UART_0

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (8)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (1000UL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
# define TIMER_POOL_SIZE  (4)
/*
 * Mode: CTC
 * Prescaler: 8 / Mode 4, CTC mit OCR1A = 1000
 * @ F_CPU = 8MHz this results in 1ms timer tick IRQ's.
 */
# define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS11) | _BV(WGM12)); \
        TIMSK |= _BV(OCIE1A); \
        OCR1A = 1000; \
    }while(0)

# define TIMER_IRQ_vect   TIMER1_COMPA_vect

/* application settings for sniffer */
#define UART_RXBUFSIZE (32)
#define UART_TXBUFSIZE (128)
#define MAX_PACKET_BUFFERS (2)

#endif /* BOARD_STK_M16_H*/
