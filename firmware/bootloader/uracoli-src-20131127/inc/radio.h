/* Copyright (c) 2007-2009 Axel Wachtler
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
 * @brief Interface for @ref grpLibRadio.
 *
 */
#ifndef RADIO_H
#define RADIO_H

/* === Includes ============================================================== */
#include <stdbool.h>
#include <stdio.h>
#include <util/crc16.h>
#include "const.h"

/**
 * @addtogroup grpRadio
 * @{
 */

/* === Externals ============================================================= */

/* === Types ================================================================= */

/**
 * Radio state enumeration
 */

/** TRX in OFF state. */
#define  STATE_OFF (0)
/** Basic mode TX state. */
#define  STATE_TX (1)
/** Basic mode RX state. */
#define STATE_RX (2)
/** Extended mode TX state (TX_ARET). */
#define STATE_TXAUTO (3)
/** Extended mode RX state (RX_AACK). */
#define STATE_RXAUTO (4)
/** Sleep state (lowest power consumption). */
#define STATE_SLEEP (5)


/** Radio state type */
typedef uint8_t radio_state_t;

/**
 * error codes for tx done event
 */
typedef enum
{
    TX_OK,			/**< transmission completed successfully */
    TX_CCA_FAIL,		/**< channel was busy (TX_AUTO only) */
    TX_NO_ACK,			/**< no ACK received (TX_AUTO only) */
    TX_FAIL,			/**< unexpected error */
} radio_tx_done_t;


/**
 * codes for CCA
 */
typedef enum SHORTENUM
{
#if defined(CCA_BUSY)
	/** @todo this is workaround for name clash for RFA1 and avr-libc
	 *   avr/iom128rfa1.h, check if this is OK or if we should change
	 *   the names of CCA_FREE, ..., etc.
	 */
	RADIO_CCA_FREE = CCA_IDLE,
	RADIO_CCA_BUSY = CCA_BUSY,
	RADIO_CCA_FAIL = 255,
#else
    /** The CCA measurement estimates, that the channel is free. */
    RADIO_CCA_FREE = 0,
    /** The CCA measurement estimates, that the channel is busy. */
    RADIO_CCA_BUSY,
    /** The CCA measurement was not finished. */
    RADIO_CCA_FAIL
#endif

} radio_cca_t;


/** Enumeration to identify radio attributes.
 */
typedef enum SHORTENUM
{
    /** Set the current channel */
    phyCurrentChannel,
    /** Currently unused */
    phyChannelsSupported,
    /** Set the Tx power */
    phyTransmitPower,
    /** Transceiver state to return to after transmission */
    phyIdleState,
    /** CCA mode to use in CSMA-CA:
     * <table>
     *   <tr><th>value</th><th>CCA mode</th></tr>
     *   <tr><td>0</td>    <td>carrier sense OR energy above threshold</td></tr>
     *   <tr><td>1</td>    <td>energy above threshold (default)</td></tr>
     *   <tr><td>2</td>    <td>carrier sense</td></tr>
     *   <tr><td>3</td>    <td>carrier sense AND energy above threshold</td></tr>
     * </table>
     */
    phyCCAMode,
    /** PAN ID to use in STATE_RXAUTO frame filter */
    phyPanId,
    /** Short (16-bit) address to use in STATE_RXAUTO frame filter */
    phyShortAddr,
    /** Pointer to long (EUI-64) address to use in STATE_RXAUTO frame filter */
    phyLongAddr,

    /** Datarate */
    phyDataRate,

    /** PA enable */
    phyTxPa,
    /** LNA enable */
    phyRxLna

} radio_attribute_t;


/**
 * @brief Container for handover of radio parameter values.
 *
 * @note
 * The elements in this union should have max. a size
 * of 2 byte, all other parameters should be configured with
 * the void pointer.
 */
typedef union radio_param_t
{
#if defined __cplusplus
public:
    radio_param_t(int8_t c) { channel = c; } /* also used for txpwr_t */
    //radio_param_t(radio_state_t s) { idle_state = s; }
    radio_param_t(uint8_t m) { cca_mode = m; } /* also used for data_rate, tx_pa, rx_lna */
    radio_param_t(uint16_t p) { pan_id = p; } /* also used for short_addr */
    radio_param_t(uint64_t *la) { long_addr = la; }
#endif
    /** Value for current radio channel. (@ref MIN_CHANNEL ... @ref MAX_CHANNEL)*/
    channel_t channel;
    /** Value for transmit power in dB.*/
    txpwr_t   tx_pwr;
    /** after TX go to idle state */
    radio_state_t idle_state;
    /** Value for cca mode. */
    ccamode_t cca_mode;
    /** Value for PANID */
    uint16_t pan_id;
    /** Value for short address */
    uint16_t short_addr;
    /** Pointer to long (64-bit) address */
    uint64_t *long_addr;
    /** data rate type */
    uint8_t data_rate;

    /** TX power amp type */
    uint8_t tx_pa;
    /** RX LNA type */
    uint8_t rx_lna;

} radio_param_t;


