//===========================================================================
//
//      uit_objs.cxx
//
//      uITRON static objects
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-03-13
// Purpose:     uITRON static system objects
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

#include <cyg/compat/uitron/uit_objs.hxx>
                                        // declarations of the objects
                                        // we define below, and everything
                                        // we need to specify them.

#include <cyg/hal/hal_arch.h>           // for CYGNUM_HAL_STACK_SIZE_MINIMUM

// ------------------------------------------------------------------------
// Mboxes have no initializer.
#ifdef CYGPKG_UITRON_MBOXES
#if 0 < CYGNUM_UITRON_MBOXES
Cyg_Mbox                CYG_UITRON_DECL( MBOXES );
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
Cyg_Mbox                *CYG_UITRON_DECL_PTRS( MBOXES );
#endif
#endif
#endif // CYGPKG_UITRON_MBOXES

// ------------------------------------------------------------------------
// Flags have no initializer.
#ifdef CYGPKG_UITRON_FLAGS
#if 0 < CYGNUM_UITRON_FLAGS
Cyg_Flag                CYG_UITRON_DECL( FLAGS );
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
Cyg_Flag                *CYG_UITRON_DECL_PTRS( FLAGS );
#endif
#endif
#endif // CYGPKG_UITRON_FLAGS

// ------------------------------------------------------------------------
// Semaphores have an optional initializer.
#ifdef CYGPKG_UITRON_SEMAS
#if (0 < CYGNUM_UITRON_SEMAS) || \
    defined( CYGDAT_UITRON_SEMA_INITIALIZERS )

#ifndef CYGNUM_UITRON_SEMAS
#error You must define CYGNUM_UITRON_SEMAS
#endif

Cyg_Counting_Semaphore2 CYG_UITRON_DECL( SEMAS )

#ifdef CYGDAT_UITRON_SEMA_INITIALIZERS
// a Macro to ease the construction:
#define CYG_UIT_SEMA( _count_  ) Cyg_Counting_Semaphore2( (cyg_count32)(_count_) )
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
#define CYG_UIT_SEMA_NOEXS       Cyg_Counting_Semaphore2( (cyg_count32)    0     )
#endif
 = {
    CYGDAT_UITRON_SEMA_INITIALIZERS
}
#undef CYG_UIT_SEMA
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
#undef CYG_UIT_SEMA_NOEXS
#endif
#endif // do we have initializers?
; // the end of the declaration, with or without initializer

#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
Cyg_Counting_Semaphore2 *CYG_UITRON_DECL_PTRS( SEMAS );
#endif
#endif
#endif // CYGPKG_UITRON_SEMAS

// ------------------------------------------------------------------------
// tasks MUST be initialized, you must have some.
#ifndef CYGDAT_UITRON_TASK_EXTERNS
#error You must define CYGDAT_UITRON_TASK_EXTERNS
#endif
#ifndef CYGDAT_UITRON_TASK_INITIALIZERS
#error You must define CYGDAT_UITRON_TASK_INITIALIZERS
#endif
#ifndef CYGNUM_UITRON_TASKS
#error You must define CYGNUM_UITRON_TASKS
#endif

// a Macro to ease the construction:
//      "name", priority, proc, stackbase, stacksize
#define CYG_UIT_TASK( _name_, _prio_, _func_, _sb_, _ss_ ) \
  Cyg_Thread(                           \
        (CYG_ADDRWORD)(_prio_),         \
        (_func_),                       \
        (CYG_ADDRWORD)0,                \
        _name_,                         \
        (CYG_ADDRESS)(_sb_),            \
        (cyg_ucount32)(_ss_) )

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
#define CYG_UIT_TASK_NOEXS( _name_, _sb_, _ss_ ) \
  Cyg_Thread(                           \
        (CYG_ADDRWORD)(CYG_SCHED_DEFAULT_INFO), \
        (cyg_thread_entry *)(0),        \
        (CYG_ADDRWORD)0,                \
        _name_,                         \
        (CYG_ADDRESS)(_sb_),            \
        (cyg_ucount32)(_ss_) )
#endif

// FIXME: Xscale tools currently in use have a preprocessor bug causing
// the below #ifs to be misinterpreted. Therefore a *temporary*
// workaround is included to define a MAX macro, and change
// CYGDAT_UITRON_TASK_EXTERNS and CYGDAT_UITRON_TASK_INITIALISERS in
// the CDL to use it.
#ifdef XSCALECPPFIXEDSOMETIME

#ifdef CYGNUM_HAL_STACK_SIZE_MINIMUM
# ifdef CYGNUM_UITRON_STACK_SIZE
#  if CYGNUM_UITRON_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_MINIMUM

