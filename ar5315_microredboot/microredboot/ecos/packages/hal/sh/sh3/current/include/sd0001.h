#ifndef CYGONCE_HAL_SD0001_H
#define CYGONCE_HAL_SD0001_H

//=============================================================================
//
//      sd0001.h
//
//      SD0001 support chip - used with SH/7729
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
// Date:        2001-05-25
// Purpose:     Support chip
// Usage:       Included from <cyg/hal/sh_regs.h>
//
//              
//####DESCRIPTIONEND####
//
//=============================================================================
#ifndef _SD0001_BASE
# define _SD0001_BASE 0xb0000000
#endif

//-----------------------------------------------------------------------------
// System configuration
#define CYGARC_REG_SD0001_RESET             (_SD0001_BASE + 0x08)
#define CYGARC_REG_SD0001_SDRAM             (_SD0001_BASE + 0x10)
#define CYGARC_REG_SD0001_INT_STS1          (_SD0001_BASE + 0x20)
#define CYGARC_REG_SD0001_INT_ENABLE        (_SD0001_BASE + 0x24)
#define CYGARC_REG_SD0001_INT_STS2          (_SD0001_BASE + 0x28)
#define CYGARC_REG_SD0001_PCI_CTL           (_SD0001_BASE + 0x50)

#define CYGARC_REG_PCI_IO_MEMOFFSET         (_SD0001_BASE + 0x58)
#define CYGARC_REG_PCI_MEM_MEMOFFSET        (_SD0001_BASE + 0x5c)


#define CYGARC_REG_PCI_CFG_ADDR             (_SD0001_BASE + 0x60)
#define CYGARC_REG_PCI_CFG_DATA             (_SD0001_BASE + 0x64)
#define CYGARC_REG_PCI_CFG_CMD              (_SD0001_BASE + 0x68)
#define CYGARC_REG_PCI_CFG_FLG              (_SD0001_BASE + 0x6c)

#define CYGARC_REG_PCI_CFG_ADDR_ENABLE      0x80000000
#define CYGARC_REG_PCI_CFG_ADDR_BUSNO_shift 16
#define CYGARC_REG_PCI_CFG_ADDR_FUNC_shift  8

#define CYGARC_REG_PCI_CFG_CMD_BE3          0x00080000
#define CYGARC_REG_PCI_CFG_CMD_BE2          0x00040000
#define CYGARC_REG_PCI_CFG_CMD_BE1          0x00020000
#define CYGARC_REG_PCI_CFG_CMD_BE0          0x00010000

#define CYGARC_REG_PCI_CFG_CMD_CMDEN        0x00008000

#define CYGARC_REG_PCI_CFG_CMD_IO_WRITE     0x00000300
#define CYGARC_REG_PCI_CFG_CMD_IO_READ      0x00000200

#define CYGARC_REG_PCI_CFG_CMD_WT           0x00000008
#define CYGARC_REG_PCI_CFG_CMD_RD           0x00000004
#define CYGARC_REG_PCI_CFG_CMD_CFWT         0x00000002
#define CYGARC_REG_PCI_CFG_CMD_CFRD         0x00000001


#define CYGARC_REG_PCI_CFG_CMD_WCFG         (CYGARC_REG_PCI_CFG_CMD_BE3|CYGARC_REG_PCI_CFG_CMD_BE2|CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0|CYGARC_REG_PCI_CFG_CMD_CFWT)
#define CYGARC_REG_PCI_CFG_CMD_RCFG         (CYGARC_REG_PCI_CFG_CMD_BE3|CYGARC_REG_PCI_CFG_CMD_BE2|CYGARC_REG_PCI_CFG_CMD_BE1|CYGARC_REG_PCI_CFG_CMD_BE0|CYGARC_REG_PCI_CFG_CMD_CFRD)



#define CYGARC_REG_SD0001_PCI_CTL_ENDIAN2          0x40000000
#define CYGARC_REG_SD0001_PCI_CTL_MAX_DEADLOCK_CNT 0x0000ff00
#define CYGARC_REG_SD0001_PCI_CTL_MAX_RETRY_CNT    0x000000f0


#define CYGARC_REG_PCI_CFG_FLG_ACTIVE       0x00000001


#define CYGARC_REG_SD0001_RESET_SWRST       0x80000000
#define CYGARC_REG_SD0001_RESET_PCIRST      0x40000000


#define CYGARC_REG_SD0001_SDRAM_SDKIND_128M 0x80000000
#define CYGARC_REG_SD0001_SDRAM_SDSIZE_8    0x10000000
#define CYGARC_REG_SD0001_SDRAM_REF_MUMBLE  0x00008000
#define CYGARC_REG_SD0001_SDRAM_LMODE_CAS2  0x00000010

#define CYGARC_REG_SD0001_SDRAM_INIT (CYGARC_REG_SD0001_SDRAM_SDKIND_128M\
                                      |CYGARC_REG_SD0001_SDRAM_SDSIZE_8\
                                      |CYGARC_REG_SD0001_SDRAM_REF_MUMBLE\
                                      |CYGARC_REG_SD0001_SDRAM_LMODE_CAS2)


#define CYGARC_REG_SD0001_INT_EN       0x80000000
#define CYGARC_REG_SD0001_INT_INTD     0x00000008
#define CYGARC_REG_SD0001_INT_INTC     0x00000004
#define CYGARC_REG_SD0001_INT_INTB     0x00000002
#define CYGARC_REG_SD0001_INT_INTA     0x00000001


#endif // CYGONCE_HAL_SD0001_H
