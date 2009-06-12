//=============================================================================
//
//      mod_regs_emi.h
//
//      EMI (External Memory Interface) Module register definitions
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Nick Garnett 
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holder.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   nickg
// Contributors:nickg
// Date:        2003-08-20
//              
//####DESCRIPTIONEND####
//
//=============================================================================

//--------------------------------------------------------------------------

#define CYGARC_REG_EMI_BASE     0xFEC00000
#define CYGARC_REG_EMI_VCR_L    (CYGARC_REG_EMI_BASE+0x00)
#define CYGARC_REG_EMI_VCR_H    (CYGARC_REG_EMI_BASE+0x04)
#define CYGARC_REG_EMI_MIM      (CYGARC_REG_EMI_BASE+0x0c)
#define CYGARC_REG_EMI_SCR      (CYGARC_REG_EMI_BASE+0x14)
#define CYGARC_REG_EMI_STR      (CYGARC_REG_EMI_BASE+0x1c)
#define CYGARC_REG_EMI_COC      (CYGARC_REG_EMI_BASE+0x24)
#define CYGARC_REG_EMI_SDRA0    (CYGARC_REG_EMI_BASE+0x3c)
#define CYGARC_REG_EMI_SDRA1    (CYGARC_REG_EMI_BASE+0x34)
#define CYGARC_REG_EMI_SDMR0    (CYGARC_REG_EMI_BASE+0x100000)
#define CYGARC_REG_EMI_SDMR1    (CYGARC_REG_EMI_BASE+0x200000)

#define CYGARC_REG_EMI_VCR_L_ID         0x0000FFFF
#define CYGARC_REG_EMI_VCR_L_BOT_MB     0x00FF0000
#define CYGARC_REG_EMI_VCR_L_TOP_MB     0xFF000000

#define CYGARC_REG_EMI_VCR_H_PERR       0x000000FF
#define CYGARC_REG_EMI_VCR_H_MERR       0x0000FF00
#define CYGARC_REG_EMI_VCR_H_VERS       0xFFFF0000

#define CYGARC_REG_EMI_MIM_DCR          0x00000001
#define CYGARC_REG_EMI_MIM_DT           0x00000002
#define CYGARC_REG_EMI_MIM_BW           0x000000c0
#define CYGARC_REG_EMI_MIM_ENDIAN       0x00000100
#define CYGARC_REG_EMI_MIM_DRE          0x00000200
#define CYGARC_REG_EMI_MIM_DIMM         0x00000400
#define CYGARC_REG_EMI_MIM_BY32AP       0x00000800
#define CYGARC_REG_EMI_MIM_DRI          0x0fff0000

#define CYGARC_REG_EMI_SCR_SMS          0x00000007
#define CYGARC_REG_EMI_SCR_BRFSH        0x00000070

#define CYGARC_REG_EMI_STR_SRP          0x00000001
#define CYGARC_REG_EMI_STR_SRCD         0x00000002
#define CYGARC_REG_EMI_STR_SCL          0x0000001c
#define CYGARC_REG_EMI_STR_SRC          0x000000e0
#define CYGARC_REG_EMI_STR_SRAS         0x00000700
#define CYGARC_REG_EMI_STR_SRRD         0x00000800
#define CYGARC_REG_EMI_STR_SDPL         0x00001000

#define CYGARC_REG_EMI_COC_LOCK         0x00000001

#define CYGARC_REG_EMI_SDRA_SPLIT       0x00000f00
#define CYGARC_REG_EMI_SDRA_BANK        0x00001000
#define CYGARC_REG_EMI_SDRA_UBA         0xffe00000


// end of mod_regs_emi.h

