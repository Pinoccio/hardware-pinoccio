/*****************************************************************************
Title:     STK500v2 compatible bootloader
           Modified for Wiring board ATMega128-16MHz
Author:    Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:      $Id: stk500boot.c,v 1.11 2006/06/25 12:39:17 peter Exp $
Compiler:  avr-gcc 3.4.5 or 4.1 / avr-libc 1.4.3
Hardware:  All AVRs with bootloader support, tested with ATmega8
License:   GNU General Public License

Modified:  Worapoht Kornkaewwattanakul <dev@avride.com>   http://www.avride.com
Date:      17 October 2007
Update:    1st, 29 Dec 2007 : Enable CMD_SPI_MULTI but ignore unused command by return 0x00 byte response..
Compiler:  WINAVR20060421
Description: add timeout feature like previous Wiring bootloader

DESCRIPTION:
    This program allows an AVR with bootloader capabilities to
    read/write its own Flash/EEprom. To enter Programming mode
    an input pin is checked. If this pin is pulled low, programming mode
    is entered. If not, normal execution is done from $0000
    "reset" vector in Application area.
    Size fits into a 1024 word bootloader section
  when compiled with avr-gcc 4.1
  (direct replace on Wiring Board without fuse setting changed)

USAGE:
    - Set AVR MCU type and clock-frequency (F_CPU) in the Makefile.
    - Set baud rate below (AVRISP only works with 115200 bps)
    - compile/link the bootloader with the supplied Makefile
    - program the "Boot Flash section size" (BOOTSZ fuses),
      for boot-size 1024 words:  program BOOTSZ01
    - enable the BOOT Reset Vector (program BOOTRST)
    - Upload the hex file to the AVR using any ISP programmer
    - Program Boot Lock Mode 3 (program BootLock 11 and BootLock 12 lock bits) // (leave them)
    - Reset your AVR while keeping PROG_PIN pulled low // (for enter bootloader by switch)
    - Start AVRISP Programmer (AVRStudio/Tools/Program AVR)
    - AVRISP will detect the bootloader
    - Program your application FLASH file and optional EEPROM file using AVRISP

Note:
    Erasing the device without flashing, through AVRISP GUI button "Erase Device"
    is not implemented, due to AVRStudio limitations.
    Flash is always erased before programming.

  AVRdude:
  Please uncomment #define REMOVE_CMD_SPI_MULTI when using AVRdude.
  Comment #define REMOVE_PROGRAM_LOCK_BIT_SUPPORT to reduce code size
  Read Fuse Bits and Read/Write Lock Bits is not supported

NOTES:
    Based on Atmel Application Note AVR109 - Self-programming
    Based on Atmel Application Note AVR068 - STK500v2 Protocol

LICENSE:
    Copyright (C) 2006 Peter Fleury

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*****************************************************************************/

//************************************************************************
//*  Edit History
//************************************************************************
//*  Jul  7,  2010  <MLS> = Mark Sproul msproul@skycharoit.com
//*  Jul  7,  2010  <MLS> Working on mega2560. No Auto-restart
//*  Jul  7,  2010  <MLS> Switched to 8K bytes (4K words) so that we have room for the monitor
//*  Jul  8,  2010  <MLS> Found older version of source that had auto restart, put that code back in
//*  Jul  8,  2010  <MLS> Adding monitor code
//*  Jul 11,  2010  <MLS> Added blinking LED while waiting for download to start
//*  Jul 11,  2010  <MLS> Added EEPROM test
//*  Jul 29,  2010  <MLS> Added recchar_timeout for timing out on bootloading
//*  Aug 23,  2010  <MLS> Added support for atmega2561
//*  Aug 26,  2010  <MLS> Removed support for BOOT_BY_SWITCH
//*  Sep  8,  2010  <MLS> Added support for atmega16
//*  Nov  9,  2010  <MLS> Issue 392:Fixed bug that 3 !!! in code would cause it to jump to monitor
//*  Jun 24,  2011  <MLS> Removed analogRead (was not used)
//*  Dec 29,  2011  <MLS> Issue 181: added watch dog timmer support
//*  Dec 29,  2011  <MLS> Issue 505:  bootloader is comparing the seqNum to 1 or the current sequence
//*  Jan  1,  2012  <MLS> Issue 543: CMD_CHIP_ERASE_ISP now returns STATUS_CMD_FAILED instead of STATUS_CMD_OK
//*  Jan  1,  2012  <MLS> Issue 543: Write EEPROM now does something (NOT TESTED)
//*  Jan  1,  2012  <MLS> Issue 544: stk500v2 bootloader doesn't support reading fuses
//************************************************************************

