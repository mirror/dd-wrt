/*
 * HND SiliconBackplane chipcommon support.
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: hndchipc.h,v 1.1.1.1 2006/02/27 03:43:16 honor Exp $
 */

#ifndef _hndchipc_h_
#define _hndchipc_h_


#if defined(mips)
#include <hndmips.h>
#endif

#if defined(__ARM_ARCH_4T__)
#include <hndarm.h>
#endif

extern void sb_serial_init(sb_t *sbh, void (*add)(void *regs, uint irq, uint baud_base,
                                                           uint reg_shift));

extern void *sb_jtagm_init(sb_t *sbh, uint clkd, bool exttap);
extern void sb_jtagm_disable(osl_t *osh, void *h);
extern uint32 jtag_rwreg(osl_t *osh, void *h, uint32 ir, uint32 dr);

#endif /* _hndchipc_h_ */
