//==========================================================================
//
//      common/clock.cxx
//
//      Clock class implementations
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Jonathan Larmour
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-15
// Purpose:     Clock class implementation
// Description: This file contains the definitions of the counter,
//              clock and alarm class member functions that are common
//              to all clock implementations.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <cyg/kernel/clock.hxx>        // our header

#include <cyg/kernel/sched.hxx>        // scheduler definitions
#include <cyg/kernel/thread.hxx>       // thread definitions
#include <cyg/kernel/intr.hxx>         // interrupt definitions

#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/clock.inl>        // Clock inlines

// -------------------------------------------------------------------------
// Static variables

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

Cyg_Clock *Cyg_Clock::real_time_clock = NULL;   // System real time clock

#endif

//==========================================================================
// Constructor for counter object

Cyg_Counter::Cyg_Counter(
    cyg_uint32      incr
    )
{
    CYG_REPORT_FUNCTION();

    counter = 0;
    increment = incr;

}

// -------------------------------------------------------------------------
// Destructor for Counter object

Cyg_Counter::~Cyg_Counter()
{
    CYG_REPORT_FUNCTION();


}

// -------------------------------------------------------------------------
// 

#ifdef CYGDBG_USE_ASSERTS

cyg_bool Cyg_Counter::check_this( cyg_assert_class_zeal zeal) const
{
    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;
    
    switch( zeal )
    {
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_quick:
    case cyg_trivial:
    case cyg_none:
    default:
        break;
    };

    return true;
}

#endif

// -------------------------------------------------------------------------
// Counter tick function

void Cyg_Counter::tick( cyg_uint32 ticks )
{
//    CYG_REPORT_FUNCTION();

    CYG_ASSERTCLASS( this, "Bad counter object" );

    // Increment the counter in a loop so we process
    // each tick separately. This is easier than trying
    // to cope with a range of increments.
    
    while( ticks-- )
    {
        Cyg_Scheduler::lock();

        // increment the counter, note that it is
        // allowed to wrap.
        counter += increment;

        // now check for any expired alarms

        Cyg_Alarm_List *alarm_list_ptr;     // pointer to list

#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)

        alarm_list_ptr = &alarm_list;

#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

        // With multiple lists, each one contains only the alarms
        // that will expire at a given tick modulo the list number.
        // So we only have a fraction of the alarms to check here.
        
        alarm_list_ptr = &(alarm_list[
                               (counter/increment) % CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE ] );
    
#else
#error "No CYGIMP_KERNEL_COUNTERS_x_LIST config"
#endif

        // Now that we have the list pointer, we can use common code for
        // both list organizations.

#ifdef CYGIMP_KERNEL_COUNTERS_SORT_LIST

        // With a sorted alarm list, we can simply pick alarms off the
        // front of the list until we find one that is in the future.

        while( !alarm_list_ptr->empty() )
        {
            Cyg_Alarm *alarm = alarm_list_ptr->get_head();
        
            CYG_ASSERTCLASS(alarm, "Bad alarm in counter list" );
            
            if( alarm->trigger <= counter )
            {
                // remove alarm from list
                alarm_list_ptr->rem_head();

                if( alarm->interval != 0 )
                {
                    // The alarm has a retrigger interval.
                    // Reset the trigger time and requeue
                    // the alarm.
                    alarm->trigger += alarm->interval;
                    add_alarm( alarm );
                }
                else alarm->enabled = false;

                CYG_INSTRUMENT_ALARM( CALL, this, alarm );
                
                // call alarm function
                alarm->alarm(alarm, alarm->data);

                // all done, loop
            }
            else break;
            
        } 
#else

        // With unsorted lists we must scan the whole list for
        // candidates. However, we must be careful here since it is
        // possible for the function of one alarm to add or remove
        // other alarms to/from this list. Having the list shift under
        // our feet in this way could be disasterous. We solve this by
        // restarting the scan from the beginning whenever we call an
        // alarm function.

        cyg_bool rescan = true;

        while( rescan )
        {
            Cyg_DNode_T<Cyg_Alarm> *node = alarm_list_ptr->get_head();

            rescan = false;
            
            while( node != NULL )
            {
                Cyg_Alarm *alarm = CYG_CLASSFROMBASE( Cyg_Alarm, Cyg_DNode, node );
                Cyg_DNode_T<Cyg_Alarm> *next = alarm->get_next();

                CYG_ASSERTCLASS(alarm, "Bad alarm in counter list" );

                if( alarm->trigger <= counter )
                {
                    alarm_list_ptr->remove(alarm);

                    if( alarm->interval != 0 )
                    {
                        // The alarm has a retrigger interval.
                        // Reset the trigger time and requeue
                        // the alarm.
                        alarm->trigger += alarm->interval;
                        add_alarm( alarm );
                    }
                    else alarm->enabled = false;

                    CYG_INSTRUMENT_ALARM( CALL, this, alarm );
                
                    // Call alarm function
                    alarm->alarm(alarm, alarm->data);

                    rescan = true;

                    break;
                }

                // If the next node is the head of the list, then we have
                // looped all the way around. The node == next test
                // catches the case where we only had one element to start
                // with.
                if( next == alarm_list_ptr->get_head() || node == next )
                    node = NULL;
                else
                    node = next;
            }

        }
        
#endif        
        Cyg_Scheduler::unlock();

    }
    
}