//************************************************************************
//*  these are used to test issues
//*  http://code.google.com/p/arduino/issues/detail?id=505
//*  Reported by mark.stubbs, Mar 14, 2011
//*  The STK500V2 bootloader is comparing the seqNum to 1 or the current sequence
//*  (IE: Requiring the sequence to be 1 or match seqNum before continuing).
//*  The correct behavior is for the STK500V2 to accept the PC's sequence number, and echo it back for the reply message.
#define  _FIX_ISSUE_505_
//************************************************************************

#include  <inttypes.h>
#include  <avr/io.h>
#include  <avr/interrupt.h>
#include  <avr/boot.h>
#include  <avr/pgmspace.h>
#include  <util/delay.h>
#include  <avr/eeprom.h>
#include  <avr/common.h>
#include  <stdlib.h>
#include  "command.h"

#ifndef EEWE
  #define EEWE    1
#endif
#ifndef EEMWE
  #define EEMWE   2
#endif

/*
 * Uncomment the following lines to save code space
 */
#define  REMOVE_PROGRAM_LOCK_BIT_SUPPORT    // disable program lock bits
//#define  REMOVE_BOOTLOADER_LED        // no LED to show active bootloader
//#define  REMOVE_CMD_SPI_MULTI        // disable processing of SPI_MULTI commands, Remark this line for AVRDUDE <Worapoht>
//


#define UART_BAUDRATE_DOUBLE_SPEED 1

/*
 * define CPU frequency in Mhz here if not defined in Makefile
 */
#ifndef F_CPU
  #define F_CPU 16000000UL
#endif

/*
 * UART Baudrate, AVRStudio AVRISP only accepts 115200 bps
 */

#ifndef BAUDRATE
  #define BAUDRATE 115200
#endif

/*
 *  Enable (1) or disable (0) USART double speed operation
 */
#ifndef UART_BAUDRATE_DOUBLE_SPEED
  #if defined (__AVR_ATmega32__)
    #define UART_BAUDRATE_DOUBLE_SPEED 0
  #else
    #define UART_BAUDRATE_DOUBLE_SPEED 1
  #endif
#endif

/*
 * HW and SW version, reported to AVRISP, must match version of AVRStudio
 */
#define CONFIG_PARAM_BUILD_NUMBER_LOW  0
#define CONFIG_PARAM_BUILD_NUMBER_HIGH  0
#define CONFIG_PARAM_HW_VER        0x0F
#define CONFIG_PARAM_SW_MAJOR      2
#define CONFIG_PARAM_SW_MINOR      0x0A

#define BOOTSIZE 4096	// bootsize in words

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

/*
 * Signature bytes are not available in avr-gcc io_xxx.h
 */
#if defined (__AVR_ATmega8__)
  #define SIGNATURE_BYTES 0x1E9307
#elif defined (__AVR_ATmega16__)
  #define SIGNATURE_BYTES 0x1E9403
#elif defined (__AVR_ATmega32__)
  #define SIGNATURE_BYTES 0x1E9502
#elif defined (__AVR_ATmega8515__)
  #define SIGNATURE_BYTES 0x1E9306
#elif defined (__AVR_ATmega8535__)
  #define SIGNATURE_BYTES 0x1E9308
#elif defined (__AVR_ATmega162__)
  #define SIGNATURE_BYTES 0x1E9404
#elif defined (__AVR_ATmega128__)
  #define SIGNATURE_BYTES 0x1E9702
#elif defined (__AVR_ATmega1280__)
  #define SIGNATURE_BYTES 0x1E9703
#elif defined (__AVR_ATmega2560__)
  #define SIGNATURE_BYTES 0x1E9801
#elif defined (__AVR_ATmega2561__)
  #define SIGNATURE_BYTES 0x1e9802
#elif defined (__AVR_ATmega256RFR2__)
  #define SIGNATURE_BYTES 0x1eA802
#elif defined (__AVR_ATmega1284P__)
  #define SIGNATURE_BYTES 0x1e9705
#elif defined (__AVR_ATmega640__)
  #define SIGNATURE_BYTES  0x1e9608
