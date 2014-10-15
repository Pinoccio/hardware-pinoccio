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
 * @brief Interface for @ref grpBoard.
 * @ingroup grpBoard
 */
#ifndef BOARD_H
#define BOARD_H (1)

#ifndef F_CPU
# error "F_CPU is undefined"
#endif

/* === includes ============================================================ */
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/crc16.h>
#include "const.h"
#include "board_cfg.h"

/**
 * @addtogroup grpBoard
 * @{
 */

/* === macros ============================================================== */
/**
 * @brief Macro for delays with us resolution.
 *
 * The avr-libc internal _delay_us() function
 * allows delays up to 255us. Since the radio
 * needs some delays, which are above this value,
 * _delay_ms is used. As long as the argument for
 * the DELAY_US  macro is a compile time constant,
 * no large overhead is produced, because the compiler
 * performs the division.
 *
 */
#define DELAY_US(x)  _delay_ms(x/1000.0)
/**
 * @brief Macro for delays with ms resolution.
 */
#define DELAY_MS(x)  _delay_ms(x)

#ifndef PULLUP_KEYS
  /** The internal pull-up resistors in the MCU are used, if this macro is defined
   * to 1 in the board definition file
   */
# define PULLUP_KEYS (0)
#endif


/**
 * Set MCU into idle mode.
 */
#define SLEEP_ON_IDLE()\
        do{\
            set_sleep_mode(SLEEP_MODE_IDLE);\
            sleep_mode();\
        }while(0);


#ifdef NO_TIMER
//# define HAVE_MALLOC_TIMERS
/** Macro is defined, if there is no TIMER definition in apropriate board*.h */
# define TIMER_POOL_SIZE  (0)
# define TIMER_INIT() do{}while(0)
# define TIMER_IRQ   TIMER1_OVF_vect
#endif

#ifndef HIF_DEFAULT_BAUDRATE
# define HIF_DEFAULT_BAUDRATE (9600)
#endif

#ifndef HIF_TYPE
/** Macro is defined, if there is no HIF definition in apropriate board*.h */
# define NO_HIF (1)
# define HIF_TYPE (HIF_NONE)
#endif

#ifndef HIF_IO_ENABLE
/** Thiis macro is used to enable the interface circuit of the HIF */
# define HIF_IO_ENABLE() do{}while(0)
#endif

#define HIF_TYPE_IS_UART  ((HIF_TYPE >= HIF_UART_0) && ( HIF_TYPE <= HIF_UART_1))
#define HIF_TYPE_IS_USB   ((HIF_TYPE == HIF_FT245) || (HIF_TYPE == HIF_AT90USB))

/* === Radio Control Pins === */
#ifndef TRX_RESET_INIT
  /** RESET pin IO initialization */
# define TRX_RESET_INIT() DDR_TRX_RESET |= MASK_TRX_RESET
#endif

#ifndef TRX_RESET_HIGH
  /** set RESET pin to high level */
# define TRX_RESET_HIGH() PORT_TRX_RESET |= MASK_TRX_RESET
#endif

#ifndef TRX_RESET_LOW
  /** set RESET pin to low level */
# define TRX_RESET_LOW()  PORT_TRX_RESET &= ~MASK_TRX_RESET
#endif

#ifndef TRX_SLPTR_INIT
  /** SLEEP_TR pin IO initialization */
# define TRX_SLPTR_INIT() DDR_TRX_SLPTR |= MASK_TRX_SLPTR
#endif

#ifndef TRX_SLPTR_HIGH
  /** set SLEEP_TR pin to high level */
# define TRX_SLPTR_HIGH() PORT_TRX_SLPTR |= MASK_TRX_SLPTR
#endif

#ifndef TRX_SLPTR_LOW
  /** set SLEEP_TR pin to low level */
# define TRX_SLPTR_LOW()  PORT_TRX_SLPTR &= ~MASK_TRX_SLPTR
#endif

#if ! defined(DI_TRX_IRQ) && ! defined(EI_TRX_IRQ)
  /* the functions (disable,enable)_all_trx_irqs are defined in atmga_rfa1.h */
# define DI_TRX_IRQ disable_all_trx_irqs
# define EI_TRX_IRQ enable_all_trx_irqs
#endif

#if defined (DBG_PORT) && defined (DBG_DDR) && defined (DBG_PIN)
# define DBG_INIT() do{DBG_DDR |= DBG_PIN; DBG_PORT &= ~DBG_PIN;}while(0)
# define DBG_SET() do{DBG_PORT |= DBG_PIN;}while(0)
# define DBG_CLR() do{DBG_PORT &= ~DBG_PIN;}while(0)
# define DBG_TOGGLE() do{DBG_PORT ^= DBG_PIN;}while(0)
#else
# define DBG_INIT() do{}while(0)
# define DBG_SET() do{}while(0)
# define DBG_CLR() do{}while(0)
# define DBG_TOGGLE() do{}while(0)
#endif


