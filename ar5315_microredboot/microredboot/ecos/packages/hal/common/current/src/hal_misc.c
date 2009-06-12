//==========================================================================
//
//      hal_misc.c
//
//      Common HAL miscellaneous functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    nickg
// Contributors: nickg, jskov
// Date:         2000-06-08
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_if.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_misc.h>           // our header

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// Macro for finding return address. 
#ifndef CYGARC_HAL_GET_RETURN_ADDRESS

#define CYGARC_HAL_GET_RETURN_ADDRESS(_x_, _dummy_)     \
    CYG_MACRO_START                                     \
    (_dummy_) = 1;                                      \
    (_x_) = (CYG_ADDRWORD)&&__backup_return_address;    \
    CYG_MACRO_END

#define CYGARC_HAL_GET_RETURN_ADDRESS_BACKUP(_dummy_)   \
    CYG_MACRO_START                                     \
__backup_return_address:                                \
    if ((_dummy_)-- > 0)                                \
        goto __backup_return_address;                   \
    CYG_MACRO_END
#endif

//--------------------------------------------------------------------------
// Macro for finding PC in saved regs
#ifndef CYGARC_HAL_GET_PC_REG
#define CYGARC_HAL_GET_PC_REG(_regs_,_val_) ((_val_) = (_regs_)->pc)
#endif

//--------------------------------------------------------------------------
// Macro for matching interrupt vector to GDB comm channel.
#ifndef CYGHWR_HAL_GDB_PORT_VECTORS_MATCH
#define CYGHWR_HAL_GDB_PORT_VECTORS_MATCH(_v_,_gv_) ((_v_)==(_gv_))
#endif

#if defined(CYGPKG_CYGMON)
unsigned long cygmon_memsize = 0;
#endif

//--------------------------------------------------------------------------
// Functions to support the detection and execution of a user provoked
// program break. These are usually called from interrupt routines.

cyg_bool
cyg_hal_is_break(char *buf, int size)
{
    while( size )
        if( buf[--size] == 0x03 ) return true;

    return false;
}

// Keep this variable global, to prevent the compiler removing it (and
// the goto-reference) due to being local to the function where it is
// used. Yes, it's ugly.
int _cyg_hal_compiler_dummy;

void 
cyg_hal_user_break( CYG_ADDRWORD *regs )
{
#if defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs) \
    || defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon) \
    || defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)

    CYG_ADDRWORD __ra;
    CYG_WORD32 __pc;
    HAL_SavedRegisters *sreg = (HAL_SavedRegisters *)regs;

    CYGARC_HAL_GET_RETURN_ADDRESS(__ra, _cyg_hal_compiler_dummy);

    if( regs == NULL ) __pc = __ra;
    else  CYGARC_HAL_GET_PC_REG(sreg, __pc);

    CYGACC_CALL_IF_INSTALL_BPT_FN((void *)__pc);

    CYGARC_HAL_GET_RETURN_ADDRESS_BACKUP(_cyg_hal_compiler_dummy);
    
#else

    HAL_BREAKPOINT(breakinst);

#endif
}


//--------------------------------------------------------------------------
// The system default interrupt ISR. It calls the architecture default
// ISR as well if necessary.
externC cyg_uint32 hal_arch_default_isr(CYG_ADDRWORD vector, 
                                        CYG_ADDRWORD data);

cyg_uint32
hal_default_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 result;

#if (defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)           \
     || defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)) &&    \
        (defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT) ||     \
          defined(CYGHWR_HAL_GDB_PORT_VECTOR) &&           \
          defined(HAL_CTRLC_ISR))

#ifndef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN    
#if CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    int gdb_vector = -1;
    // This check only to avoid crash on older stubs in case of unhandled
    // interrupts. It is a bit messy, but required in a transition period.
#ifndef CYGSEM_HAL_ROM_MONITOR
    if (CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK ==
        (CYGACC_CALL_IF_VERSION() & CYGNUM_CALL_IF_TABLE_VERSION_CALL_MASK))
#endif
    {
        hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();
        if (__chan)
            gdb_vector = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_DBG_ISR_VECTOR);
    }
    if( CYGHWR_HAL_GDB_PORT_VECTORS_MATCH(vector, gdb_vector) )
#else
    // Old code using hardwired channels. This should go away eventually.
    if( vector == CYGHWR_HAL_GDB_PORT_VECTOR )
#endif
#endif
    {
        result = HAL_CTRLC_ISR( vector, data );
        if( 0 != result ) return result;
    }
#endif

    result = hal_arch_default_isr (vector, data);
    if( 0 != result) return result;

    CYG_TRACE2(true, "Interrupt: %d, Data: 0x%08x", vector, data);
    CYG_FAIL("Spurious Interrupt!!!");
    return 0;
}


//--------------------------------------------------------------------------
// End of hal_misc.c
