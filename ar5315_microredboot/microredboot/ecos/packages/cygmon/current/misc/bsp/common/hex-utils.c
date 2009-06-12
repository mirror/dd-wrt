//==========================================================================
//
//      hex-utils.c
//
//      Utilities for dealing with hexadecimal strings.
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <bsp/hex-utils.h>

int
__hex(char ch)
{
    if ((ch >= 'a') && (ch <= 'f')) return (ch-'a'+10);
    if ((ch >= '0') && (ch <= '9')) return (ch-'0');
    if ((ch >= 'A') && (ch <= 'F')) return (ch-'A'+10);
    return (-1);
}


/*
 * Convert the hex data in 'buf' into 'count' bytes to be placed in 'mem'.
 * Returns a pointer to the character in mem AFTER the last byte written.
 */
char *
__unpack_bytes_to_mem(char *buf, char *mem, int count)
{
    int  i;
    char ch;

    for (i = 0; i < count; i++) {
	ch = __hex(*buf++) << 4;
	ch = ch + __hex(*buf++);
	*mem++ = ch;
    }
    return(mem);
}

/*
 * While finding valid hex chars, build an unsigned long int.
 * Return number of hex chars processed.
 */
int
__unpack_ulong(char **ptr, unsigned long *val)
{
    int numChars = 0;
    int hexValue;
    
    *val = 0;

    while (**ptr) {
	hexValue = __hex(**ptr);
	if (hexValue >= 0) {
	    *val = (*val << 4) | hexValue;
	    numChars ++;
	} else
	    break;
	(*ptr)++;
    }
    return (numChars);
}


/*
 * Unpack 'count' hex characters, forming them into a binary value.
 * Return that value as an int. Adjust the source pointer accordingly.
 */
int
__unpack_nibbles(char **ptr, int count)
{
    int value = 0;

    while (--count >= 0) {
	value = (value << 4) | __hex(**ptr);
	(*ptr)++;
    }
    return value;
}


