/* Copyright (c) 2009
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
 * @brief Definition of Accelerometer Sensor Board by Daniel Thiele
 *
 * The wiring of the radio and the ATmega88 is shown below:
 *
<pre>
     Transceiver
     AVR         AT86RF231
     ---        ----------
     PC0    -->   SLPTR
     XTAL1  <--   MCLK
     PD2    <--   IRQ (INT2)
     PC1    -->   RSTN
     PB0    -->   /SEL
     PB3    -->   MOSI
     PB4    <--   MISO
     PB5    -->   SCK

     Accelerometer
     AVR        MMA7455L
     ---        --------
     PB2    -->   CS
     PB3    -->   MOSI
     PB4    -->   MISO
     PB5    -->   SCK
     PD3    -->   INT1/DRDY
     PD4    -->   INT2


   Fuses/Locks:
     LF: 0xd2 - 8MHz internal RC Osc.
     HF: 0xDF
     EF: 0x01


Bootloader:
    Start at byte=...., address=....., size = 4096 instructions/ 8192 bytes

</pre>

    @image html littleG_top.png "littleGee Accellerometer Sensor Board"
    @image latex littleG_top.png "littleGee Accellerometer Sensor Board"
 */



/** Build Options */
#if defined(lgee231)
# define BOARD_TYPE (BOARD_LGEE231)    /**< board type (see const.h)*/
# define BOARD_NAME "littleGee/v3"     /**< current board name */
# define RADIO_TYPE (RADIO_AT86RF231)  /**< used radio (see const.h)*/
# define BOARDVERSION (3)
#elif defined(lgee231_v2)
# define BOARD_TYPE (BOARD_LGEE231_V2) /**< board type (see const.h)*/
# define BOARD_NAME "littleGee/v2"     /**< current board name */
# define RADIO_TYPE (RADIO_AT86RF231)  /**< used radio (see const.h)*/
# define BOARDVERSION (2)
#endif

#ifndef BOARD_LGEE231_H
#define BOARD_LGEE231_H  (1)
/*=== Compile time parameters ========================================*/

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/

#if BOARDVERSION == 2

#define DDR_TRX_RESET   DDRC          /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTC         /**< PORT register for RESET pin */
#define MASK_TRX_RESET  ((1<<1))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTC         /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRC          /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  ((1<<0))    /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

#elif BOARDVERSION == 3

#define DDR_TRX_RESET   DDRC          /**< DDR register for RESET pin */
#define PORT_TRX_RESET  PORTC         /**< PORT register for RESET pin */
#define MASK_TRX_RESET  ((1<<3))      /**< PIN mask for RESET pin */
/* check in board.h if standard definitions of TRX_RESET_INIT,
   TRX_RESET_HIGH, TRX_RESET_LOW are Ok, otherwise define here */

#define PORT_TRX_SLPTR  PORTC         /**< DDR register for SLP_TR pin */
#define DDR_TRX_SLPTR   DDRC          /**< PORT register for SLP_TR pin */
#define MASK_TRX_SLPTR  ((1<<0))    /**< PIN mask for SLP_TR pin */
/* check in board.h if standard definitions of TRX_SLPTR_INIT,
   TRX_SLPTR_HIGH, TRX_SLPTR_LOW are Ok, otherwise define here */

#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */



/*=== ACC pin access macros ==========================================*/

#if BOARDVERSION == 2

#define ACC_SELN_DDR      DDRB
#define ACC_SELN_PORT     PORTB
#define ACC_SELN_PIN      PINB
#define ACC_SELN_bp       (2)
#define ACC_SELN_ASIN()   do{ ACC_SELN_DDR &= ~(1<<ACC_SELN_bp);  }while(0)
#define ACC_SELN_ASOUT()  do{ ACC_SELN_DDR |= (1<<ACC_SELN_bp);   }while(0)
#define ACC_SELN_LO()     do{ ACC_SELN_PORT &= ~(1<<ACC_SELN_bp); }while(0)
#define ACC_SELN_HI()     do{ ACC_SELN_PORT |= (1<<ACC_SELN_bp);  }while(0)

#elif BOARDVERSION == 3

#define ACC_SELN_DDR      DDRD
#define ACC_SELN_PORT     PORTD
#define ACC_SELN_PIN      PIND
#define ACC_SELN_bp       (6)
#define ACC_SELN_ASIN()   do{ ACC_SELN_DDR &= ~(1<<ACC_SELN_bp);  }while(0)
#define ACC_SELN_ASOUT()  do{ ACC_SELN_DDR |= (1<<ACC_SELN_bp);   }while(0)
#define ACC_SELN_LO()     do{ ACC_SELN_PORT &= ~(1<<ACC_SELN_bp); }while(0)
#define ACC_SELN_HI()     do{ ACC_SELN_PORT |= (1<<ACC_SELN_bp);  }while(0)

