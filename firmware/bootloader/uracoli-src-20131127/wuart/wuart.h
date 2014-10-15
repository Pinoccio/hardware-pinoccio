/* Copyright (c) 2012 Axel Wachtler
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

#ifndef WUART_H
#define WUART_H

/* === includes ============================================================ */
#include "p2p_protocol.h"

/* === macros ============================================================== */
/** Peer address heuristic */
#define CALC_PEER_ADDRESS(x) (((x) == 0xffff) ? (x) : (x) ^ 1)

/** Max. number of payload bytes per frame */
//#define PAYLD_SIZE        (PROT_WUART_PAYLD_SIZE)

/** Number of bytes for CRC16 */
#define CRC_SIZE          (sizeof(crc_t))

/** END of line delimitter */
#define EOL "\n\r"

#ifndef UART_FRAME_SIZE
# define UART_FRAME_SIZE (MAX_FRAME_SIZE)
#endif

#ifndef DEFAULT_RADIO_CHANNEL
/** radio channel */
# if defined(TRX_SUPPORTS_BAND_800)
#  define DEFAULT_RADIO_CHANNEL    (0)
# elif defined(TRX_SUPPORTS_BAND_900) && defined(REGION_USA)
#  define DEFAULT_RADIO_CHANNEL    (5)
# elif defined(TRX_SUPPORTS_BAND_2400)
#  define DEFAULT_RADIO_CHANNEL    (17)
# else
#  error "No supported frequency band found"
# endif
#endif

/* === types =============================================================== */

/** Data type for CRC16 values */
typedef uint16_t crc_t;

/** Buffer type */
typedef struct
{
    uint8_t start;
    uint8_t end;
    union
    {
        uint8_t buf[UART_FRAME_SIZE];
        p2p_wuart_data_t hdr;
    } data;
} wuart_buffer_t;


/** application states */
typedef enum
{
    DO_CONFIGURE,
    WAIT_GUARDTIME,
    WAIT_PLUS,
    /** application is in data mode */
    DATA_MODE,
} wuart_state_t;

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

static void wuart_init(void);
static void configure_radio(void);

static  uint16_t get_number(int8_t base);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef WUART_H */
