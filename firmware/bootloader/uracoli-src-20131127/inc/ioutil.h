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

/* The function keys_debounced() use the code given in Peter Fleury's
   avr-gcc examples. File: avrgcc-examples/debounce_keys.c */

/****************************************************************************
 Title:    Debouncing 8 Keys
 Author:   Peter Fleury <pfleury@gmx.ch> http://jump.to/fleury,
           based on algorithm of Peter Dannegger <danni@specs.de>
 Date:     December 2003
 Software: AVR-GCC 3.3
 Hardware: AT90S8515 at 4 Mhz, STK200 compatible starter kit

*****************************************************************************/

/* $Id$ */
/**
 * @file
 * @brief Interface for @ref grpLibIoutil.
 */

#ifndef IOUTIL_H
#define IOUTIL_H

/* === Includes ============================================================== */
#include <stdlib.h>
#include <stdio.h>
//#include <util/atomic.h>

#include "board.h"
#include "hif.h"
#include "timer.h"

/* === Externals ============================================================= */

/* === Types ================================================================= */

/** 
 * @addtogroup grpIoUtilBuffer
 * @{ */

/** buffer structure, which supports appending and prepending
 *   of data as well as chaining other buffers.
 */

typedef struct
{
    /** pointer to next buffer */
    void * next;
    /** tag if buffer is used or free */
    uint8_t used;
    /** total lenght of the data block */
    uint8_t len;
    /** index of the start of the data block */
    uint8_t istart;
    /** index of the end of the data block */
    uint8_t iend;
    /** data block */
    uint8_t data[];
} buffer_t;


typedef struct
{
    /** number of buffers in the pool */
    uint8_t nb;
    /** total size of a buffer element */
    uint8_t elsz;
    /** data block seperated into buffer_t elemente */
    uint8_t pool[];
} buffer_pool_t;

/** @} */

/* === Macros ================================================================ */

/* === LED Handling === */

/** @addtogroup grpIoUtilLedKey */
/** @{ */

#if defined(NO_LEDS) || defined (DOXYGEN)
# undef LED_NUMBER
  /** Number of LEDs for this board */
# define LED_NUMBER    (0)
#endif

#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Initialisation of the LED port. */
# define LED_INIT() do{}while(0)
#elif !defined(LED_INIT)
# if LEDS_INVERSE == 0
#  define LED_INIT() do{\
                        LED_DDR |= (LED_MASK); LED_PORT &= ~(LED_MASK);\
                     }while(0)
# else
#   define LED_INIT() do{\
                         LED_DDR |= (LED_MASK); LED_PORT |= (LED_MASK);\
                      }while(0)
# endif
#endif /* !defined(LED_INIT)*/

#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Display a numeric value on the LED port.
   * The value x is masked out according @ref LED_MASK,
   * so that depending on the number of LEDs of the board
   * the maximum displayed value is @ref LED_MAX_VALUE.
   */
# define LED_SET_VALUE(x) do{}while(0)
#elif !defined(LED_SET_VALUE)
# if LEDS_INVERSE == 0
#  define LED_SET_VALUE(x) \
            do {\
                LED_PORT = (LED_PORT & ~LED_MASK) | ((x<<LED_SHIFT) & LED_MASK);\
            }while(0)
# else
#  define LED_SET_VALUE(x) do {\
            LED_PORT = (LED_PORT & ~LED_MASK) | ((~x<<LED_SHIFT) & LED_MASK);\
            }while(0)
# endif
#endif /* !defined(LED_SET_VALUE)*/

#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Read back the current numeric value from the LED port. */
# define LED_GET_VALUE() 0
#elif !defined(LED_GET_VALUE)
# if LEDS_INVERSE == 0
#  define LED_GET_VALUE()  ((LED_PORT & LED_MASK) >> LED_SHIFT)
# else
#  define LED_GET_VALUE()  ((~LED_PORT & LED_MASK) >> LED_SHIFT)
# endif
#endif /* !defined(LED_GET_VALUE)*/


#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Switch the LED with the number  @e ln ON. */
# define LED_SET(ln) do{}while(0)
#elif !defined(LED_SET)
# if LEDS_INVERSE == 0
#  define LED_SET(ln)      LED_PORT |= (_BV(ln+LED_SHIFT) & LED_MASK)
# else
#  define LED_SET(ln)      LED_PORT &= ~(_BV(ln+LED_SHIFT) & LED_MASK)
# endif
#endif /* !defined(LED_SET)*/


