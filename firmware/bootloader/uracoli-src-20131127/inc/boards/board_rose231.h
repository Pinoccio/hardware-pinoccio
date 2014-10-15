/* Copyright (c) 2011
    Daniel Thiele,
    Axel Wachtler
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
 * @brief Definition of Rocket Sensor by IBDT
 *
 * The wiring of the radio and the ATmega328PA is shown below:
 *
<pre>
     Transceiver
     AVR         AT86RF231
     ---        ----------
     PC0    -->   SLPTR
     PD5    <--   IRQ (INT2)
     PC2    -->   RSTN
     PB0    -->   /SEL
     PB3    -->   MOSI
     PB4    <--   MISO
     PB5    -->   SCK
     PC1    <--   DIG2

   Fuses/Locks:
     LF: 0xd2 - 8MHz internal RC Osc.
     HF: 0xDF
     EF: 0x01


Bootloader:
    Start at byte=...., address=....., size = 4096 instructions/ 8192 bytes

</pre>

    @image html
    @image latex
 */

#ifndef BOARD_ROSE231_H
#define BOARD_ROSE231_H  (1)

# define BOARD_TYPE (BOARD_ROSE231)
# define BOARD_NAME "Rocketsensor 231"     /**< current board name */
# define RADIO_TYPE (RADIO_AT86RF231)  /**< used radio (see const.h)*/

/*=== Compile time parameters ========================================*/

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/

#define DDR_TRX_RESET   DDRC          /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTC         /**< PORT register for RESET pin */
#define MASK_TRX_RESET  ((1<<2))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTC         /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRC          /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  ((1<<0))    /**< PIN mask for SLP_TR pin */



/*=== IRQ access macros ==============================================*/

#define TRX_IRQ_PORT (PORTD)
#define TRX_IRQ_DDR (DDRD)
#define TRX_IRQ_PIN (PIND)
#define TRX_IRQ_bp  (5)

# define TRX_IRQ         0x00   /**< interrupt mask for GICR */
# define TRX_IRQ_vect    PCINT2_vect    /**< interrupt vector name */

/** configuration of interrupt handling */

/** any edge triggers PCINT1 */
# define TRX_IRQ_INIT()  do{ PCICR |= (1<<PCIE2); } while(0) 
/** disable TRX interrupt */
#define DI_TRX_IRQ() { PCMSK2 &= ~(1<<PCINT21); }
/** enable TRX interrupt */
#define EI_TRX_IRQ() { PCMSK2 |= (1<<PCINT21); }

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDRB)   /**< DDR register for SPI port */
#define PORT_SPI (PORTB)  /**< PORT register for SPI port */

#define SPI_MOSI (1<<PB3)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<PB4)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<PB5)  /**< PIN mask for SCK pin */
#define SPI_SS   (1<<PB0)  /**< PIN mask for SS pin */


#define SPI_DATA_REG SPDR    /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
	
	/* pullup for SS of SPI unit to prevent setting to SLAVE, not TRX_SELN! */
	PORT_SPI |= (1<<PB2);

    PORT_SPI |= SPI_SS | SPI_MISO | SPI_MOSI | SPI_SCK;
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;

    SPCR = ((1<<SPE) | (1<<MSTR));
	
    SPCR &= ~((1<<SPR1) | (1<<SPR0) );
    SPSR &= ~(1<<SPI2X);

    SPCR |= (spirate & 0x03);
    SPSR |= ((spirate >> 2) & 0x01);
}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI &=~SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI |= SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()        do { while((SPSR & (1<<SPIF)) == 0);} while(0)

/*=== LED access macros ==============================================*/

#define LED_PORT      PORTD      /**< PORT register for LEDs */
#define LED_DDR       DDRD       /**< DDR register for LEDs */
#define LED_MASK      (0x18)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (3)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (0)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */

/* special handling for this board to enable light measurement */
#define LED_PIN       PIND       /**< PIN register for LEDs */
#define LED_ANODE_bp   (3)		/**< LED anode bit position */
#define LED_CATHODE_bp (4)		/**< LED cathode bit position */

/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */

/*=== Host Interface ================================================*/
/** Type of the host interface. */
#define HIF_TYPE    HIF_NONE

/*=== TIMER Interface ===============================================*/

/* setup timer with a tick of 1ms (assuming F_CPU = 8MHz fixed) */

#define HWTMR_PRESCALE  (8)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (1000UL)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
		TCCR1B = 0; \
		OCR1A = HWTIMER_TICK_NB; \
        TCCR1B |= ((1<<WGM12) | (1<<CS11)); \
        TIMSK1 |= (1<<OCIE1A); \
    }while(0)
# define TIMER_IRQ_vect   TIMER1_COMPA_vect

/*=== Special Hardware Ressources ===================================*/

#endif /* BOARD_ROSE231_H */
