#ifndef MEMCFG_H
#define MEMCFG_H
//=============================================================================
//
//      memcfg.h
//
//      Samsung SMDK2410 specific memory configuration
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
// Author(s):    michael anburaj <michaelanburaj@hotmail.com>
// Contributors: michael anburaj <michaelanburaj@hotmail.com>
// Date:         2003-08-01
// Purpose:      ARM9/SMDK2410 platform specific memory configuration
// Description: 
// Usage:        #include <cyg/hal/memcfg.h>
//     Only used by "hal_platform_setup.h"         
//
//####DESCRIPTIONEND####
//
//=============================================================================

// Bus width
#define BUSWIDTH 32

//Memory Area
//GCS6 16bit(16MB) SDRAM(0x0c000000-0x0cffffff)
//GCS7 16bit(16MB) SDRAM(0x0d000000-0x0dffffff)
//          or
//GCS6 32bit(32MB) SDRAM(0x0c000000-0x0dffffff)

 
//BWSCON
#define DW8          (0x0)
#define DW16         (0x1)
#define DW32         (0x2)
#define WAIT         (0x1<<2)
#define UBLB         (0x1<<3)

#ifndef BUSWIDTH
#error BUSWIDTH not defined
#endif
#if (BUSWIDTH == 16)
#define B1_BWSCON    (DW16)
#define B2_BWSCON    (DW16)
#define B3_BWSCON    (DW16)
#define B4_BWSCON    (DW16)
#define B5_BWSCON    (DW16)
#define B6_BWSCON    (DW16)
#define B7_BWSCON    (DW16)
#else //BUSWIDTH=32
#define B1_BWSCON    (DW32)
#define B2_BWSCON    (DW16)
#define B3_BWSCON    (DW16)
#define B4_BWSCON    (DW16)
#define B5_BWSCON    (DW16)
#define B6_BWSCON    (DW32)
#define B7_BWSCON    (DW32)
#endif

//BANK0CON 

#define B0_Tacs           0x0    //0clk
#define B0_Tcos           0x0    //0clk
#define B0_Tacc           0x7    //14clk
#define B0_Tcoh           0x0    //0clk
#define B0_Tah            0x0    //0clk
#define B0_Tacp           0x0    
#define B0_PMC            0x0    //normal

//BANK1CON
#define B1_Tacs           0x0    //0clk
#define B1_Tcos           0x0    //0clk
#define B1_Tacc           0x7    //14clk
#define B1_Tcoh           0x0    //0clk
#define B1_Tah            0x0    //0clk
#define B1_Tacp           0x0    
#define B1_PMC            0x0    //normal

//Bank 2 parameter
#define B2_Tacs           0x0    //0clk
#define B2_Tcos           0x0    //0clk
#define B2_Tacc           0x7    //14clk
#define B2_Tcoh           0x0    //0clk
#define B2_Tah            0x0    //0clk
#define B2_Tacp           0x0    
#define B2_PMC            0x0    //normal

//Bank 3 parameter
#define B3_Tacs           0x0    //0clk
#define B3_Tcos           0x0    //0clk
#define B3_Tacc           0x7    //14clk
#define B3_Tcoh           0x0    //0clk
#define B3_Tah            0x0    //0clk
#define B3_Tacp           0x0    
#define B3_PMC            0x0    //normal

//Bank 4 parameter
#define B4_Tacs           0x0    //0clk
#define B4_Tcos           0x0    //0clk
#define B4_Tacc           0x7    //14clk
#define B4_Tcoh           0x0    //0clk
#define B4_Tah            0x0    //0clk
#define B4_Tacp           0x0    
#define B4_PMC            0x0    //normal

//Bank 5 parameter
#define B5_Tacs           0x0    //0clk
#define B5_Tcos           0x0    //0clk
#define B5_Tacc           0x7    //14clk
#define B5_Tcoh           0x0    //0clk
#define B5_Tah            0x0    //0clk
#define B5_Tacp           0x0    
#define B5_PMC            0x0    //normal

//Bank 6 parameter
#define B6_MT             0x3    //SDRAM
//#define B6_Trcd           0x0    //2clk
#define B6_Trcd           0x1    //3clk
#define B6_SCAN           0x1    //9bit

//Bank 7 parameter
#define B7_MT             0x3    //SDRAM
//#define B7_Trcd           0x0    //2clk
#define B7_Trcd           0x1    //3clk
#define B7_SCAN           0x1    //9bit

//REFRESH parameter
#define REFEN             0x1    //Refresh enable
#define TREFMD            0x0    //CBR(CAS before RAS)/Auto refresh
#define Trp               0x0    //2clk
#define Trc               0x3    //7clk
        
#define Tchr              0x2    //3clk
#define REFCNT            1113    //period=15.6us, HCLK=60Mhz, (2048+1-15.6*60)

//-----------------------------------------------------------------------------
// end of memcfg.h
#endif // MEMCFG_H
