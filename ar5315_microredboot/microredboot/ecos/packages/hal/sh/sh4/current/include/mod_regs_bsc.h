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
#define CYGARC_REG_BCR1                 0xFF800000
#define CYGARC_REG_BCR2                 0xFF800004
#define CYGARC_REG_WCR1                 0xFF800008
#define CYGARC_REG_WCR2                 0xFF80000C
#define CYGARC_REG_WCR3                 0xFF800010
#define CYGARC_REG_MCR                  0xFF800014
#define CYGARC_REG_PCR                  0xFF800018
#define CYGARC_REG_RTCSR                0xFF80001C
#define CYGARC_REG_RTCNT                0xFF800020
#define CYGARC_REG_RTCOR                0xFF800024
#define CYGARC_REG_RFCR                 0xFF800028

#define CYGARC_REG_PCTRA                0xFF80002c
#define CYGARC_REG_PDTRA                0xFF800030
#define CYGARC_REG_PCTRB                0xFF800040
#define CYGARC_REG_PDTRB                0xFF800044
#define CYGARC_REG_GPIOIC               0xFF800048

#define CYGARC_REG_SDMR_AREA2_BASE      0xff900000
#define CYGARC_REG_SDMR_AREA3_BASE      0xff940000

#define CYGARC_REG_BCR1_MASTER          0x40000000

#define CYGARC_REG_MCR_RASD             0x80000000
#define CYGARC_REG_MCR_MRSET            0x40000000
#define CYGARC_REG_MCR_TCAS             0x00800000
#define CYGARC_REG_MCR_BE               0x00000100
#define CYGARC_REG_MCR_RFSH             0x00000004
#define CYGARC_REG_MCR_RMODE            0x00000002
#define CYGARC_REG_MCR_EDO_MODE         0x00000001

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


