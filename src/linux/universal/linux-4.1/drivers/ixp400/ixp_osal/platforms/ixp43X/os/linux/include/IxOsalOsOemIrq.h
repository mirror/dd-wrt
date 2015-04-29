/**
 * @file IxOsalOsOemIrq.h
 *
 * @brief Defining IXP435 IRQ vector name in an array for retrieval by private
 * function ixOsalGetIrqNameByVector implemented in IxOsalOsServices.c.
 *
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalOsOemIrq_H
#define IxOsalOsOemIrq_H

/*
 * IRQ name definition.
 */
const char *irq_name[] = {
    "IXP4XX NPE-A",	/* IRQ 0 */
    "IXP4XX RESV01",
    "IXP4XX NPE-C",
    "IXP4XX QM1",
    "IXP4XX QM2",
    "IXP4XX TIMER1",
    "IXP4XX GPIO0",
    "IXP4XX GPIO1",
    "IXP4XX PCI",
    "IXP4XX PCI DMA1",
    "IXP4XX PCI DMA2",	/* IRQ 10 */
    "IXP4XX TIMER2",
    "IXP4XX RESV12",
    "IXP4XX RESV13",
    "IXP4XX TIMESTAMP",
    "IXP4XX UART1",
    "IXP4XX WATCHDOG",
    "IXP4XX AHB PMU",
    "IXP4XX XSCALE PMU",
    "IXP4XX GPIO2",
    "IXP4XX GPIO3",	/* IRQ 20 */
    "IXP4XX GPIO4",
    "IXP4XX GPIO5",
    "IXP4XX GPIO6",
    "IXP4XX GPIO7",
    "IXP4XX GPIO8",
    "IXP4XX GPIO9",
    "IXP4XX GPIO10",
    "IXP4XX GPIO11",
    "IXP4XX GPIO12",
    "IXP4XX SW1",	/* IRQ 30 */
    "IXP4XX SW2",
    "IXP4XX USB HOST0",
    "IXP4XX USB HOST1",
    "IXP4XX SPP",
    "IXP4XX RESV35",
    "IXP4XX RESV36",
    "IXP4XX RESV37",
    "IXP4XX RESV38",
    "IXP4XX RESV39",
    "IXP4XX RESV40",	/* IRQ 40 */
    "IXP4XX RESV41",
    "IXP4XX RESV42",
    "IXP4XX RESV43",
    "IXP4XX RESV44",
    "IXP4XX RESV45",
    "IXP4XX RESV46",
    "IXP4XX RESV47",
    "IXP4XX RESV48",
    "IXP4XX RESV49",
    "IXP4XX RESV50",	/* IRQ 50 */
    "IXP4XX RESV51",
    "IXP4XX RESV52",
    "IXP4XX RESV53",
    "IXP4XX RESV54",
    "IXP4XX RESV55",
    "IXP4XX RESV56",
    "IXP4XX RESV57",
    "IXP4XX RESV58",
    "IXP4XX RESV59",
    "IXP4XX QM PE",	/* IRQ 60 */
    "IXP4XX MCU ECC",
    "IXP4XX RESV62",
    "IXP4XX RESV63"
};

/*
 * String to return when the vector number is invalid.
 */
const char *invalid_irq_name = "Invalid IXP43X IRQ";

#endif /* IxOsalOsOemIrq_H */
