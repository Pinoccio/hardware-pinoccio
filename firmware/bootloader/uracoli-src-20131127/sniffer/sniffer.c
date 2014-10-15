/* Copyright (c) 2007 - 2010 Axel Wachtler
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
 * @brief Implementation of the @ref grpAppSniffer
 *
 * @ingroup grpAppSniffer
 */
/**
 * @note
 *   In order to get higher performance in frame receiving, this
 *   application uses a own implementation of the transceiver
 *   interrrupt routine (@ref TRX_IRQ_vect),
 *   instead of the callback function from librf23x.a.
 */

/* === includes ========================================== */
#include "sniffer.h"

/* === macros ============================================ */

/* === types ============================================= */

/* === globals =========================================== */
sniffer_context_t ctx;
pcap_pool_t PcapPool;

/* === prototypes ======================================== */
void scan_update_status(void);

/* === functions ========================================= */



/**
 * @brief Initialisation of hardware ressources.
 *
 * This function initializes the following components
 *  - LED Port
 *  - Timer Module
 *  - Host Interface
 *  - SPI Port for Radio
 *  - Radio (reset and enter state RX)
 */

void sniffer_init(void)
{
uint8_t val;

    /* Phase 1: initialize MCU peripherals */
    LED_INIT();

    /* init memory locations */
    memset(&ctx, 0, sizeof(ctx));
    PcapPool.ridx = 0;
    PcapPool.widx = 0;
    ctx.state = IDLE;
    ctx.cchan = TRX_MIN_CHANNEL;
    ctx.cmask = TRX_SUPPORTED_CHANNELS;

    LED_SET_VALUE(1);

    /* initialize MCU ressources */
    timer_init();
    hif_init(HIF_DEFAULT_BAUDRATE);

    LED_SET_VALUE(1);

    /* initialize transceiver */
    trx_io_init(DEFAULT_SPI_RATE);
    trx_init();


    LED_SET_VALUE(2);

    /* check for PLL Lock */
    val = trx_check_pll_lock();
    if (val != TRX_OK)
    {
        while(1)
        {
            /* wait due to fatal error */
        };
    }

#if defined(TRX_IRQ_RX_START) && defined(TRX_IRQ_TRX_END) && defined(TRX_IRQ_UR)
    trx_reg_write(RG_IRQ_MASK, (TRX_IRQ_RX_START|TRX_IRQ_TRX_END|TRX_IRQ_UR));
#elif defined(TRX_IRQ_RX_START) && defined(TRX_IRQ_RX_END)
    trx_reg_write(RG_IRQ_MASK, (TRX_IRQ_RX_START | TRX_IRQ_RX_END));
#else
#  error "Unknown IRQ bits"
#endif

    LED_SET_VALUE(3);
    sei();

    /* done with init */
    PRINTF(NL"Sniffer V%s [%s]"NL, VERSION, BOARD_NAME);
    LED_SET_VALUE(0);
}

/**
 * @brief Start a new operating state.
 */
void sniffer_start(sniffer_state_t state)
{
    switch (state)
    {
        case IDLE:
            trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
            ctx.state = IDLE;
            LED_SET_VALUE(0);
            break;
        case SCAN:
            ctx.state = SCAN;
            scan_init();
            break;
        case SNIFF:
            trx_reg_write(RG_TRX_STATE, CMD_RX_ON);
            ctx.state = SNIFF;
            break;

        default:
            break;
    }
}

/**
 * @brief Halt current operation and enter state IDLE.
 */
