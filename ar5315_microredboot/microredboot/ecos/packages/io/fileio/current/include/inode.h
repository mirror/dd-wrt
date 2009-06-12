#ifndef CYGONCE_IO_FILEIO_INODE_H
#define CYGONCE_IO_FILEIO_INODE_H
//=============================================================================
//
//      inode.h
//
//      Header describing eCos inodes
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
// Author(s):     jlarmour
// Contributors:  
// Date:          2002-01-18
// Purpose:       Describe inodes
// Description:   This header contains the definitions that may be useful to a
//                filesystem implementation with the concept of inodes.
//                There should remain no *requirement* for a FS to use inodes.
//                Instead these are just useful functions and definitions.
//              
// Usage:
//              #include <cyg/fileio/inode.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <sys/types.h>
#include <time.h>   // time_t
#include <cyg/fileio/fileio.h>
#include <cyg/io/io.h>
#include <limits.h>

// some members are named for compatibility for Linux inodes
typedef struct CYG_INODE_TAG {
    cyg_uint32              i_ino;           // inode number
    mode_t                  i_mode;          // mode
    nlink_t                 i_nlink;         // number of (hard) links
    uid_t                   i_uid;           // UID
    gid_t                   i_gid;           // GID
    cyg_io_handle_t         i_device;        // underlying device
    off_t                   i_size;          // file size
    time_t                  i_atime;         // file access time
    time_t                  i_mtime;         // file modification time
    time_t                  i_ctime;         // file metadata change time
    struct CYG_INODE_TAG   *i_parent;        // parent inode

    // these duplicate the contents of a cyg_file, but that's okay
    cyg_mtab_entry*         i_mte;           // mount table entry
    struct CYG_FILEOPS_TAG *i_fop;           // file operations

    // inode cache related members
    cyg_atomic              i_count;         // number of references to this inode
    struct CYG_INODE_TAG   *i_cache_prev;    // previous in icache
    struct CYG_INODE_TAG   *i_cache_next;    // next in icache

    // This allows us to use a GCC extension to extend the size as appropriate
    // for individual FS's requirements.
    // Obviously anything here cannot be shared between FSs.
    CYG_ADDRWORD i_private[];
} cyg_inode;

// There should be one cyg_inodecache per FS instance (to preserve inode number
// uniqueness)
typedef void (*cyg_inodecache_freecallback_t)( cyg_inode *);
typedef struct {
    cyg_inode *head;
#if CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD > 0
    cyg_inode *freeable;
    cyg_atomic freeablelistlen;
#endif
    cyg_inodecache_freecallback_t freecallback;
    size_t privatespace;
    // should be some locking eventually
} cyg_inodecache;

static __inline__ int
cyg_inodecache_init( cyg_inodecache *ic, size_t extraprivatespace,
                     cyg_inodecache_freecallback_t freecb )
{   
    ic->head = NULL;
#if CYGNUM_IO_FILEIO_MAX_INODE_CACHE_DEAD > 0
    ic->freeable = NULL;
    ic->freeablelistlen = 0;
#endif
    ic->privatespace = extraprivatespace;
    ic->freecallback = freecb;
    return 0;
}

// Create an inode, with extra private space. Returns NULL on error.
__externC cyg_inode *
cyg_inode_create( cyg_inodecache *ic );

__externC cyg_inode *
cyg_inode_get( cyg_inodecache *ic, cyg_uint32 ino );

__externC void
cyg_inode_put( cyg_inodecache *ic, cyg_inode *ino );

__externC void
cyg_inodecache_destroy( cyg_inodecache *ic );

#endif // CYGONCE_IO_FILEIO_INODE_H

// EOF inode.h