/* === WiBO Control === */
#if ! defined(NO_KEYS) && 0 == 1
# define WIBO_FLAVOUR_KEYPRESS (1)
# define WIBO_FLAVOUR_KEYPRESS_KEYNB (0)
#endif

#if defined(GPIOR0) && 0 == 1
# define WIBO_FLAVOUR_MAILBOX (1)
# define WIBO_FLAVOUR_MAILBOX_REGISTER (GPIOR0)
# define WIBO_FLAVOUR_MAILBOX_CODE (0xA5)
#endif

#if defined(WIBO_FLAVOUR_MAILBOX)
# define WIBO_MAILBOX_SET() \
         do{WIBO_FLAVOUR_MAILBOX_REGISTER = WIBO_FLAVOUR_MAILBOX_CODE;}while(0);
# define WIBO_MAILBOX_CLR() \
         do{WIBO_FLAVOUR_MAILBOX_REGISTER = ~WIBO_FLAVOUR_MAILBOX_CODE;}while(0);
#endif

/* === types =============================================================== */

/**
 * Structure for preconfigured constants that are stored
 * at FLASHEND in programm memory or in the EEPROM.
 *
 * See also @ref get_node_config(), @ref get_node_config_eeprom()
 * and @ref store_node_config_eeprom().
 *
 */
typedef struct
{
    /** The short address of the node. */
    uint16_t short_addr;
    /** The PAN ID (network ID) of the node. */
    uint16_t pan_id;
    /** The MAC address of the node (EUI64). */
    uint64_t ieee_addr;
    /** The radio channel. */
    uint8_t  channel;
    /** For future extensions, but can be used to store user data. */
    uint8_t _reserved_[2];
    /** Ibutton CRC to validate if the structure is correct. */
    uint8_t crc;
} node_config_t;

/**
 * Read the node_config_t structure from the end of the
 * flash memory.
 *
 * @param ncfg
 *        Pointer to the node config structure that is read from FLASH END.
 * @return
 *        Returns 0 if the crc over the structure is correct.
*/
static inline uint8_t get_node_config(node_config_t *ncfg)
{
    uint8_t i = sizeof(node_config_t);
    uint8_t *pram = (uint8_t*)ncfg;
    uint8_t crc = 0, zcnt = 0;
    do
    {
    #if FLASHEND > 0xffffL
        *pram = pgm_read_byte_far(((long)FLASHEND - i + 1));
    #else
        *pram = pgm_read_byte_near((FLASHEND - i + 1));
    #endif

        crc = _crc_ibutton_update(crc, *pram);
        zcnt += *pram ? 0 : 1;
        pram ++;
    }
    while(--i);

    /* return true if crc does not match or all */
    if (zcnt == sizeof(node_config_t))
    {
        crc = 0x55;
    }

    return crc;
}

/**
 * Read the node_config_t structure from an offset in the EEPROM.
 *
 * @param ncfg
 *        Pointer to the node config structure that is read from EEPROM.
 * @param offset
 *        EEPROM offset, at where the structure is read from.
 * @return
 *        Returns 0 if the crc over the structure is correct.
 */
static inline uint8_t get_node_config_eeprom(node_config_t *ncfg, uint8_t * offset)
{
    uint8_t i = sizeof(node_config_t);
    uint8_t *pram = (uint8_t*)ncfg;
    uint8_t crc = 0, zcnt = 0;
    do
    {
        *pram = eeprom_read_byte( (const uint8_t *) offset++ );
        crc = _crc_ibutton_update(crc, *pram);
        zcnt += *pram ? 0 : 1;
        pram ++;
    }
    while(--i);
    if (zcnt == sizeof(node_config_t))
    {
        crc = 0x55;
    }
    return crc;
}

/**
 * Read the node_config_t structure from an offset in the EEPROM.
 *
 * @param ncfg
 *        Pointer to the node config structure that is written to EEPROM.
 *        The crc byte is computed within this function.
 * @param offset
 *        EEPROM offset, at which the structure is stored.
 */
static inline void store_node_config_eeprom(node_config_t *ncfg, uint8_t * offset)
{
    uint8_t i = sizeof(node_config_t) - sizeof(uint8_t);
    uint8_t *pram = (uint8_t*)ncfg;
    uint8_t crc = 0;
    do
    {
        eeprom_write_byte( (uint8_t *)offset++, *pram );
        crc = _crc_ibutton_update(crc, *pram++);
    }
    while(--i);
    eeprom_write_byte((uint8_t *)offset, crc );
}

/**
 * Jump to the bootloader code.
 */
static inline void jump_to_bootloader(void)
{
    typedef void (*func_ptr_t)(void) __attribute__((noreturn));
    const func_ptr_t jmp_boot = (func_ptr_t) BOOTLOADER_ADDRESS;
#if defined(WIBO_FLAVOUR_MAILBOX)
    WIBO_MAILBOX_SET();
#endif
    jmp_boot();
}

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */
#endif /* #ifndef BOARD_H */
