#ifndef CYGONCE_HAL_XPIC_H
#define CYGONCE_HAL_XPIC_H

//=============================================================================
//
//      hal_xpic.h
//
//      HAL eXternal Programmable Interrupt Controller support
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
// Author(s):   nickg, gthomas, hmt
// Contributors:        nickg, gthomas, hmt
// Date:        1999-01-28
// Purpose:     Define Interrupt support
// Description: The macros defined here provide the HAL APIs for handling
//              an external interrupt controller, and which interrupt is
//              used for what.
//              
// Usage:
//              #include <cyg/hal/hal_intr.h> // which includes this file
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/hal_hwio.h> // HAL_SPARC_86940_READ/WRITE

//-----------------------------------------------------------------------------
// Interrupt controller access
// 
// In the Fuitsu SPARClite Evaluation Boards, when the external IRC (86940)
// is used (set Switch 1, position 8 (SW1#8)), interrupts are as follows:
// 
// 15   : NMI Push Switch (SW7)
// 14   : N_INT# Ethernet LAN Controller (MB86964)
// 13   : EX_IRQ13 from expansion board, active HIGH
// 12   : EX_IRQ12 from expansion board, active LOW
// 11   : EX_IRQ11 from expansion board, active LOW
// 10   : RRDY0 Serial CH0 receive ready signal
//  9   : TRDY0 Serial CH0 transmit ready signal
//  8   : TIMER1 Timer 1 output counter
//  7   : RRDY1 Serial CH1 receive ready signal
//  6   : TRDY1 Serial CH1 transmit ready signal
//  5   : EX_IRQ5 from expansion board, active LOW
//  4   : EX_IRQ4 from expansion board, active LOW
//  3   : EX_IRQ3 from expansion board, active HIGH
//  2   : EX_IRQ2 from expansion board, active LOW
//  1   : TIMER2 Timer 2 output counter

// The vector used by the Real time clock

//#define CYG_VECTOR_RTC                  CYG_VECTOR_INTERRUPT_8
#define CYGNUM_HAL_INTERRUPT_RTC       CYGNUM_HAL_VECTOR_INTERRUPT_8

#define HAL_SPARC_86940_REG_IRC_TRGM0 ( 0 * 4 )
#define HAL_SPARC_86940_REG_IRC_TRGM1 ( 1 * 4 )
#define HAL_SPARC_86940_REG_IRC_RQSNS ( 2 * 4 )
#define HAL_SPARC_86940_REG_IRC_RQCLR ( 3 * 4 )
#define HAL_SPARC_86940_REG_IRC_IMASK ( 4 * 4 )
#define HAL_SPARC_86940_REG_IRC_CLIRL ( 5 * 4 )

#define HAL_SPARC_86940_FLAG_CLIRL_CL (0x10)

#define HAL_SPARC_86940_IRC_IMASK_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_IRC_IMASK, r )

#define HAL_SPARC_86940_IRC_IMASK_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_IRC_IMASK, v )

#define HAL_SPARC_86940_IRC_RQSNS_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_IRC_RQSNS, r )

#define HAL_SPARC_86940_IRC_RQCLR_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_IRC_RQCLR, v )

#define HAL_SPARC_86940_IRC_CLIRL_READ( r ) \
            HAL_SPARC_86940_READ( HAL_SPARC_86940_REG_IRC_CLIRL, r )

#define HAL_SPARC_86940_IRC_CLIRL_WRITE( v ) \
            HAL_SPARC_86940_WRITE( HAL_SPARC_86940_REG_IRC_CLIRL, v )

//-----------------------------------------------------------------------------

