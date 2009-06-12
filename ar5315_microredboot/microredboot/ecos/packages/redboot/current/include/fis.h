#ifndef _FIS_H_
#define _FIS_H_
//==========================================================================
//
//      fis.h
//
//      RedBoot - FLASH image directory layout
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
// Author(s):    gthomas
// Contributors: gthomas, tkoeller
// Date:         2000-07-28
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/redboot.h>
#ifdef CYGOPT_REDBOOT_FIS
#include <cyg/infra/cyg_type.h>

#define FIS_IMAGE_DESC_SIZE_UNPADDED \
  (16 + 4 * sizeof(unsigned long) + 3 * sizeof(CYG_ADDRESS))

struct fis_image_desc {
    unsigned char name[16];      // Null terminated name
    CYG_ADDRESS   flash_base;    // Address within FLASH of image
    CYG_ADDRESS   mem_base;      // Address in memory where it executes
    unsigned long size;          // Length of image
    CYG_ADDRESS   entry_point;   // Execution entry point
    unsigned long data_length;   // Length of actual data
    unsigned char _pad[CYGNUM_REDBOOT_FIS_DIRECTORY_ENTRY_SIZE-FIS_IMAGE_DESC_SIZE_UNPADDED];
    unsigned long desc_cksum;    // Checksum over image descriptor
    unsigned long file_cksum;    // Checksum over image data
};

struct fis_image_desc *fis_lookup(char *name, int *num);

#endif // CYGOPT_REDBOOT_FIS
#endif // _FIS_H_