// then override the configured stack size
#   undef CYGNUM_UITRON_STACK_SIZE
#   define CYGNUM_UITRON_STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM

#  endif // CYGNUM_UITRON_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_MINIMUM
# endif // CYGNUM_UITRON_STACK_SIZE
#endif // CYGNUM_HAL_STACK_SIZE_MINIMUM

#else
#define MAX(_x_,_y_) ((_x_) > (_y_) ? (_x_) : (_y_))
#endif

// declare the symbols used in the initializer
CYGDAT_UITRON_TASK_EXTERNS

Cyg_Thread      CYG_UITRON_DECL( TASKS ) =
{
    CYGDAT_UITRON_TASK_INITIALIZERS
};

#undef CYG_UIT_TASK
#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
#undef CYG_UIT_TASK_NOEXS
#endif

#ifdef CYGIMP_THREAD_PRIORITY
// An ancillary array of priorities, for managing the "original" prio
cyg_priority
cyg_uitron_task_initial_priorities[ CYG_UITRON_NUM( TASKS ) ];
#endif

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
Cyg_Thread              *CYG_UITRON_DECL_PTRS( TASKS );
#endif

// ------------------------------------------------------------------------
// fixed memory pools MUST be initialized, IF you have some.
#ifdef CYGPKG_UITRON_MEMPOOLFIXED
#if (0 < CYGNUM_UITRON_MEMPOOLFIXED) || \
    defined (CYGDAT_UITRON_MEMPOOLFIXED_INITIALIZERS) || \
    defined (CYGDAT_UITRON_MEMPOOLFIXED_EXTERNS)

#ifndef CYGDAT_UITRON_MEMPOOLFIXED_EXTERNS
#error You must define CYGDAT_UITRON_MEMPOOLFIXED_EXTERNS
#endif
#ifndef CYGDAT_UITRON_MEMPOOLFIXED_INITIALIZERS
#error You must define CYGDAT_UITRON_MEMPOOLFIXED_INITIALIZERS
#endif
#ifndef CYGNUM_UITRON_MEMPOOLFIXED
#error You must define CYGNUM_UITRON_MEMPOOLFIXED
#endif

// declare the symbols used in the initializer
CYGDAT_UITRON_MEMPOOLFIXED_EXTERNS

// a Macro to ease the construction: addr, size, blocksize
#define CYG_UIT_MEMPOOLFIXED( _a_, _s_, _bs_ ) Cyg_Mempool_Fixed( \
    (cyg_uint8 *)(_a_), (cyg_int32)(_s_), (CYG_ADDRWORD)(_bs_) )
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
// note that this just picks a suitable size for the initialization, which
// should not be too inefficient
#define CYG_UIT_MEMPOOLFIXED_NOEXS( _a_, _s_ ) Cyg_Mempool_Fixed( \
    (cyg_uint8 *)(_a_), (cyg_int32)(_s_), (CYG_ADDRWORD) ((~7)&((_s_)/2)) )
#endif

Cyg_Mempool_Fixed       CYG_UITRON_DECL( MEMPOOLFIXED ) =
{
    CYGDAT_UITRON_MEMPOOLFIXED_INITIALIZERS
};
#undef CYG_UIT_MEMPOOLFIXED
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
#undef CYG_UIT_MEMPOOLFIXED_NOEXS
#endif

#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
Cyg_Mempool_Fixed       *CYG_UITRON_DECL_PTRS( MEMPOOLFIXED );
#endif
#endif // do we have fixed memory pools at all?
#endif // CYGPKG_UITRON_MEMPOOLFIXED

// ------------------------------------------------------------------------
// variable memory pools MUST be initialized, IF you have some.
#ifdef CYGPKG_UITRON_MEMPOOLVAR
#if (0 < CYGNUM_UITRON_MEMPOOLVAR) || \
    defined (CYGDAT_UITRON_MEMPOOLVAR_INITIALIZERS) || \
    defined (CYGDAT_UITRON_MEMPOOLVAR_EXTERNS)

#ifndef CYGDAT_UITRON_MEMPOOLVAR_EXTERNS
#error You must define CYGDAT_UITRON_MEMPOOLVAR_EXTERNS
#endif
#ifndef CYGDAT_UITRON_MEMPOOLVAR_INITIALIZERS
#error You must define CYGDAT_UITRON_MEMPOOLVAR_INITIALIZERS
#endif
#ifndef CYGNUM_UITRON_MEMPOOLVAR
#error You must define CYGNUM_UITRON_MEMPOOLVAR
#endif

// declare the symbols used in the initializer
CYGDAT_UITRON_MEMPOOLVAR_EXTERNS

