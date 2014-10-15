/* Copyright (c) 2007 - 2011 Axel Wachtler
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
 * @brief IMST GmbH WIMOD w/ mega328 and AT86RF230B.
 *
 * see http://www.mikrocontroller.net/attachment/101618/WiMOD240.jpg

 * The wiring of the radio and the ATmega is shown below.
<pre>
     AVR         RF230
     ---         -----
     PB0    <--  INT / ICP1
     PB1    -->  SLPTR
     PB2    -->  /SEL
     PB3    -->  MOSI
     PB4    <--  MISO
     PB5    -->  SCK

     PB6 intentionally unused for XTAL1
     PB7 intentionally unused for XTAL2


    (T1) PD5   <--  CLKM
         PD6   --> /RST
         PD7   --> TST

    USART:
        PD0 RXD
        PD1 TXD

    LED:  None
    KEY:  None


  IM240A      Mega328  Adapter   EvalBoard
  Pin Name              AB_01
  -----------------------------------------
   8  ADC1    ADC7      X1.12
   9  ADC2    ADC6      X1.14
  12  GPIO1   PC0       X1.15     Button1
  13  GPIO2   PC1       X1.16     Button2
  14  GPIO3   PC2       X1.19     Button3
  15  GPIO4   PC3       X1.18     DSW1
  21  GPIO5   PC5       X2.9
  23  GPIO6   PC4       X2.11

  18  DIO1    PD0       X2.18      UART-RX
  19  DIO2    PD1       X2.15      UART-TX
  20  DIO3    PD2       X2.7       LED4
  24  DIO4    PD3       X2.6       LED3
  25  DIO5    PD4       X2.5       LED2

</pre>

  @par Build Options

 - im240a : AT86RF230B w/ mega328.
 - im240a_eval : AT86RF230B w/ mega328 eval board.

 @note
  In order to use the eval board be shure to follow the install guide.
   - replace jumper e by cable e1 on X5
   - switch S1 to power supply USB.

   To program the mega328pa connect JTAG-ICE 10 pin header to X4.

   avrdude -P usb -c jtag2isp -p m328p -B10 -t

   The original fuses are
   LF 0xe2
   HF 0xd4
   EF 0x06


   The original bootloader seems to be a proprietary one and handles
   the ISMT *.wfi format. It is a one line string starting with
   "WIMOD_ID_3#" followed by probably the ASCII representation
   of the firmware ... to be investigated.

   In order to do a backup of the board use the following commands:
   <pre>
    avrdude -P usb -c jtag2isp -p m328p -B 10 -U fl:r:im240a_flash_initial.hex:i
    avrdude -P usb -c jtag2isp -p m328p -B 10 -U ee:r:im240a_eeprom_initial.hex:i
   </pre>

  Restoration of the backup can be done by the command:
  <pre>
  avrdude -P usb -c jtag2isp -p m328p -B 10 \
            -U fl:w:im240a_flash_initial.hex:i \
            -U ee:w:im240a_eeprom_initial.hex:i \
            -U lf:w:0xe2:m \
            -U hf:w:0xd4:m \
            -U ef:w:0x06:m
    </pre>
 */


#ifndef BOARD_WIMOD_H
#define BOARD_WIMOD_H (1)
#if defined(im240a)
# define BOARD_NAME "im240a"
# define BOARD_TYPE BOARD_IM240A
#elif defined(im240a_eval)
# define BOARD_NAME "im240a_eval"
# define BOARD_TYPE BOARD_IM240A_EVAL
#endif
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

#define PORT_TRX_SLPTR  PORTB
#define DDR_TRX_SLPTR   DDRB
#define MASK_TRX_SLPTR  (_BV(PB1))

#define TRX_SLPTR_INIT() DDR_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_HIGH() PORT_TRX_SLPTR |= MASK_TRX_SLPTR
#define TRX_SLPTR_LOW()  PORT_TRX_SLPTR &= ~MASK_TRX_SLPTR


/*=== IRQ access macros ==============================================*/
# define TRX_IRQ_vect    TIMER1_CAPT_vect    /**< interrupt vector name */

/** init interrupt handling
 *  - transceiver IRQ at PB0
 *  - rising edge triggers ICP1 (ICES1),
 *  - timer capture is enabled (ICF1)

 */
# define TRX_IRQ_INIT()  do{\
                            /* TCCR1B |= (_BV(ICNC1) | _BV(ICES1) | _BV(CS12) | _BV(CS10)); */\
                            TCCR1B |= (_BV(ICNC1) | _BV(ICES1));\
                            TIFR1 = _BV(ICF1);\
                          } while(0)

/** disable TRX interrupt */
#define DI_TRX_IRQ() {TIMSK1 &= ~_BV(ICIE1);}

/** enable TRX interrupt */
#define EI_TRX_IRQ() {TIMSK1 |= _BV(ICIE1);}

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
#if (BOARD_TYPE == BOARD_IM240A_EVAL)
/* need to implement bit reversed LED mapping,
 * Current Usage
 * LSB : LED4, LSB+1 : LED3, MSB : LED2
 */
# define LED_PORT      PORTD
# define LED_DDR       DDRD
# define LED_MASK      (0x1c)
# define LED_SHIFT     (2)
# define LED_NUMBER    (3)
# define LEDS_INVERSE  (1)
#else
# define NO_LEDS       (1)
#endif

/*=== KEY access macros ==============================================*/
#if (BOARD_TYPE == BOARD_IM240A_EVAL)
# define PORT_KEY      PORTC
# define PIN_KEY       PINC
# define DDR_KEY       DDRC
# define MASK_KEY      (0x07)
# define SHIFT_KEY     (0)
# define INVERSE_KEYS  (1)
# define PULLUP_KEYS   (1)
#else
# define NO_KEYS       (1)
#endif


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
        TIMSK1 |= _BV(TOIE1); \
    }while(0)
# define TIMER_IRQ_vect   TIMER1_OVF_vect

#endif /* BOARD_WIMOD_H */