#elif defined (__AVR_ATmega64__)
  #define SIGNATURE_BYTES  0x1E9602
#elif defined (__AVR_ATmega169__)
  #define SIGNATURE_BYTES  0x1e9405
#elif defined (__AVR_AT90USB1287__)
  #define SIGNATURE_BYTES  0x1e9782
#else
  #error "no signature definition for MCU available"
#endif

#if defined(_AVR_ATmega256RFR2_XPLAINED_PRO_)
  // Xplained Pro board uses UART1 for virtual com port
  #define  UART_BAUD_RATE_LOW      UBRR1L
  #define  UART_STATUS_REG        UCSR1A
  #define  UART_CONTROL_REG      UCSR1B
  #define  UART_ENABLE_TRANSMITTER    TXEN1
  #define  UART_ENABLE_RECEIVER    RXEN1
  #define  UART_TRANSMIT_COMPLETE    TXC1
  #define  UART_RECEIVE_COMPLETE    RXC1
  #define  UART_DATA_REG        UDR1
  #define  UART_DOUBLE_SPEED      U2X1
#elif defined(_BOARD_ROBOTX_) || defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB1286__)
  #define  UART_BAUD_RATE_LOW      UBRR1L
  #define  UART_STATUS_REG        UCSR1A
  #define  UART_CONTROL_REG      UCSR1B
  #define  UART_ENABLE_TRANSMITTER    TXEN1
  #define  UART_ENABLE_RECEIVER    RXEN1
  #define  UART_TRANSMIT_COMPLETE    TXC1
  #define  UART_RECEIVE_COMPLETE    RXC1
  #define  UART_DATA_REG        UDR1
  #define  UART_DOUBLE_SPEED      U2X1

#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) \
  || defined(__AVR_ATmega8515__) || defined(__AVR_ATmega8535__)
  /* ATMega8 with one USART */
  #define  UART_BAUD_RATE_LOW      UBRRL
  #define  UART_STATUS_REG        UCSRA
  #define  UART_CONTROL_REG      UCSRB
  #define  UART_ENABLE_TRANSMITTER    TXEN
  #define  UART_ENABLE_RECEIVER    RXEN
  #define  UART_TRANSMIT_COMPLETE    TXC
  #define  UART_RECEIVE_COMPLETE    RXC
  #define  UART_DATA_REG        UDR
  #define  UART_DOUBLE_SPEED      U2X

#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) || defined(__AVR_ATmega162__) \
   || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) || defined(__AVR_ATmega256RFR2__)    \
   || defined(_PINOCCIO_256RFR2_)
  /* ATMega with two USART, use UART0 */
  #define  UART_BAUD_RATE_LOW      UBRR0L
  #define  UART_STATUS_REG        UCSR0A
  #define  UART_CONTROL_REG      UCSR0B
  #define  UART_ENABLE_TRANSMITTER    TXEN0
  #define  UART_ENABLE_RECEIVER    RXEN0
  #define  UART_TRANSMIT_COMPLETE    TXC0
  #define  UART_RECEIVE_COMPLETE    RXC0
  #define  UART_DATA_REG        UDR0
  #define  UART_DOUBLE_SPEED      U2X0
#elif defined(UBRR0L) && defined(UCSR0A) && defined(TXEN0)
  /* ATMega with two USART, use UART0 */
  #define  UART_BAUD_RATE_LOW      UBRR0L
  #define  UART_STATUS_REG        UCSR0A
  #define  UART_CONTROL_REG      UCSR0B
  #define  UART_ENABLE_TRANSMITTER    TXEN0
  #define  UART_ENABLE_RECEIVER    RXEN0
  #define  UART_TRANSMIT_COMPLETE    TXC0
  #define  UART_RECEIVE_COMPLETE    RXC0
  #define  UART_DATA_REG        UDR0
  #define  UART_DOUBLE_SPEED      U2X0
#elif defined(UBRRL) && defined(UCSRA) && defined(UCSRB) && defined(TXEN) && defined(RXEN)
  //* catch all
  #define  UART_BAUD_RATE_LOW      UBRRL
  #define  UART_STATUS_REG        UCSRA
  #define  UART_CONTROL_REG      UCSRB
  #define  UART_ENABLE_TRANSMITTER    TXEN
  #define  UART_ENABLE_RECEIVER    RXEN
  #define  UART_TRANSMIT_COMPLETE    TXC
  #define  UART_RECEIVE_COMPLETE    RXC
  #define  UART_DATA_REG        UDR
  #define  UART_DOUBLE_SPEED      U2X
