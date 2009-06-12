#ifndef CYGONCE_HAL_CALM32_REGS_H
#define CYGONCE_HAL_CALM32_REGS_H
//========================================================================
//
//      calm32-regs.h
//
//      Register defines for CalmRISC 16 processors
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
// Author(s):     Red Hat, msalter
// Contributors:  Red Hat, msalter
// Date:          2001-02-12
// Purpose:       
// Description:   Register defines for CalmRISC 16 processors
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>

#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

// This value must agree with NUMREGS in calm32-stub.h.

#define NUM_REGS 23

#define REG_SIZE 4

// Status register fields
#define SR_T            0x00000001      // TRUE flag
#define SR_V            0x00000008      // Overflow flag
#define SR_Z0           0x00000010      // R6 Zero flag
#define SR_Z1           0x00000020      // R7 Zero flag.
#define SR_DS           0x00000040      // Divisor sign
#define SR_QT           0x00000080      // Quotient
#define SR_BX           0x00001000      // Byte sign extension
#define SR_HX           0x00002000      // Halfword sign extension
#define SR_SYSX_MASK    0x000f0000      // system extension
#define SR_SYSX_SHIFT   16
#define SR_IE           0x01000000      // IRQ enable
#define SR_FE           0x02000000      // FIQ enable
#define SR_TE           0x04000000      // Trace enable
#define SR_BS           0x10000000      // FIQ enable
#define SR_RS0          0x20000000      // Register bank select
#define SR_RS1          0x40000000      // Register bank select
#define SR_PM           0x80000000      // Privilege mode

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#endif // ifndef CYGONCE_HAL_CALM32_REGS_H
