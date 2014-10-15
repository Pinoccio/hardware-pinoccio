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

#ifndef ISL29020_H
#define ISL29020_H
/**
 * @file
 * Intersil ISL 29020 Light Sensor
 *
 * A Low Power, High Sensitivity, Light-to Digital Sensor With I2C Interface.
 *
 * Datasheet: http://www.intersil.com/content/dam/Intersil/documents/fn65/fn6505.pdf
 *
 */
/* === includes ============================================================ */

/* === macros ============================================================== */
/**
 * 1st. I2C address option, see macro ISL29020_ADDR in board*.h
 * for actual configuration.
 */
#define ISL29020_ADDR_0 (0x44)
/** 2nd. I2C address option */
#define ISL29020_ADDR_1 (0x45)

#define _ISL29020_EN    (0x80)
#define _ISL29020_MODE  (0x40)
#define _ISL29020_LIGHT (0x20)
#define _ISL29020_RES   (0x1c)
#define _ISL29020_RANGE (0x03)

#define ISL29020_SET_ENABLE(c)       do{ (c) |= _ISL29020_EN;}while(0)
#define ISL29020_SET_DISABLE(c)      do{ (c) &= ~_ISL29020_EN;}while(0)

#define ISL29020_SET_MODE_SINGLE(c)  do{ (c) &= ~_ISL29020_MODE;}while(0)
#define ISL29020_SET_MODE_CONT(c)    do{ (c) |=  _ISL29020_MODE;}while(0)

#define ISL29020_SET_IR(c)           do{ (c) |= _ISL29020_LIGHT;}while(0)
#define ISL29020_SET_LIGHT(c)        do{ (c) &= ~_ISL29020_LIGHT;}while(0)

#define ISL29020_SET_RESOLUTION(c,r) do{ (c) &=  ~_ISL29020_RES;\
                                         (c) |= (r<<2)&_ISL29020_RES; }while(0)
#define ISL29020_SET_RANGE(c,r)      do{ (c) &=  ~_ISL29020_RANGE;\
                                         (c) |= (r)&_ISL29020_RANGE; }while(0)


#define ISL29020_GET_ENABLE(c)      (((c) & _ISL29020_EN)>>7)
#define ISL29020_GET_MODE_CONT(c)   (((c) & _ISL29020_MODE)>>6)
#define ISL29020_GET_IR(c)          (((c) & _ISL29020_LIGHT)>>5)
#define ISL29020_GET_RES(c)         (((c) & _ISL29020_RES)>>2)
#define ISL29020_GET_RANGE(c)       (((c) & _ISL29020_RANGE))


/* === types =============================================================== */
typedef struct {
    uint8_t addr;
    uint8_t cmd;
} isl29020_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
static inline uint8_t isl29020_init(isl29020_ctx_t *pctx, uint8_t addr)
{
    uint8_t rv;

    rv = i2c_probe(addr);
    if (rv)
    {
        pctx->addr = addr;
        pctx->cmd = 0;

    }
    return rv;
}

static inline void isl29020_set_command(isl29020_ctx_t *pctx, uint8_t cmd)
{
    uint8_t buf[3] = {0,};
    buf[1] = cmd;
    pctx->cmd = cmd;
    i2c_master_writeread(pctx->addr, buf, 2, NULL, 0);
    i2c_master_writeread(pctx->addr, buf, 1, buf, 3);
}

static inline uint8_t isl29020_get_command(isl29020_ctx_t *pctx)
{
    uint8_t buf[3] = {0,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 1);
    pctx->cmd = buf[0];
    return buf[0] ;
}

static inline uint16_t isl29020_get(isl29020_ctx_t *pctx)
{
    uint8_t buf[3] = {1,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 2);
    return buf[0] | buf[1] * 256;
}

static inline float isl29020_scale(isl29020_ctx_t *pctx, uint16_t val)
{
    float rv = 0.0;
    uint8_t res, range;
    int32_t cnt_max;
    res = ISL29020_GET_RES(pctx->cmd);
    range = ISL29020_GET_RANGE(pctx->cmd);
    cnt_max = (1L << (16 - (res * 4))) - 1;
    switch (range)
    {
        case 0:
            rv = (float)(1000.0 / cnt_max) * (float)val;
            break;
        case 1:
            rv = (float)(4000.0 / cnt_max) * (float)val;
            break;
        case 2:
            rv = (float)(16000.0 / cnt_max) * (float)val;
            break;
        case 3:
            rv = (float)(64000.0 / cnt_max) * (float)val;
            break;
        default:
            rv = -1.0;
    }

    return rv;

}


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef ISL29020_H */
