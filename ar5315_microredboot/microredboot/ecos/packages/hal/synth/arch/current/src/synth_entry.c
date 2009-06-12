//==========================================================================
//
//      synth_entry.c
//
//      Entry code for Linux synthetic target.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2002 Bart Veer
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
// Author(s):   proven
// Contributors:proven, jskov, bartv
// Date:        1999-01-06
// Purpose:     Entry point for Linux synthetic target.
//
//####DESCRIPTIONEND####
//
//=========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_io.h>
#include CYGHWR_MEMORY_LAYOUT_H

/*------------------------------------------------------------------------*/
/* C++ support - run initial constructors                                 */

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
cyg_bool cyg_hal_stop_constructors;
#endif

typedef void (*pfunc) (void);
extern pfunc __CTOR_LIST__[];
extern pfunc __CTOR_END__[];

void
cyg_hal_invoke_constructors (void)
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    static pfunc *p = &__CTOR_END__[-1];
    
    cyg_hal_stop_constructors = 0;
    for (; p >= __CTOR_LIST__; p--) {
        (*p) ();
        if (cyg_hal_stop_constructors) {
            p--;
            break;
        }
    }
#else
    pfunc *p;

    for (p = &__CTOR_END__[-1]; p >= __CTOR_LIST__; p--)
        (*p) ();
#endif
}

// ----------------------------------------------------------------------------
// The low-level entry point is platform-specific, typically in the
// assember file vectors.S. However that entry point simply jumps
// directly here, with no further processing or stack manipulation.
// The HAL specification defines clearly what should happen during
// startup.

externC void    cyg_start( void );
externC void    synth_hardware_init(void);
externC void    synth_hardware_init2(void);

void _linux_entry( void )
{
    void* new_top = (void*) 0;
    
    // "Initialize various cpu status registers, including disabling interrupts."
    // That is a no-op for the synthetic target, in particular interrupts are
    // already disabled.

    // "Set up any CPU memory controller to access ROM, RAM, and I/O devices
    // correctly".
    //
    // This involves using the brk() system call to allocate the RAM used
    // for the heaps. There are no variables mapped there so the system
    // will not have done this for us. Note that the implementation of
    // brk() (mm/mmap.c) differs from the documentation - the return
    // value is the new brk value, not an error code.
    new_top = (void*) (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE);
    if (new_top != cyg_hal_sys_brk(new_top)) {
        CYG_FAIL("Failed to initialize memory");
        cyg_hal_sys_exit(1);
    }
    
    // Again a no-op for the synthetic target. All memory is readily
    // accessible. Arguably the auxiliary should be started up here, but
    // instead that is left to platform initialization.

    // "Enable the cache". Effectively the synthetic target has no cache,
    // anything provided by the hardware is not readily accessible.

    // "Set up the stack pointer". The system starts up a program with a
    // suitable stack.

    // "Initialize any global pointer register". There is no such register.

    // Perform platform-specific initialization. Actually, all Linux
    // platforms can share this. It involves setting up signal handlers,
    // starting the I/O auxiliary, and so on.
    synth_hardware_init();

    // This is not a ROM startup, so no need to worry about copying the
    // .data section.

    // "Zero the .bss section". Linux will have done this for us.

    // "Create a suitable C stack frame". Already done.

    // Invoke the C++ constructors.
    cyg_hal_invoke_constructors();

    // Once the C++ constructors have been invoked, a second stage
    // of hardware initialization is desirable. At this point all
    // eCos device drivers should have been initialized so the
    // I/O auxiliary will have loaded the appropriate support
    // scripts, and the auxiliary can now map the window(s) on to
    // the display and generally operate normally.
    synth_hardware_init2();
    
    // "Call cyg_start()". OK.
    cyg_start();

    // "Drop into an infinite loop". Not a good idea for the synthetic
    // target. Instead, exit.
    cyg_hal_sys_exit(0);
}

// ----------------------------------------------------------------------------
// Stub functions needed for linking with various versions of gcc
// configured for Linux rather than i386-elf.

#if (__GNUC__ < 3)
// 2.95.x libgcc.a __pure_virtual() calls __write().
int __write(void)
{
    return -1;
}
#endif

#if (__GNUC__ >= 3)
// Versions of gcc/g++ after 3.0 (approx.), when configured for Linux
// native development (specifically, --with-__cxa_enable), have
// additional dependencies related to the destructors for static
// objects. When compiling C++ code with static objects the compiler
// inserts a call to __cxa_atexit() with __dso_handle as one of the
// arguments. __cxa_atexit() would normally be provided by glibc, and
// __dso_handle is part of crtstuff.c. Synthetic target applications
// are linked rather differently, so either a differently-configured
// compiler is needed or dummy versions of these symbols should be
// provided. If these symbols are not actually used then providing
// them is still harmless, linker garbage collection will remove them.

void
__cxa_atexit(void (*arg1)(void*), void* arg2, void* arg3)
{
}
void*   __dso_handle = (void*) &__dso_handle;

// gcc 3.2.2 (approx). The libsupc++ version of the new operator pulls
// in exception handling code, even when using the nothrow version and
// building with -fno-exceptions. libgcc_eh.a provides the necessary
// functions, but requires a dl_iterate_phdr() function. That is related
// to handling dynamically loaded code so is not applicable to eCos.
int
dl_iterate_phdr(void* arg1, void* arg2)
{
    return -1;
}
#endif

//-----------------------------------------------------------------------------
// End of entry.c
