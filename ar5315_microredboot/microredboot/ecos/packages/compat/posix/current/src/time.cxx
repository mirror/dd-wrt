//==========================================================================
//
//      time.cxx
//
//      POSIX time functions implementation
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-03-27
// Purpose:             POSIX time functions implementation
// Description:         This file contains the implementation of the POSIX time
//                      functions.
//              
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/posix.h>

#ifdef CYGPKG_POSIX_CLOCKS

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>          // base kernel types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include "pprivate.h"                   // POSIX private header

#include <time.h>                       // our header

#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/clock.hxx>

#include <cyg/kernel/thread.inl>
#include <cyg/kernel/clock.inl>

// -------------------------------------------------------------------------
// Internal definitions

// Handle entry to a pthread package function. 
#define TIME_ENTRY() CYG_REPORT_FUNCTYPE( "returning %d" );

// Do a time package defined return. This requires the error code
// to be placed in errno, and if it is non-zero, -1 returned as the
// result of the function. This also gives us a place to put any
// generic tidyup handling needed for things like signal delivery and
// cancellation.
#define TIME_RETURN(err)                        \
CYG_MACRO_START                                 \
    int __retval = 0;                           \
    if( err != 0 ) __retval = -1, errno = err;  \
    CYG_REPORT_RETVAL( __retval );              \
    return __retval;                            \
CYG_MACRO_END

//==========================================================================
// Timer control structures

#ifdef CYGPKG_POSIX_TIMERS
typedef struct
{
    timer_t             id;             // id value for checking
    Cyg_Alarm           *alarm;         // eCos alarm object
    cyg_bool            armed;          // is alarm enabled?
    cyg_bool            pending;        // is expiry pending?
    int                 overrun;        // Overrun count
    struct sigevent     sigev;          // Sigevent to raise on expiry
    
    // Space for alarm object
    cyg_uint8           alarm_obj[sizeof(Cyg_Alarm)];
    
} posix_timer;

// Mutex for controlling access to shared data structures
static Cyg_Mutex timer_mutex CYGBLD_POSIX_INIT;

// Array of timer objects
static posix_timer timer_table[_POSIX_TIMER_MAX];

// Index of next timer to allocate from array
static int timer_next = 0;

// This is used to make timer_t values unique even when reusing
// a table slot. This allows _POSIX_TIMER_MAX to range
// up to 1024.
#define TIMER_ID_COOKIE_INC 0x00000400
#define TIMER_ID_COOKIE_MASK (TIMER_ID_COOKIE_INC-1)
static timer_t timer_id_cookie = TIMER_ID_COOKIE_INC;

#endif // ifdef CYGPKG_POSIX_TIMERS

//-----------------------------------------------------------------------------
// new operator to allow us to invoke the constructor on
// posix_timer.alarm_obj.

inline void *operator new(size_t size,  cyg_uint8 *ptr) { return (void *)ptr; };

//==========================================================================
// Time conversion variables
// These are used to interconvert between ticks and POSIX timespecs.

// Converters from sec and ns to ticks
static struct Cyg_Clock::converter ns_converter, sec_converter;

// Converters from ticks to sec and ns
static struct Cyg_Clock::converter ns_inverter, sec_inverter;

// tickns is the number of nanoseconds per tick.
static cyg_tick_count tickns;

static cyg_bool converters_initialized = false;

//==========================================================================
// Local functions

static void init_converters()
{
    if( !converters_initialized )
    {

        // Create the converters we need.
        Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1, &ns_converter );
        Cyg_Clock::real_time_clock->get_other_to_clock_converter( 1000000000, &sec_converter );
        Cyg_Clock::real_time_clock->get_clock_to_other_converter( 1, &ns_inverter );    
        Cyg_Clock::real_time_clock->get_clock_to_other_converter( 1000000000, &sec_inverter );

        tickns = Cyg_Clock::convert( 1, &ns_inverter );
            
        converters_initialized = true;
    }
}

static cyg_bool valid_timespec( const struct timespec *tp )
{
    // Fail a NULL pointer
    if( tp == NULL ) return false;

    // Fail illegal nanosecond values
    if( tp->tv_nsec < 0 || tp->tv_nsec > 1000000000 )
        return false;

    return true;
}

