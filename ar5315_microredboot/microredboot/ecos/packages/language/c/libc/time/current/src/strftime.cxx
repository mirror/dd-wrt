//========================================================================
//
//      strftime.cxx
//
//      ISO C date and time implementation for strftime()
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
// Date:         1999-02-26
// Purpose:      Provide implementation of the ISO C function strftime()
//               from ISO C section 7.12.3.5
// Description:  This file provides the implementation of strftime()
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure
#include <time.h>                  // Main date and time definitions
#include <cyg/libc/time/timeutil.h>// For cyg_libc_time_{day,month}_name{,_len}

// FUNCTION PROTOTYPES

// FIXME: This all needs to be internationalized and localized, both in
// terms of reading and writing multibyte characters, and that it should
// refer to the settings of LC_TIME

// FUNCTIONS

// This helper function actually processes the format. Which format to insert
// is indicated by fmtchar, sizeleft is the maximum space allowed to be used
// in buf. The number of bytes written is returned, or -1 if there was no
// space

static cyg_count8
do_format(cyg_uint8 fmtchar, cyg_ucount32 sizeleft, char *buf,
          const struct tm *timeptr)
{
    cyg_count8 i;

    switch (fmtchar) {
    case 'a':
        if (sizeleft<3)
            return -1;
        buf[0] = cyg_libc_time_day_name[timeptr->tm_wday][0];
        buf[1] = cyg_libc_time_day_name[timeptr->tm_wday][1];
        buf[2] = cyg_libc_time_day_name[timeptr->tm_wday][2];
        return 3;
    case 'A':
        if (sizeleft < cyg_libc_time_day_name_len[timeptr->tm_wday])
            return -1;
        for (i=0; i<cyg_libc_time_day_name_len[timeptr->tm_wday]; ++i)
            buf[i] = cyg_libc_time_day_name[timeptr->tm_wday][i];
        return i;
#ifdef CYGFUN_LIBC_TIME_SUS_EXTNS
    case 'h':
        // ** fall through **
#endif
    case 'b':
        if (sizeleft<3)
            return -1;
        buf[0] = cyg_libc_time_month_name[timeptr->tm_mon][0];
        buf[1] = cyg_libc_time_month_name[timeptr->tm_mon][1];
        buf[2] = cyg_libc_time_month_name[timeptr->tm_mon][2];
        return 3;
    case 'B':
        if (sizeleft < cyg_libc_time_month_name_len[timeptr->tm_mon])
            return -1;
        for (i=0; i<cyg_libc_time_month_name_len[timeptr->tm_mon]; ++i)
            buf[i] = cyg_libc_time_month_name[timeptr->tm_mon][i];
        return i;
    case 'c':
        if (sizeleft < 26)
            return -1;

        // Recurse! Note that we know that we will have left room for the
        // trailing NULL in the strftime body
        
        i = strftime( buf, sizeleft+1, "%a %b %d %I:%M:%S%p %Y", timeptr);
        
        return ((0==i) ? -1 : i);
    case 'd':
        if (sizeleft < 2)
            return -1;
        buf[0] = (timeptr->tm_mday / 10) + '0';
        buf[1] = (timeptr->tm_mday % 10) + '0';
        return 2;
#ifdef CYGFUN_LIBC_TIME_SUS_EXTNS
    case 'e':
        if (sizeleft < 2)
            return -1;
        i = (timeptr->tm_mday / 10);
        buf[0] = (0 == i) ? ' ' : i + '0';
        buf[1] = (timeptr->tm_mday % 10) + '0';
        return 2;
#endif
    case 'H':
        if (sizeleft < 2)
            return -1;
        buf[0] = (timeptr->tm_hour / 10) + '0';
        buf[1] = (timeptr->tm_hour % 10) + '0';
        return 2;
    case 'I':
        if (sizeleft < 2)
            return -1;
        buf[0] = ((timeptr->tm_hour%12 + 1) / 10) + '0';
        buf[1] = ((timeptr->tm_hour%12 + 1) % 10) + '0';
        return 2;
    case 'j':
        if (sizeleft < 3)
            return -1;
        buf[0] = (timeptr->tm_yday / 100) + '0';
        buf[1] = ((timeptr->tm_yday % 100) / 10) + '0';
        buf[2] = (timeptr->tm_yday % 10) + '0';
        return 3;
    case 'm':
        if (sizeleft < 2)
            return -1;
        buf[0] = ((timeptr->tm_mon+1) / 10) + '0';
        buf[1] = ((timeptr->tm_mon+1) % 10) + '0';
        return 2;
    case 'M':
        if (sizeleft < 2)
            return -1;
        buf[0] = (timeptr->tm_min / 10) + '0';
        buf[1] = (timeptr->tm_min % 10) + '0';
        return 2;
    case 'p':
        if (sizeleft < 2)
            return -1;
        buf[0] = (timeptr->tm_hour > 11) ? 'p' : 'a';
        buf[1] = 'm';
        return 2;
    case 'S':
        if (sizeleft < 2)
            return -1;
        buf[0] = (timeptr->tm_sec / 10) + '0';
        buf[1] = (timeptr->tm_sec % 10) + '0';
        return 2;
#ifdef CYGFUN_LIBC_TIME_SUS_EXTNS
    case 'T':
        if (sizeleft < 8)
            return -1;

        // Recurse! Note that we know that we will have left room for the
        // trailing NULL in the strftime body
        
        i = strftime( buf, sizeleft+1, "%H:%M:%S", timeptr);
        
        return ((0==i) ? -1 : i);
#endif
    case 'U':
        if (sizeleft < 2)
            return -1;
        i = (timeptr->tm_yday - timeptr->tm_wday + 7) / 7;
        buf[0] = (i / 10) + '0';
        buf[1] = (i % 10) + '0';
        return 2;
    case 'w':
        // Don't need to check size - we'll always be called with sizeleft > 0
        buf[0] = timeptr->tm_wday + '0';
        return 1;
    case 'W':
        if (sizeleft < 2)
            return -1;
        i = (timeptr->tm_yday + ((8-timeptr->tm_wday) % 7)) / 7;
        buf[0] = (i / 10) + '0';
        buf[1] = (i % 10) + '0';
        return 2;
    case 'x':
        if (sizeleft < 15)
            return -1;

        // Recurse! Note that we know that we will have left room for the
        // trailing NULL in the strftime body

        i = strftime( buf, sizeleft+1, "%a %b %d %Y", timeptr);

        return (0==i) ? -1 : i;
    case 'X':
        if (sizeleft < 10)
            return -1;

        // Recurse! Note that we know that we will have left room for the
        // trailing NULL in the strftime body

        i = strftime( buf, sizeleft+1, "%I:%M:%S%p", timeptr);

        return (0==i) ? -1 : i;
    case 'y':
        if (sizeleft < 2)
            return -1;
        buf[0] = ((timeptr->tm_year % 100) / 10) + '0';
        buf[1] = ((timeptr->tm_year % 100) % 10) + '0';
        return 2;
    case 'Y':
        if (sizeleft < 4)
            return -1;
        buf[0] = ((1900+timeptr->tm_year) / 1000) + '0';
        buf[1] = (((1900+timeptr->tm_year) % 1000) / 100) + '0';
        buf[2] = ((timeptr->tm_year % 100) / 10) + '0';
        buf[3] = (timeptr->tm_year % 10) + '0';
        return 4;
    case 'Z':
        // FIXME: Should store zone in timeptr->tm_zone, or failing that
        // read TZ env variable using tzset()
        return 0;
    case '%':
        // Don't need to check size - we'll always be called with sizeleft > 0
        buf[0] = '%';
        return 1;
    default:
        CYG_FAIL("invalid format character!");
        return -1;
    } // switch

} // do_format()

