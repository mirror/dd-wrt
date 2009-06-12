//==========================================================================
//
//      instrmnt/meminst.cxx
//
//      Memory buffer instrumentation functions
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
// Contributors: nickg, andrew.lunn@ascom.ch
// Date:        1997-10-27
// Purpose:     Instrumentation functions
// Description: The functions in this file are implementations of the
//              standard instrumentation functions that place records
//              into a memory buffer.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation

#include <cyg/kernel/intr.hxx>         // interrupt control
#include <cyg/kernel/sched.hxx>        // scheduler defines

#include <cyg/kernel/sched.inl>        // scheduler inlines
#include <cyg/kernel/clock.inl>        // clock inlines

#ifdef CYGPKG_KERNEL_INSTRUMENT

// -------------------------------------------------------------------------
// Instrumentation record.

struct Instrument_Record
{
    CYG_WORD16  type;                   // record type
    CYG_WORD16  thread;                 // current thread id
    CYG_WORD    timestamp;              // 32 bit timestamp
    CYG_WORD    arg1;                   // first arg
    CYG_WORD    arg2;                   // second arg
};

// -------------------------------------------------------------------------
// Buffer base and end. This buffer must be a whole number of 

#ifdef CYGVAR_KERNEL_INSTRUMENT_EXTERNAL_BUFFER

externC Instrument_Record       instrument_buffer[];
externC cyg_uint32              instrument_buffer_size;

#else

extern "C"
{
    
Instrument_Record       instrument_buffer[CYGNUM_KERNEL_INSTRUMENT_BUFFER_SIZE];
   
cyg_uint32              instrument_buffer_size = CYGNUM_KERNEL_INSTRUMENT_BUFFER_SIZE;
    
};

#endif

extern "C"
{

#define instrument_buffer_start instrument_buffer[0]
#define instrument_buffer_end   instrument_buffer[instrument_buffer_size]

extern "C"
{
Instrument_Record       *instrument_buffer_pointer = &instrument_buffer_start;
};

#ifdef CYGDBG_KERNEL_INSTRUMENT_FLAGS

// This array contains a 32 bit word for each event class. The
// bits in the word correspond to events. By setting or clearing
// the appropriate bit, the selected instrumentation event may
// be enabled or disabled dynamically.
    
cyg_uint32 instrument_flags[(CYG_INSTRUMENT_CLASS_MAX>>8)+1];
    
#endif
    
};

// -------------------------------------------------------------------------

#ifdef CYGPKG_KERNEL_SMP_SUPPORT

HAL_SPINLOCK_TYPE instrument_lock = HAL_SPINLOCK_INIT_CLEAR;

#else

#define HAL_SPINLOCK_SPIN( __lock )

#define HAL_SPINLOCK_CLEAR( __lock )

#endif

// -------------------------------------------------------------------------

void cyg_instrument( cyg_uint32 type, CYG_ADDRWORD arg1, CYG_ADDRWORD arg2 )
{

    cyg_uint32 old_ints;

#ifdef CYGDBG_KERNEL_INSTRUMENT_FLAGS    
    
    cyg_ucount8 cl = (type>>8)&0xff;
    cyg_ucount8 event = type&0xff;

    if( instrument_flags[cl] & (1<<event) )
#endif        
    {
        HAL_DISABLE_INTERRUPTS(old_ints);
        HAL_SPINLOCK_SPIN( instrument_lock );
        
        Instrument_Record *p = instrument_buffer_pointer;
        Cyg_Thread *t = Cyg_Scheduler::get_current_thread();
        p->type             = type;
        p->thread           = (t==0)?0x0FFF:t->get_unique_id();
#ifdef CYGPKG_KERNEL_SMP_SUPPORT
        // Add CPU id to in top 4 bytes of thread id
        p->thread           = (p->thread&0x0FFF)|(CYG_KERNEL_CPU_THIS()<<12);
#endif        
#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK        
//        p->timestamp        = Cyg_Clock::real_time_clock->current_value_lo();
        HAL_CLOCK_READ( &p->timestamp );
#else
        p->timestamp        = 0;
#endif
        p->arg1             = arg1;
        p->arg2             = arg2;
    
        p++;
#ifdef CYGDBG_KERNEL_INSTRUMENT_BUFFER_WRAP
        if( p == &instrument_buffer_end )
            instrument_buffer_pointer = &instrument_buffer_start;
        else instrument_buffer_pointer = p;
#else
        // when not wrapping, just continue to put further entries
        // in the last slot.
        if( p != &instrument_buffer_end )
            instrument_buffer_pointer = p;
#endif
        HAL_SPINLOCK_CLEAR( instrument_lock );        
        HAL_RESTORE_INTERRUPTS(old_ints);
    }
    
    return;
}

// -------------------------------------------------------------------------
// Functions to enable and disable selected instrumentation events
// when the flags are enabled.

#ifdef CYGDBG_KERNEL_INSTRUMENT_FLAGS

externC void cyg_instrument_enable( cyg_uint32 cl, cyg_uint32 event)
{
    if( 0 != event )
        instrument_flags[cl>>8] |= 1<<event;
    else
        instrument_flags[cl>>8] = ~0;
}

externC void cyg_instrument_disable( cyg_uint32 cl, cyg_uint32 event)
{
    if( 0 != event )
        instrument_flags[cl>>8] &= ~(1<<event);
    else
        instrument_flags[cl>>8] = 0;

}

externC cyg_bool cyg_instrument_state( cyg_uint32 cl, cyg_uint32 event)
{
    return (instrument_flags[cl>>8] & (1<<event)) != 0;
}

#endif

// -------------------------------------------------------------------------

#ifdef CYGDBG_KERNEL_INSTRUMENT_MSGS
#define CYGDBG_KERNEL_INSTRUMENT_MSGS_DEFINE_TABLE
#include <cyg/kernel/instrument_desc.h>
#define NELEM(x) (sizeof(x)/sizeof*(x))
externC char * cyg_instrument_msg(CYG_WORD16 type) {

  struct instrument_desc_s *record;
  struct instrument_desc_s *end_record;
  CYG_WORD cl, event;

  record = instrument_desc;
  end_record = &instrument_desc[NELEM(instrument_desc)-1];
  cl = type & 0xff00;
  event = type & 0x00ff;

  while ((record != end_record) && (record->num != cl)) {
    record++;
  }

  if (record->num == cl) {
    record++;
    while ((record != end_record) && (record->num != event) &&
           (record->num < 0xff)) {
      record++;
    }

    if (record->num == event) {
      return (record->msg);
    }
  }
  return("Unknown event");
}
#endif // CYGDBG_KERNEL_INSTRUMENT_MSGS
#endif // CYGPKG_KERNEL_INSTRUMENT

// EOF instrmnt/meminst.cxx
