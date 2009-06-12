#ifndef CYGONCE_HAL_VAR_REGS_H
#define CYGONCE_HAL_VAR_REGS_H

//==========================================================================
//
//      var_regs.h
//
//      PowerPC 60x variant CPU definitions
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
// Author(s):    jskov
// Contributors: jskov
// Date:         2000-02-04
// Purpose:      Provide PPC60x register definitions
// Description:  Provide PPC60x register definitions
//               The short difinitions (sans CYGARC_REG_) are exported only
//               if CYGARC_HAL_COMMON_EXPORT_CPU_MACROS is defined.
// Usage:        Included via the acrhitecture register header:
//               #include <cyg/hal/ppc_regs.h>
//               ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/plf_regs.h>  // Get any platform specifics

//--------------------------------------------------------------------------
// Cache
#define CYGARC_REG_HID0   1008
#define _HID0       CYGARC_REG_HID0
#define _HID0_EMCP  0x80000000  // Enable machine check
#define _HID0_EBA   0x20000000  // Enable bus address parity
#define _HID0_EBD   0x10000000  // Enable bus data parity
#define _HID0_BCLK  0x08000000
#define _HID0_EICE  0x04000000
#define _HID0_ECLK  0x02000000
#define _HID0_PAR   0x01000000
#define _HID0_DOZE  0x00800000
#define _HID0_NAP   0x00400000
#define _HID0_SLEEP 0x00200000
#define _HID0_DPM   0x00100000
#define _HID0_ICE   0x00008000  // Enable Instruction Cache
#define _HID0_DCE   0x00004000  // Enable Data Cache
#define _HID0_ILOCK 0x00002000  // Instruction Cache Lock
#define _HID0_DLOCK 0x00001000  // Data Cache Lock
#define _HID0_ICFI  0x00000800  // Instruction Cache [flash] Invalidate
#define _HID0_DCFI  0x00000400  // Data Cache [flash] Invalidate
#define _HID0_IFEM  0x00000080
#define _HID0_FBIOB 0x00000010
#define _HID0_ABE   0x00000008
#define _HID0_NOOPT 0x00000001

//--------------------------------------------------------------------------
// BATs
#ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#define IBAT0U          528
#define IBAT0L          529
#define IBAT1U          530
#define IBAT1L          531
#define IBAT2U          532
#define IBAT2L          533
#define IBAT3U          534
#define IBAT3L          535

#define DBAT0U          536
#define DBAT0L          537
#define DBAT1U          538
#define DBAT1L          539
#define DBAT2U          540
#define DBAT2L          541
#define DBAT3U          542
#define DBAT3L          543

#define UBAT_BEPIMASK   0xfffe0000      // effective address mask
#define UBAT_BLMASK     0x00001ffc      // block length mask
#define UBAT_VS         0x00000002      // supervisor mode valid bit
#define UBAT_VP         0x00000001      // problem mode valid bit

#define LBAT_BRPNMASK   0xfffe0000      // real address mask
#define LBAT_W          0x00000040      // write-through
#define LBAT_I          0x00000020      // caching-inhibited
#define LBAT_M          0x00000010      // memory coherence
#define LBAT_G          0x00000008      // guarded

#define LBAT_PP_NA      0x00000000      // no access
#define LBAT_PP_RO      0x00000001      // read-only
#define LBAT_PP_RW      0x00000002      // read/write
#endif // ifdef CYGARC_HAL_COMMON_EXPORT_CPU_MACROS

//-----------------------------------------------------------------------------
#endif // ifdef CYGONCE_HAL_VAR_REGS_H
// End of var_regs.h
