/* Copyright (c) 2009 - 2013 Daniel Thiele, Eric Gnoske, Axel Wachtler
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
 * @brief Dresden Elektronik Radio Module deRFmega128-22A001
 *
 *
 *
<pre>

Fuses/Locks:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes


radiofaro:
  LEDS: PG1, PG2

</pre>


@par Build Options

  - derfa1 : Radio Module deRFmega128-22A001

  - radiofaro : wireless Ardino board
  - radiofaro_v1 : wireless Ardino board

  - zigduino : Logos Electromechanical LLC
               http://www.logos-electro.com/blog/2010/5/16/zigduino.html
  - pinoccio : http://pinocc.io/
  - raspbee : Dresden Electronic Raspberry Pi Modul.
  - derfn256u0, derfn256u0, derfn128u0:
              Dresden Electronic deRFNode w/ UART0 (header X5)
  - derfn128: Dresden Electronic deRFNode w/ USB

 */

#ifndef BOARD_DERFA_H
#define BOARD_DERFA_H

#if defined(derfa1)
# define BOARD_TYPE BOARD_DERFA1
# define BOARD_NAME "derfa1"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(radiofaro)
# define BOARD_TYPE BOARD_RADIOFARO
# define BOARD_NAME "radiofaro"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#elif defined(radiofaro_v1)
# define BOARD_TYPE BOARD_RADIOFARO_V1
# define BOARD_NAME "radiofaro v1"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(zigduino)
# define BOARD_TYPE BOARD_ZIGDUINO
# define BOARD_NAME "zigduino"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(xxo)
# define BOARD_TYPE BOARD_XXO
# define BOARD_NAME "tic_tac_toe"
#define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(wprog)
# define BOARD_TYPE BOARD_WPROG
# define BOARD_NAME "wprog"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#elif defined(pinoccio)
# define BOARD_TYPE BOARD_PINOCCIO
# define BOARD_NAME "pinoccio"
# define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#elif defined(raspbee)
# define BOARD_TYPE BOARD_RASPBEE
# define BOARD_NAME "raspbee"
# define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#elif defined(derfn256u0)
# define BOARD_TYPE BOARD_DERFN256U0
# define BOARD_NAME "derfn256u0"
# define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#elif defined(derfn256u0pa)
# define BOARD_TYPE BOARD_DERFN256U0PA
# define BOARD_NAME "derfn256u0pa"
# define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#elif defined(derfn128)
# define BOARD_TYPE BOARD_DERFN128
# define BOARD_NAME "derfn128"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#elif defined(derfn128u0)
# define BOARD_TYPE BOARD_DERFN128U0
# define BOARD_NAME "derfn128u0"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_D)
#endif
/*=== Compile time parameters ========================================*/

/*=== Hardware Components ============================================*/

/** timestamp register for RX_START event */
#define TRX_TSTAMP_REG TCNT1

#if BOARD_TYPE == BOARD_RADIOFARO
# define LED_PORT      PORTG
# define LED_DDR       DDRG
# define LED_MASK      (0x06)
# define LED_SHIFT     (1)
# define LEDS_INVERSE  (0)
# define LED_NUMBER    (2)
#elif BOARD_TYPE == BOARD_WPROG
# define LED_PORT      PORTD
# define LED_DDR       DDRD
# define LED_MASK      (0x40)
# define LED_SHIFT     (6)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (1)
#elif BOARD_TYPE == BOARD_PINOCCIO
# define LED_PORT      PORTB
# define LED_DDR       DDRB
# define LED_MASK      (0x70)
# define LED_SHIFT     (4)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (3)
#elif BOARD_TYPE == BOARD_XXO

/* We just define the first three LEDs in row 1
 * use a special display driver, that is frequently
 * updated to handle the full LED matrix.
 */

#define LED_INIT() \
    do {\
        DDRB |= (_BV(PB0) | _BV(PB1) | _BV(PB2) | _BV(PB3));\
        PORTB |= (_BV(PB0) | _BV(PB1) | _BV(PB2) | _BV(PB3));\
    } while(0)


#define LED_SET(x) \
  switch (x) { \
  case 0: PORTB &= ~_BV(0); break; \
  case 1: PORTB &= ~_BV(1); break; \
  case 2: PORTB &= ~_BV(2); break; \
  }

#define LED_CLR(x) \
  switch (x) { \
  case 0: PORTB |= _BV(0); break; \
  case 1: PORTB |= _BV(1); break; \
  case 2: PORTB |= _BV(2); break; \
  }

#define LED_SET_VALUE(x) \
  do { \
  if (x & 1) PORTB &= ~_BV(0); else PORTB |= _BV(0); \
  if (x & 2) PORTB &= ~_BV(1); else PORTB |= _BV(1); \
  if (x & 4) PORTB &= ~_BV(2); else PORTB |= _BV(2); \
  } while (0)

