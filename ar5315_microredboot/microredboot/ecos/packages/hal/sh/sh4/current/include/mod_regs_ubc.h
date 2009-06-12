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
#define CYGARC_REG_BARA                 0xFF200000 // 32 bit
#define CYGARC_REG_BAMRA                0xFF200004 // 8 bit
#define CYGARC_REG_BBRA                 0xFF200008 // 16 bit
#define CYGARC_REG_BARB                 0xFF20000C // 32 bit
#define CYGARC_REG_BAMRB                0xFF200010 // 8 bit

#define CYGARC_REG_BBRB                 0xFF200014 // 16 bit
#define CYGARC_REG_BDRB                 0xFF200018 // 32 bit
#define CYGARC_REG_BDMRB                0xFF20001C // 32 bit

#define CYGARC_REG_BRCR                 0xFF200020 // 16 bit


#define CYGARC_REG_BRCR_CMFA            0x8000 // condition match flag A
#define CYGARC_REG_BRCR_CMFB            0x4000 // condition match flag B
#define CYGARC_REG_BRCR_PCBA            0x0400 // post execute channel A
#define CYGARC_REG_BRCR_DBEB            0x0080 // data break enable B
#define CYGARC_REG_BRCR_PCBB            0x0040 // post execute channel B
#define CYGARC_REG_BRCR_SEQ             0x0008 // sequence condition select
#define CYGARC_REG_BRCR_UBDE            0x0001 // User Break Debug Enable
#define CYGARC_REG_BRCR_ONE_STEP        (CYGARC_REG_BRCR_PCBA)

#if defined(CYGARC_SH_MOD_UBC) && (CYGARC_SH_MOD_UBC == 1)
#define CYGARC_REG_BAMRA_BASMA          0x04   // BASRA masked
#define CYGARC_REG_BAMRA_BARA_10BIT     0x01   // Lowest 10 bit masked
#define CYGARC_REG_BAMRA_BARA_12BIT     0x02   // Lowest 12 bit masked
#define CYGARC_REG_BAMRA_BARA_MASKED    0x03   // All bits masked
#define CYGARC_REG_BAMRA_BARA_16BIT     0x08   // Lowest 16 bit masked
#define CYGARC_REG_BAMRA_BARA_20BIT     0x09   // Lowest 20 bit masked

#define CYGARC_REG_BAMRA_BARA_UNMASKED  (0x00|CYGARC_REG_BAMRA_BASMA)   // BARA not masked, ignore ASID

#else
// mask is fully configurable in other versions of the UBC
#endif

#define CYGARC_REG_BBRA_DFETCH          0x0020 // Break on DFETCH
#define CYGARC_REG_BBRA_IFETCH          0x0010 // Break on IFETCH
#define CYGARC_REG_BBRA_WRITE           0x0008 // Break on WRITE
#define CYGARC_REG_BBRA_READ            0x0004 // Break on READ
#define CYGARC_REG_BBRA_SIZE_QUAD       0x0040 // Break on quad word access
#define CYGARC_REG_BBRA_SIZE_LONG       0x0003 // Break on long word access
#define CYGARC_REG_BBRA_SIZE_WORD       0x0002 // Break on word access
#define CYGARC_REG_BBRA_SIZE_BYTE       0x0001 // Break on byte access
#define CYGARC_REG_BBRA_SIZE_ANY        0x0000 // Break on any size