// -------------------------------------------------------------------------
// Add an alarm to this counter

void Cyg_Counter::add_alarm( Cyg_Alarm *alarm )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERTCLASS( this, "Bad counter object" );
    CYG_ASSERTCLASS( alarm, "Bad alarm passed" );
    CYG_ASSERT( Cyg_Scheduler::get_sched_lock() > 0, "Scheduler not locked");
    
    // set this now to allow an immediate handler call to manipulate
    // this alarm sensibly.
    alarm->enabled = true;

    // Check here for an alarm that triggers now or in the past and
    // call its alarm function immediately. 
    if( alarm->trigger <= counter )
    {
        CYG_INSTRUMENT_ALARM( CALL, this, alarm );

        // call alarm function. Note that this is being
        // called here before the add_alarm has returned.
        // Note that this function may disable the alarm.
        
        alarm->alarm(alarm, alarm->data);

        // Note that this extra check on alarm->enabled is in case the
        // handler function disables this alarm!
        if( alarm->interval != 0 && alarm->enabled )
        {
            // The alarm has a retrigger interval.
            // Reset the trigger interval and drop
            // through to queue it.
            alarm->trigger += alarm->interval;
            // ensure the next alarm time is in our future, and in phase
            // with the original time requested.
            alarm->synchronize();
        }
        else
        {
            // The alarm is all done with, disable it
            // unlock and return.
            alarm->enabled = false;
            return;
        }
    }
    
    CYG_INSTRUMENT_ALARM( ADD, this, alarm );
 
    // Find the pointer to the relevant list _after_ a retrigger
    // alarm has been given its new trigger time.

    Cyg_Alarm_List *alarm_list_ptr;     // pointer to list

#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)

    alarm_list_ptr = &alarm_list;

#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

    // Each alarm must go into the list that covers the tick that is
    // going to happen _after_ the trigger time (or at it if trigger
    // happens to fall on a tick.
    
    alarm_list_ptr = &(alarm_list[
        ((alarm->trigger+increment-1)/increment) %
        CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE ] );
    
#else
#error "No CYGIMP_KERNEL_COUNTERS_x_LIST config"
#endif

#ifdef CYGIMP_KERNEL_COUNTERS_SORT_LIST
        
    // Now that we have the list pointer, we can use common code for
    // both list organizations.

    Cyg_Alarm *list_alarm = alarm_list_ptr->get_head();

    if( list_alarm != NULL )
    {
        do
        {
            CYG_ASSERTCLASS(list_alarm, "Bad alarm in counter list" );

            // The alarms are in ascending trigger order. If we
            // find an alarm that triggers later than us, we go
            // in front of it.
        
            if( list_alarm->trigger > alarm->trigger )
            {
                alarm_list_ptr->insert( list_alarm, alarm );
                return;
            }

            list_alarm = list_alarm->get_next();
            
        } while( list_alarm != alarm_list_ptr->get_head() );
        // a lower or equal alarm time was not found, so drop through
        // so it is added to the list tail
    }
#endif

    alarm_list_ptr->add_tail( alarm );
}

// -------------------------------------------------------------------------
// Remove an alarm from this counter