externC cyg_tick_count cyg_timespec_to_ticks( const struct timespec *tp,
                                              cyg_bool roundup)
{
    init_converters();

    // Short circuit zero timespecs
    if( tp->tv_sec == 0 && tp->tv_nsec == 0 )
    {
        return 0;
    }
        
    // Convert the seconds field to ticks.
    cyg_tick_count ticks = Cyg_Clock::convert( tp->tv_sec, &sec_converter );

    if( roundup )
    {
        // Convert the nanoseconds. We add (tickns-1) to round the value up
        // to the next whole tick.

        ticks += Cyg_Clock::convert( (cyg_tick_count)tp->tv_nsec+tickns-1, &ns_converter );    
    }
    else
    {
        // Convert the nanoseconds. This will round down to nearest whole tick.
        ticks += Cyg_Clock::convert( (cyg_tick_count)tp->tv_nsec, &ns_converter );
    }

    return ticks;
}

externC void cyg_ticks_to_timespec( cyg_tick_count ticks, struct timespec *tp )
{
    init_converters();

    // short circuit zero ticks values
    if( ticks == 0 )
    {
        tp->tv_sec = 0;
        tp->tv_nsec = 0;
        return;
    }

    // Convert everything to nanoseconds with a long long. For 64-bits,
    // this is safe for 544 years. We'll think about it more closer to
    // the time...

    unsigned long long nsecs = Cyg_Clock::convert( ticks, &ns_inverter );

    tp->tv_sec = (long)(nsecs / 1000000000ll);
    tp->tv_nsec = (long)(nsecs % 1000000000ll);

    CYG_POSTCONDITION(valid_timespec(tp), "Failed to make valid timespec!");
}

//==========================================================================
// Startup routine.

externC void cyg_posix_clock_start()
{
    init_converters();
}

#ifdef CYGPKG_POSIX_TIMERS
//==========================================================================
// Alarm action routine
// This is called each time an alarm set up by a timer expires.

static void alarm_action( Cyg_Alarm *alarm, CYG_ADDRWORD data )
{
    posix_timer *timer = (posix_timer *)data;

    if( timer->pending )
    {
        // If the pending flag is already set, count an overrun and
        // do not bother to try and deliver the expiry.
        
        timer->overrun++;
    }
    else
    {
        if( timer->sigev.sigev_notify == SIGEV_SIGNAL )
        {
            // Set the expiry pending and wake a thread to
            // deliver the signal.
        
            timer->pending = true;
    
            sigset_t mask;
            sigemptyset( &mask );
            sigaddset( &mask, timer->sigev.sigev_signo );
            cyg_posix_signal_sigwait();
            cyg_posix_pthread_release_thread( &mask );
        }
        else if( timer->sigev.sigev_notify == SIGEV_THREAD )
        {
            // Thread style notification
            // FIXME: implement SIGEV_THREAD
        }
        // else do nothing
    }
}

//==========================================================================
// Timer ASR routine

externC void cyg_posix_timer_asr( pthread_info *self )
{

    // Loop over the timers looking for any that have an
    // expiry pending and call cyg_sigqueue() for each.
    
    for( int i = 0; i < _POSIX_TIMER_MAX; i++ )
    {
        posix_timer *timer = &timer_table[i];

        if( timer->id != 0 && timer->pending )
        {
            timer->pending = false;
                
            // Call into signal subsystem...
            cyg_sigqueue( &timer->sigev, SI_TIMER );

            timer->overrun = 0;
        }
    }
}

#endif // ifdef CYGPKG_POSIX_TIMERS

//==========================================================================
// Clock functions

//-----------------------------------------------------------------------------
// Set the clocks current time

externC int clock_settime( clockid_t clock_id, const struct timespec *tp)
{
    TIME_ENTRY();

    if( clock_id != CLOCK_REALTIME )
        TIME_RETURN(EINVAL);

    if( !valid_timespec( tp ) )
        TIME_RETURN(EINVAL);
        
    cyg_tick_count ticks = cyg_timespec_to_ticks( tp );

    Cyg_Clock::real_time_clock->set_value( ticks );
    
    TIME_RETURN(0);
}

//-----------------------------------------------------------------------------
// Get the clocks current time

externC int clock_gettime( clockid_t clock_id, struct timespec *tp)
{
    TIME_ENTRY();

    if( clock_id != CLOCK_REALTIME )
        TIME_RETURN(EINVAL);

    if( tp == NULL )
        TIME_RETURN(EINVAL);
    
    cyg_tick_count ticks = Cyg_Clock::real_time_clock->current_value();

    cyg_ticks_to_timespec( ticks, tp );

    TIME_RETURN(0);
}
   

