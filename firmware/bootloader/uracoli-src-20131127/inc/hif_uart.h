/* Copyright (c) 2007,2008,2009
    Marco Arena
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
 * @file
 * Internal UART abstraction
 *
 */

#if ! defined(HIF_UART_H) && (HIF_TYPE_IS_UART == 1)
#define HIF_UART_H

/* === types ============================================= */
typedef struct
{
    uint8_t * rxbuf;
    uint16_t rxsize;
    uint8_t * txbuf;
    uint16_t txsize;
} hif_uart_mgm_t;

/* === macros ============================================ */
#define ENCODE_BAUDRATE(br) ((F_CPU)/((br)*16l)-1)
#define ENCODE_BAUDRATE_U2X(br) ((F_CPU)/((br)*8l)-1)

/* === UART definitions for ATmega1281/ATmega128RFA1 === */
#if defined(__AVR_ATmega1281__) || defined(__AVR_ATmega128RFA1__) \
    || defined(__AVR_ATmega1284P__) || defined(__AVR_AT90CAN128__) \
    || defined(__AVR_ATmega256RFR2__)
# if HIF_TYPE == HIF_UART_0
#  define HIF_UART_STATUS   UCSR0A
#  define HIF_UART_CONTROL  UCSR0B
#  define HIF_UART_DATA     UDR0
#  define HIF_UART_UDRIE    UDRIE0
#  define HIF_UART_RX_vect  USART0_RX_vect
#  define HIF_UART_TX_vect  USART0_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE0))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE0))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   uint16_t br;

#if defined(HIF_UART_FORCE_U2X)
		br = ENCODE_BAUDRATE_U2X(baudrate);
		HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
#else
   	   br = ENCODE_BAUDRATE(baudrate);
       /* Set baud rate */
       if ( br & 0x8000 )
       {
   		   HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
   		   br &= ~0x8000;
   	   }
#endif

       UBRR0H = (uint8_t)(br>>8);
       UBRR0L = (uint8_t) br;

       /* Enable USART receiver and transmitter and receive complete interrupt */
       HIF_UART_CONTROL = (_BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0));

       /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
       #ifdef URSEL0
       UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
       #else
       UCSR0C = (3<<UCSZ00);
       #endif

   }
# elif HIF_TYPE == HIF_UART_1
#  define HIF_UART_STATUS   UCSR1A
#  define HIF_UART_CONTROL  UCSR1B
#  define HIF_UART_DATA     UDR1
#  define HIF_UART_UDRIE    UDRIE1
#  define HIF_UART_RX_vect  USART1_RX_vect
#  define HIF_UART_TX_vect  USART1_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE1))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE1))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   /* init uart 1 mega 1281*/
   uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
      /* Set baud rate */
      if ( br & 0x8000 )
      {
   	      HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
   	      br &= ~0x8000;
      }
      UBRR1H = (uint8_t)(br>>8);
      UBRR1L = (uint8_t) br;

      /* Enable USART receiver and transmitter and receive complete interrupt */
      HIF_UART_CONTROL = (_BV(RXCIE1)|(1<<RXEN1)|(1<<TXEN1));

      /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
      #ifdef URSEL0
      UCSR1C = (1<<URSEL0)|(3<<UCSZ00);
      #else
      UCSR1C = (3<<UCSZ00);
      #endif
   }
# else
#  error "Unknown UART for __AVR_ATmega1281__ or __AVR_ATmega128RFA1__"
# endif /*HIF_UART_x*/

