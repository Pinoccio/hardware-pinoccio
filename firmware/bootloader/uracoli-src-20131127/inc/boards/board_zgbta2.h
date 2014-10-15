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
 * @brief Meshnetics Zigbit ATmega1281-A2
 *        with ATmega1281 and AT86RF230.
 *
 * See also
 * - <a href="http://www.meshnetics.com/dev-tools/meshbean/"
 *      target="_ext">http://www.meshnetics.com/dev-tools/meshbean/</a>
 * - http://www.mikrocontroller.net/articles/Meshnetics_Zigbee
 *
 * The wiring of the radio and the ATmega is shown below:
 *
<pre>
     AVR      RF230
     ---      -----
     PB4  -->  SLPTR
     PE5  <--  IRQ (INT5)
     PA7  -->  RSTN
     PB0  -->  SS
     PB2  -->  MOSI
     PB3  <--  MISO
     PB1  -->  SCK
     XTAL1 <--  MCLK

Fuses/Locks:
    uracoli settings:
     LF: 0xe2 - 8MHz internal RC Osc.
     HF: 0x91 - without boot loader (0x11 for debugging)
     HF: 0x90 - with boot loader    (0x10 for debugging)
     EF: 0xff
     LOCK: 0xef - protection of boot section

    original settings
     LF: 0x62 - RC oscillator 1 MHz
     HF: 0x9c - JTAG enabled, ISP enabled, reset to bootloader,
                1024 words bootloader size
     EF: 0xff - no brownout

Bootloader:
    Start at byte=0x1e000, address=0xf000, size = 4096 instructions/ 8192 bytes

</pre>

 - Host Interface: @ref HIF_TYPE


@par Build Options

 - zgbt1281a2uart0 : Zigbit, using UART0 for HIF (ISP pins!)
 - zgbt1281a2uart1 : Zigbit, using UART1 for HIF
 - zgbt1281a2nouart : Zigbit, no HIF
 */

/*
 * Courtesy Martin Vejnár:
 *
 * http://ratatanek.cz/blog/zigbit-atzb-24-a2-pinout
 * http://ratatanek.cz/files/node/16/zigbit_mcu_pinout_mapping.svg
 *
 * This is the pin mapping of the Zigbit module:
 *
 * ZGBT  AVR   Atmel Zigbit    Remark
 * pin#  port  datasheet name
 * ===================================================
 *   1   B1    SPI_CLK         shared with trx and ISP
 *   2   B3    SPI_MISO        -"-, *not* ISP!
 *   3   B2    SPI_MOSI        -"-, *not* ISP
 *   4   B5    GPIO0
 *   5   B6    GPIO1
 *   6   B7    GPIO2
 *   7   TOSC1 OSC32K          from 32.768 kHz oscillator
 *   8   /RST  RESET
 *   9   DGND  DGND
 *  10   CLKM  CPU_CLK         CLKM from transceiver,
 *  11   D0    I2C_CLK            [ can be fused as CPU
 *  12   D1    I2C_DATA           [ clock input
 *  13   D2    UART_TXD        UART1
 *  14   D3    UART_RXD        -"-
 *  15   D4    UART_RTS
 *  16   D5    UART_CTS
 *  17   D6    GPIO6
 *  18   D7    GPIO7
 *  19   G0    GPIO3
 *  20   G1    GPIO4
 *  21   G2    GPIO5
 *  22   DGND  DGND
 *  23   DGND  DGND
 *  24   DVCC  DVCC
 *  25   DVCC  DVCC
 *  26   F5    JTAG_TMS
 *  27   F7    JTAG_TDI
 *  28   F6    JTAG_TDO
 *  29   F4    JTAG_TCK
 *  30   F3    ADC_INPUT3
 *  31   F2    ADC_INPUT2
 *  32   F1    ADC_INPUT1
 *  33   F0    BAT             voltage divider?
 *  34   AREF  V_AREF
 *  35   AGND  AGND
 *  36   G5    GPIO_1WR
 *  37   E4    UART_DTR
 *  38   E0    USART0_RXD      also ISP PDI
 *  39   E1    USART0_TXD      also ISP PDO
 *  40   E2    USART0_EXTCLK
 *  41   E3    GPIO8
 *  42   E7    IRQ_7
 *  43   E6    IRQ_6
 */

#if defined(zgbt1281a2uart0)
# define BOARD_TYPE BOARD_ZGBT1281A2UART0
# define BOARD_NAME "zgbt1281a2uart0"
# define RADIO_TYPE (RADIO_AT86RF230)
#endif
#if defined(zgbt1281a2uart1)
# define BOARD_TYPE BOARD_ZGBT1281A2UART1
# define BOARD_NAME "zgbt1281a2uart1"
# define RADIO_TYPE (RADIO_AT86RF230)
#endif
#if defined(zgbt1281a2nouart)
# define BOARD_TYPE BOARD_ZGBT1281A2NOUART
# define BOARD_NAME "zgbt1281a2nouart"
# define RADIO_TYPE (RADIO_AT86RF230)
#endif

#ifndef BOARD_ZGBTA2_H
#define BOARD_ZGBTA2_H

/*=== Compile time parameters ========================================*/

/*=== radio interface definition =====================================*/
#include "base_zdma1281.h"

/*=== LED access macros ==============================================*/
#define LED_NUMBER    (0)   /**< number of LEDs for this board */
#define NO_LEDS       (1)

/*=== KEY access macros ==============================================*/
#define NO_KEYS       (1)

/*=== Host Interface ================================================*/
/**
 * UART0 is routed to the ISP pins, alternatively, UART1 can be used.
 *
 */
#if defined(zgbt1281a2uart0)
#  define HIF_TYPE (HIF_UART_0)
#elif defined(zgbt1281a2uart1)
#  define HIF_TYPE (HIF_UART_1)
#elif defined(zgbt1281a2nouart)
#  define HIF_TYPE HIF_NONE
#else
#  error "Don't know which UART to use."
#endif

/*=== TIMER Interface ===============================================*/
/**
 *  - CS1[2:0]  = 1 : Prescaler = 1
 *  - WGM1[3:0] = 0 : Mode = 4 : CTC operation
 *
 * Timer is clocked at F_CPU, and @ref TIMER_IRQ_vect is called every
 * 65535 ticks.
 */
#define HWTMR_PRESCALE  (1)
#define HWTIMER_TICK    ((1.0*HWTMR_PRESCALE)/F_CPU)
#define HWTIMER_TICK_NB (0xFFFFUL+1)
#define HWTIMER_REG     (TCNT1)
#define TIMER_TICK      (HWTIMER_TICK_NB * HWTIMER_TICK)
#define TIMER_POOL_SIZE (4)
#define TIMER_INIT() \
    do{ \
        TCCR1B |= (_BV(CS10));\
        TIMSK1 |= _BV(TOIE1); \
    }while(0)
#define TIMER_IRQ_vect  TIMER1_OVF_vect

#endif /* BOARD_ZGBTA2_H*/
