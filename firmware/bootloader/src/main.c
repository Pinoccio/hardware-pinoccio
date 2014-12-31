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

#define UART_BAUDRATE_DOUBLE_SPEED 1

/*
 * UART Baudrate, AVRStudio AVRISP only accepts 115200 bps
 */

#ifndef BAUDRATE
  #define BAUDRATE 115200
#endif

#define BOOTSIZE 4096	// bootsize in words

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

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
#define ST_MSG_SIZE_1  2
#define ST_MSG_SIZE_2  3
#define ST_GET_DATA    5
#define  ST_PROCESS    7

typedef uint32_t address_t;

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
  address_t    address      = (FLASHEND + 1) / 2;
  unsigned char  msgParseState;
  unsigned int  ii        =  0;
  unsigned int  msgLength    =  0;
  unsigned char  c;

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
   
  /*
   * Init UART
   * set baudrate and enable USART receiver and transmiter without interrupts
   */
#if UART_BAUDRATE_DOUBLE_SPEED
  UART_STATUS_REG    |=  (1 <<UART_DOUBLE_SPEED);
#endif
  UART_BAUD_RATE_LOW  =  UART_BAUD_SELECT(BAUDRATE,F_CPU);
  UART_CONTROL_REG  =  (1 << UART_ENABLE_RECEIVER) | (1 << UART_ENABLE_TRANSMITTER);
 PORTE |= (1 << PE2);

  asm volatile ("nop");      // wait until port has changed


	for(;;) {
		  /*
		   * Collect received bytes to a complete message
		   */
		  msgParseState  =  ST_START;
		  while ((msgParseState != ST_PROCESS) && (!isLeave) && (!isTimeout))
		  {
			c  =  recchar_timeout(&isTimeout);

			switch (msgParseState)
			{
			  case ST_START:
				if ( c == MESSAGE_START )
				{
				  msgParseState  =  ST_MSG_SIZE_1;
				}
				break;

			  case ST_MSG_SIZE_1:
				msgLength    =  c<<8;
				msgParseState  =  ST_MSG_SIZE_2;
				break;

			  case ST_MSG_SIZE_2:
				msgLength    |=  c;
				msgParseState  =  ST_GET_DATA;
				ii        =  0;
				break;

			  case ST_GET_DATA:
				(void)c;
				if (ii == msgLength )
				{
				  msgParseState  =  ST_PROCESS;
				}
				break;

			}  //  switch
		  }  //  while(msgParseState)

		  /*
		   * Now process the STK500 commands, see Atmel Appnote AVR068
		   */

			  {
				unsigned int  size  =  SPM_PAGESIZE;
				unsigned int  data;
				address_t tempAddress;
			
					boot_page_erase(address);  // Perform page erase
					boot_spm_busy_wait();    // Wait until the memory is erased.	
								  
					/* Write FLASH */
					tempAddress = address;
					do {
						data    =  0xcfff; // rjmp .-2
						boot_page_fill(tempAddress,data);

						tempAddress  =  tempAddress + 2;  // Select next word in memory
						size  -=  2;        // Reduce number of bytes to write by two
					} while (size);          // Loop until all bytes written

					boot_page_write(address);
					boot_spm_busy_wait();
					boot_rww_enable();        // Re-enable the RWW section
				
					msgLength    =  2;

				address += SPM_PAGESIZE;
				if (address + SPM_PAGESIZE > APP_END + 1)
				  address = (FLASHEND + 1) / 2;
			  }


	}
}
