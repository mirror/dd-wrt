/*
 * HND SiliconBackplane MIPS/ARM cores software interface.
 *
 * Copyright (C) 2009, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: hndcpu.h,v 13.10.42.1 2010/03/02 22:17:32 Exp $
 */

#ifndef _hndcpu_h_
#define _hndcpu_h_

#if defined(mips)
#include <hndmips.h>
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#include <hndarm.h>
#endif

extern uint si_irq(si_t *sih);
extern uint32 si_cpu_clock(si_t *sih);
extern uint32 si_mem_clock(si_t *sih);
extern void hnd_cpu_wait(si_t *sih);
extern void hnd_cpu_jumpto(void *addr);
extern void hnd_cpu_reset(si_t *sih);
extern void si_router_coma(si_t *sih, int reset, int delay);

#endif /* _hndcpu_h_ */
