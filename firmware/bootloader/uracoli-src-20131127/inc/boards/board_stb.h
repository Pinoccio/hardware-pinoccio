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
 * @brief This board is describes the Sensor Terminal Board from
 * Dresden Electronic. It is a carrier board for the RCB family
 *
 * The Sensor Terminal Board is a carrier board for the radio
 * controller board family.
 * The transceiver wiring fits the common RCBs.
 * The wiring of the radio and the ATmega is shown below:
 *
 *
<pre>
     AVR      RF230
     ---      -----
     PB4  -->  SLPTR
     PD6  <--  CLKM
     PD4  <--  IRQ (ICP1)
     PB5  -->  RSTN
     PB0  -->  SS
     PB2  -->  MOSI
     PB3  <--  MISO
     PB1  -->  SCK

    The STBxxx has memory mapped LEDS and Keys.
    KEY: PE5
    LEDS
     - 2 x memory mapped at 0x4000  (Option 1)
     - 3 x IO mapped at PE2:PE4 (Option 2)


    You can select between using the LEDs on the RCB or on the STB.
    In order to use the RCB LEDS, you have to define the macro USE_RCB_LEDS.

    The LEDs on STB have a write-only memory interface, so we can't read back the LED status.
    So we need to have a shadow register, which stores the current LED state.
    Since the board interface consists only of a header file, we use register
    GPIO2 on ATmega1281 for shadowing, because it would be hard to ensure the
    single instantiation of a global variable from a header file, which is used
    in many module files.


Fuses/Locks:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Original Settings w/ rdk231
     LF: 0x61
     HF: 0x91
     EF: 0xfe

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

@par Build Options

  - stb230  : STB + RCB230 prior V3.2 / V3.3.1 (AT86RF230A)
  - stb230b : STB + RCB230 V3.2 / RCB230 V3.3.1 (AT86RF230B)
  - stb231  : STB * RCB231 V4.0.2 / RCB231ED V4.1.1
  - stb212  : STB + RCB212SMA V5.3.2

 */

/** ID String for this hardware */
#if defined(stb230)
# define BOARD_TYPE (BOARD_STB230)
# define BOARD_NAME "stb230"
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(stb230b)
# define BOARD_TYPE (BOARD_STB230B)
# define BOARD_NAME "stb230b"
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(stb231)
# define BOARD_TYPE (BOARD_STB231)
# define BOARD_NAME "stb231"
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(stb212)
# define BOARD_TYPE (BOARD_STB212)
# define BOARD_NAME "stb212"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(stb232)
# define BOARD_TYPE (BOARD_STB232)
# define BOARD_NAME "stb232"
# define RADIO_TYPE (RADIO_AT86RF232)
#elif defined(stb233)
# define BOARD_TYPE (BOARD_STB233)
# define BOARD_NAME "stb233"
# define RADIO_TYPE (RADIO_AT86RF233)
#endif

#ifndef BOARD_STB2XX_H
#define BOARD_STB2XX_H

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== radio interface definition =====================================*/
#if BOARD_TYPE == BOARD_STB230 || BOARD_TYPE == BOARD_STB230B
# include "base_rdk230.h"
#else
# include "base_rdk2xx.h"
#endif

/*
 * The IO subsystem of the Sensor Terminal Board is, a little
 * cumbersome to operate.  Besides the LEDs that are operated on an
 * MMIO register and cannot be read back, the FT245 is attached to the
 * external memory bus through a 74VHC245MTC bus driver which is by
 * default enabled to drive the bus (rather than to read it).  Thus,
 * much care must be taken to avoid a bus contention where both, the
 * 74VHC245MTC and the AVR drive the multiplexed address/data bus.
 *
 * We avoid that bus contention by swapping the meaning of the chip
 * select signal for the FT245 and the actual /WR line: both are ORed
 * together, so for the actual effect towards the FT245, it does not
 * matter whether we pulse /WR, or we pulse the correct high address
 * selection.
 */

static inline void hif_mmio_init(void)
{
    DDRC |= 0xc0;               /* xmem A14...A15 */
    PORTC = (PORTC & ~0xc0) | 0x80; /* high addr 0x80 => external
                                     * SRAM */
    PORTG &= ~0x05;             /* PG0: xmem /WR, activate to make the
                                 * 74VHC245MTC not drive the bus
                                 * PG2: xmem ALE; not used but keep
                                 * low all the time */
    PORTG |= 0x02;              /* PG1: xmem /RD, deactivate */
    DDRG |= 0x07;               /* make /RD, /WR, and ALE outputs */
}

static inline void hif_usb_write(uint8_t val)
{
    DDRA = 0xFF;
    PORTA = val;
    PORTC = (PORTC & ~0xc0);    /* select FT245 -> /WR pulse */
    __asm volatile("nop");
    PORTC = (PORTC & ~0xc0) | 0x80; /* re-select SRAM */
    DDRA = 0;
    PORTA = 0;                  /* avoid pullups */
}

