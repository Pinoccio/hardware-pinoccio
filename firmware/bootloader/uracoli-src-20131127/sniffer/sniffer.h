/* Copyright (c) 2007 Axel Wachtler
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
/**
 * @file
 * @brief Interface of the @ref grpAppSniffer
 *
 * @ingroup grpAppSniffer
 */


/* === includes ============================================================ */
#include <avr/pgmspace.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdbool.h>
#include <util/crc16.h>
#include <avr/interrupt.h>
#include "transceiver.h"
#include "ioutil.h"
#include "timer.h"
#ifndef SNIFFER_H
#define SNIFFER_H

/* === macros ============================================================== */

#define SCAN_PERIOD_MS  (2000)
#define NL "\n"
#define CHANNEL_OFFSET(x)  (x > TRX_MAX_CHANNEL ? TRX_MIN_CHANNEL : (x - TRX_MIN_CHANNEL))
#define CHANNEL_MAX_OFFSET (TRX_NB_CHANNELS-1)
#define VERSION "0.1"
/**
 * Increment channel number and wraps back to lowest channel, if
 * upper channel + 1 is reached.
 */
#define CHANNEL_NEXT_CIRCULAR(x) \
    do \
    { \
        (x)++; \
        if ((x) > TRX_MAX_CHANNEL) \
        { \
            (x) = TRX_MIN_CHANNEL; \
        } \
    } \
    while(0)


#ifndef MAX_PACKET_BUFFERS
# define MAX_PACKET_BUFFERS (8)
#endif
/* === types =============================================================== */
/**
 * @brief Appication States.
 */
typedef enum
{
    /** Application is in idle mode, receiving commands from HIF. */
    IDLE,
    /** Application is in scanning mode. */
    EDSCAN,
    /** Application is in scanning mode. */
    SCAN,
    /** Application is in scanning mode. */
    SCAN_DONE,
    /** Application is in sniffing mode. */
    SNIFF
} SHORTENUM sniffer_state_t;

/**
 * @brief Data structure for scan results.
 */
typedef struct scan_result_tag
{
    /** total number of received frames */
    uint16_t framecnt;
    /** total number of frames with valid crc */
    uint16_t crc_ok;
    uint16_t edsum;
    uint16_t lqisum;
    uint16_t ftypes[8];
} scan_result_t;

/**
 * @brief Data structure for internal state variables of
 * the application.
 */
typedef struct sniffer_context_tag
{
    /** state */
    volatile sniffer_state_t state;
    /** current channel */
    channel_t cchan;
    /** current channel page */
    uint8_t cpage;
    /** mask for scanned channels */
    uint32_t cmask;

    /** upload only frames with CRC ok if set to 1*/
    bool chkcrc;

    /** timer handle */
    timer_hdl_t     thdl;
    /** scan period */
    time_t          scanper;
    /** table for scan results */
    scan_result_t   scanres[TRX_NB_CHANNELS];
    uint8_t         scanres_reset;
    uint16_t frames;
    uint16_t irq_ur;
    uint16_t missed_frames;
} sniffer_context_t;

typedef struct pcap_packet_tag
{
    /** length value (frame length + sizeof(time_stamp_t)) */
    uint8_t len;
    /** time stamp storage */
    time_stamp_t ts;
    /** buffer that holds a frame with maximum length */
    uint8_t frame[MAX_FRAME_SIZE];
} pcap_packet_t;


typedef struct pcap_pool_tag
{
    volatile uint8_t ridx;
    volatile uint8_t widx;
    pcap_packet_t packet[MAX_PACKET_BUFFERS];
} pcap_pool_t;


/* === externals =========================================================== */
extern sniffer_context_t ctx;

/* === inline functions ==================================================== */

/**
 * @brief update the scan table for a channel.
 */
static inline void scan_update_frame(uint8_t flen, bool crc_ok, uint8_t lqi, uint8_t ed, uint8_t *rxbuf)
{
scan_result_t *scres;

     scres = &ctx.scanres[(ctx.cchan - TRX_MIN_CHANNEL)];
     scres->framecnt ++;
     scres->edsum +=ed;
     scres->lqisum += lqi;

     if (flen < 0x80)
     {
         /* process valid frame length */
         if (crc_ok == true)
         {
             scres->crc_ok++;
             scres->ftypes[rxbuf[0]&7] ++;
         }
         /* parse beacon */
     }
}



/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Process data received from HIF.
 *
 * The function checks for available input data and
 * performs depending on the current state the processing.
 *
 */
void sniffer_start(sniffer_state_t state);
void sniffer_stop(void);
void ctrl_process_input(void);
void scan_init(void);
void scan_continue(void);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef SNIFFER_H */