//-----------------------------------------------------------------------------
// Get the clocks resolution

externC int clock_getres( clockid_t clock_id, struct timespec *tp)
{
    TIME_ENTRY();

    if( clock_id != CLOCK_REALTIME )
        TIME_RETURN(EINVAL);

    if( tp == NULL )
        TIME_RETURN(EINVAL);

    // Get the resolution of 1 tick
    cyg_ticks_to_timespec( 1, tp );
    
    TIME_RETURN(0);
}
    

//==========================================================================
// Timer functions

#ifdef CYGPKG_POSIX_TIMERS

//-----------------------------------------------------------------------------
// Create a timer based on the given clock.

externC int timer_create( clockid_t clock_id,
                          struct sigevent *evp,
                          timer_t *timer_id)
{
    TIME_ENTRY();

    if( clock_id != CLOCK_REALTIME )
        TIME_RETURN(EINVAL);

    timer_mutex.lock();

    posix_timer *timer;
    int next = timer_next;

    // Look for an unused slot in the table
    while( timer_table[next].id != 0 )
    {
        next++;
        if( next >= _POSIX_TIMER_MAX )
            next = 0;

        if( next == timer_next )
        {
            timer_mutex.unlock();
            TIME_RETURN(EAGAIN);
        }
    }

    timer = &timer_table[next];

    timer_next = next;

    // Make sure we never allocate a zero timer id.
    while( timer->id == 0 )
    {
        timer_id_cookie += TIMER_ID_COOKIE_INC;
        timer->id        = next+timer_id_cookie;
    }

    if( evp == NULL )
    {
        // If no evp is supplied, set up the timer
        // to use a default set.
        timer->sigev.sigev_notify               = SIGEV_SIGNAL;
        timer->sigev.sigev_signo                = SIGALRM;
        timer->sigev.sigev_value.sival_int      = timer->id;
    }
    else timer->sigev = *evp;
        
    timer->alarm        = new( timer->alarm_obj )
                               Cyg_Alarm( Cyg_Clock::real_time_clock,
                                          alarm_action,
                                          (CYG_ADDRWORD)timer );

    timer->armed        = false;
    timer->overrun      = 0;

    *timer_id = timer->id;
    
    timer_mutex.unlock();
    
    TIME_RETURN(0);
}

//-----------------------------------------------------------------------------
// Delete the timer

externC int timer_delete( timer_t timerid )
{
    int err = EINVAL;
    TIME_ENTRY();
    
    posix_timer *timer = &timer_table[timerid & TIMER_ID_COOKIE_MASK];

    timer_mutex.lock();

    if( timer->id == timerid )
    {
        // This is a valid timer, disable the kernel
        // alarm and delete it.

        // disable alarm
        timer->alarm->disable();

        // destroy it
        timer->alarm->~Cyg_Alarm();

        // Mark POSIX timer free
        timer->id = 0;

        err = 0;
    }

    timer_mutex.unlock();
    
    TIME_RETURN( err );
}

//-----------------------------------------------------------------------------
// Set the expiration time of the timer.

externC int timer_settime( timer_t timerid, int flags,
                           const struct itimerspec *value,
                           struct itimerspec *ovalue )
{
    int err = EINVAL;
    TIME_ENTRY();
    
    if( value == NULL )
        TIME_RETURN(EINVAL);

    // convert trigger and interval values to ticks.
    cyg_tick_count trigger = cyg_timespec_to_ticks( &value->it_value, true );
    cyg_tick_count interval = cyg_timespec_to_ticks( &value->it_interval, true );
    
    posix_timer *timer = &timer_table[timerid & TIMER_ID_COOKIE_MASK];

    timer_mutex.lock();

