#ifndef CYGONCE_LIBC_TIME_INL
#define CYGONCE_LIBC_TIME_INL
//===========================================================================
//
//      time.inl
//
//      Inline implementations of date and time routines from <time.h>
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: 
// Date:         1999-02-25
// Purpose:      Provide inline implementations for some of the date and time
//               routines declared in <time.h> for ISO C section 7.12 and
//               POSIX 1003.1 8.3.4-8.3.7
// Description: 
// Usage:        Do not include this file directly. Instead include <time.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>          // C library configuration

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <time.h>                  // Header for this file
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure

// DEFINES

// The following are overriden by the libc implementation to get a non-inline
// version to prevent duplication of code

#ifndef CYGPRI_LIBC_TIME_ASCTIME_R_INLINE
# define CYGPRI_LIBC_TIME_ASCTIME_R_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_CTIME_R_INLINE
# define CYGPRI_LIBC_TIME_CTIME_R_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_GMTIME_R_INLINE
# define CYGPRI_LIBC_TIME_GMTIME_R_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_LOCALTIME_R_INLINE
# define CYGPRI_LIBC_TIME_LOCALTIME_R_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_DIFFTIME_INLINE
# define CYGPRI_LIBC_TIME_DIFFTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_MKTIME_INLINE
# define CYGPRI_LIBC_TIME_MKTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_ASCTIME_INLINE
# define CYGPRI_LIBC_TIME_ASCTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_CTIME_INLINE
# define CYGPRI_LIBC_TIME_CTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_GMTIME_INLINE
# define CYGPRI_LIBC_TIME_GMTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_LOCALTIME_INLINE
# define CYGPRI_LIBC_TIME_LOCALTIME_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_GETZONEOFFSETS_INLINE
# define CYGPRI_LIBC_TIME_GETZONEOFFSETS_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_SETZONEOFFSETS_INLINE
# define CYGPRI_LIBC_TIME_SETZONEOFFSETS_INLINE extern __inline__
#endif

#ifndef CYGPRI_LIBC_TIME_SETDST_INLINE
# define CYGPRI_LIBC_TIME_SETDST_INLINE extern __inline__
#endif

#define CYGNUM_LIBC_TIME_EPOCH_WDAY       4  // Jan 1st 1970 was a Thursday

