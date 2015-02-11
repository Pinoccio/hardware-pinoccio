#include  <avr/io.h>
#include  <avr/boot.h>

#define BOOTSIZE 4096	// bootsize in words

#define APP_END  (FLASHEND -(2*BOOTSIZE) + 1)

//*****************************************************************************/
int main(void)
{
  // Setup some pins as outputs. PD6/PG2/PE2 has active low leds, PORTF is for
  // logic analyzer outputs
  DDRF |= (1 << PF0) | (1 << PF1) | (1 << PF2) | (1 << PF3);
  DDRD |= (1 << PD6); // Red
  DDRG |= (1 << PG2); // Green
  DDRE |= (1 << PE2); // Yellow

  if (!MCUSR) {
    // If we get here, we "reset" without a proper hardware reset. This
    // should never happen.
    PORTD &= ~(1 << PD6); // Red
    PORTF |= (1 << PF1);
    while(1);
  } else {
    // Normal startup: Turn off leds (active low), provide trigger to
    // analyzer
    PORTD |= (1 << PD6);
    PORTG |= (1 << PG2);
    PORTE |= (1 << PE2);
    PORTF |= (1 << PF0);
  }
  MCUSR = 0;

  // Write fixed data to the second half of flash
  uint32_t    address      = (FLASHEND + 1) / 2;
  for(;;) {
    // Erase
    PORTG &= ~(1 << PG2); // Green
    PORTF &= ~(1 << PF2);
    boot_page_erase(address);  // Perform page erase
    boot_spm_busy_wait();    // Wait until the memory is erased.	
    PORTF |= (1 << PF2);
    PORTG |= (1 << PG2);

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

    PORTF &= ~(1 << PF3);
    PORTE &= ~(1 << PE2); // Yellow
    boot_page_write(address);
    boot_spm_busy_wait();
    PORTF |= (1 << PF3);
    PORTE |= (1 << PE2);

    // Wrap around from APP_END to halfway
    // FLASHEND
    address += SPM_PAGESIZE;
    if (address + SPM_PAGESIZE > APP_END + 1)
      address = (FLASHEND + 1) / 2;
  }
}