#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Switch the LED with the number  @e ln OFF. */
# define LED_CLR(ln) do{}while(0)
#elif !defined(LED_CLR)
# if LEDS_INVERSE == 0
#  define LED_CLR(ln)      LED_PORT &= ~(_BV(ln+LED_SHIFT) & LED_MASK)
# else
#  define LED_CLR(ln)      LED_PORT |= (_BV(ln+LED_SHIFT) & LED_MASK)
# endif
#endif /* !defined(LED_CLR)*/

#if defined(NO_LEDS) || defined (DOXYGEN)
  /** ..... */
# define LED_VAL(msk,val) do{}while(0)
#elif !defined(LED_VAL)
# if LEDS_INVERSE == 0
#  define LED_VAL(msk,val) LED_PORT |= ((LED_MASK|msk) << LED_SHIFT); \
                           LED_PORT |= ~((val << LED_SHIFT)& (LED_MASK|(msk<<LED_SHIFT)) )
# else
#  define LED_VAL(msk,val) LED_PORT &= ~(LED_MASK|(msk<<LED_SHIFT)); LED_PORT |= ~(val & (LED_MASK|msk))

# endif
#endif /* !defined(LED_VAL)*/

#if defined(NO_LEDS) || defined (DOXYGEN)
  /** Toggle the LED with the number  @e n. */
# define LED_TOGGLE(ln) do{}while(0)
#elif !defined(LED_TOGGLE)
# define LED_TOGGLE(ln) LED_PORT ^= (_BV(ln+LED_SHIFT) & LED_MASK)
#endif /* !defined(LED_TOGGLE)*/

/** Maximum value, that can be displayed on the LEDs */
#define LED_MAX_VALUE ((1<<LED_NUMBER)-1)

/** @} */


/* === KEY Handling === */

/** 
 * @addtogroup grpIoUtilLedKey
 * @{
 */
#if defined(NO_KEYS) || defined (DOXYGEN)
  /** Initialisation of the KEY port */
# define KEY_INIT()

  /** Reading of the KEY port directly and return the value LSB aligbed. */
# define KEY_GET() (0)

#else /* defined(NO_KEYS) || defined (DOXYGEN) */
# if PULLUP_KEYS != 0
#  define PULL_MASK (MASK_KEY)
# else /* PULLUP_KEYS != 0 */
#  define PULL_MASK (0)
# endif /* PULLUP_KEYS != 0 */
# if !defined KEY_INIT
#  define KEY_INIT()  do{PORT_KEY |= PULL_MASK; DDR_KEY &= ~(MASK_KEY); }while(0)
# endif /*!defined KEY_INIT()*/
# if !defined KEY_GET
# if INVERSE_KEYS == 0
#  define KEY_GET()\
                ((PIN_KEY & MASK_KEY) >> SHIFT_KEY)
# else /* INVERSE_KEYS == 0 */
#  define KEY_GET()\
                ((~PIN_KEY & MASK_KEY) >> SHIFT_KEY)
# endif /* INVERSE_KEYS == 0 */
# endif /* !defined KEY_GET */
#endif /* defined(NO_KEYS) || defined (DOXYGEN) */



/**
 * @brief Debounce Key values, returned from the macro KEY_GET()
 * @return status ot the debounced key
 */
static inline uint8_t keys_debounced(void)
{
  static uint8_t key_state;        // debounced and inverted key state:
  static uint8_t ct0, ct1;      // holds two bit counter for each key
  uint8_t i;


  /*
   * read current state of keys (active-low),
   * clear corresponding bit in i when key has changed
   */
  i = key_state ^ KEY_GET();   // key changed ?

  /*
   * ct0 and ct1 form a two bit counter for each key,
   * where ct0 holds LSB and ct1 holds MSB
   * After a key is pressed longer than four times the
   * sampling period, the corresponding bit in key_state is set
   */
  ct0 = ~( ct0 & i );            // reset or count ct0
  ct1 = (ct0 ^ ct1) & i;        // reset or count ct1
  i &= ct0 & ct1;                // count until roll over ?
  key_state ^= i;                // then toggle debounced state

  /*
   * To notify main program of pressed key, the correspondig bit
   * in global variable key_press is set.
   * The main loop needs to clear this bit
   */
  return key_state & i;    // 0->1: key press detect

}

/**
 * @brief Debounce Key values, returned from the macro KEY_GET()
 * @return status ot the debounced key
 */
static inline void trap_if_key_pressed(void)
{

    KEY_INIT();
    DELAY_MS(10);
    if (KEY_GET())
    {
        LED_INIT();
        while(1)
        {
            DELAY_MS(400);
            LED_SET(0);
            DELAY_MS(10);
            LED_CLR(0);
        }
    }
}

/** @} */

/* === Bootloader Interface === */
#if BOOTLOADER_ADDRESS != 0

