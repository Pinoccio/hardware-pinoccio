/* Copyright (c) 2011 Joerg Wunsch
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
 *
 * The wiring of this module is very specific; normally, all the
 * functionality on the Sensor Terminal Board (FTDI, LEDs, button) is
 * controlled by memory-mapped IO, using the external memory interface
 * of the ATmegas.  This is not possible with the ATmega128RFA1 as it
 * does not feature an external memory interface.  For that reason,
 * GPIO lines have been re-wired to allow an external memory interface
 * emulation.  The schematics of the RCB128RFA1 document the following
 * wiring:
 *
 *
<pre>
RCB2xx        | Pin           | RCB128RFA1   | Remark
------------------------------------------------------------------
PA0             EXT1.23         PB0          \
PA1             EXT1.24         PB1          |  This is the
PA2             EXT1.25         PB2          |  multiplexed
PA3             EXT1.26         PB3          >  data/low address
PA4             EXT1.27         PB4          |  bus of the classic
PA5             EXT1.28         PB5          |  XMEM interface
PA6             EXT1.29         PB6          |
PA7             EXT1.30         PB7          /
PE4             EXT1.6          RSTON        Reset output
PE5             EXT1.5          TST          Used for "HV" prog
PC4             EXT0.25         PD4          \  Matches A12...A15
PC5             EXT0.26         PD5          |  of normal address bus;
PC6             EXT0.27         PD6          >  used to distinguish
PC7             EXT0.28         PD7          /  FTDI vs. LED vs. button
PG0             EXT0.17         PE4          xmem /WR
PG1             EXT0.18         PE5          xmem /RD
PB6             EXT0.1          PG0          GPIO
PB7             EXT0.2          PG1          GPIO
XTAL1           EXT0.7          CLKI
</pre>
    In addition, the standard STB pin mapping is retained for:

    PE7  FT245 /RXF
    PE6  FT245 /TXE
    PG2  ALE

    The STBxxx has memory mapped LEDS and Keys.
    KEY: memory mapped on external memory bus address 0x4000
    LEDS
     - 2 x memory mapped at 0x4000  (Option 1)
     - 3 x IO mapped at PE2:PE4 (Option 2)


    You can select between using the LEDs on the RCB or on the STB.
    In order to use the RCB LEDS, you have to define the macro USE_RCB_LEDS.

    The LEDs on STB have a write-only memory interface, so we can't read back the LED status.
    So we need to have a shadow register, which stores the current LED state.
    Since the board interface consists only of a header file, we use register
    GPIO2 on ATmega128RFA1 for shadowing, because it would be hard to ensure the
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

@par Build Options

  - stb128rfa1 : STB + RCB128RFA1

 */
#ifndef BOARD_STBRFA1_H
#define BOARD_STBRFA1_H

/** ID String for this hardware */
#if defined(stb128rfa1)
# define BOARD_TYPE (BOARD_STB128RFA1)
# define BOARD_NAME "stb128rfa1"
# define RADIO_TYPE (RADIO_ATMEGA128RFA1_C)
#elif defined(stb256rfr2)
# define BOARD_TYPE (BOARD_STB256RFR2)
# define BOARD_NAME "stb256rfr2"
# define RADIO_TYPE (RADIO_ATMEGA256RFR2)
#endif


/*=== Compile time parameters ========================================*/

#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== radio interface definition =====================================*/
#if BOARD_TYPE == BOARD_STB230 || BOARD_TYPE == BOARD_STB230B
# include "base_rdk230.h"
#elif BOARD_TYPE == BOARD_STB128RFA1 || BOARD_TYPE == BOARD_STB256RFR2
# define TRX_RESET_LOW()   do { TRXPR &= ~_BV(TRXRST); } while (0)
# define TRX_RESET_HIGH()  do { TRXPR |= _BV(TRXRST); } while (0)
# define TRX_SLPTR_LOW()   do { TRXPR &= ~_BV(SLPTR); } while (0)
# define TRX_SLPTR_HIGH()  do { TRXPR |= _BV(SLPTR); } while (0)
# define TRX_TSTAMP_REG TCNT1
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


static inline void hif_led_write(uint8_t val)
{
    PORTE |= 0x10;              /* de-assert /WR */
    DDRB = 0xFF;
    PORTB = val;
    PORTD = (PORTD & ~0xc0) | 0x40; /* select IO address */
    PORTE &= ~0x10;                 /* /WR pulse */
    PORTE |= 0x10;
    __asm volatile("nop");
    PORTD = (PORTD & ~0xc0) | 0x80; /* re-select SRAM */
    DDRB = 0;
    PORTB = 0;                  /* avoid pullups */
    PORTE &= ~0x10;             /* re-assert /WR */
}

static inline uint8_t hif_key_read(void)
{
    PORTE |= 0x10;              /* de-assert /WR */
    PORTD = (PORTD & ~0xc0) | 0x40; /* select IO address */
    PORTE &= ~0x20;             /* /RD pulse */
    PORTE |= 0x20;
    __asm volatile("nop");
    uint8_t rv = PINB;
    PORTD = (PORTD & ~0xc0) | 0x80; /* re-select SRAM */
    PORTE &= ~0x10;                 /* re-assert /WR */

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
