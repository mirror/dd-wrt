//=============================================================================
//
//      i557_eep.h - Cyclone Diagnostics
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Scott Coulter, Jeff Frazier, Eric Breeden
// Contributors:
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/* Public define's and function prototypes */
#define EEPROM_SIZE			128	/* Maximum # bytes in serial eeprom */
#define EEPROM_WORD_SIZE	64	/* Maximum # shorts in serial eeprom */

/* result codes for the functions below */
#define OK						0	/* Operation completed successfully */
#define EEPROM_ERROR			1	/* generic error */
#define EEPROM_NOT_RESPONDING	2	/* eeprom not resp/not installed */
#define EEPROM_TO_SMALL			3	/* req write/read past end of eeprom */
#define EEPROM_INVALID_CMD      4       /* op code not supported */

/* layout of the Serial EEPROM register */
#define I557_EESK		(1 << 0)
#define I557_EECS		(1 << 1)
#define I557_EEDI		(1 << 2)
#define I557_EEDO		(1 << 3)

/* EEPROM commands */
#define EEPROM_WRITE	1
#define EEPROM_READ		2
#define EEPROM_ERASE	3
#define EEPROM_EWEN     4
#define EEPROM_EWDS		5
#define EEPROM_EWEN_OP  0x30
#define EEPROM_EWDS_OP 	0x00

/* EEPROM Chip Select */
#define SELECT_557_EEP(n)	(*(unsigned char *)(n+0x0e) |= I557_EECS)
#define DESELECT_557_EEP(n)	(*(unsigned char *)(n+0x0e) &= ~I557_EECS)

/* EEPROM Serial Clock */
#define SK_HIGH_557_EEP(n)	(*(unsigned char *)(n+0x0e) |= I557_EESK)
#define SK_LOW_557_EEP(n)	(*(unsigned char *)(n+0x0e) &= ~I557_EESK)

/* EEPROM Serial Data In -> out to eeprom */
#define EEDI_HIGH_557_EEP(n)	(*(unsigned char *)(n+0x0e) |= I557_EEDI)
#define EEDI_LOW_557_EEP(n)	(*(unsigned char *)(n+0x0e) &= ~I557_EEDI)

/* EEPROM Serial Data Out -> in from eeprom */
#define EEDO_557_EEP(n)	((*(unsigned char *)(n+0x0e) & I557_EEDO) >> 3)

/* global functions declared in serial_eep.c */

int eeprom_read (unsigned long pci_addr,
		 int eeprom_addr,	/* word offset from start of eeprom */
		 unsigned short *p_data,/* buffer pointer */
		 int nwords		/* number of bytes to read */
		 );
