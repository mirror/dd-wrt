/*
 * HNDRTE ARM run-time initialization.
 *
 * This file assumes that we are in RAM, mapped at address 0,
 * so it is for images that get downloaded and run via a write
 * to corecontrol to turn off the remapzero bit.
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: headarm.S,v 1.5 2007/03/06 03:00:47 Exp $
 */

#include <arminc.h>

#ifdef	CONFIG_XIP
#error	Use bootarm.S for XIP
#endif

#include "startarm.S"

	.text

	FUNC(inflrom)

	/* We don't do flash/ROM */
	TRACE(200)
	b	.

	END(inflrom)
