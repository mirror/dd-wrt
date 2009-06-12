//=============================================================================
//
//      mod_regs_ubc.h
//
//      UBC (user break controller) Module register definitions
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
// User Break Control

#define CYGARC_REG_BARA                 0xffffff00 // 32 bit
#define CYGARC_REG_BAMRA                0xffffff04 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BBRA                 0xffffff08 // 16 bit

#define CYGARC_REG_BARB                 0xffffff20 // 32 bit
#define CYGARC_REG_BAMRB                0xffffff24 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BBRB                 0xffffff28 // 16 bit

#define CYGARC_REG_BARC                 0xffffff40 // 32 bit
#define CYGARC_REG_BAMRC                0xffffff44 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BBRC                 0xffffff48 // 16 bit

#define CYGARC_REG_BARD                 0xffffff60 // 32 bit
#define CYGARC_REG_BAMRD                0xffffff64 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BBRD                 0xffffff68 // 16 bit


#define CYGARC_REG_BRCR                 0xffffff30 // 32 bit

#ifdef CYGARC_SH_MOD_UBC

#if (CYGARC_SH_MOD_UBC == 1)
#else
# define CYGARC_REG_BRCR_CMFCA          0x80000000
# define CYGARC_REG_BRCR_CMFPA          0x40000000
# define CYGARC_REG_BRCR_PCTE           0x08000000 // PC trace enable
# define CYGARC_REG_BRCR_PCBA           0x04000000 // post execute
# define CYGARC_REG_BRCR_CMFCB          0x00800000
# define CYGARC_REG_BRCR_CMFPB          0x00400000
# define CYGARC_REG_BRCR_SEQ1           0x00100000 // A and B channel in sequence
# define CYGARC_REG_BRCR_SEQ0           0x00080000 // A and B channel in sequence
# define CYGARC_REG_BRCR_PCBB           0x00040000

# define CYGARC_REG_BRCR_CMFCC          0x00008000
# define CYGARC_REG_BRCR_CMFPC          0x00004000
# define CYGARC_REG_BRCR_ETBEC          0x00002000
# define CYGARC_REG_BRCR_DBEC           0x00000800
# define CYGARC_REG_BRCR_PCBC           0x00000400
# define CYGARC_REG_BRCR_CMFCD          0x00000080
# define CYGARC_REG_BRCR_CMFPD          0x00000040
# define CYGARC_REG_BRCR_ETBED          0x00000020
# define CYGARC_REG_BRCR_DBED           0x00000008
# define CYGARC_REG_BRCR_PCBD           0x00000004

# define CYGARC_REG_BRCR_ONE_STEP       (CYGARC_REG_BRCR_PCBA)
#endif


#if (CYGARC_SH_MOD_UBC == 1)
#else
// mask is fully configurable in other versions of the UBC
#endif

#define CYGARC_REG_BBRA_DFETCH          0x0020 // Break on DFETCH
#define CYGARC_REG_BBRA_IFETCH          0x0010 // Break on IFETCH
#define CYGARC_REG_BBRA_WRITE           0x0008 // Break on WRITE
#define CYGARC_REG_BBRA_READ            0x0004 // Break on READ
#define CYGARC_REG_BBRA_SIZE_LONG       0x0003 // Break on long access
#define CYGARC_REG_BBRA_SIZE_WORD       0x0002 // Break on word access
#define CYGARC_REG_BBRA_SIZE_BYTE       0x0001 // Break on byte access
#define CYGARC_REG_BBRA_SIZE_ANY        0x0000 // Break on any size


//----------------------------------------------------------------------------
// Other types
#if (CYGARC_SH_MOD_UBC == 2)
#define CYGARC_REG_BETR                 0xffffff9c // 16 bit
#define CYGARC_REG_BRSR                 0xffffffac // 32 bit
#define CYGARC_REG_BRDR                 0xffffffbc // 32 bit

#define CYGARC_REG_BBRA_DMA             0x0080 // Break on DMAC cycle
#define CYGARC_REG_BBRA_CPU             0x0040 // Break on CPU cycle
#endif

#endif // CYGARC_SH_MOD_UBC
