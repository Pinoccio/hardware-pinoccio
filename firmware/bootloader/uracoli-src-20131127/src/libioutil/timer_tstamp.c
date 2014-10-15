/* Copyright (c) 2008 Axel Wachtler
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
 * @brief ....
 * @_addtogroup grpApp...
 */


/* === includes ============================================================ */
#include "board.h"
#include "timer.h"
#ifndef NO_TIMER

/* === macros ============================================================== */

/* === types =============================================================== */

/* === globals ============================================================= */
extern time_t systime;
extern time_t timebase;
/* === prototypes ========================================================== */

/* === functions =========================================================== */
#define US_2_SEC (1000000UL)
void timer_get_tstamp(time_stamp_t *ts)
{

#if 0
    hwticks = HWTIMER_REG;
    hwticks += (systime * HWTIMER_TICK_NB);
    /* could be expensive */
    ts->time_sec = hwticks / US_2_SEC;
    ts->time_usec = hwticks - (ts->time_sec * US_2_SEC);
    ts->time_sec += timebase;
#else
    /* FIXME: rename structure members to ticks and systicks */
    ts->time_usec = HWTIMER_REG;
    ts->time_sec = systime;
#endif
}
#endif /*ifndef NO_TIMER*/
