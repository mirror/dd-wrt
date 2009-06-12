//==========================================================================
//
//      net/inet_addr.c
//
//      Stand-alone IP address parsing for RedBoot
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <net/net.h>

bool
inet_aton(const char *s, in_addr_t *addr)
{
    int i, val, radix, digit;
    unsigned long res = 0;
    bool first;
    char c;
    
    for (i = 0;  i < 4;  i++) {
        // Parse next digit string
        first = true;
        val = 0;
        radix = 10;
        while ((c = *s++) != '\0') {
            if (first && (c == '0') && (_tolower(*s) == 'x')) {
                radix = 16;
                s++;  // Skip over 0x
                c = *s++;
            }
            first = false;
            if (_is_hex(c) && ((digit = _from_hex(c)) < radix)) {
                // Valid digit
                val = (val * radix) + digit;
            } else if (c == '.' && i < 3) { // all but last terminate by '.'
                break;
            } else {
                return false;
            }
        }
        // merge result
#ifdef __LITTLE_ENDIAN__
        res |= val << ((3-i)*8);  // 24, 16, 8, 0
#else
        res = (res << 8) | val;
#endif
        if ('\0' == c) {
            if (0 == i) { // first field found end of string
                res = val; // no shifting, use it as the whole thing
                break; // permit entering a single number
            }
            if (3 > i) // we found end of string before getting 4 fields
                return false;
        }
        // after that we check that it was 0..255 only
        if (val &~0xff) return false;
    }
    addr->s_addr = htonl(res);
    return true;
}

// Assumes address is in network byte order
char *
inet_ntoa(in_addr_t *addr)
{
    static char str[32];
    unsigned char *ap;

    ap = (unsigned char *)addr;
    diag_sprintf(str, "%d.%d.%d.%d", ap[0], ap[1], ap[2], ap[3]);
    return str;
}
