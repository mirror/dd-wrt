//==========================================================================
//
//      e2fs.h
//
//      Second extended filesystem defines.
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
// Date:         2001-06-25
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef CYGONCE_REDBOOT_E2FS_H
#define CYGONCE_REDBOOT_E2FS_H

//
// Structure of the super block
//
struct e2fs_super_block {
    cyg_uint32  inodes_count;
    cyg_uint32  blocks_count;
    cyg_uint32  r_blocks_count;
    cyg_uint32  free_blocks_count;
    cyg_uint32  free_inodes_count;
    cyg_uint32  first_data_block;
    cyg_uint32  log_block_size;
    cyg_int32   log_frag_size;
    cyg_uint32  blocks_per_group;
    cyg_uint32  frags_per_group;
    cyg_uint32  inodes_per_group;
    cyg_uint32  mtime;
    cyg_uint32  wtime;
    cyg_uint16  mnt_count;
    cyg_int16   max_mnt_count;
    cyg_uint16  magic;
    cyg_uint16  state;
    cyg_uint16  errors;
    cyg_uint16  minor_rev_level;
    cyg_uint32  lastcheck;
    cyg_uint32  checkinterval;
    cyg_uint32  creator_os;
    cyg_uint32  rev_level;
    cyg_uint16  def_resuid;
    cyg_uint16  def_resgid;
};

#define E2FS_PRE_02B_MAGIC	0xEF51
#define E2FS_SUPER_MAGIC	0xEF53

#define E2FS_PTRS_PER_BLOCK(e)   ((e)->blocksize / sizeof(cyg_uint32))

#define E2FS_BLOCK_SIZE(s)	  (E2FS_MIN_BLOCK_SIZE << SWAB_LE32((s)->log_block_size))
#define	E2FS_ADDR_PER_BLOCK(s)	  (E2FS_BLOCK_SIZE(s) / sizeof(unsigned int))
#define E2FS_BLOCK_SIZE_BITS(s)	  (SWAB_LE32((s)->log_block_size) + 10)

#define	E2FS_NR_DIR_BLOCKS	12

#define	E2FS_IND_BLOCK		E2FS_NR_DIR_BLOCKS
#define	E2FS_DIND_BLOCK		(E2FS_IND_BLOCK + 1)
#define	E2FS_TIND_BLOCK		(E2FS_DIND_BLOCK + 1)

#define	E2FS_N_BLOCKS		(E2FS_TIND_BLOCK + 1)


// Structure of an inode on the disk
//
typedef struct e2fs_inode {
    cyg_uint16   mode;
    cyg_uint16   uid;
    cyg_uint32   size;
    cyg_uint32   atime;
    cyg_uint32   ctime;
    cyg_uint32   mtime;
    cyg_uint32   dtime;
    cyg_uint16   gid;
    cyg_uint16   links_count;
    cyg_uint32   blocks;
    cyg_uint32   flags;
    cyg_uint32   reserved1;
    cyg_uint32   block[E2FS_N_BLOCKS];
    cyg_uint32   version;
    cyg_uint32   file_acl;
    cyg_uint32   dir_acl;
    cyg_uint32   faddr;
    cyg_uint8    frag;
    cyg_uint8    fsize;
    cyg_uint16   pad1;
    cyg_uint32   reserved2[2];
} e2fs_inode_t;


#define	E2FS_INODES_PER_BLOCK(e)  ((e)->blocksize / sizeof (struct e2fs_inode))

#define E2FS_MIN_BLOCK_SIZE	  1024
#define	E2FS_MAX_BLOCK_SIZE	  4096

// Special inode numbers
//
#define	E2FS_BAD_INO		 1
#define E2FS_ROOT_INO		 2

typedef struct e2fs_dir_entry {
    cyg_uint32 inode;
    cyg_uint16 reclen;
    cyg_uint8  namelen;
    cyg_uint8  filetype;
    char       name[2];
} e2fs_dir_entry_t;

#define E2FS_FTYPE_UNKNOWN  0
#define E2FS_FTYPE_REG_FILE 1
#define E2FS_FTYPE_DIR      2
#define E2FS_FTYPE_CHRDEV   3
#define E2FS_FTYPE_BLKDEV   4
#define E2FS_FTYPE_FIFO     5
#define E2FS_FTYPE_SOCK     6
#define E2FS_FTYPE_SYMLINK  7

typedef struct e2fs_group
{
    cyg_uint32 block_bitmap;	   // blocks bitmap block
    cyg_uint32 inode_bitmap;	   // inodes bitmap block
    cyg_uint32 inode_table;	   // inodes table block
    cyg_uint16 free_blocks_count;
    cyg_uint16 free_inodes_count;
    cyg_uint16 used_dirs_count;
    cyg_uint16 pad;
    cyg_uint32 reserved[3];
} e2fs_group_t;

#define E2FS_BLOCKS_PER_GROUP(s)  (SWAB_LE32((s)->blocks_per_group))
#define E2FS_INODES_PER_GROUP(s)  (SWAB_LE32((s)->inodes_per_group))

#define	E2FS_GDESC_PER_BLOCK(e)	  ((e)->blocksize / sizeof (struct e2fs_e2fs_group_desc))
#define E2FS_GDESC_PER_SECTOR     (SECTOR_SIZE/sizeof(e2fs_group_t))
#define E2FS_GDESC_CACHE_SIZE     (E2FS_GDESC_PER_SECTOR * 1)
#define E2FS_GDESC_PER_SECTOR     (SECTOR_SIZE/sizeof(e2fs_group_t))

typedef struct e2fs_desc {
    partition_t  *part;     		// partition holding this filesystem
    cyg_uint32   blocksize;		// fs blocksize
    cyg_uint32   ngroups;		// number of groups in fs
    cyg_uint32   blocks_per_group;
    cyg_uint32   inodes_per_group;
    cyg_uint32   gdesc_block;           // block nr of group descriptors
    cyg_int32    gdesc_first;           // which gdesc is first in cache
    e2fs_group_t gdesc_cache[E2FS_GDESC_CACHE_SIZE];
    cyg_uint32   nr_ind_blocks;
    cyg_uint32   nr_dind_blocks;
    cyg_uint32   nr_tind_blocks;
} e2fs_desc_t;

#define E2FS_BLOCK_TO_SECTOR(e,b)  ((b) * ((e)->blocksize / SECTOR_SIZE))

extern fs_funs_t redboot_e2fs_funs;

#endif  // CYGONCE_REDBOOT_E2FS_H
