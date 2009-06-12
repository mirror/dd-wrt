#ifndef CYGONCE_CYG_FS_FAT_H
#define CYGONCE_CYG_FS_FAT_H
//=============================================================================
//
//      fatfs.h
//
//      FAT FS attributes and stat information
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2004 Red Hat, Inc.
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
// Author(s):     David Brennam <eCos@brennanhome.com>
// Contributors:  
// Date:          2004-10-22
// Purpose:       
// Description:   This header contains attributes and stat like mode
//                information for FAT filesystems.
//              
// Usage:
//              #include <cyg/fs/fatfs.h>
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================
// -------------------------------------------------------------------------
// FAT filesystem dir entry attributes

#define S_FATFS_RDONLY  (0x01) // Read only
#define S_FATFS_HIDDEN  (0x02) // Hidden
#define S_FATFS_SYSTEM  (0x04) // System
#define S_FATFS_VOLUME  (0x08) // Volume label
#define S_FATFS_DIR     (0x10) // Subdirectory
#define S_FATFS_ARCHIVE (0x20) // Needs archiving

// Mode bits which are allowed to be changed by attrib
#define S_FATFS_ATTRIB   (S_FATFS_RDONLY | S_FATFS_HIDDEN | S_FATFS_SYSTEM | \
                          S_FATFS_ARCHIVE)
// -------------------------------------------------------------------------
// mode FAT dir entry attributes macros

#define S_FATFS_ISRDONLY(__mode)  ((__mode) & S_FATFS_RDONLY)
#define S_FATFS_ISHIDDEN(__mode)  ((__mode) & S_FATFS_HIDDEN)
#define S_FATFS_ISSYSTEM(__mode)  ((__mode) & S_FATFS_SYSTEM)
#define S_FATFS_ISVOLUME(__mode)  ((__mode) & S_FATFS_VOLUME)
#define S_FATFS_ISDIR(__mode)     ((__mode) & S_FATFS_DIR)
#define S_FATFS_ISARCHIVE(__mode) ((__mode) & S_FATFS_ARCHIVE)

#endif // CYGONCE_CYG_FS_FAT_H
// End of fatfs.h

