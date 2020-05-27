/******************************************************************************
 *
 * Name:	skspilole.c
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Purpose:	Contains Low Level Functions for the integration of the skspi.c module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	Driver for Marvell Yukon/2 chipset and SysKonnect Gigabit Ethernet
 *	Server Adapters.
 *
 *	    Address all question to: support@marvell.com
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 *****************************************************************************/

static const char SysKonnectFileId[] =
	"@(#) $Id: skgespilole.c,v 1.1.2.2 2007/11/20 10:22:20 mlindner Exp $ (C) Marvell.";

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

int spi_timer(SK_AC *pAC, unsigned int t)
{
	if (t) {
		pAC->TimeBuf = SkOsGetTime(pAC)+(SK_U64)t*SK_TICKS_PER_SEC;
	} else {
		if (pAC->TimeBuf <= SkOsGetTime(pAC)) {
			return(1);
		}
	}

	return(0);
}

/*  dummies */
void fl_print(char *msg, ...)
{
}

unsigned char *spi_malloc( unsigned short size )
{
	return(kmalloc(size,GFP_KERNEL));
}

void spi_free( unsigned char *buf )
{
	kfree(buf);
}

