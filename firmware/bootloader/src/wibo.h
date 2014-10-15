/*
 * wibo.h
 *
 *  Created on: 12.10.2013
 *      Author: dthiele
 */

#ifndef WIBO_H_
#define WIBO_H_

/*
 * \brief Program a page
 * (1) Erase the page containing at the given address
 * (2) Fill the page buffer
 * (3) Write the page buffer
 * (4) Wait for finish
 *
 * @param addr The address containing the page to program
 * @param *buf Pointer to buffer of page content
 */
static inline void boot_program_page(uint32_t addr, uint8_t *buf)
{
#if SPM_PAGESIZE > 255
	uint16_t i;
#else
	uint8_t i;
#endif

#if defined(SERIALDEBUG)
	printf("Write Flash addr=%04lX"EOL, addr);
	/*
	 for (i = 0; i < SPM_PAGESIZE; i++)
	 {
	 if (!(i % 16))
	 {
	 putchar('\n');
	 putchar(' ');
	 putchar(' ');
	 }
	 printf(" %02X", buf[i]);
	 }
	 putchar('\n');
	 */
#else /* defined(SERIALDEBUG) */
	boot_page_erase(addr);
	boot_spm_busy_wait();

	i = SPM_PAGESIZE;
	do
	{
		/* Set up little-endian word */
		uint16_t w = *buf++;
		w += (*buf++) << 8;

		boot_page_fill(addr + SPM_PAGESIZE - i, w);

		i -= 2;
	} while (i);

	boot_page_write(addr);
	boot_spm_busy_wait();
#endif /* defined(SERIALDEBUG) */
}

void wibo_init(uint8_t channel, uint16_t pan_id, uint16_t short_addr, uint64_t ieee_addr);
uint8_t wibo_available(void);
uint8_t wibo_run(void);

#endif /* WIBO_H_ */
