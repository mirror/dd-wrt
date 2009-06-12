//=============================================================================
//
//      mod_regs_bsc.h
//
//      BSC (bus state controller) Module register definitions
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
// Date:        2002-01-16
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Register definitions
#define CYGARC_REG_BCR1                 0xffffffe0
#define CYGARC_REG_BCR2                 0xffffffe4
#define CYGARC_REG_WCR1                 0xffffffe8
#define CYGARC_REG_WCR2                 0xffffffc0
#define CYGARC_REG_WCR3                 0xffffffc4
#define CYGARC_REG_MCR                  0xffffffec
#define CYGARC_REG_RTCSR                0xfffffff0
#define CYGARC_REG_RTCNT                0xfffffff4
#define CYGARC_REG_RTCOR                0xfffffff8

#define CYGARC_REG_BSC_WRITE_MAGIC      0xa55a0000

#define CYGARC_REG_BCR1_A3LW1           0x00004000
#define CYGARC_REG_BCR1_A3LW0           0x00002000
#define CYGARC_REG_BCR1_A2ENDIAN        0x00001000 // 0 = big, 1 = little
#define CYGARC_REG_BCR1_BSTROM          0x00000800
#define CYGARC_REG_BCR1_AHLW1           0x00000200
#define CYGARC_REG_BCR1_AHLW0           0x00000100
#define CYGARC_REG_BCR1_A1LW1           0x00000080
#define CYGARC_REG_BCR1_A1LW0           0x00000040
#define CYGARC_REG_BCR1_A0LW1           0x00000020
#define CYGARC_REG_BCR1_A0LW0           0x00000010
#define CYGARC_REG_BCR1_A4ENDIAN        0x00000008 // 0 = big, 1 = little
#define CYGARC_REG_BCR1_DRAM2           0x00000004
#define CYGARC_REG_BCR1_DRAM1           0x00000002
#define CYGARC_REG_BCR1_DRAM0           0x00000001

// Bus widths for areas
#define CYGARC_REG_BCR2_A4_8            0x00000100
#define CYGARC_REG_BCR2_A4_16           0x00000200
#define CYGARC_REG_BCR2_A4_32           0x00000300
#define CYGARC_REG_BCR2_A3_8            0x00000040
#define CYGARC_REG_BCR2_A3_16           0x00000080
#define CYGARC_REG_BCR2_A3_32           0x000000c0
#define CYGARC_REG_BCR2_A2_8            0x00000010
#define CYGARC_REG_BCR2_A2_16           0x00000020
#define CYGARC_REG_BCR2_A2_32           0x00000030
#define CYGARC_REG_BCR2_A1_8            0x00000004
#define CYGARC_REG_BCR2_A1_16           0x00000008
#define CYGARC_REG_BCR2_A1_32           0x0000000c

// Intercycle wait states
#define CYGARC_REG_WCR1_A3WI_MASK        0x0000c000 // Intercycle Idle Specification
#define CYGARC_REG_WCR1_A3WI_SHIFT       14
#define CYGARC_REG_WCR1_A2WI_MASK        0x00003000
#define CYGARC_REG_WCR1_A2WI_SHIFT       12
#define CYGARC_REG_WCR1_A1WI_MASK        0x00000c00
#define CYGARC_REG_WCR1_A1WI_SHIFT       10
#define CYGARC_REG_WCR1_A0WI_MASK        0x00000300
#define CYGARC_REG_WCR1_A0WI_SHIFT       8
#define CYGARC_REG_WCR1_A3W_MASK         0x000000c0 // waits
#define CYGARC_REG_WCR1_A3W_SHIFT        6
#define CYGARC_REG_WCR1_A2W_MASK         0x00000030
#define CYGARC_REG_WCR1_A2W_SHIFT        4
#define CYGARC_REG_WCR1_A1W_MASK         0x0000000c
#define CYGARC_REG_WCR1_A1W_SHIFT        2
#define CYGARC_REG_WCR1_A0W_MASK         0x00000003
#define CYGARC_REG_WCR1_A0W_SHIFT        0

#define CYGARC_REG_WCR1_WI_0         0
#define CYGARC_REG_WCR1_WI_1         1
#define CYGARC_REG_WCR1_WI_2         2
#define CYGARC_REG_WCR1_WI_4         3

#define CYGARC_REG_WCR1_W_0          0
#define CYGARC_REG_WCR1_W_1          1
#define CYGARC_REG_WCR1_W_2          2
#define CYGARC_REG_WCR1_W_LONG       3


// Wait states
#define CYGARC_REG_WCR2_A4WD_MASK         0x0000c000 // External waits for A4
#define CYGARC_REG_WCR2_A4WD_SHIFT        14
#define CYGARC_REG_WCR2_A4WM              0x00001000 // external wait mask
#define CYGARC_REG_WCR2_A3WM              0x00000800 // external wait mask
#define CYGARC_REG_WCR2_A2WM              0x00000400 // external wait mask
#define CYGARC_REG_WCR2_A1WM              0x00000200 // external wait mask
#define CYGARC_REG_WCR2_A0WM              0x00000100 // external wait mask
#define CYGARC_REG_WCR2_A4WI_MASK         0x0000000c // Intercycle Wait states
#define CYGARC_REG_WCR2_A4WI_SHIFT        2
#define CYGARC_REG_WCR2_A4W_MASK          0x00000003 // Wait states
#define CYGARC_REG_WCR2_A4W_SHIFT         0

#define CYGARC_REG_WCR2_A4WD_0WS             0
#define CYGARC_REG_WCR2_A4WD_1WS             1
#define CYGARC_REG_WCR2_A4WD_4WS             2


//--------------------------------------------------------------------------
// Additional type definitions
#if (CYGARC_SH_MOD_BCN > 1)
#define CYGARC_REG_BCR3                 0xfffffffc
#endif



//-----------------------------------------------------------------------------
// Calculate constants needed to drive the proper SDRAM refresh rate. Argument
// is delay between required refresh events in microseconds (us). Should be
// available off the SDRAM spec sheet.
// These should be a part of a fully CDLicized memory controller setup.
#define CYGARC_RTCSR_PRESCALE(_r_)                                      \
(((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(4*1000000))<256) ? 4 :              \
 ((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(16*1000000))<256) ? 16 :            \
 ((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(64*1000000))<256) ? 64 :            \
 ((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(256*1000000))<256) ? 256 :          \
 ((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(1024*1000000))<256) ? 1024 :        \
 ((CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(2048*1000000))<256) ? 2048 : 4096)

// These two macros provide the static values we need to stuff into the
// registers.
#define CYGARC_RTCSR_CKSx(_r_)                                  \
    ((   4 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x08 :              \
     (  16 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x10 :              \
     (  64 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x18 :              \
     ( 256 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x20 :              \
     (1024 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x28 :              \
     (2048 == CYGARC_RTCSR_PRESCALE(_r_)) ? 0x30 : 0x38 )

#define CYGARC_RTCSR_N(_r_)        \
       (CYGHWR_HAL_SH_BOARD_SPEED*(_r_)/(CYGARC_RTCSR_PRESCALE(_r_)*1000000))