static inline uint8_t hif_usb_read(void)
{
    PORTG |= 0x01;              /* de-assert /WR */
    PORTC = (PORTC & ~0xc0);    /* select FT245 */
    PORTG &= ~0x02;             /* /RD pulse */
    PORTG |= 0x02;
    __asm volatile("nop");
    uint8_t rv = PINA;
    PORTC = (PORTC & ~0xc0) | 0x80; /* re-select SRAM */
    PORTG &= ~0x01;                 /* re-assert /WR */

    return rv;
}


static inline void hif_led_write(uint8_t val)
{
    PORTG |= 0x01;              /* de-assert /WR */
    DDRA = 0xFF;
    PORTA = val;
    PORTC = (PORTC & ~0xc0) | 0x40; /* select IO address */
    PORTG &= ~0x01;                 /* /WR pulse */
    PORTG |= 0x01;
    __asm volatile("nop");
    PORTC = (PORTC & ~0xc0) | 0x80; /* re-select SRAM */
    DDRA = 0;
    PORTA = 0;                  /* avoid pullups */
    PORTG &= ~0x01;             /* re-assert /WR */
}

static inline uint8_t hif_key_read(void)
{
    PORTG |= 0x01;              /* de-assert /WR */
    PORTC = (PORTC & ~0xc0) | 0x40; /* select IO address */
    PORTG &= ~0x02;             /* /RD pulse */
    PORTG |= 0x02;
    __asm volatile("nop");
    uint8_t rv = PINA;
    PORTC = (PORTC & ~0xc0) | 0x80; /* re-select SRAM */
    PORTG &= ~0x01;                 /* re-assert /WR */

    return rv;
}

/*=== LED access macros ==============================================*/
#if !defined(USE_RCB_LEDS)
/*=== use LEDs on STB (Memory Mapped) ===*/

# define LED_SHADOW    GPIOR2
# define LED_MASK      (0x03)
# define LED_SHIFT     (0)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (2)

# define LED_INIT()\
        do{\
            hif_mmio_init(); \
            LED_SHADOW = LED_MASK;\
            hif_led_write(LED_SHADOW);        \
        }while(0)

# define LED_SET_VALUE(x) \
        do{\
            LED_SHADOW = (LED_SHADOW & ~LED_MASK) | ((~x<<LED_SHIFT) & LED_MASK);\
            hif_led_write(LED_SHADOW);\
        }while(0)

# define LED_GET_VALUE()\
        ((~LED_SHADOW & LED_MASK) >> LED_SHIFT)

# define LED_SET(ln)\
        do{\
            LED_SHADOW &= ~(_BV(ln+LED_SHIFT) & LED_MASK);\
            hif_led_write(LED_SHADOW);\
        }while(0)

# define LED_CLR(ln)\
        do{\
            LED_SHADOW |= (_BV(ln+LED_SHIFT) & LED_MASK);\
            hif_led_write(LED_SHADOW);\
        }while(0)

# define LED_VAL(msk,val)\
        do{\
            LED_SHADOW &= ~(LED_MASK|(msk<<LED_SHIFT)); \
            LED_SHADOW |= ~(val & (LED_MASK|msk));\
            hif_led_write(LED_SHADOW);\
        }while(0)


# define LED_TOGGLE(ln)\
        do{\
            LED_SHADOW ^= (_BV(ln+LED_SHIFT) & LED_MASK);\
            hif_led_write(LED_SHADOW);\
        }while(0)

#else
/*=== use LEDs on RCB (IO Mapped) ===*/
# define LED_PORT      PORTE
# define LED_DDR       DDRE
# define LED_MASK      (0x1c)
# define LED_SHIFT     (2)
# define LEDS_INVERSE  (1)
# define LED_NUMBER    (3)
#endif
/*=== KEY access macros ==============================================*/
#define PIN_KEY       (hif_key_read())
#define MASK_KEY      (0x1)
#define SHIFT_KEY     (0)
#define INVERSE_KEYS  (0)
#define PULLUP_KEYS   (0)
#define KEY_INIT      hif_mmio_init

/*=== Host Interface ================================================*/
#define HIF_TYPE      (HIF_FT245)
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

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL+1)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE     (4)

/** Vector for Timer IRQ routine */
#define TIMER_IRQ_vect   TIMER1_OVF_vect

/**
 * Intialization of the hardware timer T1 (16bit)
 *
 *  - CS1[2:0]  = 1 : Prescaler = 1
 *  - WGM1[3:0] = 0 : Mode = 4 : CTC operation
 *
 * Timer is clocked at F_CPU, and @ref TIMER_IRQ_vect is called every
 * 65535 ticks.
 */
# define TIMER_INIT() \
    do{ \
        TCCR1B |= _BV(CS10); \
        TIMSK1 |= _BV(TOIE1); \
    }while(0)

#endif /* BOARD_STB_H*/