/**
 * @brief Error codes.
 */
typedef enum SHORTENUM
{
	#ifndef SUCCESS
	/** @todo same like CCA_FREE, include/avr/iom128rfa1.h:3687:#define SUCCESS*/
    SUCCESS = 0,        /**< OK Code*/
    #endif
    STATE_SET_FAILED = 1,   /**< function radio_set_state failed */
    SET_PARM_FAILED,    /**< function radio_set_param failed */
    GET_PARM_FAILED,    /**< function radio_get_param failed */
    GENERAL_ERROR,      /**< something unexpected happened */
} radio_error_t;


/**
 * @brief Structure for storage of radio parameters.
 */
typedef struct
{
    uint8_t  channel;      /**< Current radio channel.
                                (@ref MIN_CHANNEL ... @ref MAX_CHANNEL) */
    uint8_t  tx_pwr;       /**< Current transmit power. */
    uint8_t  cca_mode;     /**< Current cca mode. */
    radio_state_t state;      /**< Current transceiver state. */
    radio_state_t idle_state; /**< after TX go to idle state */
    uint8_t  *rxframe;      /**< Pointer for frame data storage. */
    uint8_t  rxframesz;     /**< Length of the buffer rxframesz */
    uint8_t tx_pa;
    uint8_t rx_lna;
} radio_status_t;

/* === Macros ================================================================ */

/**
 * Code for invalid RSSI value.
 */
#define VOID_RSSI (0xff)

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the channel number to @c x.
 */
#  define RP_CHANNEL(x)
#elif defined __cplusplus
#  define RP_CHANNEL(x) phyCurrentChannel,radio_param_t((channel_t)x)
#else
#  define RP_CHANNEL(x) phyCurrentChannel,(radio_param_t){.channel=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the tx power value to @c x.
 */
#  define RP_TXPWR(x)
#elif defined __cplusplus
#  define RP_TXPWR(x)   phyTransmitPower,radio_param_t((txpwr_t)x)
#else
#  define RP_TXPWR(x)   phyTransmitPower,(radio_param_t){.tx_pwr=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the transceiver's idle state to @c x.
 */
#  define RP_IDLESTATE(x)
#elif defined __cplusplus
#  define RP_IDLESTATE(x)  phyIdleState,radio_param_t(x)
#else
#  define RP_IDLESTATE(x)  phyIdleState,(radio_param_t){.idle_state=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the CCA mode to @c x.
 */
#  define RP_CCAMODE(x)
#elif defined __cplusplus
#  define RP_CCAMODE(x)  phyCCAMode,radio_param_t((ccamode_t)x)
#else
#  define RP_CCAMODE(x) phyCCAMode,(radio_param_t){.cca_mode=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the PAN ID to @c x.
 */
#  define RP_PANID(x)
#elif defined __cplusplus
#  define RP_PANID(x) phyPanId,radio_param_t((uint16_t)x)
#else
#  define RP_PANID(x) phyPanId,(radio_param_t){.pan_id=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the short address to @c x.
 */
#  define RP_SHORTADDR(x)
#elif defined __cplusplus
#  define RP_SHORTADDR(x) phyShortAddr,radio_param_t((uint16_t)x)
#else
#  define RP_SHORTADDR(x) phyShortAddr,(radio_param_t){.short_addr=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the long address pointer to @c x.
 */
#  define RP_LONGADDR(x)
#elif defined __cplusplus
#  define RP_LONGADDR(x) phyLongAddr,radio_param_t((uint64_t *)x)
#else
#  define RP_LONGADDR(x) phyLongAddr,(radio_param_t){.long_addr=x}
#endif


#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the data rate to @c x.
 */
#  define RP_DATARATE(x)
#elif defined __cplusplus
#  define RP_DATARATE(x) phyDataRate,radio_param_t((uint16_t)x)
#else
#  define RP_DATARATE(x) phyDataRate,(radio_param_t){.data_rate=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the TX power amplifier enable number to @c x.
 */
#  define RP_CHANNEL(x)
#elif defined __cplusplus
#  define RP_TX_PA(x) phyTxPa,radio_param_t((tx_pa_t)x)
#else
#  define RP_TX_PA(x) phyTxPa,(radio_param_t){.tx_pa=x}
#endif

#if defined(DOXYGEN)
/**
 * Helper macro to construct the arguments for @ref radio_set_param in
 * order to set the TX power amplifier enable number to @c x.
 */
#  define RP_RX_LNA(x)
#elif defined __cplusplus
#  define RP_RX_LNA(x) phyRxLna,radio_param_t((rx_lna_t)x)
#else
#  define RP_RX_LNA(x) phyRxLna,(radio_param_t){.rx_lna=x}
#endif

#define CRC_CCITT_UPDATE(crc, data) _crc_ccitt_update(crc, data)

#ifndef RADIO_CFG_EEOFFSET
/** offset of radio config data in EEPROM */
#define RADIO_CFG_EEOFFSET (8)
#endif