void sniffer_stop(void)
{
sniffer_state_t curr_state;

    trx_reg_write(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
    cli();
    curr_state = ctx.state;
    ctx.state = IDLE;
    sei();

    switch(curr_state)
    {
        case SCAN:
            ctx.cchan = TRX_MIN_CHANNEL;
            ctx.thdl = timer_stop(ctx.thdl);
            break;
        case SNIFF:
        case IDLE:
            break;
        default:
            PRINTF("Unknown state %d"NL,ctx.state);
            break;

    }
}

/**
 * @brief Main function of sniffer application.
 *
 * The main loop of the application processes the
 * input coming from the HIF and frames and frame data
 * received by the radio transceiver.
 */
int main(void)
{

    sniffer_init();

    while(1)
    {
        ctrl_process_input();

        if(ctx.state == SCAN_DONE)
        {
            scan_update_status();
        }
        if ((ctx.state == SNIFF) && (PcapPool.widx != PcapPool.ridx))
        {
            uint8_t tmp, len, *p;
            pcap_packet_t *ppcap = &PcapPool.packet[PcapPool.ridx];
            hif_putc(1);
            #if 0
                hif_put_blk((uint8_t*)ppcap, ppcap->len+1);
            #else
                len = ppcap->len+1;
                p = (uint8_t*)ppcap;
                do
                {
                    tmp = hif_put_blk(p, len);
                    p += tmp;
                    len -= tmp;
                }
                while(len>0);
            #endif
            hif_putc(4);
            /* mark buffer as processed */
            ppcap->len = 0;
            PcapPool.ridx++;
            PcapPool.ridx &= (MAX_PACKET_BUFFERS-1);
        }
    }
}

/**
 * @brief Trx interrupt service routine for sniffer/scanner.
 */
#if defined(DOXYGEN)
void TRX_IRQ_vect()
#elif !defined(TRX_IF_RFA1)
ISR(TRX_IRQ_vect)
{
static volatile uint8_t cause;
uint8_t ed, flen, lqi = 0;
bool crc_ok = 0;
extern time_t systime;
static pcap_packet_t *ppcap;

    DI_TRX_IRQ();
    sei();
    cause = trx_reg_read(RG_IRQ_STATUS);

    if (cause & TRX_IRQ_RX_START)
    {
        ppcap = &PcapPool.packet[PcapPool.widx];
        if (ppcap->len != 0)
        {
            /* drop packet, no free buffers*/
            ppcap = NULL;
            return;
        }
        ppcap->ts.time_usec = TRX_TSTAMP_REG;
        ppcap->ts.time_sec = systime;
    }

    if (cause & TRX_IRQ_TRX_END)
    {
        /* Upload frame at TRX_END IRQ */
        if (ppcap != NULL)
        {
            ed = trx_reg_read(RG_PHY_ED_LEVEL);
            flen = trx_frame_read_crc(&ppcap->frame[0], MAX_FRAME_SIZE, &crc_ok);
            trx_sram_read(flen, 1, &lqi);
            if (ctx.state == SCAN)
            {
                scan_update_frame(flen, crc_ok, lqi, ed, ppcap->frame);
            }
            if (ctx.state == SNIFF)
            {
                ppcap->len = flen + sizeof(time_stamp_t);
                PcapPool.widx++;
                PcapPool.widx &= (MAX_PACKET_BUFFERS-1);
            }
        }
        ctx.frames++;
        LED_SET_VALUE(ctx.frames);
    }

    if (cause & TRX_IRQ_UR)
    {
        ctx.irq_ur ++;
    }

    cli();
    EI_TRX_IRQ();
}
#endif /* defined(DOXYGEN) || !RFA1 */

#if defined(TRX_IF_RFA1)

pcap_packet_t *ppcap_trx24 = NULL;

ISR(TRX24_RX_START_vect)
{
extern time_t systime;
    ppcap_trx24 = &PcapPool.packet[PcapPool.widx];
    if (ppcap_trx24->len != 0)
    {
        /* drop packet, no free buffers*/
        ppcap_trx24 = NULL;
        return;
    }
    ppcap_trx24->ts.time_usec = TRX_TSTAMP_REG;
    ppcap_trx24->ts.time_sec = systime;
}

ISR(TRX24_RX_END_vect)
{
uint8_t ed, flen, lqi = 0;
bool crc_ok = 0;

    if (ppcap_trx24 != NULL)
    {
        ed = trx_reg_read(RG_PHY_ED_LEVEL);
        flen = trx_frame_read_crc(&ppcap_trx24->frame[0], MAX_FRAME_SIZE, &crc_ok);
        trx_sram_read(flen, 1, &lqi);
        if (ctx.state == SCAN)
        {
            scan_update_frame(flen, crc_ok, lqi, ed, ppcap_trx24->frame);
        }
        if (ctx.state == SNIFF)
        {
            ppcap_trx24->len = flen + sizeof(time_stamp_t);
            PcapPool.widx++;
            PcapPool.widx &= (MAX_PACKET_BUFFERS-1);
        }
    }
    ctx.frames++;
    LED_SET_VALUE(ctx.frames);
}
#endif  /* RFA1 */

/* EOF */
