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

#ifndef LM73_H
#define LM73_H

/* === includes ============================================================ */

/* === macros ============================================================== */
#define LM73_ADDR_0 (0x48) /**< LM3-0, ADDR pin is floating */
#define LM73_ADDR_1 (0x49) /**< LM3-0, ADDR pin is pulled to GND */
#define LM73_ADDR_2 (0x4A) /**< LM3-0, ADDR pin is pulled to VDD */

#define LM73_ADDR_3 (0x4C) /**< LM3-1, ADDR pin is floating */
#define LM73_ADDR_4 (0x4D) /**< LM3-1, ADDR pin is pulled to GND */
#define LM73_ADDR_5 (0x4E) /**< LM3-1, ADDR pin is pulled to VDD */

#define LM73_PTR_TEMP   (0x00)
#define LM73_PTR_CFG    (0x01)
#define LM73_PTR_THIGH  (0x02)
#define LM73_PTR_TLOW   (0x03)
#define LM73_PTR_STATUS (0x04)
#define LM73_PTR_ID     (0x07)

#define LM73_CFG_ONE_SHOT  (0x04)
#define LM73_CFG_ALERT_RST (0x08)
#define LM73_CFG_ALERT_POL (0x10)
#define LM73_CFG_ALERT_EN  (0x20)
#define LM73_CFG_PWR_DOWN  (0x80)

#define LM73_STATUS_DAV   (0x01)
#define LM73_STATUS_TLOW  (0x02)
#define LM73_STATUS_THIGH (0x04)
#define LM73_STATUS_ALERT (0x08)
#define LM73_STATUS_RES   (0x60)


/**
 * This value is the time between a stop-start cycle of the LM73.
 * It is speced as tBuf = 1.2us min, but on mesh bean it was found
 * that 8us ensure stable behaviour.
 */
#define LM73_T_BUS_FREE_US (8)

#define LM73_REG_TO_TEMP_SCALE (0.0078125) // LM73_TEMP_SCALE = 1/128
#define LM73_TEMP_TO_REG_SCALE (128)
/* === types =============================================================== */
typedef struct
{
    uint8_t addr;
} lm73_ctx_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

static inline uint8_t lm73_byte_read(lm73_ctx_t *pctx, uint8_t addr)
{
    uint8_t buf[1] = {addr,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 1);
    return buf[0];
}

static inline void lm73_byte_write(lm73_ctx_t *pctx, uint8_t addr, uint8_t val)
{
    uint8_t buf[2] = {addr, val};
    i2c_master_writeread(pctx->addr, buf, 2, NULL, 0);
}

static inline uint16_t lm73_word_read(lm73_ctx_t *pctx, uint8_t addr)
{
    uint8_t buf[2] = {addr,};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 2);
    return (buf[0]<<8) | buf[1];
}

static inline void lm73_word_write(lm73_ctx_t *pctx, uint8_t addr, uint16_t val)
{
    uint8_t buf[3] = {addr, ((val>>8) &0xff), (val & 0xff)};
    i2c_master_writeread(pctx->addr, buf, 3, NULL, 0);
}

static inline uint8_t lm73_init(lm73_ctx_t *pctx, uint8_t addr)
{
    uint8_t rv;
    rv = i2c_probe(addr);
    if (rv)
    {
        pctx->addr = addr;
    }
    return rv;
}

static inline int16_t lm73_get(lm73_ctx_t *pctx)
{
    int16_t rv;
    rv = lm73_word_read(pctx, LM73_PTR_TEMP);
    return rv;
}

static inline void lm73_set_upper_limit(lm73_ctx_t *pctx, int16_t val)
{
    lm73_word_write(pctx, LM73_PTR_THIGH, val);
}

static inline void lm73_set_lower_limit(lm73_ctx_t *pctx, int16_t val)
{
    lm73_word_write(pctx, LM73_PTR_TLOW, val);
}

static inline uint16_t lm73_get_id(lm73_ctx_t *pctx)
{
    uint8_t buf[2] = {7};
    i2c_master_writeread(pctx->addr, buf, 1, buf, 2);
    return buf[0] | buf[1]*256;
}

static inline float lm73_scale(lm73_ctx_t *pctx, int16_t val)
{
    return  LM73_REG_TO_TEMP_SCALE * (val);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef LM73_H */