externC size_t
strftime( char *s, size_t maxsize, const char *format,
          const struct tm *timeptr)
{
    CYG_REPORT_FUNCNAMETYPE("strftime", "returning string length %d");
    CYG_CHECK_DATA_PTR(s, "string s is not a valid address!");
    CYG_CHECK_DATA_PTR(format, "format string is not at a valid address!");
    CYG_CHECK_DATA_PTR(timeptr,
                       "struct tm argument is not at a valid address!");
    CYG_REPORT_FUNCARG4("s is at address %08x, maxsize=%d, format=\"%s\", "
                        "timeptr is at address %08x",
                        s, maxsize, format, timeptr);

    CYG_PRECONDITION((timeptr->tm_sec >= 0) && (timeptr->tm_sec < 62),
                     "timeptr->tm_sec out of range!");
    CYG_PRECONDITION((timeptr->tm_min >= 0) && (timeptr->tm_min < 60),
                     "timeptr->tm_min out of range!");
    CYG_PRECONDITION((timeptr->tm_hour >= 0) && (timeptr->tm_hour < 24),
                     "timeptr->tm_hour out of range!");
    // Currently I don't check _actual_ numbers of days in each month here
    // FIXME: No reason why not though
    CYG_PRECONDITION((timeptr->tm_mday >= 1) && (timeptr->tm_mday < 32),
                     "timeptr->tm_mday out of range!");
    CYG_PRECONDITION((timeptr->tm_mon >= 0) && (timeptr->tm_mon < 12),
                     "timeptr->tm_mon out of range!");
    CYG_PRECONDITION((timeptr->tm_wday >= 0) && (timeptr->tm_wday < 7),
                     "timeptr->tm_wday out of range!");
    CYG_PRECONDITION((timeptr->tm_yday >= 0) && (timeptr->tm_yday < 366),
                     "timeptr->tm_yday out of range!");
    CYG_PRECONDITION((timeptr->tm_year > -1900) &&
                     (timeptr->tm_year < 8100),
                     "timeptr->tm_year out of range!");

    if (!maxsize) {
        CYG_REPORT_RETVAL(0);
        return 0;
    } // if

    // lets leave the room for the trailing null up front
    --maxsize;

    cyg_ucount32 i, spos;
    cyg_count8 dof_ret;
    
    for (i=0, spos=0; (spos<maxsize) && (format[i] != '\0'); ++i) {
        if (format[i] == '%') {
            if (format[++i] == '\0')
                break;
            dof_ret = do_format(format[i], maxsize - spos, &s[spos], timeptr);
            // was there room to write _anything_?
            if (dof_ret < 0) {
                spos=0;  // ISO says we must return 0 and the contents of s
                         // are indeterminate
                break;
            }
            spos += dof_ret;
        }
        else {
            s[spos++] = format[i];
        } // else
    } // for
    
    s[spos] = '\0';
    
    CYG_REPORT_RETVAL(spos);
    return spos;

} // strftime()


// EOF strftime.cxx
