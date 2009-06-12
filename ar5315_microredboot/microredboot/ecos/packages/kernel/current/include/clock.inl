#ifndef CYGONCE_KERNEL_CLOCK_INL
#define CYGONCE_KERNEL_CLOCK_INL

//==========================================================================
//
//      clock.inl
//
//      Clock and Alarm class inlines
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
// Date:        1997-09-30
// Purpose:     Define Clock and Alarm class inlines
// Description: Define inline functions for counter, clock and alarm
//              classes.
// Usage:       #include <cyg/kernel/clock.hxx>
//              #include <cyg/kernel/clock.inl>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/clock.hxx>

// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// Counter class inlines

// Return current value of counter
inline cyg_tick_count Cyg_Counter::current_value()
{
    return counter;
}

inline cyg_uint32 Cyg_Counter::current_value_lo()
{
    return counter&0xFFFFFFFF;
}

inline cyg_uint32 Cyg_Counter::current_value_hi()
{
    return (counter>>32)&0xFFFFFFFF;
}

// Set the counter's current value
inline void Cyg_Counter::set_value( cyg_tick_count new_value)
{
    counter = new_value;
}
        
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------
// Clock class inlines

// Return current resolution
inline Cyg_Clock::cyg_resolution Cyg_Clock::get_resolution()
{
    return resolution;
}

// Set new resolution
inline void Cyg_Clock::set_resolution(                
        Cyg_Clock::cyg_resolution new_resolution
        )
{
    resolution = new_resolution;
}

inline cyg_tick_count Cyg_Clock::convert(
    cyg_tick_count value,
    struct converter *pcc )
{
    cyg_uint64 t = (cyg_uint64)value;
    // Do this in an order to prevent overflow at the expense of
    // accuracy:
    t *= pcc->mul1;
    t += pcc->div1 / 2;
    t /= pcc->div1;
    t *= pcc->mul2;
    t += pcc->div2 / 2;
    t /= pcc->div2;
    // The alternative would be to do the 2 multiplies first
    // for smaller arguments.
    return (cyg_tick_count)t;
}

// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_CLOCK_INL
// EOF clock.inl
