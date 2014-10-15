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
 * @brief Implementation of the scanning functions for @ref grpAppSniffer
 *
 * @ingroup grpAppSniffer
 */

/* === includes ============================================================ */
#include "sniffer.h"

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */

/* === prototypes ========================================================== */
static void show_status(channel_t choffs);
time_t timer_scan(timer_arg_t t);

/* === functions =========================================================== */

/**
 * @brief Initialize the scan mode.
 */
void scan_init(void)
{

    PRINTF("Scanning channels, scan period %ums"NL,
           SCAN_PERIOD_MS);
    PRINT("             bad"
          "    Avg. "
          "        802.15.4 frames   "
          "    "NL);
    PRINT("chan   frm   crc "
          "  ed lqi "
          "     B     D     A     C "
          " PER"NL);

    ctx.cchan = TRX_MIN_CHANNEL;
    trx_bit_write(SR_CHANNEL, ctx.cchan);
    trx_reg_write(RG_TRX_STATE, CMD_RX_ON);

    ctx.thdl = timer_start(timer_scan,MSEC(SCAN_PERIOD_MS),0);
}

/**
 * @brief Update status in scan mode.
 */
void scan_update_status(void)
{
scan_result_t *scres;
uint32_t cmask;

    cmask = TRX_SUPPORTED_CHANNELS;
    do
    {
        show_status(ctx.cchan);
        ctx.cchan += 1;
        if (ctx.cchan > TRX_MAX_CHANNEL)
        {
            ctx.cchan = TRX_MIN_CHANNEL;
        }
        cmask &= ~(1UL<<ctx.cchan);
        if(((ctx.cmask & (1UL<<ctx.cchan)) != 0) || (cmask == 0))
        {
            /* exit loop if we find a channel that is to be
               scanned or there are no more (supported) channels to scan */
            break;
        }
    }
    while(1);
    if (ctx.scanres_reset)
    {
        scres = &ctx.scanres[CHANNEL_OFFSET(ctx.cchan)];
        memset(scres,0,sizeof(scan_result_t));
        if (ctx.scanres_reset < 32)
        {
            ctx.scanres_reset --;
        }
    }

    /* set channel next channel and continue scanning */
    trx_bit_write(SR_CHANNEL, ctx.cchan);
    cli();
    ctx.state = SCAN;
    sei();

    ctx.thdl = timer_start(timer_scan,MSEC(SCAN_PERIOD_MS),0);

}

/**
 * @brief Display scan result on HIF.
 * @param channel number of the channel to be displayed
 */
static void show_status(channel_t channel)
{
static uint16_t updates = 0;
scan_result_t *scres;
uint16_t per = 0, lqi, ed;
uint8_t choffs;

    updates++;
    choffs = CHANNEL_OFFSET(channel);
    if((ctx.cmask & (1UL<<channel)) == 0)
    {
        /* this is an unscanned channel */
        PRINTF(" %2d  n/a"NL, channel);
    }
    else
    {
        /* this is an scanned channel */
        scres = &ctx.scanres[choffs];
        if (scres->framecnt > 0)
        {
            per = (scres->framecnt - scres->crc_ok) * 100 / scres->framecnt;
        }

        lqi = (scres->framecnt > 0) ? scres->lqisum / scres->framecnt : 0;
        ed  = (scres->framecnt > 0) ? scres->edsum / scres->framecnt : 0;

        PRINTF(" %2d  % 5u % 5u "
               " % 3u %3u ",
                channel, scres->framecnt, scres->framecnt - scres->crc_ok,
                ed, lqi);

        PRINTF(" % 5u % 5u % 5u % 5u"
               "  % 3u"NL,
                scres->ftypes[0], scres->ftypes[1], scres->ftypes[2], scres->ftypes[3],
                per
                );
    }
    if (choffs == CHANNEL_MAX_OFFSET)
    {
        PRINTF("=== ur %d frames: %d ==="NL, ctx.irq_ur, ctx.frames);
        /*         0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16 */
        hif_puts("\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\n");
    }
    hif_puts("\r*\r\b");


    if (ctx.scanres_reset)
    {
        if (ctx.scanres_reset <= TRX_NB_CHANNELS)
        {
            hif_puts("\rr");
        }
        else
        {
            hif_puts("\rR");
        }
        hif_puts("                                                      \r\b");
    }
    else
    {
        hif_puts("\r*\r\b");
    }
}


/**
 * @brief Timer routine called once for each scan period.
 */
time_t timer_scan(timer_arg_t t)
{
    ctx.state = SCAN_DONE;
    return 0;
}
/* EOF */
