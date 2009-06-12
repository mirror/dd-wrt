//========================================================================
//
//      timeutil.cxx
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
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-28
// Purpose:       Provide support functions used by the ISO C date and time
//                implementation
// Description:   This file provides the functions:
//                  cyg_libc_time_normalize_structtm()
//                  cyg_libc_time_getzoneoffsets()
//                  cyg_libc_time_setzoneoffsets()
//                  cyg_libc_time_setdst()
//                  cyg_libc_time_itoa()
//                and the globals:
//                  cyg_libc_time_day_name
//                  cyg_libc_time_day_name_len
//                  cyg_libc_time_month_name
//                  cyg_libc_time_month_name_len
//                  cyg_libc_time_month_lengths
//                  cyg_libc_time_current_dst_stat
//                  cyg_libc_time_current_std_offset
//                  cyg_libc_time_current_dst_offset
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

// define these functions as outline, not inline, functions in this file
#define CYGPRI_LIBC_TIME_GETZONEOFFSETS_INLINE extern "C"
#define CYGPRI_LIBC_TIME_SETZONEOFFSETS_INLINE extern "C"
#define CYGPRI_LIBC_TIME_SETDST_INLINE extern "C"

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/libc/time/timeutil.h>// Header for this file
#include <time.h>                  // Main date and time definitions
#include <stdlib.h>                // for div() and abs()


// GLOBALS

// FIXME: PR19440 - const char & -fwritable-strings don't mix
const char cyg_libc_time_day_name[7][10] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
    "Saturday"
};
const cyg_uint8 cyg_libc_time_day_name_len[7] = { 6, 6, 7, 9, 8, 6, 8 };

// FIXME: PR19440 - const char & -fwritable-strings don't mix
const char cyg_libc_time_month_name[12][10] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};
const cyg_uint8 cyg_libc_time_month_name_len[12] = { 7, 8, 5, 5, 3, 4,
                                                     4, 6, 9, 7, 8, 8 };