#else
  #error "no UART definition for MCU available"
#endif

/*
 * Macro to calculate UBBR from XTAL and baudrate
 */
#if defined(__AVR_ATmega32__) && UART_BAUDRATE_DOUBLE_SPEED
  #define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu / 4 / baudRate - 1) / 2)
#elif defined(__AVR_ATmega32__)
  #define UART_BAUD_SELECT(baudRate,xtalCpu) ((xtalCpu / 8 / baudRate - 1) / 2)
#elif UART_BAUDRATE_DOUBLE_SPEED
  #define UART_BAUD_SELECT(baudRate,xtalCpu) (((float)(xtalCpu))/(((float)(baudRate))*8.0)-1.0+0.5)
#else
  #define UART_BAUD_SELECT(baudRate,xtalCpu) (((float)(xtalCpu))/(((float)(baudRate))*16.0)-1.0+0.5)
#endif


/*
 * States used in the receive state machine
 */
#define  ST_START    0
#define  ST_GET_SEQ_NUM  1
#define ST_MSG_SIZE_1  2
#define ST_MSG_SIZE_2  3
#define ST_GET_TOKEN  4
#define ST_GET_DATA    5
#define  ST_GET_CHECK  6
#define  ST_PROCESS    7

/*
 * use 16bit address variable for ATmegas with <= 64K flash
 */
#if defined(RAMPZ)
  typedef uint32_t address_t;
#else
  typedef uint16_t address_t;
#endif

/*
 * function prototypes
 */
void sendchar(char c);
static unsigned char recchar(void);

/*
 * since this bootloader is not linked against the avr-gcc crt1 functions,
 * to reduce the code size, we need to provide our own initialization
 */
void __jumpMain  (void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
#include <avr/sfr_defs.h>

//#define  SPH_REG  0x3E
//#define  SPL_REG  0x3D

//*****************************************************************************
void __jumpMain(void)
{
//*  July 17, 2010  <MLS> Added stack pointer initialzation
//*  the first line did not do the job on the ATmega128

  asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );

//*  set stack pointer to top of RAM

  asm volatile ( "ldi  16, %0" :: "i" (RAMEND >> 8) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_HI_ADDR) );

  asm volatile ( "ldi  16, %0" :: "i" (RAMEND & 0x0ff) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_LO_ADDR) );

  asm volatile ( "clr __zero_reg__" );                  // GCC depends on register r1 set to 0
  asm volatile ( "out %0, __zero_reg__" :: "I" (_SFR_IO_ADDR(SREG)) );  // set SREG to 0
  asm volatile ( "jmp main");                        // jump to main()
}

//*****************************************************************************
/*
 * send single byte to USART, wait until transmission is completed
 */
void sendchar(char c)
{
  UART_DATA_REG  =  c;                    // prepare transmission
  while (!(UART_STATUS_REG & (1 << UART_TRANSMIT_COMPLETE)));  // wait until byte sent
  UART_STATUS_REG |= (1 << UART_TRANSMIT_COMPLETE);      // delete TXCflag
}


//************************************************************************
static int  Serial_Available(void)
{
  return(UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE));  // wait for data
}


//*****************************************************************************
/*
 * Read single byte from USART, block if no data available
 */
static unsigned char recchar(void)
{
  while (!(UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE)))
  {
    // wait for data
  }
  return UART_DATA_REG;
}

#define  MAX_TIME_COUNT  (F_CPU >> 1)
//*****************************************************************************
static unsigned char recchar_timeout(unsigned char* timedout)
{
uint32_t count = 0;

  while (!(UART_STATUS_REG & (1 << UART_RECEIVE_COMPLETE)))
  {
    // wait for data
    count++;
    if (count > MAX_TIME_COUNT)
    {
		*timedout = 1;
		return 0;	// NULL character
    }
  }
  return UART_DATA_REG;
}

//*  for watch dog timer startup
void (*app_start)(void) = 0x0000;

