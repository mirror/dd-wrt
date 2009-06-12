//==========================================================================
//
//      mbox.cxx
//
//      Mbox mbox template class implementation
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
// Date:        1998-02-11
// Purpose:     Mbox implementation
// Description: This file contains the implementations of the mbox class
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros
#include <cyg/kernel/instrmnt.h>       // instrumentation

#include <cyg/kernel/thread.inl>       // Cyg_Thread inlines

#include <cyg/kernel/mbox.hxx>         // our own header

#ifndef CYGIMP_MBOXT_INLINE            // force inlining
#define CYGIMP_MBOXT_INLINE            // of implementation
#endif

#ifdef CYGIMP_MBOX_USE_MBOXT_PLAIN
#include <cyg/kernel/mboxt.inl>        // mbox template implementation
#else
#include <cyg/kernel/mboxt2.inl>       // mbox template implementation
#endif

// -------------------------------------------------------------------------
// This module exists to cause exactly one instance of these functions to
// exist; this is just like a vanilla class, except we use the template
// class to acquire an implementation.  The template functions are inlined
// in each of these methods.


// -------------------------------------------------------------------------
// Constructor

Cyg_Mbox::Cyg_Mbox()
{
}

// -------------------------------------------------------------------------
// Destructor

Cyg_Mbox::~Cyg_Mbox()
{
}

// -------------------------------------------------------------------------
// debugging/assert function

#ifdef CYGDBG_USE_ASSERTS
cyg_bool 
Cyg_Mbox::check_this(cyg_assert_class_zeal zeal) const
{
    return m.check_this(zeal);
}
#endif

// -------------------------------------------------------------------------
// now the members themselves:
    
void *
Cyg_Mbox::get()
{
    void * p;
    if ( ! m.get( p ) )
        return NULL;
    return p;
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
void *
Cyg_Mbox::get( cyg_tick_count timeout )
{
    void * p;
    if ( ! m.get( p, timeout ) )
        return NULL;
    return p;
}
#endif

void *
Cyg_Mbox::tryget()
{
    void * p;
    if ( ! m.tryget( p ) )
        return NULL;
    return p;
}

#ifdef  CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT
cyg_bool
Cyg_Mbox::put( void *item )
{
    return m.put( item );
}

#ifdef CYGFUN_KERNEL_THREADS_TIMER
cyg_bool
Cyg_Mbox::put( void *item, cyg_tick_count timeout )
{
    return m.put( item, timeout );
}
#endif
#endif // CYGMFN_KERNEL_SYNCH_MBOXT_PUT_CAN_WAIT

cyg_bool
Cyg_Mbox::tryput( void *item )
{
    return m.tryput( item );
}

void *
Cyg_Mbox::peek_item()                   // Get next item to be returned
{
    void *p;
    if ( ! m.peek_item( p ) )
        return NULL;
    return p;
}

// -------------------------------------------------------------------------
// EOF mbox.cxx
