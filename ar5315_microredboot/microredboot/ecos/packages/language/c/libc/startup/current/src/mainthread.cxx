//=======================================================================
//
//      mainthread.cxx
//
//      Support for startup of ISO C environment
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-30
// Purpose:       Provides a thread object to call into a user-supplied
//                main()
// Description:   Here we define the thread object that calls
//                cyg_libc_invoke_main() which in turn will invoke
//                the user-supplied main() entry point function (or
//                alternatively the dummy empty one supplied by eCos)
// Usage:         Both the stack (cyg_libc_main_stack) and the thread
//                (cyg_libc_main_thread) can be overriden if you provide
//                your own symbols with those names. In the case of the
//                stack obviously you need to ensure
//                CYGNUM_LIBC_MAIN_STACK_SIZE corresponds to your own
//                stack.
//                The thread object is also available externally if you
//                want to control it (suspend/resume/etc.) either by
//                extern Cyg_Thread cyg_libc_main_thread; from C++, using
//                the kernel C++ API, or
//                extern cyg_handle_t cyg_libc_main_thread; from C using
//                the kernel C API.
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_startup.h>          // C library configuration

#ifdef CYGSEM_LIBC_STARTUP_MAIN_THREAD

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <pkgconf/kernel.h>        // eCos kernel configuration
#include <cyg/kernel/thread.hxx>   // eCos thread support
#include <cyg/kernel/thread.inl>
#include <cyg/hal/hal_arch.h>      // for CYGNUM_HAL_STACK_SIZE_TYPICAL


// EXTERNS

#ifdef CYGSEM_LIBC_INVOKE_DEFAULT_STATIC_CONSTRUCTORS
extern cyg_bool cyg_hal_stop_constructors;
#endif

// FUNCTION PROTOTYPES

externC void
cyg_libc_invoke_main( CYG_ADDRWORD );

// STATICS

#ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

// override stack size on some platforms
#ifdef CYGNUM_HAL_STACK_SIZE_TYPICAL
# if CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE < CYGNUM_HAL_STACK_SIZE_TYPICAL
#  undef CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE
#  define CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
# endif
#endif

static cyg_uint8 cyg_libc_main_stack[ CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE ]
  CYGBLD_ATTRIB_ALIGN(CYGARC_ALIGNMENT);

#else // !ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

extern char *cyg_libc_main_stack;
extern int cyg_libc_main_stack_size;

#endif // !ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM

// GLOBALS

// let the main thread be global so people can play with it (e.g. suspend
// or resume etc.) if that's what they want to do
Cyg_Thread cyg_libc_main_thread CYGBLD_ATTRIB_INIT_PRI(CYG_INIT_LIBC) =
    Cyg_Thread(CYGNUM_LIBC_MAIN_THREAD_PRIORITY,
                &cyg_libc_invoke_main, (CYG_ADDRWORD) 0,
                "main",
                (CYG_ADDRESS) &cyg_libc_main_stack[0],
#ifdef CYGSEM_LIBC_MAIN_STACK_FROM_SYSTEM
                CYGNUM_LIBC_MAIN_DEFAULT_STACK_SIZE
#else
                cyg_libc_main_stack_size
#endif
              );

#endif // ifdef CYGSEM_LIBC_STARTUP_MAIN_THREAD

// EOF mainthread.cxx
