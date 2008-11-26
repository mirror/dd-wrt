/******************************************************************************
 *
 * Name:	skspilole.c
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: 1.1.2.1 $
 * Date:	$Date: 2006/08/28 09:06:52 $
 * Purpose:	Contains Low Level Functions for the integration of the skspi.c module
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect
 *	(C)Copyright 2002-2003 Marvell
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SYSKONNECT
 *	The copyright notice above does not evidence any
 *	actual or intended publication of such source code.
 *
 *	This Module contains Proprietary Information of SysKonnect
 *	and should be treated as Confidential.
 *
 *	The information in this file is provided for the exclusive use of
 *	the licensees of SysKonnect.
 *	Such users have the right to use, modify, and incorporate this code
 *	into products for purposes authorized by the license agreement
 *	provided they include this notice and the associated copyright notice
 *	with any such product.
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

static const char SysKonnectFileId[] =
	"@(#) $Id: skgespilole.c,v 1.1.2.1 2006/08/28 09:06:52 mlindner Exp $ (C) Marvell.";

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skgespi.h"


SK_AC *lpAC;
static  SK_U32 timebuf;

int fl_type;
/*
 * global vars
 */
long max_pages = 0;
long max_faddr = 0;


/* low level SPI programming interface */

void spi_init_pac( SK_AC *pAC )  {
	lpAC = pAC;
}

void spi_in8(unsigned short offs, unsigned char *val )  {
	SK_IN8( lpAC->IoBase, offs, val ); 
}

void spi_in16(unsigned short offs, unsigned short *val ){
	SK_IN16( lpAC->IoBase, offs, val ); 
}

void spi_in32(unsigned short offs, unsigned long *val ){
	SK_IN32( lpAC->IoBase, offs, val ); 
}

void spi_out8(unsigned short offs, unsigned char val ){
	SK_OUT8( lpAC->IoBase, offs, val ); 
}

void spi_out16(unsigned short offs, unsigned short val ){
	SK_OUT16( lpAC->IoBase, offs, val ); 
}

void spi_out32(unsigned short offs, unsigned long val ){
	SK_OUT32( lpAC->IoBase, offs, val ); 
}

int  spi_timer(unsigned int t){
	if(t)
	{
		timebuf = (SK_U32)SkOsGetTime(lpAC)+(SK_U32)t*SK_TICKS_PER_SEC ; 
	} else
	{
		if((timebuf <= (SK_U32)SkOsGetTime(lpAC)))
		{
			return(1); 
		}
	}
	return(0); 
}

/*  dummies */
void fl_print(char *msg, ...) {
}

unsigned char *spi_malloc( unsigned short size )  {
	return( kmalloc(size,GFP_KERNEL) );
}

void spi_free( unsigned char *buf )  {
	kfree(buf);
}