void Cyg_Counter::rem_alarm( Cyg_Alarm *alarm )
{
    CYG_REPORT_FUNCTION();

    CYG_ASSERTCLASS( this, "Bad counter object" );
    CYG_ASSERTCLASS( alarm, "Bad alarm passed" );
    CYG_ASSERT( Cyg_Scheduler::get_sched_lock() > 0, "Scheduler not locked");
    
    Cyg_Alarm_List *alarm_list_ptr;     // pointer to list

#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)

    alarm_list_ptr = &alarm_list;

#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

    alarm_list_ptr = &(alarm_list[
        ((alarm->trigger+increment-1)/increment) %
                              CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE ] );
    
#else
#error "No CYGIMP_KERNEL_COUNTERS_x_LIST config"
#endif

    // Now that we have the list pointer, we can use common code for
    // both list organizations.

    CYG_INSTRUMENT_ALARM( REM, this, alarm );

    alarm_list_ptr->remove( alarm );
    
    alarm->enabled = false;

}

//==========================================================================
// Constructor for clock object

Cyg_Clock::Cyg_Clock(
    cyg_resolution      res
    )
{
    CYG_REPORT_FUNCTION();

    resolution = res;
}

// -------------------------------------------------------------------------
// Destructor for Clock objects

Cyg_Clock::~Cyg_Clock()
{
    CYG_REPORT_FUNCTION();

}

// -------------------------------------------------------------------------
// 

#ifdef CYGDBG_USE_ASSERTS

cyg_bool Cyg_Clock::check_this( cyg_assert_class_zeal zeal) const
{
    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;
    
    switch( zeal )
    {
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
    case cyg_quick:
    case cyg_trivial:
    case cyg_none:
    default:
        break;
    };

    return true;
}

#endif

// -------------------------------------------------------------------------
// 
// Clock Converters: split a rational into 4 factors to try to prevent
// overflow whilst retaining reasonable accuracy.
// 
// typically we get numbers like 1,000,000 for ns_per and
// 100 and 1,000,000,000 for the dividend and divisor.
// So we want answers like 1/10 and 10/1 out of these routines.