//*****************************************************************************
int main(void)
{
  address_t    address      =  0;
  unsigned char  msgParseState;
  unsigned int  ii        =  0;
  unsigned char  checksum    =  0;
  unsigned char  seqNum      =  0;
  unsigned int  msgLength    =  0;
  unsigned char  msgBuffer[285];
  unsigned char  c, *p;
  unsigned char   isLeave = 0;
  unsigned char   isTimeout = 0;
  unsigned char badMessage = 0;
  unsigned long  boot_timeout;
  unsigned long  boot_timer;
  unsigned int  boot_state;

  //*  some chips dont set the stack properly
  asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );
  asm volatile ( "ldi  16, %0" :: "i" (RAMEND >> 8) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_HI_ADDR) );
  asm volatile ( "ldi  16, %0" :: "i" (RAMEND & 0x0ff) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_LO_ADDR) );


 //************************************************************************
 /* Feb 11, 2014  Use WDT to enter OTA bootloader section
  * Do not clear WDT status flag, leave that for the main app
  */
 GPIOR0 = MCUSR;	// store status register in GPIOR0 to provide feedback on reset reason to main app
 
 // make sure watchdog is off!
 __asm__ __volatile__ ("cli");
 __asm__ __volatile__ ("wdr");
 MCUSR = 0;
 WDTCSR  |=  _BV(WDCE) | _BV(WDE);
 WDTCSR  =  0;
 __asm__ __volatile__ ("sei");
   
 boot_timeout  =   10000;    //*  short delay for serial
 boot_timer  =  0;
 boot_state  =  0;
  /*
   * Init UART
   * set baudrate and enable USART receiver and transmiter without interrupts
   */
#if UART_BAUDRATE_DOUBLE_SPEED
  UART_STATUS_REG    |=  (1 <<UART_DOUBLE_SPEED);
