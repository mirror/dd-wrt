#ifndef CYGONCE_ISO_TIME_H
#define CYGONCE_ISO_TIME_H
/*========================================================================
//
//      time.h
//
//      ISO time functions
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
// Date:          2000-04-14
// Purpose:       This file provides the time macros, types and functions
//                required by ISO C and POSIX 1003.1.
// Description:   The real contents of this file get set from the
//                configuration (set by the implementation)
// Usage:         #include <time.h>
//
//####DESCRIPTIONEND####
//
//======================================================================
*/

/* CONFIGURATION */

#include <pkgconf/isoinfra.h>          /* Configuration header */

/* INCLUDES */

/* This is the "standard" way to get NULL and size_t from stddef.h,
 * which is the canonical location of the definitions.
 */
#define __need_NULL
#define __need_size_t
#include <stddef.h>

#ifdef CYGBLD_ISO_TIME_T_HEADER
# include CYGBLD_ISO_TIME_T_HEADER
#else

/* Define time_t as per ISO C para 7.12.1
 * It is conventionally given the meaning of seconds since the
 * Epoch (00:00:00 GMT, 1970-01-01), but don't rely on this - always use
 * the type opaquely
 */
typedef int time_t;
# define __time_t_defined

#endif

#ifdef CYGBLD_ISO_CLOCK_T_HEADER
# include CYGBLD_ISO_CLOCK_T_HEADER
#else

/* What to divide clock_t by to get seconds (para 7.12.1)
 * According to SUS3, CLOCKS_PER_SEC must be 1 million on all
 * XSI-conformant systems.
 */
#  define CLOCKS_PER_SEC 1000000

/* Define clock_t as per ISO C para 7.12.1 */

typedef long clock_t;
#  define __clock_t_defined
#endif

#ifdef CYGBLD_ISO_STRUCTTIMEVAL_HEADER
# include CYGBLD_ISO_STRUCTTIMEVAL_HEADER
#else
# ifndef _POSIX_SOURCE

/*
 * Structure returned by gettimeofday(2),
 * and used in other calls such as select(2).
 */
struct timeval {
	long	tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};

# endif /* _POSIX_SOURCE */
#endif


#ifdef CYGINT_ISO_POSIX_CLOCK_TYPES
# include CYGBLD_ISO_POSIX_CLOCK_TYPES_HEADER
#else

/* Provide a default struct timespec. */

struct timespec
{
    time_t      tv_sec;
    long        tv_nsec;
};
#endif

#ifdef CYGINT_ISO_POSIX_TIMER_TYPES
# include CYGBLD_ISO_POSIX_TIMER_TYPES_HEADER
#else

/* Provide a default struct itimerspec. */
struct itimerspec
{
    struct timespec     it_interval;
    struct timespec     it_value;
};

#endif

#ifdef CYGINT_ISO_C_TIME_TYPES
# include CYGBLD_ISO_C_TIME_TYPES_HEADER
#endif

#ifdef CYGINT_ISO_POSIX_CLOCKS
# include CYGBLD_ISO_POSIX_CLOCKS_HEADER
#endif

#ifdef CYGINT_ISO_POSIX_TIMERS
# include CYGBLD_ISO_POSIX_TIMERS_HEADER
#endif

#ifdef CYGINT_ISO_C_CLOCK_FUNCS
# include CYGBLD_ISO_C_CLOCK_FUNCS_HEADER
#endif

#ifdef CYGINT_ISO_TZSET
# include CYGBLD_ISO_TZSET_HEADER
#endif

#endif /* CYGONCE_ISO_TIME_H multiple inclusion protection */

/* EOF time.h */
