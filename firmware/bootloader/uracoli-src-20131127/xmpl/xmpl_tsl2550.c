/* Copyright (c) 2009 Axel Wachtler
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
/* Example for using the I2C ISL29020 light sensor */
#include <stdlib.h>
#include "board.h"
#include "ioutil.h"
#include "i2c.h"
#include "sensors/tsl2550.h"
#include "hif.h"
#include "xmpl.h"


/* === includes ============================================================ */

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
bool do_measure;

/* === prototypes ========================================================== */
bool process_command(int chr);

/* === functions =========================================================== */

void xmpl_init(void)
{
    /* setting up UART and adjusting the baudrate */
    hif_init(HIF_DEFAULT_BAUDRATE);
    LED_INIT();
    LED_SET(0);
    sei();
#if HIF_TYPE == HIF_AT90USB
    /*
     * Wait for terminal user pressing a key so there is time to
     * attach a terminal emulator after the virtual serial port has
     * been established within the host OS.
     */
    do
    {
        inchar = hif_getc();
    }
    while (EOF == inchar);
#endif

    /* init i2c bus and check presence of sensor */
    i2c_init( 4000000UL );

}

int main(void)
{
uint8_t rv, adc0, adc1;
uint16_t lv_f;

int chr;

    xmpl_init();

    PRINTF("\n\rTSL2550 Light Sensor Example : %s : %ld bit/s\n\r",
            BOARD_NAME, HIF_DEFAULT_BAUDRATE);

    rv = tsl2550_init();
    if (rv == 0)
    {
        PRINTF("ERROR: init, addr=0x%x\n", TSL2550_ADDR);
    }
    else
    {
        PRINTF("OK: init, addr=0x%x\n", TSL2550_ADDR);
    }


    do_measure = 1;

    /* wakeup sensor */
    tsl2550_set_command(TSL2550_RD_CMD);


    while (1)
    {
        chr = hif_getc();
        if (chr != -1)
        {
            do_measure = process_command(chr);
        }
        if (do_measure)
        {
            adc0 = tsl2550_get(0);
             /* a time gap between two reads is needed, todo: check datasheet */
            DELAY_US(10);
            adc1 = tsl2550_get(1);

            if ( TSL2550_ADC_VALID(adc0) && TSL2550_ADC_VALID(adc1))
            {
                lv_f = tsl2550_scale( adc0 , adc1);
                PRINTF("adc0: 0x%02x adc1: 0x%02x E[lux]: %d\n", adc0, adc1, lv_f);
            }
            else
            {
                PRINTF("err adc0: 0x%02x adc1: 0x%02x\n", adc0, adc1);
            }
            /* conversion cycle of TSL2550 is 800ms for both adc's. */
            WAIT_MS(1000);
            LED_TOGGLE(0);
        }
    }
}

bool process_command(int chr)
{
    int nb;
    bool rv = 0;

    if (chr == 'p')
    {
        PRINT("power down\n");
        tsl2550_set_command(TSL2550_PWR_DOWN);
    }
    else if (chr == 'P')
    {
        PRINT("power up\n");
        tsl2550_set_command(TSL2550_RD_CMD);
    }
    else if (chr == 'e')
    {
        PRINT("use ext. range\n");
        tsl2550_set_command(TSL2550_EXT_RANGE);
    }
    else if (chr == 's')
    {
        PRINT("use std. range\n");
        tsl2550_set_command(TSL2550_STD_RANGE);
    }
    else if (chr == 'm' || chr == ' ')
    {
        PRINT("run measurement\n");
        rv = 1;
    }
    else if (chr == 'h')
    {
        PRINT("Help\n"\
              "p - power down\n"\
              "P - power up\n"\
              "e - use ext. range\n"\
              "s - use std. range\n"\
              "m - run measurement\n"
             );
    }

    return rv;
}

/* E_O_F */