#define JUMP_BOOT_LOADER() \
    do {\
        void (*funcptr)( uint8_t flag ) = BOOTLOADER_ADDRESS;\
        funcptr(42);\
    }while(0)
#else /* BOOTLOADER_ADDRESS != 0 */
#define JUMP_BOOT_LOADER()
#endif /* BOOTLOADER_ADDRESS != 0 */




/* === Prototypes ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup grpIoUtilBuffer */
/** @{ */

#if 0
#define BUFFER_SET_USED(b) do{ATOMIC_BLOCK(ATOMIC_FORCEON){b->used|=1}}while(0)
#define BUFFER_SET_UNUSED(b) do{ATOMIC_BLOCK(ATOMIC_FORCEON){b->used&=~1}}while(0)
#define BUFFER_IS_USED(b) ((b->used&1)!=0)

#define BUFFER_SET_LOCK(b) do{ATOMIC_BLOCK(ATOMIC_FORCEON){b->used|=2}}while(0)
#define BUFFER_SET_UNLOCK(b) do{ATOMIC_BLOCK(ATOMIC_FORCEON){b->used&=~2}}while(0)
#define BUFFER_IS_LOCKED(b) ((b->used&2)!=0)
#endif

#define BUFFER_SET_USED(b) do{b->used|=1;}while(0)
#define BUFFER_SET_UNUSED(b) do{b->used&=~1;}while(0)
#define BUFFER_IS_USED(b) ((b->used&1)!=0)

#define BUFFER_SET_LOCK(b) do{b->used|=2;}while(0)
#define BUFFER_SET_UNLOCK(b) do{b->used&=~2;}while(0)
#define BUFFER_IS_LOCKED(b) ((b->used&2)!=0)


#define BUFFER_SIZE(b) (b->iend - b->istart)
#define BUFFER_PDATA(b) (b->data + b->istart)
#define BUFFER_SEEK(b,offset) (b->data + (b->iend=offset))

#define BUFFER_GET_MEMBLOCK(b,pmem,size) \
    do{\
        b->used = 1;\
        pmem = (b->data + b->iend);\
        size = (b->len - b->iend);\
    }while(0)

#define BUFFER_UPDATE_MEMBLOCK(b,end) \
    do{\
        b->iend = end;\
        b->used = 0;\
    }while(0);

#define BUFFER_LAST_CHAR(b) \
    (b->iend <= b->istart) ? EOF : (char)b->data[b->iend-1]
#define BUFFER_FREE_AT_END(b) (b->len - b->iend)
#define BUFFER_FREE_AT_START(b) (b->istart)
#define BUFFER_ELSZ(x) (sizeof(buffer_t) + (x))
#define BUFFER_RESET(b,start) do{ b->iend = b->istart = start;}while(0)
#define BUFFER_ADVANCE(b,more) do{ b->istart += more;}while(0)

/** format a chunk of memory as buffer_t structure */
buffer_t * buffer_init(void * pmem, uint8_t size, uint8_t start);
/** append a char at the end of a buffer */
int buffer_append_char(buffer_t *b, uint8_t c);
/** prepend a char at the start of a buffer */
int buffer_prepend_char(buffer_t *b, int c);
/** read a char from the start of a buffer */
int buffer_get_char(buffer_t *b);
/** append a data block at the end of a buffer */
uint8_t buffer_append_block(buffer_t *b, void *pdata, uint8_t size);
/** prepend a data block at the start of a buffer */
uint8_t buffer_prepend_block(buffer_t *b, void *pdata, uint8_t size);
/** read a datablock from the start of a buffer */
uint8_t buffer_get_block(buffer_t *b, void *pdata, uint8_t size);


buffer_pool_t * buffer_pool_init(uint8_t *pmem, size_t memsz, uint8_t bsz);
buffer_t * buffer_alloc(buffer_pool_t *ppool, uint8_t istart);
void buffer_free(buffer_t * pbuf);



/* buffer stream function */
//typedef void (*incb)(buffer_t *pbuf) buffer_stream_incb_t;
//typedef void (*outcb)(buffer_t *pbuf) buffer_stream_outcb_t;

typedef struct
{
    FILE bstream;
    buffer_t *pbufin;
    buffer_t *pbufout;
    void (*incb)(buffer_t *pbuf);
    void (*outcb)(buffer_t *pbuf);
} buffer_stream_t;



int buffer_stream_init( buffer_stream_t *pbs,
                        void (*incb)(buffer_t *pbuf),
                        void (*outcb)(buffer_t *pbuf));

int buffer_stream_putchar(char c,FILE *f);
int buffer_stream_getchar(FILE *f);



/** @} */
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  /* #ifndef IOUTIL_H */
/* EOF */
