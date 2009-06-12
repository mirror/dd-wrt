//==========================================================================
//
//      memcpy.c
//
//      memcpy() routine for coldfire
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

/* INCLUDES */

#include <cyg/infra/cyg_type.h> /* Common type definitions */
#include <stddef.h>             /* Compiler defns such as size_t, NULL etc. */

/* EXPORTED SYMBOLS */

externC void *
memcpy( void * s1, const void * s2, size_t n ) __attribute__((alias("_memcpy")));

/* FUNCTIONS */

void *
_memcpy( void * s1, const void * s2, size_t n )
{
    char * dst = (char *) s1;
    const char * src = (const char *) s2;
    long longwords;
    int_t rem_bytes;
    int_t loops;
    int_t loop_index;

    /*   Don't  worry  about  alignment   on  the  coldfire.   Most   large */
    /* structures should be aligned anyway.                                 */

    longwords  = (long)(n / 4);
    rem_bytes  = n % 4;
    loops      = (int_t)(longwords / 32);
    loop_index = (int_t)(longwords % 32);

    switch (loop_index)
    {
        do
        {
                   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 31:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 30:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 29:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 28:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 27:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 26:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 25:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 24:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 23:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 22:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 21:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 20:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 19:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 18:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 17:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 16:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 15:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 14:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 13:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 12:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 11:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 10:   *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 9:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 8:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 7:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 6:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 5:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 4:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 3:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 2:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 1:    *((cyg_uint32*)dst)++ = *((cyg_uint32*)src)++;
        case 0:    ; /* Keep compiler from complaining. */
        } while (--loops >= 0);
    }

    /* Clean up the remaining bytes. */

    while (--rem_bytes >= 0)
    {
        *dst++ = *src++;
    }

    return s1;

} /* _memcpy() */

/* EOF memcpy.c */
