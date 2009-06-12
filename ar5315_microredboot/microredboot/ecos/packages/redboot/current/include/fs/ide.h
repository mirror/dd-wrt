//==========================================================================
//
//      ide.h
//
//      IDE Interface Driver Tables for RedBoot
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    msalter
// Contributors: msalter
// Date:         2001-07-06
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_REDBOOT_IDE_H
#define CYGONCE_REDBOOT_IDE_H

// IDE Register Indices
#define IDE_REG_DATA      0
#define IDE_REG_ERROR     1
#define IDE_REG_FEATURES  1
#define IDE_REG_COUNT     2
#define IDE_REG_REASON    2  // ATAPI
#define IDE_REG_LBALOW    3
#define IDE_REG_LBAMID    4
#define IDE_REG_LBAHI     5
#define IDE_REG_DEVICE    6
#define IDE_REG_STATUS    7
#define IDE_REG_COMMAND   7

#define IDE_STAT_BSY      0x80
#define IDE_STAT_DRDY     0x40
#define IDE_STAT_SERVICE  0x10
#define IDE_STAT_DRQ      0x08
#define IDE_STAT_CORR     0x04
#define IDE_STAT_ERR      0x01

#define IDE_REASON_REL    0x04
#define IDE_REASON_IO     0x02
#define IDE_REASON_COD    0x01

//
// Drive ID offsets of interest
//
#define IDE_DEVID_GENCONFIG      0
#define IDE_DEVID_SERNO         20
#define IDE_DEVID_MODEL         54
#define IDE_DEVID_LBA_CAPACITY 120

struct ide_priv {
    cyg_uint8   controller;
    cyg_uint8   drive;
    cyg_uint16  flags;
};

/* flag values */
#define IDE_DEV_PRESENT  1  // Device is present
#define IDE_DEV_PACKET   2  // Supports packet interface
#define IDE_DEV_ADDR48   3  // Supports 48bit addressing

#define CDROM_SECTOR_SIZE 2048
#define SECTORS_PER_CDROM_SECTOR (CDROM_SECTOR_SIZE/SECTOR_SIZE)

#endif // CYGONCE_REDBOOT_IDE_H
