#ifndef CYGONCE_HAL_HAL_STUB_H
#define CYGONCE_HAL_HAL_STUB_H

//=============================================================================
//
//      hal_stub.h
//
//      HAL header for GDB stub support.
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
// Author(s):   jskov
// Contributors:jskov
// Date:        1999-02-12
// Purpose:     Generic HAL stub header.
// Usage:       #include <cyg/hal/hal_stub.h>
// Description: This header is included by generic-stub.c to provide an
//              interface to the eCos-specific stub implementation. It is
//              not to be included by user code, and is only placed in a
//              publically accessible directory so that platform stub packages
//              are able to include it if required.
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#define USE_GDBSTUB_PROTOTYPES 0        // avoid stub-tservice.h atm
#ifndef __ECOS__
#define __ECOS__                        // use to mark eCos hacks
#endif

#include <cyg/hal/basetype.h>           // HAL_LABEL_NAME
#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/infra/cyg_type.h>         // cyg_uint32 and CYG_BYTEORDER

#ifndef __CYGMON_TYPES
#define __CYGMON_TYPES
typedef cyg_uint32 uint32;
typedef cyg_int32  int32;
#endif // __CYGMON_TYPES

#ifdef CYGBLD_HAL_PLATFORM_STUB_H
#include CYGBLD_HAL_PLATFORM_STUB_H
#else
#include <cyg/hal/plf_stub.h>
#endif
#include <cyg/hal/generic-stub.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} // to make below format correctly.
#endif

//-----------------------------------------------------------------------------
// Definitions for generic-stub.c

#define __set_mem_fault_trap(x) ({__mem_fault = 0; x(); __mem_fault;})

#if (CYG_BYTEORDER==CYG_LSBFIRST)
# if !defined(__LITTLE_ENDIAN__)
#  define __LITTLE_ENDIAN__
# endif
# if !defined(_LITTLE_ENDIAN)
#  define _LITTLE_ENDIAN
# endif
#endif

//-----------------------------------------------------------------------------
// Dummy definitions for harvard memory support for princeton memory systems.
#ifndef TARGET_HAS_HARVARD_MEMORY
typedef target_register_t target_addr_t;
#define TARGET_ADDR_IS_PROGMEM(x) 0
#define TARGET_ADDR_TO_PTR(x)     ((char *) (x))
#endif

//----------------------------------------------------------------------------
// Signal definitions to avoid 'signal.h'/
#define SIGHUP  1       /* hangup */
#define SIGINT  2       /* interrupt */
#define SIGQUIT 3       /* quit */
#define SIGILL  4       /* illegal instruction (not reset when caught) */
#define SIGTRAP 5       /* trace trap (not reset when caught) */
#define SIGIOT  6       /* IOT instruction */
#define SIGABRT 6       /* used by abort, replace SIGIOT in the future */
#define SIGEMT  7       /* EMT instruction */
#define SIGFPE  8       /* floating point exception */
#define SIGKILL 9       /* kill (cannot be caught or ignored) */
#define SIGBUS  10      /* bus error */
#define SIGSEGV 11      /* segmentation violation */
#define SIGSYS  12      /* bad argument to system call */
#define SIGPIPE 13      /* write on a pipe with no one to read it */
#define SIGALRM 14      /* alarm clock */
#define SIGTERM 15      /* software termination signal from kill */

//----------------------------------------------------------------------------
// Thread support. This setting is used in thread-pkts.h
#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#define DEBUG_THREADS 1
#else
#define DEBUG_THREADS 0
#endif

// The function stub_copy_registers() is statically defined in
// thread-packets.c, but in eCos this external stub is defined as it
// is used in dbg_gdb.cxx.
externC void __stub_copy_registers(target_register_t *dest, 
                                   target_register_t *src);

