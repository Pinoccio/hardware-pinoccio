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
 * @brief This board is describes the STK541 that comes with Atmels
 * ATAVRRZ541 Packet Sniffer Kit.
 *
 * The Packet Sniffer Kit STK541 is a carrier board, which fits on
 * top of a STK500 and is used for the radio controller board family.
 * The transceiver wiring fits the common RCBs.
 * The wiring of the radio and the ATmega is shown below:
 *
 *
<pre>
     AVR      RF230
     ---      -----
     PB4  -->  SLPTR
     P??  <--  MCLK
     PD4  <--  IRQ (ICP1)
     PB5  -->  RSTN
     PB0  -->  SS
     PB2  -->  MOSI
     PB3  <--  MISO
     PB1  -->  SCK

    The STK541 has no LEDS and Keys, so we define the ressources
    from the  RCB230 here.

    KEY: PE5
    LEDS: PE2:PE4

    DBG: PD0 - connector X20:2 (ground X20:1)

Fuses/Locks:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x11 - without boot loader
     HF: 0x10 - with boot loader
     EF: 0xff
     LOCK: 0xef - protection of boot section

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

    @image html atmel_stk541.jpg "Atmel STK541 and Radio Controller Board"
    @image latex atmel_stk541.jpg "Atmel STK541 and Radio Controller Board"
</pre>


@par Build Options

  - psk230  : STK541 + RCB230 prior V3.2 / V3.3.1 (AT86RF230A)
  - psk230b : STK541 + RCB230 V3.2 / RCB230 V3.3.1 (AT86RF230B)
  - psk231  : STK541 * RCB231 V4.0.2 / RCB231ED V4.1.1
  - psk212  : STK541 + RCB212SMA V5.3.2


 */

#if defined(psk230)
# define BOARD_TYPE (BOARD_PSK230)
# define BOARD_NAME "psk230"
# define RADIO_TYPE (RADIO_AT86RF230A)
#elif defined(psk230b)
# define BOARD_TYPE (BOARD_PSK230B)
# define BOARD_NAME "psk230b"
# define RADIO_TYPE (RADIO_AT86RF230B)
#elif defined(psk231)
# define BOARD_TYPE (BOARD_PSK231)
# define BOARD_NAME "psk231"
# define RADIO_TYPE (RADIO_AT86RF231)
#elif defined(psk212)
# define BOARD_TYPE (BOARD_PSK212)
# define BOARD_NAME "psk212"
# define RADIO_TYPE (RADIO_AT86RF212)
#elif defined(psk232)
# define BOARD_TYPE (BOARD_PSK232)
# define BOARD_NAME "psk232"
# define RADIO_TYPE (RADIO_AT86RF232)
#elif defined(psk233)
# define BOARD_TYPE (BOARD_PSK233)
# define BOARD_NAME "psk233"
# define RADIO_TYPE (RADIO_AT86RF233)
#endif

#ifndef BOARD_STK541_H
#define BOARD_STK541_H

/*=== Compile time parameters ========================================*/
#ifndef DEFAULT_SPI_RATE
# define DEFAULT_SPI_RATE  (SPI_RATE_1_2)
#endif

/*=== radio interface definition =====================================*/
#if BOARD_TYPE == BOARD_PSK230 || BOARD_TYPE == BOARD_PSK230B
# include "base_rdk230.h"
#else
# include "base_rdk2xx.h"
#endif

/*=== LED access macros ==============================================*/
#define LED_PORT      PORTE
#define LED_DDR       DDRE
#define LED_MASK      (0x1c)
#define LED_SHIFT     (2)
#define LEDS_INVERSE  (1)
#define LED_NUMBER    (3)
/*=== KEY access macros ==============================================*/
#define PORT_KEY      PORTE
#define PIN_KEY       PINE
#define DDR_KEY       DDRE
#define MASK_KEY      (0x20)
#define SHIFT_KEY     (5)
#define INVERSE_KEYS  (1)
#define PULLUP_KEYS   (1)

/*=== Host Interface ================================================*/
#define HIF_TYPE      (HIF_FT245)
#define HIF_IO_ENABLE XRAM_ENABLE

#define USB_FIFO_AD  0xF000
#define HIF_USB_READ()  (*(volatile uint8_t*)(USB_FIFO_AD))
#define HIF_USB_WRITE(x) do { (*(volatile uint8_t*)(USB_FIFO_AD)) = (x); }while(0)
#define XRAM_ENABLE() do {\
    DDRC = 0xFF;\
    PORTC = 0x00;\
    XMCRA |= (1 << SRE);\
    XMCRB = (1 << XMBK);\
    } while(0)

#define HIF_NO_DATA   (0x0100)
#define FT245_DDR    DDRE
#define FT245_PORT   PORTE
#define FT245_PIN    PINE
#define FT245_TXE    _BV(6)
#define FT245_RXF    _BV(7)
#define FT245_INIT() do { \
           XMCRA |= (1 << SRE); \
           XMCRB = (1 << XMBK);\
           FT245_DDR &= ~(FT245_TXE|FT245_RXF);\
           FT245_PORT |= (FT245_TXE|FT245_RXF);\
        } while(0)

#define FT245_TX_IS_BLOCKED()    (0 != (FT245_PIN & FT245_TXE))
#define FT245_RX_HAS_DATA()      (0 == (FT245_PIN & FT245_RXF))

/*=== TIMER Interface ===============================================*/
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL+1)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)

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
        TCCR1B |= (_BV(CS10));\
        TIMSK1 |= _BV(TOIE1); \
    }while(0)


/*=== DBG interface =================================================*/
# define DBG_PORT PORTD
# define DBG_DDR DDRD
# define DBG_PIN (1<<PD0)

#endif /* BOARD_STK541_H */
