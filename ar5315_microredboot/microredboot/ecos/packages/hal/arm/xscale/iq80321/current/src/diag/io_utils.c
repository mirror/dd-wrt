//=============================================================================
//
//      io_utils.c - Cyclone Diagnostics
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
// Contributors: Mark Salter
// Date:        2001-01-25
// Purpose:     
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

/*
 * i/o routines for tests.  Greg Ames, 9/17/90.
 *
 * Version: @(#)test_io.c	1.2 8/26/93
 */
#include <redboot.h>

/* Returns true if theChar is a valid hex digit, false if not */
int
diag_ishex(char theChar)
{
    switch(theChar) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'A':
      case 'a':
      case 'B':
      case 'b':
      case 'C':
      case 'c':
      case 'D':
      case 'd':
      case 'E':
      case 'e':
      case 'F':
      case 'f':
	return 1;
      default:
	return 0;
    }
}

/* Convert ascii code of hex digit to number (0-15) */
int
diag_hex2dec(char hex)
{
    if ((hex >= '0') && (hex <= '9'))
        return hex - '0';
    else if ((hex >= 'a') && (hex <= 'f'))
        return hex - 'a' + 10;
    else
        return hex - 'A' + 10;
}

/* Input a number as (at most 8) hex digits - returns value entered */
CYG_ADDRWORD
hexIn(void)
{
    char input[40], *p;
    CYG_ADDRWORD num;

    while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
	;

    for (num = 0, p = input; diag_ishex(*p); p++)
	num = num*16 + diag_hex2dec(*p);

    return num;
}


/* Input a number as decimal digits - returns value entered */
CYG_ADDRWORD
decIn(void)
{
    char input[40], *p;
    CYG_ADDRWORD num;

    while (_rb_gets(input, sizeof(input), 0) != _GETS_OK)
	;

    for (num = 0, p = input; '0' <= *p && *p <= '9'; p++)
	num = num*10 + (*p - '0');

    return num;
}

