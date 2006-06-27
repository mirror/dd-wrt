/*
 * HND SiliconBackplane Gigabit Ethernet core software interface.
 *
 * Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: hndgige.h,v 1.1.1.1 2006/02/27 03:43:16 honor Exp $
 */

#ifndef _hndgige_h_
#define _hndgige_h_

extern void sb_gige_init(sb_t *sbh, void *regs, bool *rgmii);

#endif /* _hndgige_h_ */
