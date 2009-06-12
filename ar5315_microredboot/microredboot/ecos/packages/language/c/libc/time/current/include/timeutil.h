#ifndef CYGONCE_LIBC_TIME_TIMEUTIL_H
#define CYGONCE_LIBC_TIME_TIMEUTIL_H
//========================================================================
//
//      timeutil.h
//
//      ISO C date and time implementation support functions
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: jlarmour
// Date:         1999-03-03
// Purpose:      Provide declarations for support functions used by the
//               ISO C date and time implementation
// Description:   
// Usage:        #include <cyg/libc/time/timeutil.h>
//
//####DESCRIPTIONEND####
//
//========================================================================

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support

#ifdef __cplusplus
extern "C" {
#endif

// GLOBALS

// Strings of the days and month names
// FIXME: should comment out "const" in multi-dimensional arrays until
// PR 19440 is fixed
extern const char cyg_libc_time_day_name[7][10];
extern const cyg_uint8 cyg_libc_time_day_name_len[7];
extern const char cyg_libc_time_month_name[12][10];
extern const cyg_uint8 cyg_libc_time_month_name_len[12];

// number of days in each month, defined for normal and leap years
extern const cyg_uint8 cyg_libc_time_month_lengths[2][12];


// FUNCTION PROTOTYPES

////////////////////////////////////////
// cyg_libc_time_normalize_structtm() //
////////////////////////////////////////
//
// cyg_libc_time_normalize_structtm() will adjust the fields of a struct tm
// so that they are within the normal ranges expected.
// tm_wday, tm_yday, and tm_isdst are ignored

extern void
cyg_libc_time_normalize_structtm( struct tm *__timeptr );

//////////////////////////
// cyg_libc_time_itoa() //
//////////////////////////
//
// This converts num to a string and puts it into s padding with
// "0"'s if padzero is set, or spaces otherwise if necessary.
// The number of chars written to s is returned
//

extern cyg_ucount8
cyg_libc_time_itoa( cyg_uint8 *__s, cyg_int32 __num, cyg_uint8 __width,
                    cyg_bool __padzero );

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CYGONCE_LIBC_TIME_TIMEUTIL_H multiple inclusion protection

// EOF timeutil.h