static void construct_converter( Cyg_Clock::converter *pcc,
                                        cyg_uint64 m1, cyg_uint64 d1,
                                        cyg_uint64 m2, cyg_uint64 d2 )
{
    cyg_uint64 upper, lower;
    unsigned int i;
    static cyg_uint16 primes[] = {
        3,5,7,11,13,17,19,23,29,31,37,41,43,47,
        53,59,61,67,71,73,79,83,89,97,
        101,103,107,109,113,127,131,137,139,149,
        151,157,163,167,173,179,181,191,193,197,199,
        239,                            // for 1,111,111
        541,                            // for 10,101,011
        1667,                           // for 8,333,333
    };

    int rounding = 0;

    // Here we assume that our workings will fit in a 64; the point is to
    // allow calculations with a number of ticks that may be large.
    upper = m1 * m2;
    lower = d1 * d2;
#ifdef CYGDBG_USE_ASSERTS
    cyg_uint64 save_upper = upper;
    cyg_uint64 save_lower = lower;
#endif

 retry_rounding:
    // First strip out common powers of 2
    while ( (0 == (1 & upper)) && ( 0 == (1 & lower)) ) {
        upper >>= 1;
        lower >>= 1;
    }

    // then common factors - use lazy table above
    for ( i = 0 ; i < (sizeof( primes )/sizeof( primes[0] )); i++ ) {
        cyg_uint64 j, k, p = (cyg_uint64)(primes[i]);
        j = upper / p;
        while ( j * p == upper ) {
            k = lower / p;
            if ( k * p != lower )
                break;
            upper = j;
            lower = k;
            j = upper / p;
        }
    }

    m1 = upper;
    d1 = lower;
    m2 = 1;
    d2 = 1;

    if ( m1 > 0x10000 ) {
        // only bother if there are more than 16 bits consumed here

        // now move powers of 2 from d1 to d2
        // keeping them the same order of magnitude
        while ( (0 == (1 & d1)) && (d2 < d1) ) {
            d1 >>= 1;
            d2 <<= 1;
        }

        // and factors from the table - go too far, if anything
        int cont = (d2 < d1);
        for ( i = 0 ; cont && (i < (sizeof( primes )/sizeof( primes[0] ))); i++ ) {
            cyg_uint64 k, p = (cyg_uint64)(primes[i]);
            k = d1 / p;
            while ( cont && ((k * p) == d1) ) {
                // we can extract a prime
                d1 = k;
                d2 *= p;
                k = d1 / p;
                cont = (d2 < d1);
            }
        }
        
        // move powers of 2 from m1 to m2 so long as we do not go less than d1
        while ( (0 == (1 & m1)) && (m2 < m1) && (m1 > (d1 << 5)) ) {
            m1 >>= 1;
            m2 <<= 1;
            if ( m1 < 0x10000 )
                break;
        }
        
        // and factors from the table - ensure m1 stays well larger than d1
        cont = ((m2 < m1) && (m1 > (d1 << 4)) && (m1 > 0x10000));
        for ( i = 0 ; cont && (i < (sizeof( primes )/sizeof( primes[0] ))); i++ ) {
            cyg_uint64 k, p = (cyg_uint64)(primes[i]);
            k = m1 / p;
            cont = cont && (k > (d1 << 4) && (k > 0x10000));
            while ( cont && ((k * p) == m1) ) {
                // we can extract a prime
                m1 = k;
                m2 *= p;
                k = m1 / p; // examine k for getting too small
                cont = ((m2 < m1) && (k > (d1 << 4)) && (k > 0x10000));
            }
        }

        // if, after all that, m1 odd and unchanged, and too large,
        // decrement it just the once and try again: then try it
        // incremented once.
        if ( (m1 & 1) && (m1 == upper) && (m1 > 0x10000) && (rounding < 2) ) {
            CYG_ASSERT( 1 == m2, "m2 should be 1 to try rounding" );
            m1--;
            upper = m1;
            rounding++;
            goto retry_rounding;
        }
        // likewise for d1 - each of the pair can be odd only once each
        if ( (d1 & 1) && (d1 == lower) && (d1 > 0x10000) && (rounding < 2) ) {
            CYG_ASSERT( 1 == d2, "d2 should be 1 to try rounding" );
            d1--;
            lower = d1;
            rounding++;
            goto retry_rounding;
        }
    }

    CYG_ASSERT( 0 != m1, "m1 zero" );
    CYG_ASSERT( 0 != m2, "m2 zero" );
    CYG_ASSERT( 0 != d1, "d1 zero" );
    CYG_ASSERT( 0 != d2, "d2 zero" );
    CYG_ASSERT( rounding || save_upper/save_lower == (m1 * m2)/(d1 * d2),
                "Unequal in forwards direction" );
    CYG_ASSERT( rounding || save_lower/save_upper == (d1 * d2)/(m1 * m2),
                "Unequal in reverse direction" );

    pcc->mul1 = m1;
    pcc->div1 = d1;
    pcc->mul2 = m2;
    pcc->div2 = d2;
}

// other to clocks is (other * ns_per * dividend / divisor)
void Cyg_Clock::get_other_to_clock_converter(
    cyg_uint64 ns_per_other_tick,
    struct converter *pcc )
{
    construct_converter( pcc,
                         ns_per_other_tick, 1,
                         resolution.divisor, resolution.dividend );
}

// clocks to other is (ticks * divisor / dividend / ns_per)
void Cyg_Clock::get_clock_to_other_converter(
    cyg_uint64 ns_per_other_tick,
    struct converter *pcc )
{
    construct_converter( pcc,
                         1, ns_per_other_tick,
                         resolution.dividend, resolution.divisor );
}


//==========================================================================
// Constructor for alarm object

Cyg_Alarm::Cyg_Alarm(
        Cyg_Counter     *c,             // Attached to this counter
        cyg_alarm_fn    *a,             // Call-back function
        CYG_ADDRWORD    d               // Call-back data
        )
{
    CYG_REPORT_FUNCTION();

    counter     = c;
    alarm       = a;
    data        = d;
    trigger     = 0;
    interval    = 0;
    enabled     = false;

}

Cyg_Alarm::Cyg_Alarm(){}

// -------------------------------------------------------------------------
// Destructor

Cyg_Alarm::~Cyg_Alarm()
{
    CYG_REPORT_FUNCTION();

    disable();
}

// -------------------------------------------------------------------------
// 

