/*
 * HNDRTE Trap handling.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: hndrte_trap.h,v 13.7 2007/06/19 20:09:10 Exp $
 */

#ifndef	_HNDRTE_TRAP_H
#define	_HNDRTE_TRAP_H


/* Trap handling */


#if defined(mips)
#include <hndrte_mipstrap.h>
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#include <hndrte_armtrap.h>
#endif


#ifndef	_LANGUAGE_ASSEMBLY

#include <typedefs.h>

extern uint32 hndrte_set_trap(uint32 hook);
extern void hndrte_die(uint32 line);

#endif	/* !_LANGUAGE_ASSEMBLY */
#endif	/* _HNDRTE_TRAP_H */
