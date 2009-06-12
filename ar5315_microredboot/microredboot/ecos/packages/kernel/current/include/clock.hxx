#ifndef CYGONCE_KERNEL_CLOCK_HXX
#define CYGONCE_KERNEL_CLOCK_HXX

//==========================================================================
//
//      clock.hxx
//
//      Clock and Alarm class declaration(s)
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
// Author(s):   nickg
// Contributors:        nickg
// Date:        1997-09-09
// Purpose:     Define Clock and Alarm class interfaces
// Description: The classes defined here collectively implement the
//              internal API used to create, configure and manage Counters,
//              Clocks and Alarms.
// Usage:       #include <cyg/kernel/clock.hxx>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>            // assertion macros

#include <cyg/infra/clist.hxx>

// -------------------------------------------------------------------------
// Forward definitions and typedefs.

class Cyg_Alarm;

typedef void cyg_alarm_fn(Cyg_Alarm *alarm, CYG_ADDRWORD data);

typedef Cyg_CList_T<Cyg_Alarm> Cyg_Alarm_List;

// -------------------------------------------------------------------------
// Counter object.

class Cyg_Counter
{

    friend class Cyg_Alarm;

#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST)

    Cyg_Alarm_List      alarm_list;     // Linear list of Alarms

#elif defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)

    Cyg_Alarm_List      alarm_list[CYGNUM_KERNEL_COUNTERS_MULTI_LIST_SIZE];
    
#endif

    volatile cyg_tick_count counter;    // counter value

    cyg_uint32          increment;      // increment per tick

    // Add an alarm to this counter
    void add_alarm( Cyg_Alarm *alarm );

    // Remove an alarm from this counter
    void rem_alarm( Cyg_Alarm *alarm );
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Counter(
        cyg_uint32      increment = 1
    );

    ~Cyg_Counter();
    
    // Return current value of counter
    cyg_tick_count current_value();

    // Return low and high halves of the
    // counter value.
    cyg_uint32 current_value_lo();
    cyg_uint32 current_value_hi();
    
    // Set the counter's current value
    void set_value( cyg_tick_count new_value);
        
    // Advance counter by some number of ticks
    void tick( cyg_uint32 ticks = 1);

};

// -------------------------------------------------------------------------
// Clock class. This is derived from a Counter and defines extra
// features to support clock-like behaviour.

class Cyg_Clock
    : public Cyg_Counter
{

public:

    CYGDBG_DEFINE_CHECK_THIS

    // This structure allows a more accurate representation
    // of the resolution than a single integer would allow.
    // The resolution is defined as dividend/divisor nanoseconds
    // per tick.
    struct cyg_resolution {
        cyg_uint32  dividend;
        cyg_uint32  divisor;
    };

private:

    cyg_resolution      resolution;     // Current clock resolution

public:

    Cyg_Clock(                          // Create clock with given resolution
        cyg_resolution resolution
        );

    ~Cyg_Clock();                       // Destructor
        
    cyg_resolution get_resolution();    // Return current resolution

    void set_resolution(                // Set new resolution
        cyg_resolution resolution
        ); 

    // There is a need for converting from "other" ticks to clock ticks.
    // We will construct 4 numbers to do the conversion as:
    //   clock_ticks = (((otherticks*mul1)/div1)*mul2/div2)
    // with the values chosen to minimize the possibility of overflow.
    // Do the arithmetic in cyg_uint64s throughout.
    struct converter {
        cyg_uint64 mul1, div1, mul2, div2;
    };

    // There are two of these because the 4 numbers are different depending
    // on the direction of the conversion, to prevent loss of significance.
    // NB these relate to the resolution of the clock object they are
    // called against, not necessarily "the" system real time clock.
    void get_other_to_clock_converter( cyg_uint64 ns_per_other_tick,
                                       struct converter *pcc );

    void get_clock_to_other_converter( cyg_uint64 ns_per_other_tick,
                                       struct converter *pcc );

    // A utility to perform the conversion in the obvious way, with
    // rounding to nearest at each stage.  Static because it uses a
    // previously acquired converter.
    static cyg_tick_count convert( cyg_tick_count value,
                                   struct converter *pcc );
        
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK 
    
    // There is a system supplied real time clock...

    static Cyg_Clock *real_time_clock;

#endif    
        
};

// -------------------------------------------------------------------------
// Alarm class. An alarm may be attached to a counter (or a clock) to be
// called when the trigger value is reached.

class Cyg_Alarm
#if defined(CYGIMP_KERNEL_COUNTERS_SINGLE_LIST) || defined(CYGIMP_KERNEL_COUNTERS_MULTI_LIST)
    : public Cyg_DNode_T<Cyg_Alarm>
#endif
{
    friend class Cyg_Counter;
    
protected:
    Cyg_Counter         *counter;       // Attached to this counter/clock

    cyg_alarm_fn        *alarm;         // Call-back function

    CYG_ADDRWORD        data;           // Call-back data

    cyg_tick_count      trigger;        // Absolute trigger time

    cyg_tick_count      interval;       // Retrigger interval

    cyg_bool            enabled;        // True if enabled

    Cyg_Alarm();

    void synchronize( void );           // deal with times in the past,
                                        // make next alarm in synch.
    
public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Alarm                           // Constructor
    (
        Cyg_Counter     *counter,       // Attached to this counter
        cyg_alarm_fn    *alarm,         // Call-back function
        CYG_ADDRWORD    data            // Call-back data
        );

    ~Cyg_Alarm();                       // Destructor
        
    void initialize(                    // Initialize Alarm
        cyg_tick_count    trigger,      // Absolute trigger time
        cyg_tick_count    interval = 0  // Relative retrigger interval
        );

    void enable();                      // Ensure alarm enabled

    void disable();                     // Ensure alarm disabled
    
    void get_times(
        cyg_tick_count  *trigger,       // Next trigger time
        cyg_tick_count  *interval       // Current interval
        );
};

// -------------------------------------------------------------------------

#endif // ifndef CYGONCE_KERNEL_CLOCK_HXX
// EOF clock.hxx
