//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
/**********************************************************************
 *      Copyright (c) 1999 Delphi Communication Systems
 *      Maynard, MA.     ALL RIGHTS RESERVED
 ***********************************************************************/
/**********************************************************************
 * File:
 *	$RCSfile: types.h,v $
 *	$Revision: #3 $
 *	$Date: 2005/08/09 $
 *
 * Purpose:
 *      This file defines basic types used in the ITU-T G.729A Speech
 *      codec.  These are defined here so that we may control
 *      how many bits of precision a type has on a particular
 *      platform.
 *
 * Operation:
 *	We define the following in this file:
 *
 *      typedef ... INT16
 *            This type definition defines the data type used for
 *            variables that must hold exactly 16 bits (signed).
 *
 *      typedef ... INT32
 *            This type definition defines the data type used for
 *            variables that must hold exactly 32 bits (signed).
 *
 * Notes/Issues:
 *      This file is correct for the following platforms (so far):
 *
 *            GNUWIN32 compiled with GCC
 *
 * $Log: types.h,v $
 * Revision 1.1.1.2  2002/03/14 17:54:24  pfine
 * Fixed CR/LF Problem
 *
 * Revision 1.1.1.1  2002/03/13 18:20:24  pfine
 * DCS Ecos with Device Drivers
 *
 *
 ***********************************************************************/
#ifndef TYPES_H
#define TYPES_H

typedef	char	        INT8;
typedef unsigned char 	UINT8;	
typedef	short		INT16;
typedef	unsigned short	UINT16;
typedef	long	   	INT32; 
typedef unsigned long 	UINT32;	

typedef	volatile char	        VINT8;
typedef volatile unsigned char 	VUINT8;	
typedef	volatile short		VINT16;
typedef	volatile unsigned short	VUINT16;
typedef	volatile long	   	VINT32; 
typedef volatile unsigned long 	VUINT32;	

typedef char            OCTET;
typedef int             INT_NATIVE;
typedef unsigned int    UINT_NATIVE;

#endif /* TYPES_H */