// a Macro to ease the construction: addr, size
#define CYG_UIT_MEMPOOLVAR( _a_, _s_ ) Cyg_Mempool_Variable( \
    (cyg_uint8 *)(_a_),(cyg_int32)(_s_))
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
#define CYG_UIT_MEMPOOLVAR_NOEXS( _a_, _s_ ) Cyg_Mempool_Variable( \
    (cyg_uint8 *)(_a_),(cyg_int32)(_s_))
#endif

Cyg_Mempool_Variable CYG_UITRON_DECL( MEMPOOLVAR ) =
{
    CYGDAT_UITRON_MEMPOOLVAR_INITIALIZERS
};
#undef CYG_UIT_MEMPOOLVAR
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
#undef CYG_UIT_MEMPOOLVAR_NOEXS
#endif

#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
Cyg_Mempool_Variable    *CYG_UITRON_DECL_PTRS( MEMPOOLVAR );
#endif
#endif // do we have variable memory pools at all?
#endif // CYGPKG_UITRON_MEMPOOLVAR

// ------------------------------------------------------------------------
// Cyclic alarm handlers might be initialized, if you have some.
//
#ifdef CYGPKG_UITRON_CYCLICS
#if (0 < CYGNUM_UITRON_CYCLICS) || \
    defined( CYGDAT_UITRON_CYCLIC_EXTERNS ) || \
    defined( CYGDAT_UITRON_CYCLIC_INITIALIZERS )

#ifndef CYGNUM_UITRON_CYCLICS
#error You must define CYGNUM_UITRON_CYCLICS
#endif

#if defined( CYGDAT_UITRON_CYCLIC_INITIALIZERS ) || \
    defined( CYGDAT_UITRON_CYCLIC_EXTERNS )

#ifndef CYGDAT_UITRON_CYCLIC_INITIALIZERS
#error You must define CYGDAT_UITRON_CYCLIC_INITIALIZERS
#endif
#ifndef CYGDAT_UITRON_CYCLIC_EXTERNS
#error You must define CYGDAT_UITRON_CYCLIC_EXTERNS
#endif

// declare the symbols used in the initializer
CYGDAT_UITRON_CYCLIC_EXTERNS

#endif // have externs or initializers

Cyg_Timer               CYG_UITRON_DECL( CYCLICS )

#ifdef CYGDAT_UITRON_CYCLIC_INITIALIZERS

#error *** CYCLIC INITIALIZERS ARE NOT SUPPORTED IN THIS RELEASE***

// a Macro to ease the construction: proc, arg, time
#define CYG_UIT_CYCLIC( ... ) Cyg_Timer()
 = {
    CYGDAT_UITRON_CYCLIC_INITIALIZERS
}
#undef CYG_UIT_CYCLIC
#endif // do we have initializers?
; // the end of the declaration, with or without initializer

#endif // do we have cyclic alarms at all?
#endif // CYGPKG_UITRON_CYCLICS

// ------------------------------------------------------------------------
// Oneshot alarm handlers might be initialized, if you have some.
//
#ifdef CYGPKG_UITRON_ALARMS
#if (0 < CYGNUM_UITRON_ALARMS) || \
    defined( CYGDAT_UITRON_ALARM_EXTERNS ) || \
    defined( CYGDAT_UITRON_ALARM_INITIALIZERS )

#ifndef CYGNUM_UITRON_ALARMS
#error You must define CYGNUM_UITRON_ALARMS
#endif

#if defined( CYGDAT_UITRON_ALARM_INITIALIZERS ) || \
    defined( CYGDAT_UITRON_ALARM_EXTERNS )

#ifndef CYGDAT_UITRON_ALARM_INITIALIZERS
#error You must define CYGDAT_UITRON_ALARM_INITIALIZERS
#endif
#ifndef CYGDAT_UITRON_ALARM_EXTERNS
#error You must define CYGDAT_UITRON_ALARM_EXTERNS
#endif

// declare the symbols used in the initializer
CYGDAT_UITRON_ALARM_EXTERNS

#endif // have externs or initializers

Cyg_Timer               CYG_UITRON_DECL( ALARMS )

#ifdef CYGDAT_UITRON_ALARM_INITIALIZERS

#error *** ALARM INITIALIZERS ARE NOT SUPPORTED IN THIS RELEASE***

// a Macro to ease the construction: proc, arg, time
#define CYG_UIT_ALARM( ... ) Cyg_Timer()
 = {
    CYGDAT_UITRON_ALARM_INITIALIZERS
}
#undef CYG_UIT_ALARM
#endif // do we have initializers?
; // the end of the declaration, with or without initializer

#endif // do we have oneshot alarms at all?
#endif // CYGPKG_UITRON_ALARMS

// ------------------------------------------------------------------------
#endif // CYGPKG_UITRON

// EOF uit_objs.cxx
