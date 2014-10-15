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
#include "sensors/isl29020.h"
#include "hif.h"
#include "xmpl.h"


/* === includes ============================================================ */

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
isl29020_ctx_t light1;
bool do_measure;
/* === prototypes ========================================================== */
bool process_command(char chr);

/* === functions =========================================================== */
int main(void)
{
const uint32_t br = HIF_DEFAULT_BAUDRATE;

 uint8_t cmd;
 uint8_t rv;
 uint16_t lv;
 float lv_f;

 int chr;

    /* setting up UART and adjusting the baudrate */
    hif_init(br);
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
    PRINTF("\n\rISL29020 Light Sensor Example : %s : %ld bit/s\n\r", BOARD_NAME, br);

    /* init i2c bus and check presence of sensor */
    i2c_init( 4000000UL );
    rv = isl29020_init(&light1, ISL29020_ADDR);
    if (rv == 0)
    {
        PRINTF("ERROR: init, addr=0x%x\n", ISL29020_ADDR);
    }
    else
    {
        PRINTF("OK: init, addr=0x%x\n", ISL29020_ADDR);
    }

    /* 1.st configure sensor */
    cmd = 0;
    ISL29020_SET_ENABLE(cmd);
    ISL29020_SET_MODE_CONT(cmd);
    ISL29020_SET_LIGHT(cmd);
    isl29020_set_command(&light1, cmd);
    do_measure = 1;

    while (1)
    {
        chr = hif_getc();
        if (chr != -1)
        {
            do_measure = process_command(chr);
        }
        if (do_measure)
        {
            lv = isl29020_get(&light1);
            lv_f = isl29020_scale(&light1, lv);
            PRINTF("data: %-8u E[lux]: %f\n", lv, lv_f);
            WAIT_MS(1000);
            LED_TOGGLE(0);
        }
    }
}

bool process_command(char chr)
{
    int nb;
    uint8_t cmd;
    bool rv = 0;

    cmd = isl29020_get_command(&light1);
    if (chr == 'R')
    {
        PRINT("Enter range [0-3]:");
        nb = hif_get_dec_number();
        PRINTF(" %d\n", nb);

        ISL29020_SET_RANGE(cmd, nb);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'r')
    {
        PRINT("Enter resolution [0-3]:");
        nb = hif_get_dec_number();
        PRINTF(" %d\n", nb);

        ISL29020_SET_RESOLUTION(cmd, nb);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'l')
    {
        PRINT("detect light\n");

        ISL29020_SET_LIGHT(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'i')
    {
        PRINT("detect infrared\n");
        ISL29020_SET_IR(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 's')
    {
        PRINT("single measurement\n");
        ISL29020_SET_MODE_SINGLE(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'c')
    {
        PRINT("continous measurement\n");
        ISL29020_SET_MODE_CONT(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'p')
    {
        PRINT("power down\n");
        ISL29020_SET_DISABLE(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'P')
    {
        PRINT("power up\n");
        ISL29020_SET_ENABLE(cmd);
        isl29020_set_command(&light1, cmd);
    }
    else if (chr == 'm' || chr == ' ')
    {
        ISL29020_SET_ENABLE(cmd);
        isl29020_set_command(&light1, cmd);
        PRINT("run measurement\n");
        rv = 1;
    }
    else if (chr == 'h')
    {
        PRINT("Help\n"\
              "R - set range 0 - 3 \n"\
              "r - set resolution 0 - 3\n"\
              "l - detect light\n"\
              "i - detect infrared\n"\
              "s - single measurement\n"\
              "c - continous measurement\n"\
              "p - power down\n"\
              "P - power up\n"\
              "m - run measurement\n"
             );
    }

    PRINTF("command: 0x%02x, "\
           "enable: %d, "\
           "mode cont: %d, "\
           "detect IR: %d, "\
           "range: %d, "\
           "resolution: %d\n",
            cmd,
            ISL29020_GET_ENABLE(cmd),
            ISL29020_GET_MODE_CONT(cmd),
            ISL29020_GET_IR(cmd),
            ISL29020_GET_RANGE(cmd),
            ISL29020_GET_RES(cmd));

    return rv;
}

/* E_O_F */
