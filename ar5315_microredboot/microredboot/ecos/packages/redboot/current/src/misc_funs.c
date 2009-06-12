//==========================================================================
//
//      misc_funs.c
//
//      Miscellaneous [library] functions for RedBoot
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

int
strcasecmp(const char *s1, const char *s2)
{
    char c1, c2;
    while ((c1 = _tolower(*s1++)) == (c2 = _tolower(*s2++)))
        if (c1 == 0)
            return (0);
    return ((unsigned char)c1 - (unsigned char)c2);
}

int
strncasecmp(const char *s1, const char *s2, size_t len)
{
    char c1, c2;

    if (len == 0)
        return 0;
    do {
        if ((c1 = _tolower(*s1++)) != (c2 = _tolower(*s2++)))
            return ((unsigned char)c1 - (unsigned char)c2);
        if (c1 == 0)
            break;
    } while (--len != 0);
    return 0;
}
