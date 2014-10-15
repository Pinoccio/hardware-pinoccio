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

/* === Includes ============================================================ */
#include "board.h"
#include "transceiver.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#if !defined(TRX_IF_RFA1)
/* === globals ============================================================= */
static trx_irq_handler_t pIrqHandler = 0;

/* === prototypes ========================================================== */

/* === functions =========================================================== */

/* === internal functions =================================================== */

/** IRQ Handler
 */
#if defined(DOXYGEN)
void TRX_IRQ_vect();
#else
ISR(TRX_IRQ_vect)
#endif
{
uint8_t cause;
    DI_TRX_IRQ();
    sei();
    cause = trx_reg_read(RG_IRQ_STATUS);
    if (pIrqHandler)
    {
        pIrqHandler(cause);
    }
    cli();
    EI_TRX_IRQ();
}

/* === external functions =================================================== */

void trx_set_irq_handler(trx_irq_handler_t irqhandler)
{
    pIrqHandler = irqhandler;
}

#endif /* #if !defined(TRX_IF_RFA1) */
/* EOF */