#define HAL_INTERRUPT_MASK( _vector_ ) CYG_MACRO_START                      \
    cyg_uint32 _traps_, _mask_;                                             \
    HAL_DISABLE_TRAPS( _traps_ );                                           \
    HAL_SPARC_86940_IRC_IMASK_READ( _mask_ );                               \
    _mask_ |= (1 << (_vector_) );                                           \
    HAL_SPARC_86940_IRC_IMASK_WRITE( _mask_ );                              \
    HAL_SPARC_86940_IRC_RQCLR_WRITE( (1 << (_vector_) ) );                  \
    HAL_SPARC_86940_IRC_CLIRL_WRITE( HAL_SPARC_86940_FLAG_CLIRL_CL );       \
    HAL_RESTORE_INTERRUPTS( _traps_ );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ ) CYG_MACRO_START                    \
    cyg_uint32 _traps_, _mask_;                                             \
    HAL_DISABLE_TRAPS( _traps_ );                                           \
    HAL_SPARC_86940_IRC_IMASK_READ( _mask_ );                               \
    _mask_ &=~ (1 << (_vector_) );                                          \
    HAL_SPARC_86940_IRC_IMASK_WRITE( _mask_ );                              \
    HAL_RESTORE_INTERRUPTS( _traps_ );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ ) CYG_MACRO_START               \
    cyg_uint32 _traps_;                                                     \
    cyg_uint32 _req_, _irl_;                                                \
    HAL_DISABLE_TRAPS( _traps_ );                                           \
    HAL_SPARC_86940_IRC_RQCLR_WRITE( (1 << (_vector_) ) );                  \
    do {                                                                    \
        HAL_SPARC_86940_IRC_CLIRL_WRITE( HAL_SPARC_86940_FLAG_CLIRL_CL );   \
        HAL_SPARC_86940_IRC_RQSNS_READ( _req_ );                            \
        HAL_SPARC_86940_IRC_CLIRL_READ( _irl_ );                            \
        _req_ &= (1 << (_vector_)); /* if there really is a new intr  */    \
        _irl_ &= 0x0f;              /* then get out - else poll until */    \
    } while ( (!_req_) && (_irl_ == (_vector_)) ); /* no intr here    */    \
    HAL_RESTORE_INTERRUPTS( _traps_ );                                      \
CYG_MACRO_END

// Set an interrupt source's sensitivity:
// _level_ != 0 ? level-sensitive : edge-sensitive
// _up_    != 0 ? high/up : low/down
// SPARClite's 86940 has values as follows:
//    0  : high level
//    1  : low level
//    2  : rising edge
//    3  : falling edge
// TRGM0 controls sources 15-8, TRGM1 sources 7-1.

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ ) CYG_MACRO_START  \
    int _reg_ = (8 > (_vector_)) ? HAL_SPARC_86940_REG_IRC_TRGM1            \
                                 : HAL_SPARC_86940_REG_IRC_TRGM0;           \
    int _val_, _myvect_ = (_vector_);                                       \
    cyg_uint32 _traps_;                                                     \
    HAL_DISABLE_TRAPS( _traps_ );                                           \
    (_myvect_) &= 7;                                                        \
    (_myvect_) <<= 1;                                                       \
    HAL_SPARC_86940_READ( _reg_, _val_ );                                   \
    _val_ &=~ (3 << (_myvect_));                                            \
    _val_ |= ( ((_level_) ? 0 : 2) + ((_up_) ? 0 : 1) ) << (_myvect_);      \
    HAL_SPARC_86940_WRITE( _reg_, _val_ );                                  \
    HAL_INTERRUPT_ACKNOWLEDGE( _vector_ );                                  \
    HAL_RESTORE_INTERRUPTS( _traps_ );                                      \
CYG_MACRO_END

// This is not a standard macro - platform dependent for debugging only:
//  o level, up tell you how this source was configured as above.
//  o hipri returns the number of the highest prio requesting interrupt
//  o mask tells you whether this source is masked off
//  o req tells you whether this source is requesting right now.
#define HAL_INTERRUPT_QUERY_INFO( _vector_, _level_, _up_,                  \
                                   _hipri_, _mask_, _req_ ) CYG_MACRO_START \
    int _reg_ = (8 > (_vector_)) ? HAL_SPARC_86940_REG_IRC_TRGM1            \
                                 : HAL_SPARC_86940_REG_IRC_TRGM0;           \
    int _val_, _myvect_ = (_vector_);                                       \
    HAL_SPARC_86940_IRC_IMASK_READ( _val_ );                                \
    _mask_ = (0 != ((1 << (_vector_)) & _val_ ));                           \
    HAL_SPARC_86940_IRC_RQSNS_READ( _val_ );                                \
    _req_ = (0 != ((1 << (_vector_)) & _val_ ));                            \
    HAL_SPARC_86940_IRC_CLIRL_READ( _hipri_ );                              \
    (_myvect_) &= 7;                                                        \
    (_myvect_) <<= 1;                                                       \
    HAL_SPARC_86940_READ( _reg_, _val_ );                                   \
    _val_ >>= (_myvect_);                                                   \
    (_level_) = !(_val_ & 2);                                               \
    (_up_)    = !(_val_ & 1);                                               \
CYG_MACRO_END

// This may set the priority of a vector to a particular level:
// SPARClite does not support this.
#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ ) /* nothing */

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_XPIC_H
// End of hal_xpic.h