    if( timer->id == timerid )
    {
        // disable the timer
        timer->alarm->disable();
        
        if( ovalue != NULL )
        {
            cyg_tick_count otrigger, ointerval;

            timer->alarm->get_times( &otrigger, &ointerval );

            if( timer->armed )
            {
                // convert absolute trigger time to interval until next trigger
                otrigger -= Cyg_Clock::real_time_clock->current_value();
            }
            else otrigger = 0;

            // convert ticks to timespecs
            cyg_ticks_to_timespec( otrigger, &ovalue->it_value );
            cyg_ticks_to_timespec( ointerval, &ovalue->it_interval );
        }
        
        if( trigger == 0 )
        {
            // Mark timer disarmed
            timer->armed = false;            
        }
        else
        {
            // If the ABSTIME flag is not set, add the current time
            if( (flags & TIMER_ABSTIME) == 0 )
                trigger += Cyg_Clock::real_time_clock->current_value();

            // Set the alarm running.
            timer->alarm->initialize( trigger, interval );

            // Mark timer armed
            timer->armed = true;

        }
        
        err = 0;
    }
    
    timer_mutex.unlock();
    
    TIME_RETURN(err);
}

//-----------------------------------------------------------------------------
// Get current timer values

externC int timer_gettime( timer_t timerid, struct itimerspec *value )
{
    int err = EINVAL;

    TIME_ENTRY();
    
    if( value == NULL )
        TIME_RETURN(EINVAL);
    
    posix_timer *timer = &timer_table[timerid & TIMER_ID_COOKIE_MASK];

    timer_mutex.lock();

    if( timer->id == timerid )
    {
        cyg_tick_count trigger, interval;

        timer->alarm->get_times( &trigger, &interval );

        if( timer->armed )
        {
            // convert absolute trigger time to interval until next trigger
            trigger -= Cyg_Clock::real_time_clock->current_value();
        }
        else trigger = 0;

        // convert ticks to timespecs
        cyg_ticks_to_timespec( trigger, &value->it_value );
        cyg_ticks_to_timespec( interval, &value->it_interval );
        err = 0;
    }
    
    timer_mutex.unlock();
    
    TIME_RETURN(err);
}

//-----------------------------------------------------------------------------
// Get number of missed triggers

externC int timer_getoverrun( timer_t timerid )
{
    int overrun = 0;
    
    TIME_ENTRY();

    posix_timer *timer = &timer_table[timerid & TIMER_ID_COOKIE_MASK];

    timer_mutex.lock();

    if( timer->id == timerid )
    {
        overrun = timer->overrun;
    }
    
    timer_mutex.unlock();
    
    CYG_REPORT_RETVAL(overrun);
    return overrun;
}

#endif // ifdef CYGPKG_POSIX_TIMERS

//==========================================================================
// Nanosleep
// Sleep for the given time.

externC int nanosleep( const struct timespec *rqtp,
                       struct timespec *rmtp)
{
    cyg_tick_count ticks, now, then;

    TIME_ENTRY();

    // check for cancellation first.
    PTHREAD_TESTCANCEL();

    // Fail an invalid timespec
    if( !valid_timespec( rqtp ) )
        TIME_RETURN(EINVAL);

    // Return immediately for a zero delay.
    if( rqtp->tv_sec == 0 && rqtp->tv_nsec == 0 )
        TIME_RETURN(0);

    // Convert timespec to ticks
    ticks = cyg_timespec_to_ticks( rqtp, true );

    CYG_ASSERT( ticks != 0, "Zero tick count");
    
    Cyg_Thread *self = Cyg_Thread::self();
    
    // Do the delay, keeping track of how long we actually slept for.
    then = Cyg_Clock::real_time_clock->current_value();

    self->delay( ticks );

    now = Cyg_Clock::real_time_clock->current_value();

    
    if( rmtp != NULL && (then+ticks) > now )
    {
        // We woke up early, return the time left.
        // FIXME: strictly we only need to do this if we were woken
        //        by a signal.

        // Calculate remaining number of ticks.
        ticks -= (now-then);

        cyg_ticks_to_timespec( ticks, rmtp );
    }
    
    // check if we were woken up because we were cancelled.
    PTHREAD_TESTCANCEL();

    TIME_RETURN(0);
}    

// -------------------------------------------------------------------------
// Wait for a signal, or the given number of seconds

externC unsigned int sleep( unsigned int seconds )
{
    TIME_ENTRY();

    struct timespec timeout;

    timeout.tv_sec = seconds;
    timeout.tv_nsec = 0;

    if( nanosleep( &timeout, &timeout ) != 0 )
    {
        CYG_REPORT_RETVAL(timeout.tv_sec);
        return timeout.tv_sec;
    }

    TIME_RETURN(0);
} 

#endif // ifdef CYGPKG_POSIX_CLOCKS

// -------------------------------------------------------------------------
// EOF time.cxx
