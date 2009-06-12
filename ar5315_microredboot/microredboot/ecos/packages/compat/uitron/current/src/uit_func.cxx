//===========================================================================
//
//      uit_func.cxx
//
//      uITRON compatibility functions
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
// Purpose:     uITRON compatibility functions
// Description: 
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/uitron.h>             // uITRON setup CYGNUM_UITRON_SEMAS
                                        // CYGPKG_UITRON et al

#ifdef CYGPKG_UITRON

// invoke the inline function definition to create static C linkage
// functions here:
#define CYGPRI_UITRON_FUNCS_HERE_AND_NOW
#include <cyg/compat/uitron/uit_func.h>

cyg_uint32 cyg_uitron_dis_dsp_old_priority = 0;

// ------------------------------------------------------------------------
//                  STARTUP
// this routine is outside the uITRON specification; call it from
// cyg_start(), cyg_package_start(), cyg_prestart() or cyg_user_start()
// to start the uITRON tasks and scheduler.

#if CYGNUM_UITRON_START_TASKS < 0
#error CYGNUM_UITRON_START_TASKS should be >= 0
#endif

#if CYGNUM_UITRON_START_TASKS == 0
#define START_TASKS CYGNUM_UITRON_TASKS
#else
#define START_TASKS CYGNUM_UITRON_START_TASKS
#endif

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
#if START_TASKS > CYGNUM_UITRON_TASKS_INITIALLY
#undef START_TASKS
#define START_TASKS CYGNUM_UITRON_TASKS_INITIALLY
#endif
#endif

#if START_TASKS > CYGNUM_UITRON_TASKS
#undef START_TASKS
#define START_TASKS CYGNUM_UITRON_TASKS
#endif

#if START_TASKS <= 0
#error Number of uITRON tasks to start initially must be >= 0
#endif


#define SET_UP_PTRS( _which_ ) CYG_MACRO_START                            \
    for ( i = 0;                                                          \
          (i < CYGNUM_UITRON_ ## _which_ ## _INITIALLY) &&                \
          (i < CYGNUM_UITRON_ ## _which_              )    ;              \
          i++ ) {                                                         \
        CYG_UITRON_PTRS( _which_ )[ i ] = CYG_UITRON_OBJS( _which_ ) + i; \
    }                                                                     \
    if ( (CYGNUM_UITRON_ ## _which_ ## _INITIALLY)                        \
          < (CYGNUM_UITRON_ ## _which_) )                                 \
        for ( /* i as is */; i < CYGNUM_UITRON_ ## _which_ ; i++ ) {      \
            CYG_UITRON_PTRS( _which_ )[ i ] = NULL;                       \
        }                                                                 \
CYG_MACRO_END

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#ifdef CYGSEM_UITRON_TIME_IS_MILLISECONDS
Cyg_Clock::converter uit_clock_to_system;
Cyg_Clock::converter uit_clock_from_system;
#endif
#endif

void cyg_uitron_start( void )
{
    cyg_int32 i;

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK
#ifdef CYGSEM_UITRON_TIME_IS_MILLISECONDS
    // initialize the clock converters
    Cyg_Clock::real_time_clock->
        get_other_to_clock_converter( 1000000, &uit_clock_to_system );
    Cyg_Clock::real_time_clock->
        get_clock_to_other_converter( 1000000, &uit_clock_from_system );
#endif
#endif

    for ( i = 0; i < START_TASKS; i++ ) {
#ifdef CYGIMP_THREAD_PRIORITY
        // save the initial priority in our private array
        cyg_uitron_task_initial_priorities[ i ] = 
            cyg_uitron_TASKS[ i ].get_priority();
#endif
        // and awaken the task:
        cyg_uitron_TASKS[ i ].resume();
    }
    for ( /* i as is */; i < CYGNUM_UITRON_TASKS; i++ ) {
#ifdef CYGIMP_THREAD_PRIORITY
        // save the initial priority in our private array
        cyg_uitron_task_initial_priorities[ i ] = 
            cyg_uitron_TASKS[ i ].get_priority();
#endif
        // but ensure the task state is dormant.
        cyg_uitron_TASKS[ i ].kill();
    }

#ifdef CYGPKG_UITRON_TASKS_CREATE_DELETE
    SET_UP_PTRS( TASKS );
#endif
#ifdef CYGPKG_UITRON_SEMAS_CREATE_DELETE
    SET_UP_PTRS( SEMAS );
#endif
#ifdef CYGPKG_UITRON_MBOXES_CREATE_DELETE
    SET_UP_PTRS( MBOXES );
#endif
#ifdef CYGPKG_UITRON_FLAGS_CREATE_DELETE
    SET_UP_PTRS( FLAGS );
#endif
#ifdef CYGPKG_UITRON_MEMPOOLFIXED_CREATE_DELETE
    SET_UP_PTRS( MEMPOOLFIXED );
#endif
#ifdef CYGPKG_UITRON_MEMPOOLVAR_CREATE_DELETE
    SET_UP_PTRS( MEMPOOLVAR );
#endif
}

// These allow programs to link when cyg_uitron_start() is called
// (often because of CYGSEM_START_UITRON_COMPATIBILITY from infra,
//  though we define these regardless just in case)
// even when there is no interest in uITRON and so the tasks are
// not externally defined; the reference to cyg_uitron_start()
// ensures the tasks array et al are still included...
extern "C" {
    void task1( unsigned int arg ) CYGBLD_ATTRIB_WEAK;
    void task2( unsigned int arg ) CYGBLD_ATTRIB_WEAK;
    void task3( unsigned int arg ) CYGBLD_ATTRIB_WEAK;
    void task4( unsigned int arg ) CYGBLD_ATTRIB_WEAK;
}

void task1( unsigned int arg ) {}
void task2( unsigned int arg ) {}
void task3( unsigned int arg ) {}
void task4( unsigned int arg ) {}

#endif // CYGPKG_UITRON

// EOF uit_func.cxx
