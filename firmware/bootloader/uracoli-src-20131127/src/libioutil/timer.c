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

/* === includes ========================================== */
#include "board.h"
#include "timer.h"
#include <string.h>

/* === types ============================================= */
#ifndef NO_TIMER
/**
 * Data type for timer entries.
 */
typedef struct timer_tag
{
    /** reference number of the timer */
    timer_hdl_t hdl;
    /** value, when the timer expires in system ticks */
    time_t expire;
    /** pointer to a timer handler function */
    timer_handler_t *func;
    /** pointer to a argument, which is given to the
     * timer handler function */
    timer_arg_t arg;
    /** pointer to the next timer in the timer queue.
     */
    struct timer_tag * next;
} timer_t;


/* === globals =========================================== */
#if TIMER_POOL_SIZE != 0
  timer_t timer_pool[TIMER_POOL_SIZE];
#else
# error "Malloc Timer not yet implemented"
#endif

timer_t *tqhead;
volatile time_t   systime;
volatile time_t   timebase;
timer_hdl_t tmrhdl = 1;


/* === internal functions ================================== */
timer_t * tmr_create(time_t expire,  timer_handler_t func)
{
timer_t *ret = NULL;

#if ! defined(HAVE_MALLOC_TIMERS)
uint8_t i;
    for(i=0;i<TIMER_POOL_SIZE;i++)
    {
        if(timer_pool[i].func == NULL)
        {
            timer_pool[i].func = func;
            timer_pool[i].expire = expire;
            ret = &timer_pool[i];
            break;
        }
    }
#else
# error "Malloc Timer not implemented"
#endif
    return ret;
}


void tmr_insert(timer_t *tmr)
{
timer_t *prev, *next;

#if 1
    /* believe it or not, this cost 12 byte less memory */
    tmr->next = NULL;
    if(tqhead == NULL)
    {
        tqhead = tmr;
    }
    else
    {
        for(prev = NULL, next = tqhead;
            next != NULL;
            prev = next, next = next->next)
        {
            if(next->expire > tmr->expire)
            {
                tmr->next = next;
                if (prev)
                {
                    prev->next = tmr;
                }
                else
                {
                    tqhead = tmr;
                }
                tmr = NULL;
                break;
            }
        }
        /* no insert, append tmr to tail */
        if (tmr && prev)
        {
            prev->next = tmr;
        }
    }

#else
    for(prev = NULL, next = tqhead;
        next != NULL;
        prev = next, next = next->next)
    {
        if(next->expire > tmr->expire)
        {
            tmr->next = next;
            if (prev)
            {
                prev->next = tmr;
            }
            else
            {
                tqhead = tmr;
            }
            tmr = NULL;
            break;
        }
    }
    /* no insert, append tmr to tail */
    if (tmr)
    {
        if (prev)
        {
            prev->next = tmr;
        }
        else
        {
            tqhead = tmr;
        }
        tmr->next = NULL;
    }
#endif
}

timer_t * tmr_queue_find_delete(timer_hdl_t th)
{
timer_t *ret = NULL;
timer_t *prev, *next;
#if 1
    if(tqhead)
    {
        for(prev = NULL, next = tqhead;
            next != NULL;
            prev = next, next = next->next)
        {
            if(next->hdl == th)
            {
                ret = next;
                if(prev)
                {
                    prev->next = next->next;
                }
                else
                {
                    tqhead = next->next;
                }

            }
        }
    }
#else
    if(tqhead)
    {
        for(prev = tqhead, next = tqhead;
            next != NULL && prev != NULL;
            prev = next, next = next->next)
        {
            if(next->hdl == th)
            {
                ret = next;
                prev->next = next->next;
            }
        }
    }
#endif

    return ret;
}

void tmr_process(void)
{
timer_t *prev, *next;
time_t trestart;
    for(prev = NULL, next = tqhead;
        next != NULL;
        prev = next, next = next->next)
    {
        if(next->expire <= systime)
        {
            if (prev)
            {
                prev->next = next->next;
            }
            else
            {
                tqhead = next->next;
            }
            trestart = next->func(next->arg);
            if (trestart != 0)
            {
                next->expire = systime + trestart;
                tmr_insert(next);
            }
            else
            {
                /* no restart needed, remove timer from queue*/
#if ! defined(HAVE_MALLOC_TIMERS)
                next->hdl = NONE_TIMER;
                next->func = NULL;
                next->next = NULL;
#else
# error "Malloc Timer not implemented"
#endif
            }
        }
        else
        {
            break;
        }
    }

}


ISR(TIMER_IRQ_vect)
{
    systime++;
    tmr_process();
}

/* === interface functions ================================= */

/**
* @brief Initialization of the timer module
*/
void timer_init(void)
{
    TIMER_INIT();
#   ifdef HAVE_MALLOC_TIMERS
    memset(timer_pool, 0,
           sizeof(timer_t)*TIMER_POOL_SIZE);
#   endif
    tqhead = NULL;
    systime = 0;
}


timer_hdl_t timer_start(timer_handler_t *thfunc, time_t duration,
                        timer_arg_t arg)
{
timer_t *tmr;
timer_hdl_t ret = NONE_TIMER;
   tmr = tmr_create(duration + systime,  thfunc);
   if (tmr != NULL)
   {
       ret = tmr->hdl = tmrhdl ++;
       tmr->arg = arg;
       tmr_insert(tmr);
   }
   return ret;
}

timer_hdl_t timer_restart(timer_hdl_t th, time_t duration)
{
timer_hdl_t ret = NONE_TIMER;
timer_t * tmr;
time_t tmr_exp;
    tmr_exp = duration + systime;
    tmr = tmr_queue_find_delete(th);
    if(tmr)
    {
        tmr->expire = tmr_exp;
        tmr_insert(tmr);
        ret = tmr->hdl;
    }
    return ret;

}

timer_hdl_t timer_stop(timer_hdl_t th)
{
timer_hdl_t ret = NONE_TIMER;
timer_t * tmr;

    tmr = tmr_queue_find_delete(th);
    if(tmr)
    {
        ret = tmr->hdl;
        tmr->hdl = NONE_TIMER;
        tmr->func = NULL;

    }
    return ret;
}

time_t timer_systime(void)
{
   return systime;
}


void timer_set_systime(time_t sec)
{
   cli();
   HWTIMER_REG = 0;
   systime = 0;
   timebase = sec;
   sei();
}

#endif /*ifndef NO_TIMER*/
