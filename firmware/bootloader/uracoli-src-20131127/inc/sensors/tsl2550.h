/* Copyright (c) 2013 Axel Wachtler
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

#ifndef TSL2550_H
#define TSL2550_H

/* === includes ============================================================ */
#include <avr/pgmspace.h>

/* === macros ============================================================== */
/** the I2C address of the TSL2550 */
#define  TSL2550_ADDR (0x39)
/** Power-down state */
#define TSL2550_PWR_DOWN (0x00)
/** Power-up state/Read command register */
#define TSL2550_RD_CMD (0x03)
/** Write command to assert extended range mode */
#define TSL2550_EXT_RANGE (0x1D)
/** Write command to reset or return to standard range mode */
#define TSL2550_STD_RANGE (0x18)
/** Read ADC channel 0 */
#define TSL2550_RD_ADC0 (0x43)
/** Read ADC channel 1 */
#define TSL2550_RD_ADC1 (0x83)

#define ADC_VALID_MASK (0x80)

#define TSL2550_ADC_VALID(a) (a & ADC_VALID_MASK)
#define TSL2550_MAX_LUX (1846)

/* === types =============================================================== */


/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
static inline uint8_t tsl2550_init(void)
{
    uint8_t rv;
    rv = i2c_probe(TSL2550_ADDR);
    return rv;
}

static inline void tsl2550_set_command(uint8_t cmd)
{
    i2c_master_writeread(TSL2550_ADDR, &cmd, 1, NULL, 0);
}

/**
 * return raw adc reading (valid flag, log compressed value)
 */
static inline uint8_t tsl2550_get(uint8_t adc)
{
    uint8_t rv;
    uint8_t cmd;
    cmd = (adc == 0) ? TSL2550_RD_ADC0 : TSL2550_RD_ADC1;

    i2c_master_writeread(TSL2550_ADDR, &cmd, 1, &rv, 1);
    return  rv;
}


/** derived from app note dn9b_tsl2550_lux_calculation.pdf */
static inline uint16_t tsl2550_scale(uint8_t adc0, uint8_t adc1)
{

    // Lookup table for channel ratio (i.e. channel1 / channel0)
    static const unsigned char ratioLut[129] = {
        100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,99,99,
        99,99,99,99,99,99,99,99,
        99,99,99,98,98,98,98,98,
        98,98,97,97,97,97,97,96,
        96,96,96,95,95,95,94,94,
        93,93,93,92,92,91,91,90,
        89,89,88,87,87,86,85,84,
        83,82,81,80,79,78,77,75,
        74,73,71,69,68,66,64,62,
        60,58,56,54,52,49,47,44,
        42,41,40,40,39,39,38,38,
        37,37,37,36,36,36,35,35,
        35,35,34,34,34,34,33,33,
        33,33,32,32,32,32,32,31,
        31,31,31,31,30,30,30,30,
        30
        };
    // Lookup table to convert channel values to counts
    static const unsigned short countLut[128] = {
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15,
        16, 18, 20, 22, 24, 26, 28, 30,
        32, 34, 36, 38, 40, 42, 44, 46,
        49, 53, 57, 61, 65, 69, 73, 77,
        81, 85, 89, 93, 97, 101, 105, 109,
        115, 123, 131, 139, 147, 155, 163, 171,
        179, 187, 195, 203, 211, 219, 227, 235,
        247, 263, 279, 295, 311, 327, 343, 359,
        375, 391, 407, 423, 439, 455, 471, 487,
        511, 543, 575, 607, 639, 671, 703, 735,
        767, 799, 831, 863, 895, 927, 959, 991,
        1039,1103,1167,1231,1295,1359,1423,1487,
        1551,1615,1679,1743,1807,1871,1935,1999,
        2095,2223,2351,2479,2607,2735,2863,2991,
        3119,3247,3375,3503,3631,3759,3887,4015
        };

    /* lookup count from channel value */
    uint16_t count0 = countLut[adc0 & ~ADC_VALID_MASK];
    uint16_t count1 = countLut[adc1 & ~ADC_VALID_MASK];


    /* calculate ratio
     * Note: the "128" is a scaling factor
     */
    uint8_t ratio = 128;

    /* avoid division by zero and count1 cannot be greater than count0 */
    if ((count0) && (count1 <= count0))
    {
        ratio = (count1 * 128 / count0);
    }
    /* calculate lux
     * Note: the "256" is a scaling factor
     */
    unsigned long lux = ((count0 - count1) * ratioLut[ratio]) / 256;
    /* range check lux */
    if (lux > TSL2550_MAX_LUX)
    {
        lux = TSL2550_MAX_LUX;
    }
    return lux;
}
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef TSL2550_H */
