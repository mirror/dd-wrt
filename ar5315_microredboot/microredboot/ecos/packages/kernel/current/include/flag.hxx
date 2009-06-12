#ifndef CYGONCE_KERNEL_FLAG_HXX
#define CYGONCE_KERNEL_FLAG_HXX

//==========================================================================
//
//      flag.hxx
//
//      Flag object class declarations
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-02-10
// Purpose:     Define Flag class interfaces
// Description: The classes defined here provide the APIs for flags.
// Usage:       #include <cyg/kernel/flag.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/kernel/ktypes.h>
#include <cyg/infra/cyg_ass.h>                     // assertion macros
#include <cyg/kernel/thread.hxx>                   // Cyg_Thread

// -------------------------------------------------------------------------

// Flag object.  This class implements a queue of threads waiting for a
// boolean expression of the flag value (an integer) to become true; all
// relevant threads are awoken, not just the first.  A variant on this is
// the single-bit flag, which is implemented by means of default arguments.

#ifdef CYGIMP_FLAGS_16BIT
typedef cyg_uint16 Cyg_FlagValue;
#define CYG_FLAGS_SIZE 16
#endif
#ifdef CYGIMP_FLAGS_32BIT
typedef cyg_uint32 Cyg_FlagValue;
#define CYG_FLAGS_SIZE 32
#endif
#ifdef CYGIMP_FLAGS_64BIT
typedef cyg_uint64 Cyg_FlagValue;
#define CYG_FLAGS_SIZE 64
#endif

#ifndef CYG_FLAGS_SIZE
typedef cyg_uint32 Cyg_FlagValue;
#define CYG_FLAGS_SIZE 32
#endif



class Cyg_Flag
{
private:
    Cyg_FlagValue value;

    class FlagWaitInfo
    {
    public:
        Cyg_FlagValue   allmask;        // these are separate words to
        Cyg_FlagValue   anymask;        // save time in wakeup.
        Cyg_FlagValue   value_out;      // return the value that satisfied
        cyg_bool        do_clear;

        FlagWaitInfo() { value_out = 0; }
    };

    Cyg_ThreadQueue     queue;          // Queue of waiting threads

public:

    CYGDBG_DEFINE_CHECK_THIS
    
    Cyg_Flag( Cyg_FlagValue init = 0 ); // Constructor
    ~Cyg_Flag();                        // Destructor
        
    void
    setbits( Cyg_FlagValue arg = ~0 );  // -OR- the arg in
    // not inlined; this function awakens affected threads.

    void
    maskbits( Cyg_FlagValue arg = 0 );  // -AND- it in
    // this is not inlined because it needs to lock the scheduler;
    // it only really does value &= arg; nobody can be awoken in consequence.

    typedef cyg_uint8 WaitMode;
    // These values are chosen to map directly to uITRON for emulation
    // purposes:
    static const WaitMode AND = 0;      // all specified bits must be set
    static const WaitMode OR  = 2;      // any specified bit must be set
    static const WaitMode CLR = 1;      // clear value when satisfied
    static const WaitMode MASK= 3;      // might be useful

    // Wait for a match on our pattern, according to the flags given.
    // Return the matching value, or zero if interrupted.
    Cyg_FlagValue
    wait( Cyg_FlagValue pattern, WaitMode mode );

    // Wait for a match on our pattern, with an absolute timeout.
    // Return the matching value, or zero if timed out/interrupted.
    // (zero cannot match any pattern).
#ifdef CYGFUN_KERNEL_THREADS_TIMER
    Cyg_FlagValue
    wait( Cyg_FlagValue pattern, WaitMode mode,
          cyg_tick_count abs_timeout );
#endif
    // Test for a match on our pattern, according to the flags given.
    // Return the matching value if success, else zero.
    Cyg_FlagValue
    poll( Cyg_FlagValue pattern, WaitMode mode ); 

    inline Cyg_FlagValue
    peek()                              // Get current value
    {
        return value;                   // NOT atomic wrt threads
    }

    inline cyg_bool
    waiting()                           // Any threads waiting?
    {
        return !queue.empty();
    }
};



// -------------------------------------------------------------------------
#endif // ifndef CYGONCE_KERNEL_FLAG_HXX
// EOF flag.hxx
