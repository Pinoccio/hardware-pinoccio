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
 * @brief Interface for @ref grpHIF.
 *
 * @ingroup grpHIF
 */
#ifndef HIF_H
#define HIF_H


/* === types =========================================== */
#include <stdarg.h>
#include <avr/pgmspace.h>
/** @addtogroup grpHIF
 *  @{
 */

#define FLASH_STRING_T  PGM_P
#define FLASH_STRING(x) PSTR(x)

#if HIF_TYPE != HIF_NONE || defined DOXYGEN
  /** Wrapper macro for hif_printf() */
# define PRINTF(fmt, ...) hif_printf(FLASH_STRING(fmt), __VA_ARGS__)
  /** Wrapper macro for hif_echo() */
# define PRINT(fmt) hif_echo(FLASH_STRING(fmt))
  /** Wrapper macro for hif_dump() */
# define DUMP(sz,ptr) hif_dump(sz,ptr)
# define HIF_PUTS_NEWLINE() hif_puts_p(FLASH_STRING("\n\r"))
#else
# define hif_init(br)
# define PRINTF(fmt, ...)
# define PRINT(fmt)
# define DUMP(sz,ptr)
# define HIF_PUTS_NEWLINE()
#endif


/* === Prototypes ====================================== */

#if HIF_TYPE != HIF_NONE || defined DOXYGEN
/**
 * @brief Initialize host interface
 *
 * @param baudrate  data rate of the interface in bit/s
 */
void hif_init(const uint32_t baudrate);
#endif

/**
 * @brief Send a programm memory string to the interface.
 *
 * @param progmem_s  pointer to a null terminated  string,
 *           which is located in program memory.
 */
void hif_puts_p(const char *progmem_s);

/**
 * @brief Send string to the interface.
 *
 * @param s  pointer to a null terminated  string,
 *           which is located in RAM.
 */
void hif_puts(const char *s );

/**
 * @brief Send a block of characters to the interface.
 *
 * @param data  pointer to the data array.
 * @param size  size of the block.
 * @return num  number of bytes, which was send.
 */
uint8_t hif_put_blk(unsigned char *data, uint8_t size);

/**
 * @brief Send a character to the interface.
 *
 * @param data  Character to send
 * @return The Character or EOF in case of error
 */
int hif_putc(int c);

/**
 * @brief Print a string to the interface.
 *
 * @param str  string, which is located in flash memory
 */
void hif_echo(FLASH_STRING_T str);

/**
 * @brief Print a formated string to the interface.
 *
 * @param fmt  format string, which is located in flash memory
 * @param ...  variable argument list
 */
void hif_printf(FLASH_STRING_T fmt, ...);

/**
 * @brief Print hexdump of a data array to the interface.
 *
 * @param sz  number of bytes, that will be dumped.
 * @param d   pointer to the data array, that will be dumped.
 */
void hif_dump(uint16_t sz, uint8_t *d);

/**
 * @brief Get a charakter byte from the host interface.
 *
 * @return The Character or EOF in case of error or end-of-file
 */
int hif_getc(void);

/**
 * @brief Get a block of bytes from the host interface.
 *
 * @param data     buffer where the bytes are stored
 * @param max_size maximum number of bytes, which can be stored in the buffer.
 * @return  number of bytes stored in the buffer
 */
uint8_t hif_get_blk(unsigned char *data, uint8_t max_size);


/**
 * @brief Split a null terminated string.
 *
 * This function creates argc,argv style data from a null
 * terminated string. The splitting is done on the base of
 * spaces (ASCII 32).
 *
 * @param  txtline  string to split
 * @param  maxargs  maximum number of arguments to split
 * @retval argv     array of pointers, that store the arguments
 * @return number of arguments splitted (argc)
 */
static inline int hif_split_args(char *txtline, int maxargs, char **argv)
{
uint8_t argc = 0, nextarg = 1;

    while((*txtline !=0) && (argc < maxargs))
    {
        if (*txtline == ' ')
        {
            *txtline = 0;
            nextarg = 1;
        }
        else
        {
            if(nextarg)
            {
                argv[argc] = txtline;
                argc++;
                nextarg = 0;
            }
        }
        txtline++;
    }

    return argc;
}

/**
 * @brief Read a decimal number with hif_getc()
 * @return integer value of the negative number.
 */
static inline int hif_get_dec_number(void)
{
    int rv = 0;
    int scale = 0;
    char c;


    while(1)
    {
        c = hif_getc();
        if (c == '\n' || c == '\r')
        {
            rv *= scale;
            break;
        }
        else if (c == '-' && scale == 0)
        {
            scale = -1;
        }
        else if ('0' <= c || c >= '9' )
        {
            if (scale == 0)
            {
                scale = 1;
            }
            rv = 10 * rv + (c - '0');
        }
        else if ('\b' <= c )
        {
            rv /= 10;
        }

    }
    return rv;
}

/** @} */
#endif /* HIF_H */