#define LED_GET_VALUE() ( \
  ((PORTB & _BV(0))? 0: 1) | \
  ((PORTB & _BV(1))? 0: 2) | \
  ((PORTB & _BV(2))? 0: 4) \
			  )

#define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

#define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: PORTB ^= _BV(5); break; \
  case 1: PORTB ^= _BV(7); break; \
  case 2: PORTB ^= _BV(6); break; \
  }

#define LED_NUMBER (3)
#define LED_ACTIVITY (0)

#elif BOARD_TYPE == BOARD_RASPBEE

# define LED_INIT() \
    do { \
        DDRD |= _BV(7); \
        DDRG |= _BV(2); \
        PORTD |= _BV(7); \
        PORTG |= _BV(2); \
    } while(0)

# define LED_SET(x) \
    switch (x) { \
        case 0: PORTD |= _BV(7); break; \
        case 1: PORTG |= _BV(2); break; \
    }

# define LED_CLR(x) \
    switch (x) { \
        case 0: PORTD &= ~_BV(7); break; \
        case 1: PORTG &= ~_BV(2); break; \
    }

# define LED_SET_VALUE(x) \
  do { \
    if (x & 1) PORTD &= ~_BV(7); else PORTD |= _BV(7); \
    if (x & 2) PORTG &= ~_BV(2); else PORTG |= _BV(2); \
  } while (0)

# define LED_GET_VALUE() ( \
  ((PORTD & _BV(7))? 0: 1) | \
  ((PORTG & _BV(2))? 0: 2) )

# define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

# define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: PORTD ^= _BV(7); break; \
  case 1: PORTG ^= _BV(2); break; \
  }

# define LED_NUMBER (2)
# define LED_ACTIVITY (0)

#elif BOARD_TYPE == BOARD_DERFN256U0 || BOARD_TYPE == BOARD_DERFN256U0PA ||\
      BOARD_TYPE == BOARD_DERFN128 || BOARD_TYPE == BOARD_DERFN128U0

# define LED_INIT() \
    do { \
        DDRE |= _BV(3) | _BV(4); \
        DDRG |= _BV(5); \
        PORTE |= _BV(3) | _BV(4); \
        PORTG |= _BV(5); \
    } while(0)

# define LED_SET(x) \
    switch (x) { \
        case 0: PORTG |= _BV(5); break; \
        case 1: PORTE |= _BV(3); break; \
        case 2: PORTE |= _BV(4); break; \
    }

# define LED_CLR(x) \
    switch (x) { \
        case 0: PORTG &= ~_BV(5); break; \
        case 1: PORTE &= ~_BV(3); break; \
        case 2: PORTE &= ~_BV(4); break; \
    }

# define LED_SET_VALUE(x) \
  do { \
    if (x & 5) PORTG &= ~_BV(5); else PORTG |= _BV(5); \
    if (x & 3) PORTE &= ~_BV(3); else PORTE |= _BV(3); \
    if (x & 4) PORTE &= ~_BV(4); else PORTE |= _BV(4); \
  } while (0)

# define LED_GET_VALUE() ( \
  ((PORTG & _BV(5))? 0: 1) | \
  ((PORTE & _BV(3))? 0: 2) | \
  ((PORTE & _BV(4))? 0: 4) \
  	)

# define LED_VAL(msk,val) do{}while(0) /**< \todo how to implement this? */

# define LED_TOGGLE(ln) \
  switch (ln) { \
  case 0: PORTG ^= _BV(5); break; \
  case 1: PORTE ^= _BV(3); break; \
  case 2: PORTE ^= _BV(4); break; \
  }

# define LED_NUMBER (3)
# define LED_ACTIVITY (0)

#else
# define LED_NUMBER    (0)
# define NO_LEDS       (1)
#endif

#define NO_KEYS        (1)

/*=== Host Interface ================================================*/
#if BOARD_TYPE == BOARD_RADIOFARO_V1 || \
    BOARD_TYPE == BOARD_ZIGDUINO || \
    BOARD_TYPE == BOARD_RASPBEE  || \
    BOARD_TYPE == BOARD_DERFN256U0 || \
    BOARD_TYPE == BOARD_DERFN256U0PA || \
    BOARD_TYPE == BOARD_DERFN128U0
# define HIF_TYPE    HIF_UART_0
#elif BOARD_TYPE == BOARD_DERFN128
# define HIF_TYPE    (HIF_FT245)
#else
# define HIF_TYPE    HIF_UART_1
#endif

#if BOARD_TYPE == BOARD_PINOCCIO
#define HIF_UART_FORCE_U2X (1)
#endif