#ifndef RADIO_CFG_DATA
/** a default radio configuration data structure */
#define RADIO_CFG_DATA {chan: 16, txp: 0, cca: 1, edt: 11, clkm: 0, crc: 0xab12}
#endif

/* === Prototypes ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Radio related ressource initialization
 *
 * The function initializes all IO ressources,
 * needed for the usage of the radio and performs
 * a reset.
 *
 * @param rxbuf A buffer for the receive frames.
 *              This buffer needs to be static, and
 *              of size MAX_FRAME_SIZE
 *              (see also function usr_radio_receive_frame).
 * @param rxbufsz maximum size of the rx buffer, frames longer
 *                then rxbufsz will be ignored
 */
void radio_init(uint8_t * rxbuf, uint8_t rxbufsz);


/**
 * @brief Force the radio to the requested state.
 *        (using CMD_FORCE_TRX_OFF)
 * @param state  requested radio state
 */
void radio_force_state(radio_state_t state);


/**
 * @brief Bring the the radio in the requested state.
 * @param state  requested radio state
 */
void radio_set_state(radio_state_t state);

/**
 * @brief Set a radio parameter.
 *
 * Note that the radio must not be in STATE_SLEEP when setting
 * parameters.  Preferrably, it should be kept in STATE_OFF (in
 * which it is immediately after a call to @ref radio_init) in
 * order to set parameters.
 *
 * @param attr  attribute parameter (enumeration value radio_attribute_t)
 * @param parm  pointer to parameter value (union type radio_param_t)
 *
 * @par Examples:
 *
 * @code
 *   radio_set_param(RP_IDLESTATE(STATE_RX));
 *
 * @endcode
 * @code
 *   #define CHANNEL (17)
 *
 *   radio_set_param(RP_CHANNEL(CHANNEL));
 * @endcode
 */
void radio_set_param(radio_attribute_t attr, radio_param_t parm);

/**
 * @brief Frame transmission
 *
 * Initiates a frame transmission procedure, and then downloads the
 * frame passed as @c frm.  The function returns immediately once the
 * frame download procedure completed; the end of transmission is
 * signalled by the @ref usr_radio_tx_done callback.  If the radio is
 * in STATE_TXAUTO, a full unslotted CSMA-CA is procedure is performed
 * by the transceiver hardware, including frame retransmissions in
 * case the transmitted frame has requested an acknowledgement by the
 * recipient.  If the transceiver is in STATE_TX, an immediate frame
 * transmission is initiated, without CSMA-CA or frame
 * retransmissions.
 *
 * @param len length of frame to transmit
 * @param frm pointer to frame to transmit
 * @param compcrc compute CRC-16 if != 0 (currently ignored)
 */
void radio_send_frame(uint8_t len, uint8_t *frm, uint8_t compcrc);


/**
 * @brief Perform CCA Measure
 * @return cca status (see @ref radio_cca_t)
 */
radio_cca_t radio_do_cca(void);

int radio_putchar(int c);
int radio_getchar(void);

/* application call back functions to radio library */


/**
 * Error callback function which has to be implemented in the application.
 *
 * This function is called, when a fatal error occurs.
 * see also @ref radio_error_t.
 *
 * @param err error code being reported
 */
void usr_radio_error(radio_error_t err);

/**
 * Interrupt callback function.
 *
 * @param cause value of the transceiver's IRQ status register
 */
void usr_radio_irq(uint8_t cause);

/**
 * @brief Frame reception callback function.
 *
 * This function is called within an interrupt context for every
 * received frame, according to the current receive mode used (which
 * involves address filtering in case the transceiver is in state
 * RX_AUTO).  The frame has been internally stored into the receive
 * buffer that has been configured before, either by @ref radio_init,
 * or as the result of a previous call to usr_radio_receive_frame.
 * The function must return a valid pointer to a receive buffer to be
 * used for receiving the next frame; this can be the same value as
 * the parameter @c frm, or a different one in case the application
 * wants to maintain multiple buffers.
 *
 * The @c rssi parameter is obtained at the frame's RX_START
 * interrupt.  If there was no RX_START interrupt to read an RSSI
 * value at, the value @ref VOID_RSSI is passed instead.
 *
 * @param len length of frame received
 * @param frm pointer to frame received
 * @param lqi LQI value reported by transceiver
 * @param rssi RSSI value obtained from transceiver
 * @param crc_fail boolean indicating whether the received frame failed FCS verification
 * @return address of new receive buffer
 */
uint8_t * usr_radio_receive_frame(uint8_t len, uint8_t *frm, uint8_t lqi, int8_t ed, uint8_t crc_fail);


/**
 * Transmit done callback function.
 *
 * @param status completion status, @ref radio_tx_done_t
 */
void usr_radio_tx_done(radio_tx_done_t status);


#ifdef __cplusplus
} /* extern "C" */
#endif


/** @} */


#endif  /* #ifndef RADIO_H */
/* EOF */
