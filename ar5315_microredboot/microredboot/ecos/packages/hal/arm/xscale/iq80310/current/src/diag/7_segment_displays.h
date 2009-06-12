//=============================================================================
//
//      7_segment_displays.h
//
//      Definitions for IQ80310 7-segment display.      
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
// Date:        2001-01-02
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/


/* Addresses of the 7-segment displays registers */

/* 08/25/00 jwf */
/* iq80310 address decode */
#define MSB_DISPLAY_REG		(volatile unsigned char *)0xfe840000	/* 7 segment 0 */
#define LSB_DISPLAY_REG		(volatile unsigned char *)0xfe850000	/* 7 segment 1 */

/* Values for the 7-segment displays */
#define DISPLAY_OFF	0xFF
#define ZERO		0xC0
#define ONE		0xF9
#define TWO		0xA4
#define THREE		0xB0
#define FOUR		0x99
#define FIVE		0x92
#define SIX		0x82
#define SEVEN		0xF8
#define EIGHT		0x80
#define NINE		0x90
#define LETTER_A	0x88
#define LETTER_B	0x83
#define LETTER_C	0xC6
#define LETTER_D	0xA1
#define LETTER_E	0x86
#define LETTER_F	0x8E
#define	LETTER_I	0xCF
#define LETTER_L	0xC7
#define LETTER_P	0x8C
#define LETTER_S	0x92
#define DECIMAL_POINT	0x7F
#define DISPLAY_ERROR	0x06  /* Displays "E." */

/* Parameters for functions */
#define	MSB				0
#define	LSB				1
#define	BOTH				2

extern const unsigned char SevSegDecode[];




