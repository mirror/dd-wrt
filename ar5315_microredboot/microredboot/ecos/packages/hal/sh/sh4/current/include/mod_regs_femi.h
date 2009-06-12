//=============================================================================
//
//      mod_regs_femi.h
//
//      FEMI (FLASH External Memory Interface) Module register definitions
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

#define CYGARC_REG_FEMI_BASE    0xFF800000
#define CYGARC_REG_FEMI_VCR_L   (CYGARC_REG_FEMI_BASE+0x00)
#define CYGARC_REG_FEMI_VCR_H   (CYGARC_REG_FEMI_BASE+0x04)
#define CYGARC_REG_FEMI_MDCR    (CYGARC_REG_FEMI_BASE+0x0c)
#define CYGARC_REG_FEMI_A0MCR   (CYGARC_REG_FEMI_BASE+0x14)
#define CYGARC_REG_FEMI_A1MCR   (CYGARC_REG_FEMI_BASE+0x1c)
#define CYGARC_REG_FEMI_A2MCR   (CYGARC_REG_FEMI_BASE+0x24)
#define CYGARC_REG_FEMI_A3MCR   (CYGARC_REG_FEMI_BASE+0x2c)
#define CYGARC_REG_FEMI_A4MCR   (CYGARC_REG_FEMI_BASE+0x34)

#define CYGARC_REG_FEMI_VCR_L_ID        0x0000FFFF
#define CYGARC_REG_FEMI_VCR_L_BOT_MB    0x00FF0000
#define CYGARC_REG_FEMI_VCR_L_TOP_MB    0xFF000000

#define CYGARC_REG_FEMI_VCR_H_PERR      0x000000FF
#define CYGARC_REG_FEMI_VCR_H_MERR      0x0000FF00
#define CYGARC_REG_FEMI_VCR_H_VERS      0xFFFF0000

#define CYGARC_REG_FEMI_MDCR_HIZMEM     0x00000001
#define CYGARC_REG_FEMI_MDCR_BREQEN     0x00000002
#define CYGARC_REG_FEMI_MDCR_ENDIAN     0x00000004
#define CYGARC_REG_FEMI_MDCR_MERGE      0x00000008

#define CYGARC_REG_FEMI_ANMCR_TYPE      0x00000007
#define CYGARC_REG_FEMI_ANMCR_SZ        0x00000018
#define CYGARC_REG_FEMI_ANMCR_BST       0x000000e0
#define CYGARC_REG_FEMI_ANMCR_FLMD      0x00000100
#define CYGARC_REG_FEMI_ANMCR_FLWP      0x00000200
#define CYGARC_REG_FEMI_ANMCR_MBC       0x00000400
#define CYGARC_REG_FEMI_ANMCR_IW        0x00007000
#define CYGARC_REG_FEMI_ANMCR_BP        0x00070000
#define CYGARC_REG_FEMI_ANMCR_WS        0x00700000
#define CYGARC_REG_FEMI_ANMCR_HLD       0x03000000
#define CYGARC_REG_FEMI_ANMCR_STUP      0x08000000

// end of mod_regs_femi.h