/* === UART definitions for ATmega644P and  ATmega128 === */
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__)|| defined(__AVR_ATmega128__)
# if HIF_TYPE == HIF_UART_0
#  define HIF_UART_STATUS   UCSR0A
#  define HIF_UART_CONTROL  UCSR0B
#  define HIF_UART_DATA     UDR0
#  define HIF_UART_UDRIE    UDRIE0
#  define HIF_UART_RX_vect  USART0_RX_vect
#  define HIF_UART_TX_vect  USART0_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE0))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE0))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   uint16_t br;
       /* init uart 0 mega 644p*/
       br = ENCODE_BAUDRATE(baudrate);
       /* Set baud rate */
       if ( br & 0x8000 )
       {
   		   HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
   		   br &= ~0x8000;
   	   }
       UBRR0H = (uint8_t)(br>>8);
       UBRR0L = (uint8_t) br;

       /* Enable USART receiver and transmitter and receive complete interrupt */
       HIF_UART_CONTROL = (_BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0));

       /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
       #ifdef URSEL0
       UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
       #else
       UCSR0C = (3<<UCSZ00);
       #endif

   }
# elif HIF_TYPE == HIF_UART_1
#  define HIF_UART_STATUS   UCSR1A
#  define HIF_UART_CONTROL  UCSR1B
#  define HIF_UART_DATA     UDR1
#  define HIF_UART_UDRIE    UDRIE1
#  define HIF_UART_RX_vect  USART1_RX_vect
#  define HIF_UART_TX_vect  USART1_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE1))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE1))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   /* init uart 1 mega 644p*/
   uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
      /* Set baud rate */
      if ( br & 0x8000 )
      {
   	      HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
   	      br &= ~0x8000;
      }
      UBRR1H = (uint8_t)(br>>8);
      UBRR1L = (uint8_t) br;

      /* Enable USART receiver and transmitter and receive complete interrupt */
      HIF_UART_CONTROL = (_BV(RXCIE1)|(1<<RXEN1)|(1<<TXEN1));

      /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
      #ifdef URSEL0
      UCSR1C = (1<<URSEL0)|(3<<UCSZ00);
      #else
      UCSR1C = (3<<UCSZ00);
      #endif
   }
# else
#  error "Unknown UART for __AVR_ATmega644__ or __AVR_ATmega644P__"
# endif /*HIF_UART_x*/

/* === UART definitions for ATmega16 === */
#elif defined(__AVR_ATmega16__)
# if HIF_TYPE == HIF_UART_0
#  define HIF_UART_DATA     UDR
#  define HIF_UART_STATUS   UCSRA
#  define HIF_UART_CONTROL  UCSRB
#  define HIF_UART_UDRIE    UDRIE
#  define HIF_UART_RX_vect  USART_RXC_vect
#  define HIF_UART_TX_vect  USART_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   /* init uart 1 mega 1281*/
   uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
      /* Set baud rate */
      if ( br & 0x8000 )
      {
   	      HIF_UART_STATUS = _BV(U2X);  //Enable 2x speed
   	      br &= ~0x8000;
      }
      UBRRH = (uint8_t)(br>>8);
      UBRRL = (uint8_t) br;
      /* Enable USART receiver and transmitter and receive complete interrupt */
      HIF_UART_CONTROL = (_BV(RXCIE)|(1<<RXEN)|(1<<TXEN));

      /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
      UCSRC = (1<<URSEL)|(3<<UCSZ0);
   }
# else
#  error "Unknown UART for __AVR_ATmega16__"
# endif
#elif defined(__AVR_ATmega8__)
# if HIF_TYPE == HIF_UART_0
#  define HIF_UART_DATA     UDR
#  define HIF_UART_STATUS   UCSRA
#  define HIF_UART_CONTROL  UCSRB
#  define HIF_UART_UDRIE    UDRIE
#  define HIF_UART_RX_vect  USART_RXC_vect
#  define HIF_UART_TX_vect  USART_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE)|_BV(DOR)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   /* init uart 1 mega 1281*/
   uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
      /* Set baud rate */
      if ( br & 0x8000 )
      {
   	      HIF_UART_STATUS = _BV(U2X);  //Enable 2x speed
   	      br &= ~0x8000;
      }
      UBRRH = (uint8_t)(br>>8);
      UBRRL = (uint8_t) br;
      /* Enable USART receiver and transmitter and receive complete interrupt */
      HIF_UART_CONTROL = (_BV(RXCIE)|(1<<RXEN)|(1<<TXEN));

      /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
      UCSRC = (1<<URSEL)|(3<<UCSZ0);
   }