#define ACC_IRQ_DDR      DDRD
#define ACC_IRQ_PORT     PORTD
#define ACC_IRQ_PIN      PIND
#define ACC_IRQ_bp       (7)
#define ACC_IRQ_ASIN()   do{ ACC_IRQ_DDR &= ~(1<<ACC_IRQ_bp);  }while(0)
#define ACC_IRQ_ASOUT()  do{ ACC_IRQ_DDR |= (1<<ACC_IRQ_bp);   }while(0)
#define ACC_IRQ_LO()     do{ ACC_IRQ_PORT &= ~(1<<ACC_IRQ_bp); }while(0)
#define ACC_IRQ_HI()     do{ ACC_IRQ_PORT |= (1<<ACC_IRQ_bp);  }while(0)
#define ACC_IRQ_STATE() ( (ACC_IRQ_PIN & (1<<ACC_IRQ_bp)) != 0)

#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */

/*=== IRQ access macros ==============================================*/

#if BOARDVERSION == 2

# define TRX_IRQ         0x00   /**< interrupt mask for GICR */
# define TRX_IRQ_vect    PCINT2_vect    /**< interrupt vector name */

/** configuration of interrupt handling */

/** any edge triggers PCINT2 */
# define TRX_IRQ_INIT()  do{ PCICR |= (1<<PCIE2); } while(0) 

/** disable TRX interrupt */
#define DI_TRX_IRQ() { PCMSK2 &= ~(1<<PCINT18); }
/** enable TRX interrupt */
#define EI_TRX_IRQ() { PCICR |= (1<<PCIE2); PCMSK2 |= (1<<PCINT18); }

#elif BOARDVERSION == 3

#define TRX_IRQ_PORT (PORTB)
#define TRX_IRQ_DDR (DDRB)
#define TRX_IRQ_PIN (PINB)
#define TRX_IRQ_bp (1)

# define TRX_IRQ         0x00   /**< interrupt mask for GICR */
# define TRX_IRQ_vect    PCINT0_vect    /**< interrupt vector name */
# define ACC_IRQ_vect    PCINT2_vect    /**< interrupt vector name */

/** configuration of interrupt handling */

/** any edge triggers PCINT1 */
# define TRX_IRQ_INIT()  do{ PCICR |= (1<<PCIE0); } while(0) 
/** disable TRX interrupt */
#define DI_TRX_IRQ() { PCMSK0 &= ~(1<<PCINT1); }
/** enable TRX interrupt */
#define EI_TRX_IRQ() { PCMSK0 |= (1<<PCINT1); }

/** any edge triggers PCINT1 */
#define ACC_IRQ_INIT() do{ PCICR |= (1<<PCIE2); }while(0)
/** enable ACC interrupt */
#define EI_ACC_IRQ()   do{ PCMSK2 |= (1<<PCINT23); }while(0)
/** disable ACC interrupt */
#define DI_ACC_IRQ()   do{ PCMSK2 &= ~(1<<PCINT23); }while(0)

#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */


/** timestamp register for RX_START event (either TCNTx or ICRx)*/
#define TRX_TSTAMP_REG TCNT1

/*=== SPI access macros ==============================================*/
#define SPI_TYPE  SPI_TYPE_SPI
#define DDR_SPI  (DDRB)   /**< DDR register for SPI port */
#define PORT_SPI (PORTB)  /**< PORT register for SPI port */

#if BOARDVERSION == 2

#define SPI_MOSI (1<<PB3)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<PB4)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<PB5)  /**< PIN mask for SCK pin */
#define SPI_SS   (1<<PB0)  /**< PIN mask for SS pin */

#elif BOARDVERSION == 3

#define SPI_MOSI (1<<PB3)  /**< PIN mask for MOSI pin */
#define SPI_MISO (1<<PB4)  /**< PIN mask for MISO pin */
#define SPI_SCK  (1<<PB5)  /**< PIN mask for SCK pin */
#define SPI_SS   (1<<PB2)  /**< PIN mask for SS pin */

#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */


#define SPI_DATA_REG SPDR    /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    DDR_SPI  |= SPI_MOSI | SPI_SCK | SPI_SS;
    DDR_SPI  &= ~SPI_MISO;
    PORT_SPI |= SPI_SCK | SPI_SS;

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

#if BOARDVERSION == 2

#define LED_PORT      PORTD      /**< PORT register for LEDs */
#define LED_DDR       DDRD       /**< DDR register for LEDs */
#define LED_MASK      (0x60)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (5)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (1)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */

#elif BOARDVERSION == 3

#define LED_PORT      PORTC      /**< PORT register for LEDs */
#define LED_DDR       DDRC       /**< DDR register for LEDs */
#define LED_MASK      (0x30)     /**< MASK value for LEDs (msb aligned)*/
#define LED_SHIFT     (4)        /**< SHIFT value for LEDs */
#define LEDS_INVERSE  (1)        /**< = 1, if low level at port
                                      means LED on */
#define LED_NUMBER    (2)        /**< number of LEDs for this board */

#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */


/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)        /**< if defined, no KEYS are connected */

/*=== Host Interface ================================================*/
/** Type of the host interface. */
#if BOARDVERSION == 2
#define HIF_TYPE    HIF_NONE
#elif BOARDVERSION == 3
#define HIF_TYPE    (HIF_UART_0)
#else
#error "Unknown BOARDVERSION, set to 2 or 3"

#endif /* BOARDVERSION == */

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
# define TIMER_IRQ_vect   TIMER1_OVF_vect

/*=== Special Hardware Ressources ===================================*/

#endif /* BOARD_LGEE231_H */
