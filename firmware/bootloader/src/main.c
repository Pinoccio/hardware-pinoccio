#include  <avr/io.h>
#include  <avr/boot.h>

#define BOOTSIZE 4096	// bootsize in words

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

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
