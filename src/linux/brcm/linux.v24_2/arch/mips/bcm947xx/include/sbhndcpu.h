/*
 * HND SiliconBackplane MIPS/ARM hardware description.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#ifndef _sbhndcpu_h_
#define _sbhndcpu_h_

#if defined(mips)
#include <sbhndmips.h>
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#include <sbhndarm.h>
#endif

#endif /* _sbhndcpu_h_ */
