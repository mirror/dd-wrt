//===========================================================================
//
//      clock.cxx
//
//      ISO C date and time implementation for clock()
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
// Contributors: jlarmour
// Date:         1999-03-05
// Purpose:      Provides an implementation of the ISO C function clock()
//               from ISO C section 7.12.2.1
// Description:  This file uses the kernel real time clock to determine
//               the complete running time of the system - since we only
//               have one task, even though there are perhaps multiple threads
//               that is still the running time of the "program"
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/libc_time.h>    // Configuration header

#ifdef CYGSEM_LIBC_TIME_CLOCK_WORKING
# include <pkgconf/kernel.h> // Kernel config header
#endif

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_ass.h>     // Assertion infrastructure
#include <cyg/infra/cyg_trac.h>    // Tracing infrastructure

#include <time.h>                  // Header for all time-related functions

#ifdef CYGSEM_LIBC_TIME_CLOCK_WORKING
# include <cyg/kernel/clock.hxx>   // Kernel clock definitions
# include <cyg/kernel/clock.inl>   // Kernel clock inline functions
#endif


// TRACE

# if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_TIME_CLOCK_TRACE_LEVEL)
static int clock_trace = CYGNUM_LIBC_TIME_CLOCK_TRACE_LEVEL;
#  define TL1 (0 < clock_trace)
# else
#  define TL1 (0)
# endif

// FUNCTIONS

externC clock_t
clock( void )
{
    CYG_REPORT_FUNCNAMETYPE( "clock", "returning clock tick %d" );
    CYG_REPORT_FUNCARGVOID();

#ifdef CYGSEM_LIBC_TIME_CLOCK_WORKING
    cyg_tick_count curr_clock;            // kernel clock value
    Cyg_Clock::cyg_resolution resolution; // kernel clock resolution
    clock_t clocks;
    unsigned long long temp;

    CYG_TRACE0( TL1, "getting clock resolution" );
    
    // get the resolution
    resolution = Cyg_Clock::real_time_clock->get_resolution();

    CYG_TRACE2( TL1, "got resolution dividend %d divisor %d. Getting "
                "clock value", resolution.dividend, resolution.divisor );

    // get the value
    curr_clock = Cyg_Clock::real_time_clock->current_value();

    CYG_TRACE1( TL1, "got clock value %d", curr_clock );
    
    // scale the value so that clock()/CLOCKS_PER_SEC works
    // We use an unsigned long long to avoid overflow as the dividend
    // and divisors tend to be huge
    temp = (1000000000 / CLOCKS_PER_SEC);
    temp *= resolution.divisor;
    temp = (unsigned long long)curr_clock * resolution.dividend / temp;
    clocks = (clock_t)temp;
    
    CYG_REPORT_RETVAL( clocks );
    return clocks;

#else // i.e. ifndef CYGSEM_LIBC_TIME_CLOCK_WORKING
    CYG_REPORT_RETVAL( -1 );
    return (clock_t) -1;
#endif

} // clock()


// EOF clock.cxx