//----------------------------------------------------------------------------
// Hardware Watch/Breakpoint support. These are the possible return values
// of HAL_STUB_IS_STOPPED_BY_HARDWARE().
#define HAL_STUB_HW_STOP_NONE   0   // target did not stop for hw watch/break
#define HAL_STUB_HW_STOP_BREAK  1   // target stopped for hw breakpoint
#define HAL_STUB_HW_STOP_WATCH  2   // target stopped for write-only watchpoint
#define HAL_STUB_HW_STOP_RWATCH 3   // target stopped for read-only watchpoint
#define HAL_STUB_HW_STOP_AWATCH 4   // target stopped for access watchpoint

//----------------------------------------------------------------------------
// Memory accessor functions.
#define TARGET_HAS_OWN_MEM_FUNCS

//----------------------------------------------------------------------------
// Memory access checks. 

#ifndef CYG_HAL_STUB_PERMIT_DATA_READ
#define CYG_HAL_STUB_PERMIT_DATA_READ(_addr_, __count_) (1)
#endif

#ifndef CYG_HAL_STUB_PERMIT_DATA_WRITE
#define CYG_HAL_STUB_PERMIT_DATA_WRITE(_addr_, __count_) (1)
#endif

#ifdef TARGET_HAS_HARVARD_MEMORY

#ifndef CYG_HAL_STUB_PERMIT_CODE_READ
#define CYG_HAL_STUB_PERMIT_CODE_READ(_addr_, __count_) (1)
#endif

#ifndef CYG_HAL_STUB_PERMIT_CODE_WRITE
#define CYG_HAL_STUB_PERMIT_CODE_WRITE(_addr_, __count_) (1)
#endif

#endif

//----------------------------------------------------------------------------
// Target extras?!
extern int __process_target_query(char * pkt, char * out, int maxOut);
extern int __process_target_set(char * pkt, char * out, int maxout);
extern int __process_target_packet(char * pkt, char * out, int maxout);

//---------------------------------------------------------------------------
// Declarations to avoid compiler warnings.

// Set the baud rate for the current serial port.
extern void __set_baud_rate (int baud);

// Write C to the current serial port.
extern void putDebugChar (int c);

// Read one character from the current serial port.
extern int getDebugChar (void);

// Push CH back onto the debug stream.
extern void ungetDebugChar (int ch);

// Reset the board.
extern void __reset (void);

// Multi-bp support.
#ifndef __set_breakpoint
extern int __set_breakpoint (target_register_t addr, target_register_t len);
#endif
#ifndef __remove_breakpoint
extern int __remove_breakpoint (target_register_t addr, target_register_t len);
#endif
#ifndef __set_hw_breakpoint
extern int __set_hw_breakpoint (target_register_t addr, target_register_t len);
#endif
#ifndef __remove_hw_breakpoint
extern int __remove_hw_breakpoint (target_register_t addr, target_register_t len);
#endif
#ifndef __set_hw_watchpoint
extern int __set_hw_watchpoint (target_register_t addr, target_register_t len, int ztype);
#endif
#ifndef __remove_hw_watchpoint
extern int __remove_hw_watchpoint (target_register_t addr, target_register_t len, int ztype);
#endif

/* Install the standard set of trap handlers for the stub. */
extern void __install_traps (void);

/* Address in text section of a breakpoint instruction.  */

#ifndef BREAKINST_DEFINED
#define BREAKINST_DEFINED
extern void _breakinst (void);
#endif

/* The opcode for a breakpoint instruction.  */

extern unsigned long __break_opcode (void);

/* Function to flush output buffers */
extern void hal_flush_output(void);

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
// This one may assume a valid saved interrupt context on some platforms
extern void cyg_hal_gdb_interrupt    (target_register_t pc);
// This one does not; use from CRITICAL_IO_REGION macros below.
extern void cyg_hal_gdb_place_break  (target_register_t pc);
// Remove a break from either above - or not if cyg_hal_gdb_running_step
extern int  cyg_hal_gdb_remove_break (target_register_t pc);
// Bool: is such a breakpoint set?
extern int  cyg_hal_gdb_break_is_set (void);

/* This is used so that the generic stub can tell
 * cyg_hal_gdb_remove_break() not to bother when we are avoiding stepping
 * through a critical region ie. hal_diag_write_char() usually - that
 * shares the GDB IO device.
 */
