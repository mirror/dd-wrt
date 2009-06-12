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
// Date:        2000-10-30
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------
// Register definitions
#define CYGARC_REG_BCR1                 0xffffff60
#define CYGARC_REG_BCR2                 0xffffff62
#define CYGARC_REG_WCR1                 0xffffff64
#define CYGARC_REG_WCR2                 0xffffff66
#define CYGARC_REG_MCR                  0xffffff68
#define CYGARC_REG_DCR                  0xffffff6a
#define CYGARC_REG_PCR                  0xffffff6c
#define CYGARC_REG_RTCSR                0xffffff6e
#define CYGARC_REG_RTCNT                0xffffff70
#define CYGARC_REG_RTCOR                0xffffff72
#define CYGARC_REG_RFCR                 0xffffff74

#define CYGARC_REG_SDMR_AREA2_BASE      0xffffd000
#define CYGARC_REG_SDMR_AREA3_BASE      0xffffe000

#define CYGARC_REG_BCR1_DRAMTP2         0x0010
#define CYGARC_REG_BCR1_DRAMTP1         0x0008
#define CYGARC_REG_BCR1_DRAMTP0         0x0004

// Bus widths for areas
#define CYGARC_REG_BCR2_A6_8            0x1000
#define CYGARC_REG_BCR2_A6_16           0x2000
#define CYGARC_REG_BCR2_A6_32           0x3000
#define CYGARC_REG_BCR2_A5_8            0x0400
#define CYGARC_REG_BCR2_A5_16           0x0800
#define CYGARC_REG_BCR2_A5_32           0x0c00
#define CYGARC_REG_BCR2_A4_8            0x0100
#define CYGARC_REG_BCR2_A4_16           0x0200
#define CYGARC_REG_BCR2_A4_32           0x0300
#define CYGARC_REG_BCR2_A3_8            0x0040
#define CYGARC_REG_BCR2_A3_16           0x0080
#define CYGARC_REG_BCR2_A3_32           0x00c0
#define CYGARC_REG_BCR2_A2_8            0x0010
#define CYGARC_REG_BCR2_A2_16           0x0020
#define CYGARC_REG_BCR2_A2_32           0x0030

// Memory type selection and other IO behavior controls
#define CYGARC_REG_BCR1_PULA            0x8000 // Pin A25 to A0 Pull-Up
#define CYGARC_REG_BCR1_PULD            0x4000 // Pin D31 to D0 Pull-Up
#define CYGARC_REG_BCR1_HIZMEM          0x2000 // Hi-Z memory control
#define CYGARC_REG_BCR1_HIZCNT          0x1000 // High-Z Control
#define CYGARC_REG_BCR1_ENDIAN          0x0800 // Endian Flag
#define CYGARC_REG_BCR1_A0_BST_MASK     0x0600 // Area 0 Burst ROM Control
#define CYGARC_REG_BCR1_A0_BST_4        0x0200
#define CYGARC_REG_BCR1_A0_BST_8        0x0400
#define CYGARC_REG_BCR1_A0_BST_16       0x0600
#define CYGARC_REG_BCR1_A5_BST_MASK     0x0180 // Area 5 Burst ROM Control
#define CYGARC_REG_BCR1_A5_BST_4        0x0080
#define CYGARC_REG_BCR1_A5_BST_8        0x0100
#define CYGARC_REG_BCR1_A5_BST_16       0x0180
#define CYGARC_REG_BCR1_A6_BST_MASK     0x0060 // Area 6 Burst ROM Control
#define CYGARC_REG_BCR1_A6_BST_4        0x0020
#define CYGARC_REG_BCR1_A6_BST_8        0x0040
#define CYGARC_REG_BCR1_A6_BST_16       0x0060
#define CYGARC_REG_BCR1_DRAMTP_MASK     0x001c // Area 2, Area 3 Memory Type
#define CYGARC_REG_BCR1_A5PCM           0x0002 // Area 5 Bus Type
#define CYGARC_REG_BCR1_A6PCM           0x0001 // Area 6 Bus Type

// Intercycle wait states
#define CYGARC_REG_WCR1_WAITSEL         0x8000 // WAIT Sampling Timing Select
#define CYGARC_REG_WCR1_A6I_MASK        0x3000 // Intercycle Idle Specification
#define CYGARC_REG_WCR1_A6I_SHIFT       12
#define CYGARC_REG_WCR1_A5I_MASK        0x0c00
#define CYGARC_REG_WCR1_A5I_SHIFT       10
#define CYGARC_REG_WCR1_A4I_MASK        0x0300
#define CYGARC_REG_WCR1_A4I_SHIFT       8
#define CYGARC_REG_WCR1_A3I_MASK        0x00c0
#define CYGARC_REG_WCR1_A3I_SHIFT       6
#define CYGARC_REG_WCR1_A2I_MASK        0x0030
#define CYGARC_REG_WCR1_A2I_SHIFT       4
#define CYGARC_REG_WCR1_A0I_MASK        0x0003
#define CYGARC_REG_WCR1_A0I_SHIFT       0

#define CYGARC_REG_WCR1_0WS         0
#define CYGARC_REG_WCR1_1WS         1
#define CYGARC_REG_WCR1_2WS         2
#define CYGARC_REG_WCR1_3WS         3


// Wait states
#define CYGARC_REG_WCR2_A6_MASK         0xe000 // Wait states + burst pitch
#define CYGARC_REG_WCR2_A6_SHIFT        13
#define CYGARC_REG_WCR2_A5_MASK         0x1c00 // Wait states + burst pitch
#define CYGARC_REG_WCR2_A5_SHIFT        10
#define CYGARC_REG_WCR2_A4_MASK         0x0380 // Wait states
#define CYGARC_REG_WCR2_A4_SHIFT        7
#define CYGARC_REG_WCR2_A3_MASK         0x0060 // Wait states / CAS latency
#define CYGARC_REG_WCR2_A3_SHIFT        5
#define CYGARC_REG_WCR2_A2_MASK         0x0018 // Wait states / CAS latency
#define CYGARC_REG_WCR2_A2_SHIFT        3
#define CYGARC_REG_WCR2_A0_MASK         0x0007 // Wait states + burst pitch
#define CYGARC_REG_WCR2_A0_SHIFT        0

#define CYGARC_REG_WCR2_0WS             0
#define CYGARC_REG_WCR2_1WS             1
#define CYGARC_REG_WCR2_2WS             2
#define CYGARC_REG_WCR2_3WS             3
#define CYGARC_REG_WCR2_4WS             4
#define CYGARC_REG_WCR2_6WS             5
#define CYGARC_REG_WCR2_8WS             6
#define CYGARC_REG_WCR2_10WS            7

//--------------------------------------------------------------------------
// Additional type definitions
#if (CYGARC_SH_MOD_BCN > 1)
# define CYGARC_REG_BCR3                0xffffff7e
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