# else
#  error "Unknown UART for __AVR_ATmega8__"
# endif
#elif defined(__AVR_ATmega88__) || defined(__AVR_ATmega328__)
# if HIF_TYPE == HIF_UART_0
#  define HIF_UART_DATA     UDR0
#  define HIF_UART_STATUS   UCSR0A
#  define HIF_UART_CONTROL  UCSR0B
#  define HIF_UART_UDRIE    UDRIE0
#  define HIF_UART_RX_vect  USART_RX_vect
#  define HIF_UART_TX_vect  USART_UDRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE0)|_BV(DOR0)))
#  define HIF_UART_TXIRQ_EI() (HIF_UART_CONTROL |= _BV(UDRIE0))
#  define HIF_UART_TXIRQ_DI() (HIF_UART_CONTROL &= ~_BV(UDRIE0))
   inline void HIF_UART_INIT(uint32_t baudrate)
   {
   /* init uart 1 mega 1281*/
   uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
      /* Set baud rate */
      if ( br & 0x8000 )
      {
   	      HIF_UART_STATUS = _BV(U2X0);  //Enable 2x speed
   	      br &= ~0x8000;
      }
      UBRR0H = (uint8_t)(br>>8);
      UBRR0L = (uint8_t) br;
      /* Enable USART receiver and transmitter and receive complete interrupt */
      HIF_UART_CONTROL = (_BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0));

      /* Set frame format: asynchronous, 8data, no parity, 1stop bit */
      #ifdef URSEL0
      UCSR0C = (1<<URSEL0)|(3<<UCSZ00);
      #else
      UCSR0C = (3<<UCSZ00);
      #endif
   }
# else
#  error "Unknown UART for __AVR_ATmega88__ / __AVR_ATmega328__"
# endif

#elif defined(__AVR_ATxmega256A3__)
# if HIF_TYPE == HIF_UART_0

#  define HIF_UART_DATA     USARTD0.DATA
#  define HIF_UART_STATUS   USARTD0.STATUS
#  define HIF_UART_CONTROL  USARTD0.CTRLB
#  define HIF_UART_RX_vect  USARTD0_RXC_vect
#  define HIF_UART_TX_vect  USARTD0_DRE_vect
#  define HIF_UART_RX_ERROR(status) (status & (_BV(FE0)|_BV(DOR0)))
#  define HIF_UART_TXIRQ_EI() do{ USARTD0.CTRLA |= USART_DREINTLVL_MED_gc; }while(0)
#  define HIF_UART_TXIRQ_DI() do{ USARTD0.CTRLA &= ~USART_DREINTLVL_gm; }while(0)
   inline void HIF_UART_INIT(uint32_t baudrate)
   {   
      uint16_t br;
      br = ENCODE_BAUDRATE(baudrate);
	  
      USARTD0.BAUDCTRLB = (uint8_t)(br >> 8);
      USARTD0.BAUDCTRLA = (uint8_t)br;

      /* Enable Rx and Tx */
      USARTD0.CTRLB |= USART_TXEN_bm | USART_RXEN_bm;

      /* Set 8N1  */
      USARTD0.CTRLC = USART_CHSIZE1_bm | USART_CHSIZE0_bm;
	
      PORTD.DIRSET = _BV(3); /* TxD as output */
      PORTD.DIRCLR = _BV(2); /* RxD as input */
	
	  USARTD0.CTRLA = USART_DREINTLVL_MED_gc | USART_RXCINTLVL_MED_gc;
	  PMIC.CTRL |= PMIC_MEDLVLEN_bm;
   }
	
# else
#  error "Unknown UART for __AVR_ATxmega256A3__"
# endif

#else
# error "An UART is not supported for the current MCU type."
#endif /* defined(__ARV_xxxxx__) */

#endif /* ! defined(HIF_UART_H) && HIF_TYPE_IS_UART */
