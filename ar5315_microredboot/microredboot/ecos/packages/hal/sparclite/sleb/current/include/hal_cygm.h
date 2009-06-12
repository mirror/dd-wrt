#ifndef CYGONCE_HAL_CYGM_H
#define CYGONCE_HAL_CYGM_H

//=============================================================================
//
//      hal_cygm.h
//
//      HAL CygMon vector definitions
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
// Author(s):   hmt
// Contributors:        hmt
// Date:        1999-02-24
// Purpose:     Define Interrupt vectors in CygMon
// Description: 
//              
// Usage:
//              #include <cyg/hal/hal_cygm.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/hal_sparclite.h>
#include <pkgconf/hal_sparclite_sleb.h>

// *** THIS FILE IS ALSO USED IN ASSEMBLY SOURCE ***
// so it does not include eg. cyg_type.h ...

#ifdef CYG_HAL_USE_ROM_MONITOR_CYGMON

//-----------------------------------------------------------------------------
// SPARClite CygMon vector numbers and address

#define CYGMON_VECTOR_TABLE_BASE (0x04000000)

#define CYGMON_VECTOR_TABLE ((CYG_ADDRESS *)CYGMON_VECTOR_TABLE_BASE)

// The ROM vector table is located at 0x04000000 and has the
// following layout:

#define BSP_EXC_IACCESS     0   /* insn access */
#define BSP_EXC_ILL         1   /* illegal insn */
#define BSP_EXC_IPRIV       2   /* privileged insn */
#define BSP_EXC_FPDIS       3   /* FPU disabled */
#define BSP_EXC_WINOVF      4   /* window overflow */
#define BSP_EXC_WINUND      5   /* window underflow */
#define BSP_EXC_ALIGN       6   /* alignment */
#define BSP_EXC_DACCESS     7   /* data access */
#define BSP_EXC_TAGOVF      8   /* tag overflow */
#define BSP_EXC_INT1        9
#define BSP_EXC_INT2       10
#define BSP_EXC_INT3       11
#define BSP_EXC_INT4       12
#define BSP_EXC_INT5       13
#define BSP_EXC_INT6       14
#define BSP_EXC_INT7       15   /* serial Ch0 rxrdy */
#define BSP_EXC_INT8       16
#define BSP_EXC_INT9       17
#define BSP_EXC_INT10      18   /* serial Ch0 rxrdy */
#define BSP_EXC_INT11      19
#define BSP_EXC_INT12      20
#define BSP_EXC_INT13      21
#define BSP_EXC_INT14      22   /* ethernet interrupt */
#define BSP_EXC_INT15      23
#define BSP_EXC_CPDIS      24   /* CP disabled */
#define BSP_EXC_BREAK      25   /* breakpoint "ta 1" */
#define BSP_EXC_WINFLUSH   26   /* window flush "ta 3" */
#define BSP_EXC_SYSCALL    27   /* syscall "ta 8" */
#define BSP_EXC_DEBUG      28   /* DSU exception */
#define BSP_EXC_TRAP       29   /* all other traps */
#define BSP_VEC_MT_DEBUG   30   /* Multi-Threaded debugging */
#define BSP_VEC_STUB_ENTRY 31   /* low level stub entry, eg. ^C rx */

// These vectors should be called with:
//
//  %l0 - PSR
//  %l1 - PC
//  %l2 - NPC
// [%l3 - TBR ;  STUB_ENTRY only, TBR from original exception ]

#define BSP_NOTVEC_BSP_COMM_PROCS 32 /* pointer to structure for comms */

#endif // CYG_HAL_USE_ROM_MONITOR_CYGMON

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CYGM_H
// End of hal_cygm.h
