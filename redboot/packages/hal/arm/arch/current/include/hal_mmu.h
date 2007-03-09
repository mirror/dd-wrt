#ifndef CYGONCE_HAL_MMU_H
#define CYGONCE_HAL_MMU_H

//==========================================================================
//
//      hal_mmu.h
//
//      MMU definitions
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         1999-05-10
// Purpose:      Define MMU for ARM
// Usage:        #include <cyg/hal/hal_mmu.h>

//              
//####DESCRIPTIONEND####
//
//==========================================================================

#define MMU_L1_TYPE         0x03  // Descriptor type
#define MMU_L1_TYPE_Fault   0x00  // Invalid
#define MMU_L1_TYPE_Page    0x11  // Individual page mapping
#define MMU_L1_TYPE_Section 0x12  // Mapping for 1M segment

#define MMU_L2_TYPE         0x03  // Descriptor type
#define MMU_L2_TYPE_Fault   0x00  // Invalid data
#define MMU_L2_TYPE_Large   0x01  // Large page (64K)
#define MMU_L2_TYPE_Small   0x02  // Small page (4K)

#define MMU_Bufferable      0x04  // Data can use write-buffer
#define MMU_Cacheable       0x08  // Data can use cache

#define MMU_AP_Limited     0x000  // Limited access
#define MMU_AP_Supervisor  0x400  // Supervisor RW, User none
#define MMU_AP_UserRead    0x800  // Supervisor RW, User read only
#define MMU_AP_Any         0xC00  // Supervisor RW, User RW

#define MMU_AP_ap0_Any     0x030
#define MMU_AP_ap1_Any     0x0C0
#define MMU_AP_ap2_Any     0x300
#define MMU_AP_ap3_Any     0xC00
#define MMU_AP_All (MMU_AP_ap0_Any|MMU_AP_ap1_Any|MMU_AP_ap2_Any|MMU_AP_ap3_Any)

#define MMU_DOMAIN(x)      ((x)<<5)

#define MMU_PAGE_SIZE      0x1000
#define MMU_SECTION_SIZE   0x100000

#define MMU_CP               p15      // Co-processor ID 
#define MMU_Control          c1       // Control register
#define MMU_Base             c2       // Page tables base
#define MMU_DomainAccess     c3       // Domain access control
#define MMU_FaultStatus      c5       // Fault status register
#define MMU_FaultAddress     c6       // Fault Address
#define MMU_InvalidateCache  c7       // Invalidate cache data
#define MMU_TLB              c8       // Translation Lookaside Buffer

// These seem to be 710 specific
#define MMU_FlushTLB         c5
#define MMU_FlushIDC         c7

#define MMU_Control_M  0x001    // Enable MMU
#define MMU_Control_A  0x002    // Enable address alignment faults
#define MMU_Control_C  0x004    // Enable cache
#define MMU_Control_W  0x008    // Enable write-buffer
#define MMU_Control_P  0x010    // Compatability: 32 bit code
#define MMU_Control_D  0x020    // Compatability: 32 bit data
#define MMU_Control_L  0x040    // Compatability:
#define MMU_Control_B  0x080    // Enable Big-Endian
#define MMU_Control_S  0x100    // Enable system protection
#define MMU_Control_R  0x200    // Enable ROM protection
#define MMU_Control_I  0x1000   // Enable Instruction cache
#define MMU_Control_X  0x2000   // Set interrupt vectors at 0xFFFF0000
#define MMU_Control_Init (MMU_Control_P|MMU_Control_D|MMU_Control_L)

// Extras for some newer versions eg. ARM920 with architecture version 4.
#define MMU_Control_F  0x400    // IMPLEMENTATION DEFINED
#define MMU_Control_Z  0x800    // Enable branch predicion
#define MMU_Control_RR 0x4000   // Select non-random cache replacement

//-----------------------------------------------------------------------------

#endif // CYGONCE_HAL_MMU_H
// End of hal_mmu.h