#endif
  UART_BAUD_RATE_LOW  =  UART_BAUD_SELECT(BAUDRATE,F_CPU);
  UART_CONTROL_REG  =  (1 << UART_ENABLE_RECEIVER) | (1 << UART_ENABLE_TRANSMITTER);

  asm volatile ("nop");      // wait until port has changed


	for(;;) {
		  boot_state=0;
		  boot_timer=0;
		  isLeave=0;
		  isTimeout=0;
	  while (boot_state==0)
	  {
		while ((!(Serial_Available())) && (boot_state == 0))    // wait for data
		{
		  _delay_ms(0.040);
		  boot_timer++;
		  if (boot_timer > boot_timeout)
		  {
			  isTimeout = 1;
			boot_state  =  1; // get us out, this is incremented to 2 below
		  }
		}
		boot_state++; // increment to 1 for serial bootloader, 2 for other purposes
	  }

	  if (boot_state==1) // enter serial bootloader
	  {
		//*  main loop
		while ((!isLeave) && (!isTimeout))
		{
		  /*
		   * Collect received bytes to a complete message
		   */
		  msgParseState  =  ST_START;
		  while ((msgParseState != ST_PROCESS) && (!isLeave) && (!isTimeout))
		  {
			if (boot_state==1)
			{
			  boot_state  =  0;
			  c      =  UART_DATA_REG;
			}
			else
			{
			//  c  =  recchar();
			  c  =  recchar_timeout(&isTimeout);
			}

			switch (msgParseState)
			{
			  case ST_START:
				if ( c == MESSAGE_START )
				{
				  msgParseState  =  ST_GET_SEQ_NUM;
				  checksum    =  MESSAGE_START^0;
				}
				else
				{
					if (badMessage++ > 5) { isTimeout = 1; }	// isLeave is now a forced boot to main app
				}
				break;

			  case ST_GET_SEQ_NUM:
			  #ifdef _FIX_ISSUE_505_
				seqNum      =  c;
				msgParseState  =  ST_MSG_SIZE_1;
				checksum    ^=  c;
			  #else
				if ( (c == 1) || (c == seqNum) )
				{
				  seqNum      =  c;
				  msgParseState  =  ST_MSG_SIZE_1;
				  checksum    ^=  c;
				}
				else
				{
				  msgParseState  =  ST_START;
				}
			  #endif
				break;

			  case ST_MSG_SIZE_1:
				msgLength    =  c<<8;
				msgParseState  =  ST_MSG_SIZE_2;
				checksum    ^=  c;
				break;

			  case ST_MSG_SIZE_2:
				msgLength    |=  c;
				msgParseState  =  ST_GET_TOKEN;
				checksum    ^=  c;
				break;

			  case ST_GET_TOKEN:
				if ( c == TOKEN )
				{
				  msgParseState  =  ST_GET_DATA;
				  checksum    ^=  c;
				  ii        =  0;
				}
				else
				{
				  msgParseState  =  ST_START;
				}
				break;

			  case ST_GET_DATA:
				msgBuffer[ii++]  =  c;
				checksum    ^=  c;
				if (ii == msgLength )
				{
				  msgParseState  =  ST_GET_CHECK;
				}
				break;

			  case ST_GET_CHECK:
				if ( c == checksum )
				{
				  msgParseState  =  ST_PROCESS;
				}
				else
				{
				  msgParseState  =  ST_START;
				}
				break;
			}  //  switch
		  }  //  while(msgParseState)

		  /*
		   * Now process the STK500 commands, see Atmel Appnote AVR068
		   */

		  switch (msgBuffer[0])
		  {
	  #ifndef REMOVE_CMD_SPI_MULTI
			case CMD_SPI_MULTI:
			  {
				unsigned char answerByte;
				unsigned char flag=0;

				if ( msgBuffer[4]== 0x30 )
				{
				  unsigned char signatureIndex  =  msgBuffer[6];

				  if ( signatureIndex == 0 )
				  {
					answerByte  =  (SIGNATURE_BYTES >> 16) & 0x000000FF;
				  }
				  else if ( signatureIndex == 1 )
				  {
					answerByte  =  (SIGNATURE_BYTES >> 8) & 0x000000FF;
				  }
				  else
				  {
					answerByte  =  SIGNATURE_BYTES & 0x000000FF;
				  }
				}
				else if ( msgBuffer[4] & 0x50 )
				{
				//*  Issue 544:   stk500v2 bootloader doesn't support reading fuses
				//*  I cant find the docs that say what these are supposed to be but this was figured out by trial and error
				//  answerByte  =  boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
				//  answerByte  =  boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
				//  answerByte  =  boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
				  if (msgBuffer[4] == 0x50)
				  {
					answerByte  =  boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
				  }
				  else if (msgBuffer[4] == 0x58)
				  {
					if (msgBuffer[5] == 0x00)
					{
					  answerByte = boot_lock_fuse_bits_get(GET_LOCK_BITS);
					}
					else
					{
					  answerByte=boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
					}
				  }
				  else
				  {
					answerByte  =  0;
				  }
				}
				else
				{
				  answerByte  =  0; // for all others command are not implemented, return dummy value for AVRDUDE happy <Worapoht>
				}
				if ( !flag )
				{
				  msgLength    =  7;
				  msgBuffer[1]  =  STATUS_CMD_OK;
				  msgBuffer[2]  =  0;
				  msgBuffer[3]  =  msgBuffer[4];
				  msgBuffer[4]  =  0;
				  msgBuffer[5]  =  answerByte;
				  msgBuffer[6]  =  STATUS_CMD_OK;
				}
			  }
			  break;
	  #endif
			case CMD_SIGN_ON:
			  msgLength    =  11;
			  msgBuffer[1]   =  STATUS_CMD_OK;
			  msgBuffer[2]   =  8;
			  msgBuffer[3]   =  'A';
			  msgBuffer[4]   =  'V';
			  msgBuffer[5]   =  'R';
			  msgBuffer[6]   =  'I';
			  msgBuffer[7]   =  'S';
			  msgBuffer[8]   =  'P';
			  msgBuffer[9]   =  '_';
			  msgBuffer[10]  =  '2';
			  break;

			case CMD_GET_PARAMETER:
			  {
				unsigned char value;

				switch(msgBuffer[1])
				{
				case PARAM_BUILD_NUMBER_LOW:
				  value  =  CONFIG_PARAM_BUILD_NUMBER_LOW;
				  break;
				case PARAM_BUILD_NUMBER_HIGH:
				  value  =  CONFIG_PARAM_BUILD_NUMBER_HIGH;
				  break;
				case PARAM_HW_VER:
				  value  =  CONFIG_PARAM_HW_VER;
				  break;
				case PARAM_SW_MAJOR:
				  value  =  CONFIG_PARAM_SW_MAJOR;
				  break;
				case PARAM_SW_MINOR:
				  value  =  CONFIG_PARAM_SW_MINOR;
				  break;
				default:
				  value  =  0;
				  break;
				}
				msgLength    =  3;
				msgBuffer[1]  =  STATUS_CMD_OK;
				msgBuffer[2]  =  value;
			  }
			  break;

			case CMD_LEAVE_PROGMODE_ISP:
			  isLeave  =  1;
			  //*  fall thru

			case CMD_ENTER_PROGMODE_ISP:
			  msgLength    =  2;
			  msgBuffer[1]  =  STATUS_CMD_OK;
			  break;

			case CMD_LOAD_ADDRESS:
	  #if defined(RAMPZ)
			  address  =  ( ((address_t)(msgBuffer[1])<<24)|((address_t)(msgBuffer[2])<<16)|((address_t)(msgBuffer[3])<<8)|(msgBuffer[4]) )<<1;
	  #else
			  address  =  ( ((msgBuffer[3])<<8)|(msgBuffer[4]) )<<1;    //convert word to byte address
	  #endif
			  msgLength    =  2;
			  msgBuffer[1]  =  STATUS_CMD_OK;
			  break;

			case CMD_PROGRAM_FLASH_ISP:
			  {
				unsigned int  size  =  ((msgBuffer[1])<<8) | msgBuffer[2];
				unsigned char  *p  =  msgBuffer+10;
				unsigned int  data;
				unsigned char  highByte, lowByte;
				address_t tempAddress;
			
				if ( msgBuffer[0] == CMD_PROGRAM_FLASH_ISP )
				{
				  // erase only main section (bootloader protection)
				  if ((!(size % 2)) && (!(address % SPM_PAGESIZE)) && (size <= SPM_PAGESIZE) && (address + size <= APP_END))
				  {
				  
					boot_page_erase(address);  // Perform page erase
					boot_spm_busy_wait();    // Wait until the memory is erased.	
								  
					/* Write FLASH */
					tempAddress = address;
					do {
					
						lowByte    =  *p++;
						highByte   =  *p++;
					
						data    =  (highByte << 8) | lowByte;
						boot_page_fill(tempAddress,data);

						tempAddress  =  tempAddress + 2;  // Select next word in memory
						size  -=  2;        // Reduce number of bytes to write by two
					} while (size);          // Loop until all bytes written

					boot_page_write(address);
					boot_spm_busy_wait();
					boot_rww_enable();        // Re-enable the RWW section
				
					msgLength    =  2;
					msgBuffer[1]  =  STATUS_CMD_OK;				
				  }
				  else
				  {
					msgLength    =  2;
					msgBuffer[1]  =  STATUS_CMD_FAILED;
				  }
				}
				else
				{
					//*  issue 543, this should work, It has not been tested.
					uint16_t ii = address >> 1;
					/* write EEPROM */
					while (size) {
						eeprom_write_byte((uint8_t*)ii, *p++);
						address+=2;            // Select next EEPROM byte
						ii++;
						size--;
					}
					msgLength    =  2;
					msgBuffer[1]  =  STATUS_CMD_OK;
				}
			  }
			  break;

			default:
			  msgLength    =  2;
			  msgBuffer[1]  =  STATUS_CMD_FAILED;
			  break;
		  }

		  /*
		   * Now send answer message back
		   */
		  sendchar(MESSAGE_START);
		  checksum  =  MESSAGE_START^0;

		  sendchar(seqNum);
		  checksum  ^=  seqNum;

		  c      =  ((msgLength>>8)&0xFF);
		  sendchar(c);
		  checksum  ^=  c;

		  c      =  msgLength&0x00FF;
		  sendchar(c);
		  checksum ^= c;

		  sendchar(TOKEN);
		  checksum ^= TOKEN;

		  p  =  msgBuffer;
		  while ( msgLength )
		  {
			c  =  *p++;
			sendchar(c);
			checksum ^=c;
			msgLength--;
		  }
		  sendchar(checksum);
		  seqNum++;

		}

	  } 

		boot_rww_enable();        // enable application section so we can read from it
		
		unsigned int  data;
		#if (FLASHEND > 0x10000)
		  data  =  pgm_read_word_far(0);  //*  get the first word of the user program
		#else
		  data  =  pgm_read_word_near(0);  //*  get the first word of the user program
		#endif

		  if ((isLeave) || (isTimeout && (data != 0xffff)))          //*  require valid flash address on timeout
		  {
				  asm volatile ("nop");      // wait until port has changed

				  /*
				   * Now leave bootloader
				   */

				  UART_STATUS_REG  &=  0xfd;

			asm("jmp 0000");
		}
	}
}
