#ifndef __BSP_HEX_UTILS_H__
#define __BSP_HEX_UTILS_H__
//==========================================================================
//
//      hex-utils.h
//
//      Utilities for decoding hexadecimal encoded integers.
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
// Purpose:      Utilities for decoding hexadecimal encoded integers.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#ifndef __ASSEMBLER__

/*
 * Convert a single hex character to its binary value.
 * Returns -1 if given character is not a value hex character.
 */
extern int __hex(char ch);

/*
 * Convert the hex data in 'buf' into 'count' bytes to be placed in 'mem'.
 * Returns a pointer to the character in mem AFTER the last byte written.
 */
extern char *__unpack_bytes_to_mem(char *buf, char *mem, int count);

/*
 * While finding valid hex chars, build an unsigned long int.
 * Return number of hex chars processed.
 */
extern int __unpack_ulong(char **ptr, unsigned long *val);

/*
 * Unpack 'count' hex characters, forming them into a binary value.
 * Return that value as an int. Adjust the source pointer accordingly.
 */
extern int __unpack_nibbles(char **ptr, int count);

#endif

#endif // __BSP_HEX_UTILS_H__