const cyg_uint8 cyg_libc_time_month_lengths[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

Cyg_libc_time_dst cyg_libc_time_current_dst_stat =
     (Cyg_libc_time_dst)CYGNUM_LIBC_TIME_DST_DEFAULT_STATE;
time_t cyg_libc_time_current_std_offset =
     (time_t)CYGNUM_LIBC_TIME_STD_DEFAULT_OFFSET;
time_t cyg_libc_time_current_dst_offset =
     (time_t)CYGNUM_LIBC_TIME_DST_DEFAULT_OFFSET;


// FUNCTIONS

////////////////////////////////////////
// cyg_libc_time_normalize_structtm() //
////////////////////////////////////////
//
// cyg_libc_time_normalize_structtm() will adjust the fields of a struct tm
// so that they are within the normal ranges expected.
// tm_wday, tm_yday and tm_isdst are ignored

externC void
cyg_libc_time_normalize_structtm( struct tm *timeptr )
{
    div_t t;

    CYG_REPORT_FUNCNAME("cyg_libc_time_normalize_structtm");
    CYG_REPORT_FUNCARG1("timeptr is at address %08x", timeptr);

    // normalize seconds to 0..59
    if ((timeptr->tm_sec < 0) || (timeptr->tm_sec > 59)) {
        t = div(timeptr->tm_sec, 60);
        while (t.rem < 0) {
            t.rem += 60;
            --t.quot;
        }
        timeptr->tm_min += t.quot;
        timeptr->tm_sec = t.rem;
    } // if

    // normalize minutes to 0..59
    if ((timeptr->tm_min < 0) || (timeptr->tm_min > 59)) {
        t = div(timeptr->tm_min, 60);
        while (t.rem < 0) {
            t.rem += 60;
            --t.quot;
        }
        timeptr->tm_hour += t.quot;
        timeptr->tm_min = t.rem;
    } // if

    // normalize hours to 0..23
    if ((timeptr->tm_hour < 0) || (timeptr->tm_hour > 23)) {
        t = div(timeptr->tm_hour, 24);
        while (t.rem < 0) {
            t.rem += 24;
            --t.quot;
        }
        timeptr->tm_mday += t.quot;
        timeptr->tm_hour = t.rem;
    } // if
    
    // we wait before normalizing tm_mday as per ISO C 7.12.2.3 (although
    // actually it only makes sense if you think about it

    // normalize months to 0..11
    if ((timeptr->tm_mon < 0) || (timeptr->tm_mon > 11)) {
        t = div(timeptr->tm_mon, 12);
        while (t.rem < 0) {
            t.rem += 12;
            --t.quot;
        }
        timeptr->tm_year += t.quot;
        timeptr->tm_mon = t.rem;
    } // if

    // now tm_mday which needs to go to 1..31
    cyg_bool leap = cyg_libc_time_year_is_leap(timeptr->tm_year);

    while (timeptr->tm_mday < 1) {
        // move back a month

        if (--timeptr->tm_mon < 0) {
            --timeptr->tm_year;
            timeptr->tm_mon = 11;
            leap = cyg_libc_time_year_is_leap(timeptr->tm_year);
        } // if

        // we move backward the number of days in the _new_ current month
        timeptr->tm_mday += cyg_libc_time_month_lengths[leap][timeptr->tm_mon];

    } // while

    while (timeptr->tm_mday >
           cyg_libc_time_month_lengths[leap][timeptr->tm_mon]) {

        // move forward a month
        
        // we move forward the number of days in the _old_ current month
        timeptr->tm_mday -= cyg_libc_time_month_lengths[leap][timeptr->tm_mon];

        if (++timeptr->tm_mon > 11) {
            ++timeptr->tm_year;
            timeptr->tm_mon = 0;
            leap = cyg_libc_time_year_is_leap(timeptr->tm_year);
        } // if
        
    } // while

    CYG_REPORT_RETURN();

} // cyg_libc_time_normalize_structtm()


//////////////////////////
// cyg_libc_time_itoa() //
//////////////////////////
//
// This converts num to a string and puts it into s padding with
// "0"'s if padzero is set, or spaces otherwise if necessary.
// The number of chars written to s is returned
//

// This implementation is probably suboptimal in terms of performance
// but there wouldn't be much in it with only 11 chars max to convert :-/.
// Actually FIXME: what if someone passes a width >11

externC cyg_ucount8
cyg_libc_time_itoa( cyg_uint8 *s, cyg_int32 num, cyg_uint8 width,
                    cyg_bool padzero )
{
    CYG_REPORT_FUNCNAMETYPE("cyg_libc_time_itoa", "returning %d");
    CYG_REPORT_FUNCARG4( "s=%08x, num=%d, width=%d, padzero=%d",
                         s, num, width, padzero );

    CYG_CHECK_DATA_PTR(s, "input string not a valid pointer");

    // special case for zero otherwise we'd have to treat it specially later
    // on anyway
    
    if (num==0) {
        cyg_ucount8 i;
        
        for (i=0; i<width; ++i)
            s[i] = padzero ? '0' : ' ';
        CYG_REPORT_RETVAL(i);
        return i;
    }
    
    // return value
    cyg_ucount8 ret=0;
    
    // Pre-fiddle for negative numbers
    if ((num < 0) && (width > 0)) {
        *s++ = '-';
        --width;
        num = abs(num);
        ++ret;
    }
        
    cyg_ucount32 i;
    cyg_bool reachednum = false;
    cyg_uint8 c, j;

    // i starts off with factor of 10 digits - which is the string length
    // of a positive 32-bit number
    for (i=1000000000, j=10; i>0; i/=10, --j) {
        c = (num / i);

        if (!reachednum && c==0) {
            if (j <= width) {
                *s++ = padzero ?  '0' : ' ';
                ++ret;
            }
        } // if
        else {
            *s++ = c + '0';
            ++ret;
            reachednum = true;
        }
        num %= i;
    } // for
    
    CYG_POSTCONDITION(ret >= width, "Didn't output enough chars!");

    CYG_REPORT_RETVAL(ret);
    return ret;
} // cyg_libc_time_itoa()


// EOF timeutil.cxx