#ifdef __cplusplus
extern "C" {
#endif

// EXTERNS

// These are used in the dst access functions below. Do not access these
// directly - use the functions declared in time.h instead

extern Cyg_libc_time_dst cyg_libc_time_current_dst_stat;
extern time_t cyg_libc_time_current_std_offset;
extern time_t cyg_libc_time_current_dst_offset;

// INLINE FUNCTIONS

//===========================================================================
//
// Utility functions

//////////////////////////////////
// cyg_libc_time_year_is_leap() //
//////////////////////////////////
//
// This returns true if the year is a leap year.
// The argument is of type int in line with struct tm
//

static __inline__ cyg_bool
cyg_libc_time_year_is_leap( int __year )
{
    cyg_bool _leap=false;

    if (!(__year % 400))
        _leap = true;
    else if (!(__year % 4) && (__year % 100))
        _leap = true;
    return _leap;
} // cyg_libc_time_year_is_leap()


////////////////////////////////////
// cyg_libc_time_getzoneoffsets() //
////////////////////////////////////
//
// This function retrieves the current state of the Daylight Savings Time
// and the offsets of both STD and DST
// The offsets are both in time_t's i.e. seconds
//

CYGPRI_LIBC_TIME_GETZONEOFFSETS_INLINE Cyg_libc_time_dst
cyg_libc_time_getzoneoffsets( time_t *__stdoffset, time_t *__dstoffset )
{
    CYG_REPORT_FUNCNAMETYPE("cyg_libc_time_getzoneoffsets",
                            "returning DST state %d");
    CYG_REPORT_FUNCARG2("__stdoffset is at address %08x, "
                        "__dstoffset is at %08x", __stdoffset, __dstoffset);

    CYG_CHECK_DATA_PTR(__stdoffset, "__stdoffset is not a valid pointer!");
    CYG_CHECK_DATA_PTR(__dstoffset, "__dstoffset is not a valid pointer!");

    *__stdoffset = cyg_libc_time_current_std_offset;
    *__dstoffset = cyg_libc_time_current_dst_offset;

    CYG_REPORT_RETVAL(cyg_libc_time_current_dst_stat);

    return cyg_libc_time_current_dst_stat;
} // cyg_libc_time_getzoneoffsets()


////////////////////////////////////
// cyg_libc_time_setzoneoffsets() //
////////////////////////////////////
//
// This function sets the offsets used when Daylight Savings Time is enabled
// or disabled. The offsets are in time_t's i.e. seconds
//

CYGPRI_LIBC_TIME_SETZONEOFFSETS_INLINE void
cyg_libc_time_setzoneoffsets( time_t __stdoffset, time_t __dstoffset )
{
    CYG_REPORT_FUNCNAME("cyg_libc_time_setzoneoffsets");
    CYG_REPORT_FUNCARG2DV(__stdoffset, __dstoffset);

    cyg_libc_time_current_std_offset = __stdoffset;
    cyg_libc_time_current_dst_offset = __dstoffset;

    CYG_REPORT_RETURN();
} // cyg_libc_time_setzoneoffsets()


////////////////////////////
// cyg_libc_time_setdst() //
////////////////////////////
//
// This function sets the state of Daylight Savings Time: on, off, or unknown
//

CYGPRI_LIBC_TIME_SETDST_INLINE void
cyg_libc_time_setdst( Cyg_libc_time_dst __state )
{
    CYG_REPORT_FUNCNAME("cyg_libc_time_setdst");
    CYG_REPORT_FUNCARG1("__state=%d", __state);

    cyg_libc_time_current_dst_stat = __state;

    CYG_REPORT_RETURN();
} // cyg_libc_time_setdst()



//===========================================================================
//
// POSIX 1003.1 functions

/////////////////////////////////
// asctime_r() - POSIX.1 8.3.4 //
/////////////////////////////////
//
// This returns a textual representation of a struct tm, and writes
// the string to return into __buf
//

#ifdef CYGFUN_LIBC_TIME_POSIX
# define __asctime_r asctime_r
#else
// prototype internal function
__externC char *
__asctime_r( const struct tm *__timeptr, char *__buf );
#endif

#ifdef CYGIMP_LIBC_TIME_ASCTIME_R_INLINE

#include <cyg/libc/time/timeutil.h> // for cyg_libc_time_{day,month}_name
                                    // and cyg_libc_time_itoa()
#include <string.h>                 // for memcpy()

CYGPRI_LIBC_TIME_ASCTIME_R_INLINE char *
__asctime_r( const struct tm *__timeptr, char *__buf )
{
    cyg_ucount8 __i;
    
    // These initializers are [4] since C++ dictates you _must_ leave space
    // for the trailing '\0', even though ISO C says you don't need to!

    CYG_REPORT_FUNCNAMETYPE("asctime_r", "returning \"%s\"");
    CYG_REPORT_FUNCARG2("__timeptr = %08x, __buf = %08x", __timeptr, __buf);

    // paranoia - most of these aren't required but could be helpful to
    // a programmer debugging their own app.
    CYG_CHECK_DATA_PTR(__timeptr, "__timeptr is not a valid pointer!");
    CYG_CHECK_DATA_PTR(__buf, "__buf is not a valid pointer!");

    CYG_PRECONDITION((__timeptr->tm_sec >= 0) && (__timeptr->tm_sec < 62),
                     "__timeptr->tm_sec out of range!");
    CYG_PRECONDITION((__timeptr->tm_min >= 0) && (__timeptr->tm_min < 60),
                     "__timeptr->tm_min out of range!");
    CYG_PRECONDITION((__timeptr->tm_hour >= 0) && (__timeptr->tm_hour < 24),
                     "__timeptr->tm_hour out of range!");
    // Currently I don't check _actual_ numbers of days in each month here
    // FIXME: No reason why not though
    CYG_PRECONDITION((__timeptr->tm_mday >= 1) && (__timeptr->tm_mday < 32),
                     "__timeptr->tm_mday out of range!");
    CYG_PRECONDITION((__timeptr->tm_mon >= 0) && (__timeptr->tm_mon < 12),
                     "__timeptr->tm_mon out of range!");
    CYG_PRECONDITION((__timeptr->tm_wday >= 0) && (__timeptr->tm_wday < 7),
                     "__timeptr->tm_wday out of range!");
    CYG_PRECONDITION((__timeptr->tm_yday >= 0) && (__timeptr->tm_yday < 366),
                     "__timeptr->tm_yday out of range!");
    CYG_PRECONDITION((__timeptr->tm_year > -1900) &&
                     (__timeptr->tm_year < 8100),
                     "__timeptr->tm_year out of range!");
    
    // we can't use strftime because ISO C is stupid enough not to allow
    // the strings in asctime() to be localized. Duh.

    // day of the week
    memcpy(&__buf[0], cyg_libc_time_day_name[__timeptr->tm_wday], 3);
    __buf[3] = ' ';

    // month
    memcpy(&__buf[4], cyg_libc_time_month_name[__timeptr->tm_mon], 3);
    __buf[7] = ' ';

    __i = 8;

    // day of the month
    __i += cyg_libc_time_itoa( (cyg_uint8 *)&__buf[__i], __timeptr->tm_mday, 2,
                               true);
    __buf[__i++] = ' ';

    // hour
    __i += cyg_libc_time_itoa( (cyg_uint8 *)&__buf[__i], __timeptr->tm_hour, 2,
                               true);
    __buf[__i++] = ':';

    // minute
    __i += cyg_libc_time_itoa( (cyg_uint8 *)&__buf[__i], __timeptr->tm_min, 2,
                               true);
    __buf[__i++] = ':';

    // second
    __i += cyg_libc_time_itoa((cyg_uint8 *) &__buf[__i], __timeptr->tm_sec, 2,
                              true);
    __buf[__i++] = ' ';

    // year
    __i += cyg_libc_time_itoa( (cyg_uint8 *)&__buf[__i],
                               1900+__timeptr->tm_year, 0, true);
    
    __buf[__i++] = '\n';
    __buf[__i++] = '\0';

    CYG_REPORT_RETVAL(__buf);
    return __buf;
} // asctime_r()

#endif // ifdef CYGIMP_LIBC_TIME_ASCTIME_R_INLINE

////////////////////////////////
// gmtime_r() - POSIX.1 8.3.6 //
////////////////////////////////
//
// This converts a time_t into a struct tm expressed in UTC, and stores
// the result in the space occupied by __result
//

#ifdef CYGFUN_LIBC_TIME_POSIX
# define __gmtime_r gmtime_r
#else
// prototype internal function
__externC struct tm *
__gmtime_r( const time_t *__timer, struct tm *__result );
#endif

#ifdef CYGIMP_LIBC_TIME_GMTIME_R_INLINE

#include <cyg/libc/time/timeutil.h>   // for cyg_libc_time_month_lengths

CYGPRI_LIBC_TIME_GMTIME_R_INLINE struct tm *
__gmtime_r( const time_t *__timer, struct tm *__result )
{
    time_t _tim;
    const cyg_uint8 *_months_p;

    CYG_REPORT_FUNCNAMETYPE("gmtime_r", "returning %08x");
    CYG_CHECK_DATA_PTR(__timer, "__timer is not a valid pointer!");
    CYG_CHECK_DATA_PTR(__result, "__result is not a valid pointer!");
    CYG_REPORT_FUNCARG2("*__timer=%d, __result is at %08x",
                        *__timer, __result);

#define CYGNUM_LIBC_TIME_SECSPERDAY       (60*60*24)
#define CYGNUM_LIBC_TIME_SECSPERYEAR      (CYGNUM_LIBC_TIME_SECSPERDAY * 365)
#define CYGNUM_LIBC_TIME_SECSPERLEAPYEAR  (CYGNUM_LIBC_TIME_SECSPERDAY * 366)

    _tim = *__timer;

    // First, work out year. Start off with 1970 and work forwards or backwards
    // depending on the sign of _tim
    __result->tm_year = 70;

    // we also work out the day of the week of the start of the year as we
    // go along
    __result->tm_wday = CYGNUM_LIBC_TIME_EPOCH_WDAY;

    while (_tim < 0) {
        // Work backwards

        --__result->tm_year;

        // Check for a leap year.
        if (cyg_libc_time_year_is_leap(1900 + __result->tm_year)) {
            _tim += CYGNUM_LIBC_TIME_SECSPERLEAPYEAR; 
            __result->tm_wday -= 366;
        } // if
        else {
            _tim += CYGNUM_LIBC_TIME_SECSPERYEAR;
            __result->tm_wday -= 365;
        } // else

    } // while

    while (_tim >= CYGNUM_LIBC_TIME_SECSPERYEAR) {
        // Work forwards

        if (cyg_libc_time_year_is_leap(1900 + __result->tm_year)) {
                
            // But if this is a leap year, its possible that we are in the
            // middle of the last "extra" day
            if (_tim < CYGNUM_LIBC_TIME_SECSPERLEAPYEAR)
                break;

            _tim -= CYGNUM_LIBC_TIME_SECSPERLEAPYEAR;
            __result->tm_wday += 366;
        } // if
        else {
            _tim -= CYGNUM_LIBC_TIME_SECSPERYEAR;
            __result->tm_wday += 365;
        }
        ++__result->tm_year;
    } // while

    // Day of the year. We know _tim is +ve now
    CYG_ASSERT(_tim >= 0,
               "Number of seconds since start of year is negative!");
    __result->tm_yday = _tim / CYGNUM_LIBC_TIME_SECSPERDAY;

    // Day of the week. Normalize to be 0..6, and note that it might
    // be negative, so we have to deal with the modulus being
    // implementation-defined for -ve numbers (ISO C 6.3.5)
    __result->tm_wday = (((__result->tm_wday + __result->tm_yday)%7)+7)%7;
    
    // Month and Day of the month

    _months_p = cyg_libc_time_month_lengths[
        cyg_libc_time_year_is_leap(1900 + __result->tm_year) ? 1 : 0 ];

    __result->tm_mday = __result->tm_yday+1;

    for (__result->tm_mon = 0;
         __result->tm_mday > _months_p[__result->tm_mon];
         ++__result->tm_mon) {
        
        __result->tm_mday -= _months_p[__result->tm_mon];

    } // for

    _tim -= __result->tm_yday*CYGNUM_LIBC_TIME_SECSPERDAY;

    // hours, mins secs
    __result->tm_hour = (int) (_tim / 3600);
    _tim %= 3600;
    __result->tm_min  = (int) (_tim / 60);
    __result->tm_sec  = (int) (_tim % 60);

    __result->tm_isdst = 0; // gmtime always returns non-DST

    CYG_REPORT_RETVAL(__result);

    return __result;
} // gmtime_r()

#endif // ifdef CYGIMP_LIBC_TIME_GMTIME_R_INLINE

///////////////////////////////////
// localtime_r() - POSIX.1 8.3.7 //
///////////////////////////////////
//
// This converts a time_t into a struct tm expressed in local time, and
// stores the result in the space occupied by __result
//

#ifdef CYGFUN_LIBC_TIME_POSIX
# define __localtime_r localtime_r 
#else
// prototype internal function
__externC struct tm *
__localtime_r( const time_t *__timer, struct tm *__result );
#endif

#ifdef CYGIMP_LIBC_TIME_LOCALTIME_R_INLINE

#include <cyg/libc/time/timeutil.h>  // for cyg_libc_time_normalize_structtm()

CYGPRI_LIBC_TIME_LOCALTIME_R_INLINE struct tm *
__localtime_r( const time_t *__timer, struct tm *__result )
{
    time_t __stdoffset, __dstoffset;
    CYG_REPORT_FUNCNAMETYPE("localtime_r", "returning %08x");
    CYG_CHECK_DATA_PTR(__timer, "__timer is not a valid pointer!");
    CYG_CHECK_DATA_PTR(__result, "__result is not a valid pointer!");
    CYG_REPORT_FUNCARG2("*__timer=%d, __result is at %08x",
                        *__timer, __result);

    __gmtime_r(__timer, __result);

    // Adjust for STD/DST
    __result->tm_isdst = cyg_libc_time_getzoneoffsets(&__stdoffset,
                                                      &__dstoffset);

    if (__result->tm_isdst == 0) { // STD
        __result->tm_sec += __stdoffset;
        cyg_libc_time_normalize_structtm( __result );
    } // if
    else if (__result->tm_isdst > 0) { // DST
        __result->tm_sec += __dstoffset;
        cyg_libc_time_normalize_structtm( __result );
    } // if
    // Don't do anything for tm_isdst == -1

    CYG_REPORT_RETVAL(__result);

    return __result;
} // localtime_r()

#endif // ifdef CYGIMP_LIBC_TIME_LOCALTIME_R_INLINE


///////////////////////////////
// ctime_r() - POSIX.1 8.3.5 //
///////////////////////////////
//
// This returns the equivalent of ctime() but writes to __buf
// to store the returned string
//

#ifdef CYGFUN_LIBC_TIME_POSIX
# define __ctime_r ctime_r 
#else
// prototype internal function
__externC char *
__ctime_r( const time_t *__timer, char *__buf );
#endif

#ifdef CYGIMP_LIBC_TIME_CTIME_R_INLINE

CYGPRI_LIBC_TIME_CTIME_R_INLINE char *
__ctime_r( const time_t *__timer, char *__buf )
{
    struct tm _mytm;

    CYG_REPORT_FUNCNAMETYPE("ctime_r", "returning \"%s\"");

    CYG_CHECK_DATA_PTR(__timer, "__timer is not a valid pointer!");
    CYG_CHECK_DATA_PTR(__buf, "__buf is not a valid pointer!");

    CYG_REPORT_FUNCARG2("*__timer = %d, __buf=%08x", *__timer, __buf);

    __localtime_r( __timer, &_mytm );

    __asctime_r(&_mytm, __buf);

    CYG_REPORT_RETVAL(__buf);

    return __buf;
} // ctime_r()

#endif // ifdef CYGIMP_LIBC_TIME_CTIME_R_INLINE


//===========================================================================
//
// ISO C functions

// Time manipulation functions - ISO C 7.12.2

/////////////////////////////////
// difftime() - ISO C 7.12.2.2 //
/////////////////////////////////
//
// This returns (__time1 - __time0) in seconds
//

#ifdef CYGIMP_LIBC_TIME_DIFFTIME_INLINE

CYGPRI_LIBC_TIME_DIFFTIME_INLINE double
difftime( time_t __time1, time_t __time0 )
{
    double _ret;
    
    CYG_REPORT_FUNCNAMETYPE("difftime", "returning %f");
    CYG_REPORT_FUNCARG2("__time1=%d, __time0=%d", __time1, __time0);

    _ret = (double)(__time1 - __time0);
    
    CYG_REPORT_RETVAL(_ret);

    return _ret;
} // difftime()

#endif // ifdef CYGIMP_LIBC_TIME_DIFFTIME_INLINE

///////////////////////////////
// mktime() - ISO C 7.12.2.3 //
///////////////////////////////
//
// This converts a "struct tm" to a "time_t"
//

#ifdef CYGIMP_LIBC_TIME_MKTIME_INLINE

#include <cyg/libc/time/timeutil.h>  // for cyg_libc_time_normalize_structtm()
                                     // and cyg_libc_time_month_lengths

CYGPRI_LIBC_TIME_MKTIME_INLINE time_t
mktime( struct tm *__timeptr )
{
    time_t _ret;
    cyg_count16 _i;
    cyg_count32 _daycount;
    cyg_bool _leap;

    CYG_REPORT_FUNCNAMETYPE("mktime", "returning %d");
    CYG_REPORT_FUNCARG1( "__timeptr is at address %08x", __timeptr);

    CYG_CHECK_DATA_PTR(__timeptr, "__timeptr is not a valid pointer!");

    // First deal with STD/DST. If tm_isdst==-1 (the "autodetect" value)
    // we assume its already in UTC. FIXME: is this correct behaviour? Hmm....

#if 0
// FIXME: This doesn't seem to be the way to go
    if (__timeptr->tm_isdst == 0) { // STD
        // take _off_ the std offset to get us back to UTC from localtime
        __timeptr->tm_sec -= (int)cyg_libc_time_current_std_offset;
    } // if
    else if (__timeptr->tm_isdst > 0) { // DST
        // take _off_ the dst offset to get us back to UTC from localtime
        __timeptr->tm_sec -= (int)cyg_libc_time_current_dst_offset;
    } // if
#endif
    
    cyg_libc_time_normalize_structtm(__timeptr);

    // check if a time_t can hold the year. FIXME: we assume it is
    // 32 bits which gives the year range 1902 - 2038
    if ( (__timeptr->tm_year <= 2) || (__timeptr->tm_year >= 138) ) {
        CYG_REPORT_RETVAL(-1);
        return (time_t)-1;
    }

    // fill in the rest of the struct tm i.e. tm_wday and tm_yday
    
    _leap = cyg_libc_time_year_is_leap(1900 + __timeptr->tm_year);

    for (_i=0, _daycount=0; _i<12; ++_i) {
        if (_i == __timeptr->tm_mon) {
            _daycount += __timeptr->tm_mday - 1;
            break;
        } // if
        else {
            _daycount += cyg_libc_time_month_lengths[_leap][_i];
        } // else
    } // for

    CYG_ASSERT(_i<12, "Reached end of year. __timeptr->tm_mon must be bad");

    __timeptr->tm_yday = _daycount;
    
    // now tm_wday

    if (__timeptr->tm_year > 70) {
        for (_i=70; _i < __timeptr->tm_year; ++_i)
            _daycount += (cyg_libc_time_year_is_leap(1900 + _i) ? 366 : 365);
    } // if
    else if (__timeptr->tm_year < 70) {
        for (_i=70; _i > __timeptr->tm_year; --_i)
            _daycount -= (cyg_libc_time_year_is_leap(1900 + _i-1) ? 366 : 365);
    } // else if

    __timeptr->tm_wday = (_daycount + CYGNUM_LIBC_TIME_EPOCH_WDAY) % 7;

    // if _daycount was negative, on some targets the modulo operator will
    // return negative, so we adjust for that

    if (__timeptr->tm_wday < 0)
        __timeptr->tm_wday += 7;

    // now finally work out return value

    _ret = __timeptr->tm_sec + 60*__timeptr->tm_min + 60*60*__timeptr->tm_hour;
    _ret += _daycount*24*60*60;
    
    CYG_REPORT_RETVAL(_ret);

    return _ret;
} // mktime()

#endif // ifdef CYGIMP_LIBC_TIME_MKTIME_INLINE


// Time conversion functions - ISO C 7.12.3

////////////////////////////////
// asctime() - ISO C 7.12.3.1 //
////////////////////////////////
//
// This returns a textual representation of a struct tm
//

#ifdef CYGIMP_LIBC_TIME_ASCTIME_INLINE

extern char cyg_libc_time_asctime_buf[];

CYGPRI_LIBC_TIME_ASCTIME_INLINE char *
asctime( const struct tm *__timeptr )
{
    CYG_REPORT_FUNCNAMETYPE("__asctime", "returning \"%s\"");
    CYG_REPORT_FUNCARG1("__timeptr = %08x", __timeptr);

    // paranoia
    CYG_CHECK_DATA_PTR(__timeptr, "__timeptr is not a valid pointer!");

    (void)__asctime_r( __timeptr, cyg_libc_time_asctime_buf );

    CYG_REPORT_RETVAL(cyg_libc_time_asctime_buf);

    return cyg_libc_time_asctime_buf;
} // asctime()

#endif // ifdef CYGIMP_LIBC_TIME_ASCTIME_INLINE


///////////////////////////////
// gmtime() - ISO C 7.12.3.3 //
///////////////////////////////
//
// This converts a time_t into a struct tm expressed in UTC
//

#ifdef CYGIMP_LIBC_TIME_GMTIME_INLINE

extern struct tm cyg_libc_time_gmtime_buf;

CYGPRI_LIBC_TIME_GMTIME_INLINE struct tm *
gmtime( const time_t *__timer )
{
    CYG_REPORT_FUNCNAMETYPE("gmtime", "returning %08x");
    CYG_CHECK_DATA_PTR(__timer, "__timer is not a valid pointer!");
    CYG_REPORT_FUNCARG1("*__timer=%d", *__timer);

    __gmtime_r(__timer, &cyg_libc_time_gmtime_buf);

    CYG_REPORT_RETVAL(&cyg_libc_time_gmtime_buf);

    return &cyg_libc_time_gmtime_buf;
} // gmtime()

#endif // ifdef CYGIMP_LIBC_TIME_GMTIME_INLINE


//////////////////////////////////
// localtime() - ISO C 7.12.3.4 //
//////////////////////////////////
//
// This converts a time_t into a struct tm expressed in local time
//

#ifdef CYGIMP_LIBC_TIME_LOCALTIME_INLINE

extern struct tm cyg_libc_time_localtime_buf;

CYGPRI_LIBC_TIME_LOCALTIME_INLINE struct tm *
localtime( const time_t *__timer )
{
    CYG_REPORT_FUNCNAMETYPE("localtime", "returning %08x");
    CYG_CHECK_DATA_PTR(__timer, "__timer is not a valid pointer!");
    CYG_REPORT_FUNCARG1("*__timer=%d", *__timer);

    __localtime_r(__timer, &cyg_libc_time_localtime_buf);

    CYG_REPORT_RETVAL(&cyg_libc_time_localtime_buf);

    return &cyg_libc_time_localtime_buf;
} // localtime()

#endif // ifdef CYGIMP_LIBC_TIME_LOCALTIME_INLINE


//////////////////////////////
// ctime() - ISO C 7.12.3.2 //
//////////////////////////////
//
// This returns asctime(localtime(__timeptr))
//

#ifdef CYGIMP_LIBC_TIME_CTIME_INLINE

CYGPRI_LIBC_TIME_CTIME_INLINE char *
ctime( const time_t *__timer )
{
    char *_str;

    CYG_REPORT_FUNCNAMETYPE("ctime", "returning \"%s\"");
    CYG_CHECK_DATA_PTR( __timer, "__timer is not a valid pointer!");
    CYG_REPORT_FUNCARG1("*__timer = %d", *__timer);

    _str = asctime(localtime(__timer));

    CYG_REPORT_RETVAL(_str);

    return _str;
} // ctime()

#endif // ifdef CYGIMP_LIBC_TIME_CTIME_INLINE


#ifdef __cplusplus
} // extern "C"
#endif

#endif // CYGONCE_LIBC_TIME_INL multiple inclusion protection

// EOF time.inl
