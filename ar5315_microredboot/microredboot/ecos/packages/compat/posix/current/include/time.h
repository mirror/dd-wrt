#ifndef CYGONCE_POSIX_TIME_H
#define CYGONCE_POSIX_TIME_H
/*=============================================================================
//
//      time.h
//
//      POSIX time header
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg, jlarmour
// Date:          2000-03-17
// Purpose:       POSIX time header
// Description:   This header contains all the definitions needed to support
//                the POSIX timer and timer API under eCos.
//              
// Usage:         Do not include this file directly - instead include <time.h>
//              
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/posix.h>
#include <cyg/infra/cyg_type.h>

/*---------------------------------------------------------------------------*/
/* Types for timers and clocks */

typedef int clockid_t;

#ifdef CYGPKG_POSIX_TIMERS
typedef int timer_t;

/* forward declaration - if the app uses it it will have to include
 * signal.h anyway
 */
struct sigevent;
#endif

/*---------------------------------------------------------------------------*/
/* Structures */

struct timespec
{
    time_t      tv_sec;
    long        tv_nsec;
};

#ifdef CYGPKG_POSIX_TIMERS
struct itimerspec
{
    struct timespec     it_interval;
    struct timespec     it_value;
};
#endif

/*---------------------------------------------------------------------------*/
/* Manifest constants */

#define CLOCK_REALTIME          0

#ifdef CYGPKG_POSIX_TIMERS
#define TIMER_ABSTIME           1
#endif

/*---------------------------------------------------------------------------*/
/* Clock functions */

/* Set the clocks current time */
__externC int clock_settime( clockid_t clock_id, const struct timespec *tp);

/* Get the clocks current time */
__externC int clock_gettime( clockid_t clock_id, struct timespec *tp);

/* Get the clocks resolution */
__externC int clock_getres( clockid_t clock_id, struct timespec *tp);


/*---------------------------------------------------------------------------*/
/* Timer functions */

#ifdef CYGPKG_POSIX_TIMERS

/* Create a timer based on the given clock. */
__externC int timer_create( clockid_t clock_id,
                            struct sigevent *evp,
                            timer_t *timer_id);

/* Delete the timer */
__externC int timer_delete( timer_t timer_id );

/* Set the expiration time of the timer. */
__externC int timer_settime( timer_t timerid, int flags,
                             const struct itimerspec *value,
                             struct itimerspec *ovalue );

/* Get current timer values */
__externC int timer_gettime( timer_t timerid, struct itimerspec *value );

/* Get number of missed triggers */
__externC int timer_getoverrun( timer_t timerid );

#endif

/*---------------------------------------------------------------------------*/
/* Nanosleep */

/* Sleep for the given time. */
__externC int nanosleep( const struct timespec *rqtp,
                         struct timespec *rmtp);


/*---------------------------------------------------------------------------*/
#endif /* ifndef CYGONCE_POSIX_TIME_H */
/* End of time.h */
