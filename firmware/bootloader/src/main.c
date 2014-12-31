#include  <inttypes.h>
#include  <avr/io.h>
#include  <avr/interrupt.h>
#include  <avr/boot.h>
#include  <avr/pgmspace.h>
#include  <util/delay.h>
#include  <avr/eeprom.h>
#include  <avr/common.h>
#include  <stdlib.h>

#define BOOTSIZE 4096	// bootsize in words

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

/*
 * since this bootloader is not linked against the avr-gcc crt1 functions,
 * to reduce the code size, we need to provide our own initialization
 */
void __jumpMain  (void) __attribute__ ((naked)) __attribute__ ((section (".init9")));
#include <avr/sfr_defs.h>

//*****************************************************************************/
void __jumpMain(void)
{
//  July 17, 2010  <MLS> Added stack pointer initialzation
//  the first line did not do the job on the ATmega128

  asm volatile ( ".set __stack, %0" :: "i" (RAMEND) );

//  set stack pointer to top of RAM

  asm volatile ( "ldi  16, %0" :: "i" (RAMEND >> 8) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_HI_ADDR) );

  asm volatile ( "ldi  16, %0" :: "i" (RAMEND & 0x0ff) );
  asm volatile ( "out %0,16" :: "i" (AVR_STACK_POINTER_LO_ADDR) );

  asm volatile ( "clr __zero_reg__" );                  // GCC depends on register r1 set to 0
  asm volatile ( "out %0, __zero_reg__" :: "I" (_SFR_IO_ADDR(SREG)) );  // set SREG to 0
  asm volatile ( "jmp main");                        // jump to main()
  asm volatile ( "jmp 0x3e000");                        // jump to main()
}

//*****************************************************************************/
int main(void)
{
  // Setup some pins as outputs. PORTB has active low leds, PORTE is for
  // logic analyzer outputs
  DDRB |= (1 << PB4) | (1 << PB5) | (1 << PB6);
  DDRE |= (1 << PE2) | (1 << PE3) | (1 << PE4) | (1 << PE5) | (1 << PE6);

  if (!MCUSR) {
    // If we get here, we "reset" without a proper hardware reset. This
    // should never happen.
    PORTB &= ~(1 << PB6);
    PORTE |= (1 << PE3);
    while(1);
  } else {
    // Normal startup: Turn off leds (active low), provide trigger to
    // analyzer
    PORTB |= (1 << PB4) | (1 << PB5) | (1 << PB6);
    PORTE |= (1 << PE2);
  }
  MCUSR = 0;

  // Write fixed data to the second half of flash
  uint32_t    address      = (FLASHEND + 1) / 2;
  for(;;) {
    // Erase
    PORTB &= ~(1 << PB4);
    PORTE &= ~(1 << PE4);
    boot_page_erase(address);  // Perform page erase
    boot_spm_busy_wait();    // Wait until the memory is erased.	
    PORTE |= (1 << PE4);
    PORTB |= (1 << PB4);

    // Write
    uint32_t tempAddress = address;
    unsigned int  size  =  SPM_PAGESIZE;
    do {
      //uint8_t data    =  0xcfff; // rjmp .-2
      uint8_t data    =  0x0000; // nop
      boot_page_fill(tempAddress,data);

      tempAddress  =  tempAddress + 2;  // Select next word in memory
      size  -=  2;        // Reduce number of bytes to write by two
    } while (size);          // Loop until all bytes written

    PORTE &= ~(1 << PE5);
    PORTB &= ~(1 << PB5);
    boot_page_write(address);
    boot_spm_busy_wait();
    PORTE |= (1 << PE5);
    PORTB |= (1 << PB5);

    // Wrap around from APP_END to halfway
    // FLASHEND
    address += SPM_PAGESIZE;
    if (address + SPM_PAGESIZE > APP_END + 1)
      address = (FLASHEND + 1) / 2;
  }
}
