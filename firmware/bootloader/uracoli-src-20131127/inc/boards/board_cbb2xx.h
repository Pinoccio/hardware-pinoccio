/* Copyright (c) 2011 Axel Wachtler, Daniel Thiele
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
 * @brief Atmel REB-CBB with Radio Extender Board (REB) attached
 *        Any type of REB is supported
 *
 * The wiring of the REB and the MCU is shown below:
 *
<pre>
          ATxmega256A3    AT86RF2XX
          ------------    ---------
		  PC0             RSTN
		  PC1             DIG2
		  PC2             IRQ
		  PC3             SLPTR
		  PC4             SELN
		  PC5             MOSI
		  PC6             MISO
		  PC7             SCK
          PD0             CLKM
		  PD1             TXCW

          PB0             LED1
		  PB1             LED2
		  PB2             LED3
		  PB3             KEY1
</pre>
**/

#ifndef BOARD_CBB2XX_H
#define BOARD_CBB2XX_H

#if defined(cbb230)
# define BOARD_TYPE BOARD_CBB230
# define BOARD_NAME "cbb230"
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(cbb230b)
# define BOARD_TYPE BOARD_CBB230B
# define BOARD_NAME "cbb230b"
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(cbb231)
# define BOARD_TYPE BOARD_CBB231
# define BOARD_NAME "cbb231"
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(cbb212)
# define BOARD_TYPE BOARD_CBB212
# define BOARD_NAME "cbb212"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(cbb232)
# define BOARD_TYPE BOARD_CBB232
# define BOARD_NAME "cbb232"
# define RADIO_TYPE (RADIO_AT86RF232)
#elif defined(cbb233)
# define BOARD_TYPE BOARD_CBB233
# define BOARD_NAME "cbb233"
# define RADIO_TYPE (RADIO_AT86RF233)
#endif

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== Hardware Components ============================================*/

/*=== TRX pin access macros ==========================================*/

#define PORT_TRX_RESET  PORTC         /**< PORT register for RESET pin */
#define MASK_TRX_RESET  (_BV(0))      /**< PIN mask for RESET pin */

#define TRX_RESET_INIT() PORT_TRX_RESET.DIRSET = MASK_TRX_RESET    /**< RESET pin IO initialization */
#define TRX_RESET_HIGH() PORT_TRX_RESET.OUTSET = MASK_TRX_RESET   /**< set RESET pin to high level */
#define TRX_RESET_LOW()  PORT_TRX_RESET.OUTCLR = MASK_TRX_RESET  /**< set RESET pin to low level */

#define PORT_TRX_SLPTR  PORTC         /**< DDR register for SLP_TR pin */
#define MASK_TRX_SLPTR  (_BV(3))    /**< PIN mask for SLP_TR pin */

/** SLP_TR pin IO initialization */
#define TRX_SLPTR_INIT() PORT_TRX_SLPTR.DIRSET = MASK_TRX_SLPTR
/** set SLP_TR pin to high level */
#define TRX_SLPTR_HIGH() PORT_TRX_SLPTR.OUTSET = MASK_TRX_SLPTR
/**< set SLP_TR pin to low level */
#define TRX_SLPTR_LOW()  PORT_TRX_SLPTR.OUTCLR = MASK_TRX_SLPTR


/*=== IRQ access macros ==============================================*/
# define TRX_IRQ_vect    PORTC_INT0_vect    /**< interrupt vector name */
# define TRX_IRQ         _BV(2)    /**< interrupt mask for PORTC */

/** init interrupt handling
 *  - rising edge triggers ICP1 (ICES1),
 *  - timer capture is enabled (ICF1)
 */
# define TRX_IRQ_INIT()  do{\
							PORTC.INT0MASK = TRX_IRQ; \
                            PORTC.INTCTRL = PORT_INT0LVL_HI_gc; \
							PMIC.CTRL |= PMIC_HILVLEN_bm;\
                          } while(0)

/** disable TRX interrupt */
#define DI_TRX_IRQ() {PORTC.INT0MASK &= ~TRX_IRQ;}

/** enable TRX interrupt */
#define EI_TRX_IRQ() {PORTC.INT0MASK |= TRX_IRQ;}

/** timestamp register for RX_START event
 * FIXME: add and test the enabling of input capture for separate RX_START (AT86RF231/212)
 *        currently we use the timer register.
 */
#define TRX_TSTAMP_REG TCD0.CNT

/*=== SPI access macros ==============================================*/
#define PORT_SPI (PORTC)  /**< PORT register for SPI port */

#define SPI_MOSI _BV(5)  /**< PIN mask for MOSI pin */
#define SPI_MISO _BV(6)  /**< PIN mask for MISO pin */
#define SPI_SCK  _BV(7)  /**< PIN mask for SCK pin */
#define SPI_SS   _BV(4)  /**< PIN mask for SS pin */

#define SPI_DATA_REG SPIC.DATA  /**< abstraction for SPI data register */


/**
 * @brief inline function for SPI initialization
 */
static inline void SPI_INIT(uint8_t spirate)
{
    /* first configure SPI Port, then SPCR */
    PORT_SPI.OUTSET = SPI_SCK | SPI_SS;
    PORT_SPI.DIRSET = SPI_MOSI | SPI_SCK | SPI_SS;
    PORT_SPI.DIRCLR = SPI_MISO;

	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm;

    SPIC.CTRL |= (spirate & 0x03);
    SPIC.CTRL |= ((spirate << 5) & 0x80); /* CLK2X */
}

/** set SS line to low level */
#define SPI_SELN_LOW()       uint8_t sreg = SREG; cli(); PORT_SPI.OUTCLR = SPI_SS
/** set SS line to high level */
#define SPI_SELN_HIGH()      PORT_SPI.OUTSET = SPI_SS; SREG = sreg
/** wait until SPI transfer is ready */
#define SPI_WAITFOR()        do { while((SPIC.STATUS & SPI_IF_bm) == 0);} while(0)


/*=== LED access macros ==============================================*/
#define LED_PORT     PORTB_OUT
#define LED_DDR      PORTB_DIR
#define LED_MASK     (0x07)
#define LED_SHIFT    (0)
#define LEDS_INVERSE (0)
#define LED_NUMBER   (3)

/*=== KEY access macros ==============================================*/
#define PORT_KEY     PORTB_OUT
#define PIN_KEY      PORTB_IN
#define DDR_KEY      PORTB_DIR
#define MASK_KEY     (0x08)
#define SHIFT_KEY    (3)
#define INVERSE_KEYS (1)

#define KEY_INIT() do{ PORTB_PIN3CTRL = PORT_OPC_PULLUP_gc; DDR_KEY &= ~MASK_KEY; }while(0)

#define SLEEP_ON_KEY_INIT() do{}while(0)
#define SLEEP_ON_KEY() \
        do{\
        } while(0)

#define SLEEP_ON_KEY_vect PORTB_INT0_vect

/*=== Host Interface ================================================*/
#define HIF_TYPE (HIF_UART_0)

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL)
#define HWTIMER_REG     (TCD0.CNT)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCD0.CTRLA = TC_CLKSEL_DIV1_gc; \
        TCD0.INTFLAGS |= TC0_OVFIF_bm; \
       	TCD0.INTCTRLA = TC_OVFINTLVL_HI_gc; \
    }while(0)
#define TIMER_IRQ_vect   TCD0_OVF_vect

#endif /* BOARD_CBB2XX_H */
