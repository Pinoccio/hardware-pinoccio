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

/*$Id$ */
/**
 * @file
 * @brief Interface for @ref grpTimer.
 *
 * This module is inspired by Jörg Wunschs timer implementation,
 * which can be found here: http://sax.sax.de/~joerg/avr-timer/
 *
 */
#ifndef TIMER_H
#define TIMER_H
/* === includes ============================================================ */
#include <stdint.h>

/* === macros ============================================================== */
/** @addtogroup grpTimer
 *  @{
 */
/** Macro that converts the millisecond value v into TIMER_IRQ_vect ticts */
#define MSEC(v) ((time_t)(v / (1.0e3 * TIMER_TICK)))
/** Macro that converts the microsecond value v into TIMER_IRQ_vect ticts */
#define USEC(v) ((time_t)(v / (1.0e6 * TIMER_TICK)))



/* === types =============================================================== */

/* === prototypes ========================================================== */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data type for time values (measured in number of system ticks).
 */
typedef uint32_t time_t;


/**
 * Hi resolution time stamp in seconds and micro seconds
 */
typedef struct time_stamp_tag
{
    time_t  time_sec;
    time_t  time_usec;
} time_stamp_t;


/**
 * Data type for the argument of a timer handler function.
 */
typedef uint32_t timer_arg_t;

/**
 * Data type for a timer handle
 * (a reference number to identify a running timer).
 */
typedef uint16_t timer_hdl_t;

/**
 * Data type for timer expiration action function.
 * This function is called, when the expiration time is over.
 * When luanched, the function is called with a parameter p of
 * type .@ref timer_arg_t. If the function returns a value,
 * which is greate then 0, the timer is restarted again.
 */
typedef time_t (timer_handler_t)(timer_arg_t p);

/** Symbolic name for invalid timer handle */
#define NONE_TIMER (0)

/* === Prototypes ================================ */
/**
 * @brief Initialization of the timer module
 */
void timer_init(void);

/**
 * @brief Start a timer with a given handler function.
 *
 * This function initially creates a timer and assigns a timer
 * handle to it. The timer handle is reference number, which
 * identifies the timer uniquely and is needed for restart and
 * stop a running timer.
 *
 * @param thfunc  pointer to a function, which is called when the
 *                timer expires.
 * @param duration  time in system ticks from now, when the timer
 *                  expires.
 * @param arg  argument, which is passed to the timer function.
 *
 * @return the value of @ref NONE_TIMER if the timer could not
 *         be started. Otherwise a handle != NONE_TIMER, which
 *         is needed for restarting and stopping the timer.
 */
timer_hdl_t timer_start(timer_handler_t *thfunc, time_t duration,
                        timer_arg_t arg);


/**
 * @brief Restarting a running timer.
 *
 * If the timer is found in the timer queue, then it is
 * rstarted with the new duration value.
 *
 * @param th  Handle of the timer. The timer needs to exist
 *            in the timer queue, e.g. it is started with
 *            @ref timer_start() and not yet expired.
 * @param duration time in system ticks from now, when the timer
 *                 expires.
 * @return the value of @ref NONE_TIMER if the timer could not
 *         be found in the timer queue.
 *         Otherwise the value of th.
 */
timer_hdl_t timer_restart(timer_hdl_t th, time_t duration);

/**
 * @brief Stop a running timer
 *
 * @param th  Handle of the timer. The timer needs to exist
 *            in the timer queue, e.g. it is started with
 *            @ref timer_start() and not yet expired.
 * @return the value of @ref NONE_TIMER if the timer could not
 *         be found in the timer queue.
 *         Otherwise the value of th.
 */
timer_hdl_t timer_stop(timer_hdl_t th);

/**
 * @brief Return the current system time in ticks.
 */
time_t timer_systime(void);


/**
 * @brief Set the current system time given in seconds since 1.1.1970.
 */
void timer_set_systime(time_t sec);

/**
 * Function that returns the internal system time
 * counters as "libpcap" compatible time stamp.
 *
 * @note This routine takes ~548 cycles for execution.
 * In case of a 8Mhz driven system, this is a 68.5 us.
 *
 * @param ts timestamp data structure
 */
void timer_get_tstamp(time_stamp_t *ts);

#ifdef __cplusplus
} /* extern "C" */
#endif

/** @} */

#endif /* TIMER_H */
