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
#define CYGARC_REG_BRCR                 0xffffff98 // 16 bit (v1) / 32 bit

#define CYGARC_REG_BARA                 0xffffffb0 // 32 bit
#define CYGARC_REG_BAMRA                0xffffffb4 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BBRA                 0xffffffb8 // 16 bit
#define CYGARC_REG_BASRA                0xffffffe4 // 8 bit

#define CYGARC_REG_BARB                 0xffffffa0 // 32 bit
#define CYGARC_REG_BAMRB                0xffffffa4 // 8 bit (v1) / 32 bit
#define CYGARC_REG_BASRB                0xffffffe8 // 8 bit
#define CYGARC_REG_BBRB                 0xffffffa8 // 16 bit
#define CYGARC_REG_BDRB                 0xffffff90 // 32 bit
#define CYGARC_REG_BDMRB                0xffffff94 // 32 bit

#ifdef CYGARC_SH_MOD_UBC

#if (CYGARC_SH_MOD_UBC == 1)
# define CYGARC_REG_BRCR_CMFA           0x8000 // condition match flag A
# define CYGARC_REG_BRCR_CMFB           0x4000 // condition match flag B
# define CYGARC_REG_BRCR_PCBA           0x0400 // post execute channel A
# define CYGARC_REG_BRCR_DBEB           0x0080 // data break enable B
# define CYGARC_REG_BRCR_PCBB           0x0040 // post execute channel B
# define CYGARC_REG_BRCR_SEQ            0x0008 // sequence condition select
# define CYGARC_REG_BRCR_ONE_STEP       (CYGARC_REG_BRCR_PCBA)
#else
# define CYGARC_REG_BRCR_BASMA          0x00200000 // asid not checked
# define CYGARC_REG_BRCR_BASMB          0x00100000
# define CYGARC_REG_BRCR_SCMFCA         0x00008000
# define CYGARC_REG_BRCR_SCMFCB         0x00004000
# define CYGARC_REG_BRCR_SCMFDA         0x00002000
# define CYGARC_REG_BRCR_SCMFDB         0x00001000
# define CYGARC_REG_BRCR_PCTE           0x00000800 // PC trace enable
# define CYGARC_REG_BRCR_PCBA           0x00000400 // post execute
# define CYGARC_REG_BRCR_DBEB           0x00000080 // data break
# define CYGARC_REG_BRCR_PCBB           0x00000040
# define CYGARC_REG_BRCR_SEQ            0x00000008 // A and B channel in sequence
# define CYGARC_REG_BRCR_ETBE           0x00000001 // execution count on B matches
# define CYGARC_REG_BRCR_ONE_STEP       (CYGARC_REG_BRCR_BASMA | CYGARC_REG_BRCR_PCBA)
#endif


#if (CYGARC_SH_MOD_UBC == 1)
# define CYGARC_REG_BAMRA_BASMA          0x04   // BASRA masked
# define CYGARC_REG_BAMRA_BARA_UNMASKED  0x00   // BARA not masked
# define CYGARC_REG_BAMRA_BARA_10BIT     0x01   // Lowest 10 bit masked
# define CYGARC_REG_BAMRA_BARA_12BIT     0x02   // Lowest 12 bit masked
# define CYGARC_REG_BAMRA_BARA_MASKED    0x03   // All bits masked
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
