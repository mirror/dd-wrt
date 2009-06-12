#ifndef CYGONCE_HAL_CALM16_REGS_H
#define CYGONCE_HAL_CALM16_REGS_H
//========================================================================
//
//      calm16-regs.h
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

// This value must agree with NUMREGS in calm16-stub.h.

#define NUM_REGS 23

#define REG_SIZE 4

// Status register fields
#define SR_T            0x8000      // TRUE flag
#define SR_PM           0x0040      // Privilege Mode
#define SR_Z1           0x0020      // R7 Zero flag.
#define SR_Z0           0x0010      // R6 Zero flag
#define SR_V            0x0008      // Overflow flag
#define SR_TE           0x0004      // Trace Enable
#define SR_IE           0x0002      // IRQ Enable
#define SR_FE           0x0001      // FIQ Enable

#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

#endif // ifndef CYGONCE_HAL_CALM16_REGS_H