extern volatile int cyg_hal_gdb_running_step;

// Use these in hal_diag.c when about to write a whole $O packet to GDB.
// NB they require __builtin_return_address() to work: if your platform
// does not support this, use HAL_DISABLE_INTERRUPTS &c instead.

#if 1 // Can use the address of a label: this is more portable

// This macro may already have been defined by the architecture HAL
#ifndef CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION
#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )                        \
do {                                                                         \
    HAL_DISABLE_INTERRUPTS(_old_);                                           \
    cyg_hal_gdb_place_break( (target_register_t)&&cyg_hal_gdb_break_place ); \
} while ( 0 )
#endif

// This macro may already have been defined by the architecture HAL
// Notice the trick to *use* the label - sometimes the tools want to
// move the label if unused, which is bad.
#ifndef CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION
#define CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION( _old_ )                         \
do {                                                                          \
    cyg_hal_gdb_remove_break( (target_register_t)&&cyg_hal_gdb_break_place ); \
    HAL_RESTORE_INTERRUPTS(_old_);                                            \
    _old_ = 1; /* actually use the label as a label... */                     \
cyg_hal_gdb_break_place:;                                                     \
    if ( (_old_)-- > 0 ) /* ...or the compiler might move it! */              \
        goto cyg_hal_gdb_break_place;                                         \
} while ( 0 )
#endif

#else // use __builtin_return_address instead.

#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )                        \
do {                                                                         \
    HAL_DISABLE_INTERRUPTS(_old_);                                           \
    cyg_hal_gdb_place_break((target_register_t)__builtin_return_address(0)); \
} while ( 0 )

#define CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION( _old_ )                         \
do {                                                                          \
    cyg_hal_gdb_remove_break((target_register_t)__builtin_return_address(0)); \
    HAL_RESTORE_INTERRUPTS(_old_);                                            \
} while ( 0 )

#endif

#else // NO debug_gdb_break_support

// so define these just to do interrupts:
#define CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION( _old_ )   \
do {                                                    \
    HAL_DISABLE_INTERRUPTS(_old_);                      \
} while (0);

#define CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION( _old_ )   \
do {                                                    \
    HAL_RESTORE_INTERRUPTS(_old_);                      \
} while (0);

#endif

//----------------------------------------------------------------------------
// eCos extensions to the stub

extern void hal_output_gdb_string(target_register_t str, int string_len);

extern target_register_t registers[];   // The current saved registers.
extern target_register_t * _registers ;
extern HAL_SavedRegisters *_hal_registers;

extern int cyg_hal_gdb_break;

#ifdef CYGPKG_ISOINFRA
# include <pkgconf/isoinfra.h>
#endif
#ifdef CYGINT_ISO_STRING_STRFUNCS
# include <string.h>
#else
//-----------------------------------------------------------------------------
// String helpers. These really should come from ISOINFRA
static inline char *strcpy( char *s, const char *t)
{
    char *r = s;

    while( *t ) *s++ = *t++;

    // Terminate the copied string.
    *s = 0;

    return r;
}

static inline size_t strlen( const char *s )
{
    int r = 0;
    while( *s++ ) r++;
    return r;
}
#endif

//-----------------------------------------------------------------------------
// Repeat the cache definitions here to avoid too much hackery in 
// generic-stub.h
/* Flush the instruction cache. */
extern void flush_i_cache (void);

/* Flush the data cache. */
extern void __flush_d_cache (void);

typedef enum {
  CACHE_NOOP, CACHE_ENABLE, CACHE_DISABLE, CACHE_FLUSH
} cache_control_t;

/* Perform the specified operation on the instruction cache. 
   Returns 1 if the cache is enabled, 0 otherwise. */
extern int __instruction_cache (cache_control_t request);
/* Perform the specified operation on the data cache. 
   Returns 1 if the cache is enabled, 0 otherwise. */
extern int __data_cache (cache_control_t request);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_STUB_H
// End of hal_stub.h