#ifdef CYGDBG_USE_ASSERTS

cyg_bool Cyg_Alarm::check_this( cyg_assert_class_zeal zeal) const
{
    // check that we have a non-NULL pointer first
    if( this == NULL ) return false;
    
    switch( zeal )
    {
    case cyg_system_test:
    case cyg_extreme:
    case cyg_thorough:
        if( trigger != 0 && !enabled ) return false;
    case cyg_quick:
    case cyg_trivial:
    case cyg_none:
    default:
        break;
    };

    return true;
}

#endif

// -------------------------------------------------------------------------
// Initialize Alarm and enable

void Cyg_Alarm::initialize(                
    cyg_tick_count    t,                // Absolute trigger time
    cyg_tick_count    i                 // Relative retrigger interval
    )
{
    CYG_REPORT_FUNCTION();

    Cyg_Scheduler::lock();
    
    // If already enabled, remove from counter
    
    if( enabled ) counter->rem_alarm(this);

    CYG_INSTRUMENT_ALARM( INIT,     this, 0 );
    CYG_INSTRUMENT_ALARM( TRIGGER,
                          ((cyg_uint32 *)&t)[0],
                          ((cyg_uint32 *)&t)[1] );
    CYG_INSTRUMENT_ALARM( INTERVAL,
                          ((cyg_uint32 *)&i)[0],
                          ((cyg_uint32 *)&i)[1] );
 
    trigger = t;
    interval = i;

    counter->add_alarm(this);

    Cyg_Scheduler::unlock();    
}

// -------------------------------------------------------------------------
// Synchronize with a past alarm stream that had been disabled,
// bring past times into synch, and the like.

void
Cyg_Alarm::synchronize( void )
{
    if( interval != 0 ) {
        // This expression sets the trigger to the next whole interval
        // at or after the current time. This means that alarms will
        // continue at the same intervals as if they had never been
        // disabled. The alternative would be to just set trigger to
        // (counter->counter + interval), but this is less satisfying
        // than preserving the original intervals. That behaviour can
        // always be obtained by using initialize() rather than
        // enable(), while the current behaviour would be more
        // difficult to achieve that way.
        cyg_tick_count d;
        d = counter->current_value() + interval - trigger;
        if ( d > interval ) {
            // then trigger was in the past, so resynchronize
            trigger += interval * ((d - 1) / interval );
        }
        // otherwise, we were just set up, so no worries.
    }
}

// -------------------------------------------------------------------------
// Ensure alarm enabled

void Cyg_Alarm::enable()
{
    Cyg_Scheduler::lock();
    
    if( !enabled )
    {
        // ensure the alarm time is in our future:
        synchronize();
        enabled = true;
        counter->add_alarm(this);
    }

    Cyg_Scheduler::unlock();    
}

// -------------------------------------------------------------------------
// Ensure alarm disabled

void Cyg_Alarm::disable()
{
    Cyg_Scheduler::lock();

    if( enabled ) counter->rem_alarm(this);

    Cyg_Scheduler::unlock();
}

// -------------------------------------------------------------------------
// Get the current time values from the alarm

void Cyg_Alarm::get_times(
        cyg_tick_count  *t,      // Next trigger time
        cyg_tick_count  *i       // Current interval
        )
{
    // Lock the scheduler while we do this to avoid
    // race conditions.
    Cyg_Scheduler::lock();

    if( t != NULL ) *t = trigger;
    if( i != NULL ) *i = interval;
    
    Cyg_Scheduler::unlock();
}

//==========================================================================
// System clock object

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

class Cyg_RealTimeClock
    : public Cyg_Clock
{
    Cyg_Interrupt       interrupt;

    static cyg_uint32 isr(cyg_vector vector, CYG_ADDRWORD data);

    static void dsr(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data);

    Cyg_RealTimeClock();

    static Cyg_RealTimeClock rtc;
};

Cyg_Clock::cyg_resolution rtc_resolution = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;

//Cyg_RealTimeClock Cyg_RealTimeClock::rtc __attribute__((init_priority (1)));

Cyg_RealTimeClock Cyg_RealTimeClock::rtc CYG_INIT_PRIORITY( CLOCK );

// -------------------------------------------------------------------------

