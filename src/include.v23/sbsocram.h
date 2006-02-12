/*
 * BCM47XX Sonics SiliconBackplane embedded ram core
 *
 * Copyright 2004, Broadcom Corporation      
 * All Rights Reserved.      
 *       
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 *
 * $Id: sbsocram.h,v 1.1.1.1 2004/08/26 06:56:18 honor Exp $
 */

#ifndef	_SBSOCRAM_H
#define	_SBSOCRAM_H

#define	SOCRAM_MEMSIZE		0x00
#define	SOCRAM_BISTSTAT		0x0c


#ifndef _LANGUAGE_ASSEMBLY

/* Memcsocram core registers */
typedef volatile struct sbsocramregs {
	uint32	memsize;
	uint32	biststat;
} sbsocramregs_t;

#endif

/* Them memory size is 2 to the power of the following
 * base added to the contents of the memsize register.
 */
#define SOCRAM_MEMSIZE_BASESHIFT 16

#endif	/* _SBSOCRAM_H */
