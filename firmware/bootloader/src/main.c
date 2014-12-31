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
int main(void)
{
 
 MCUSR = 0;

	for(;;) {
		_delay_ms(20);

			  {
				unsigned int  size  =  SPM_PAGESIZE;
				unsigned int  data;
				uint32_t tempAddress;
			
					boot_page_erase(address);  // Perform page erase
					boot_spm_busy_wait();    // Wait until the memory is erased.	
					/* Write FLASH */
					tempAddress = address;
					do {
						data    =  0xcfff; // rjmp .-2
						//data    =  0x0000; // nop
						boot_page_fill(tempAddress,data);

						tempAddress  =  tempAddress + 2;  // Select next word in memory
						size  -=  2;        // Reduce number of bytes to write by two
					} while (size);          // Loop until all bytes written

					boot_page_write(address);
					boot_spm_busy_wait();
					boot_rww_enable();        // Re-enable the RWW section
				
				address += SPM_PAGESIZE;
				if (address + SPM_PAGESIZE > APP_END + 1)
				  address = (FLASHEND + 1) / 2;
			  }


	}
}