Cyg_RealTimeClock::Cyg_RealTimeClock()
    : Cyg_Clock(rtc_resolution),
      interrupt(CYGNUM_HAL_INTERRUPT_RTC,
                CYGNUM_KERNEL_COUNTERS_CLOCK_ISR_PRIORITY,
                (CYG_ADDRWORD)this, isr, dsr)
{
    CYG_REPORT_FUNCTION();

    HAL_CLOCK_INITIALIZE( CYGNUM_KERNEL_COUNTERS_RTC_PERIOD );
    
    interrupt.attach();

    interrupt.unmask_interrupt(CYGNUM_HAL_INTERRUPT_RTC);

    Cyg_Clock::real_time_clock = this;
}

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
cyg_tick_count total_clock_latency, total_clock_interrupts;
cyg_int32 min_clock_latency = 0x7FFFFFFF;
cyg_int32 max_clock_latency = 0;
bool measure_clock_latency = false;
#endif

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY)
cyg_tick_count total_clock_dsr_latency, total_clock_dsr_calls;
cyg_int32 min_clock_dsr_latency = 0x7FFFFFFF;
cyg_int32 max_clock_dsr_latency = 0;
cyg_uint32 clock_dsr_start = 0;
#endif

// -------------------------------------------------------------------------

cyg_uint32 Cyg_RealTimeClock::isr(cyg_vector vector, CYG_ADDRWORD data)
{
//    CYG_REPORT_FUNCTION();

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY) && defined(HAL_CLOCK_LATENCY)
    if (measure_clock_latency) {
        cyg_int32 delta;
        HAL_CLOCK_LATENCY(&delta);
        // Note: Ignore a latency of 0 when finding min_clock_latency.
        if (delta > 0) {
            // Valid delta measured
            total_clock_latency += delta;
            total_clock_interrupts++;
            if (min_clock_latency > delta) min_clock_latency = delta;
            if (max_clock_latency < delta) max_clock_latency = delta;
        }
    }
#endif

    CYG_INSTRUMENT_CLOCK( ISR, 0, 0);

    HAL_CLOCK_RESET( CYGNUM_HAL_INTERRUPT_RTC, CYGNUM_KERNEL_COUNTERS_RTC_PERIOD );

    Cyg_Interrupt::acknowledge_interrupt(CYGNUM_HAL_INTERRUPT_RTC);

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY)
    HAL_CLOCK_READ(&clock_dsr_start);
#endif    
    return Cyg_Interrupt::CALL_DSR|Cyg_Interrupt::HANDLED;
}

// -------------------------------------------------------------------------

void Cyg_RealTimeClock::dsr(cyg_vector vector, cyg_ucount32 count, CYG_ADDRWORD data)
{
//    CYG_REPORT_FUNCTION();

#if defined(CYGVAR_KERNEL_COUNTERS_CLOCK_DSR_LATENCY) && defined(HAL_CLOCK_LATENCY)
    if (measure_clock_latency) {
        cyg_int32 delta;
        HAL_CLOCK_READ((cyg_uint32 *)&delta);
        delta -= clock_dsr_start;
        // Note: Ignore a latency of <= 0 when finding min_clock_latency.
        if (delta > 0 ) {
            // Valid delta measured
            total_clock_dsr_latency += delta;
            total_clock_dsr_calls++;
            if (min_clock_dsr_latency > delta) min_clock_dsr_latency = delta;
            if (max_clock_dsr_latency < delta) max_clock_dsr_latency = delta;
        }
    }
#endif    

    Cyg_RealTimeClock *rtc = (Cyg_RealTimeClock *)data;

    CYG_INSTRUMENT_CLOCK( TICK_START,
                          rtc->current_value_lo(),
                          rtc->current_value_hi());
                          
    rtc->tick( count );

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
#if    0 == CYGINT_KERNEL_SCHEDULER_UNIQUE_PRIORITIES

    // If timeslicing is enabled, call the scheduler to
    // handle it. But not if we have unique priorities.
    
    Cyg_Scheduler::scheduler.timeslice();

#endif
#endif

    CYG_INSTRUMENT_CLOCK( TICK_END,
                          rtc->current_value_lo(),
                          rtc->current_value_hi());
    
}

#endif

// -------------------------------------------------------------------------
// EOF common/clock.cxx