#if BOARD_TYPE == BOARD_DERFN128

    /*
     * USBRD = Pin27 = PD2
     * USBWR = Pin10 = PD3
     * RXF   = Pin30 = PE2
     * TXE   = Pin21 = PB5
     * Data Port:
     * D0 =  Pin16 = PB0
     * D1 =  Pin41 = PF2
     * D2 =  Pin12 = PD5
     * D3 =  Pin06 = PG2
     * D4 =  Pin37 = PE6
     * D5 =  Pin19 = PB4
     * D6 =  Pin39 = PE7
     * D7 =  Pin18 = PB6
     */

    static inline void hif_mmio_init(void)
    {
        DDRD |= 0xc0;               /* xmem A14...A15 */
        PORTD = (PORTD & ~0xc0) | 0x80; /* high addr 0x80 => external
                                         * SRAM */
        PORTE &= ~0x10;             /* PE4: xmem /WR, activate to make the
                                     * 74VHC245MTC not drive the bus */
        PORTE |= 0x20;              /* PE5: xmem /RD, deactivate */
        DDRE |= 0x30;               /* make /RD and /WR outputs */
        PORTG &= ~0x04;             /* PG2: xmem ALE; not used but keep
                                     * low all the time */
        DDRG |= 0x04;
    }

    static inline void hif_usb_write(uint8_t val)
    {
        DDRB = 0xFF;
        PORTB = val;
        PORTD = (PORTD & ~0xc0);    /* select FT245 -> /WR pulse */
        __asm volatile("nop");
        PORTD = (PORTD & ~0xc0) | 0x80; /* re-select SRAM */
        DDRB = 0;
        PORTB = 0;                  /* avoid pullups */
    }

    static inline uint8_t hif_usb_read(void)
    {
        PORTE |= 0x10;              /* de-assert /WR */
        PORTD = (PORTD & ~0xc0);    /* select FT245 */
        PORTE &= ~0x20;             /* /RD pulse */
        PORTE |= 0x20;
        __asm volatile("nop");
        uint8_t rv = PINB;
        PORTD = (PORTD & ~0xc0) | 0x80; /* re-select SRAM */
        PORTE &= ~0x10;                 /* re-assert /WR */

        return rv;
    }
    #define HIF_IO_ENABLE hif_mmio_init
    #define HIF_USB_READ()  hif_usb_read()
    #define HIF_USB_WRITE(x) hif_usb_write(x)
    #define HIF_NO_DATA   (0x0100)
    #define FT245_DDR    DDRE
    #define FT245_PIN    PINE
    #define FT245_TXE    _BV(6)
    #define FT245_RXF    _BV(7)
    #define FT245_INIT() do { \
               FT245_DDR &= ~(FT245_TXE|FT245_RXF);\
            } while(0)

    #define FT245_TX_IS_BLOCKED()    (0 != (FT245_PIN & FT245_TXE))
    #define FT245_RX_HAS_DATA()      (0 == (FT245_PIN & FT245_RXF))

#endif

#define TRX_RESET_LOW()   do { TRXPR &= ~_BV(TRXRST); } while (0)
#define TRX_RESET_HIGH()  do { TRXPR |= _BV(TRXRST); } while (0)
#define TRX_SLPTR_LOW()   do { TRXPR &= ~_BV(SLPTR); } while (0)
#define TRX_SLPTR_HIGH()  do { TRXPR |= _BV(SLPTR); } while (0)

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
#define TIMER_IRQ_vect   TIMER1_OVF_vect

/*=== PA/LNA init ==================================================*/
#if BOARD_TYPE == BOARD_RASPBEE || BOARD_TYPE == BOARD_DERFN256U0PA || BOARD_TYPE == BOARD_DERFN128U0

# define TRX_PA_LNA_INIT() \
        do {\
            PORTD &= ~_BV(PD6);\
            DDRD |= _BV(PD6);\
        } while(0)

# define TRX_TX_PA_EI() do {PORTD |= _BV(PD6);} while(0)
# define TRX_TX_PA_DI() do {PORTD &= ~_BV(PD6);} while(0)
# define TRX_RX_LNA_EI() do {PORTD |= _BV(PD6);} while(0)
# define TRX_RX_LNA_DI() do {PORTD &= ~_BV(PD6);} while(0)
#endif /* BOARD_TYPE == BOARD_RASPBEE || BOARD_TYPE == BOARD_DERFN256U0PA || BOARD_TYPE == BOARD_DERFN128U0 */


#if BOARD_TYPE == BOARD_DERFN128U0 || BOARD_TYPE == BOARD_DERFN128
#define I2C_ENABLE() do{\
        DDRD |= (1<<PD6);\
        PORTD &= ~(1<<PD6);\
    }while(0)

#define ISL29020_ADDR (0x44)

#endif



#endif /* BOARD_DERFA_H */
