//==========================================================================
//
//      e2fs.c
//
//      RedBoot support for second extended filesystem
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas <gary@mind.be>
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
// Date:         2001-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>
#include <fs/disk.h>
#include <fs/e2fs.h>

#define DEBUG_E2FS 0

#if DEBUG_E2FS > 4
static void dump_sb(struct e2fs_super_block *s);
static void dump_inode(struct e2fs_inode *i);
#endif

static void *e2fs_open(partition_t *p, const char *path);
static int  e2fs_read(void *fp, char *buf, cyg_uint32 nbytes);

// This structure is the only thing exported by this module.
// These filesystem function pointers are attached to disk
// partitions in the generic disk handling code.
//
fs_funs_t redboot_e2fs_funs = {
    e2fs_open,
    e2fs_read
};

// A single block buffer to be shared carefully.
static cyg_uint32 blockbuf[E2FS_MAX_BLOCK_SIZE/sizeof(cyg_uint32)];

#define __READ_BLOCK(n)                                          \
  PARTITION_READ(e2fs->part, E2FS_BLOCK_TO_SECTOR(e2fs, (n)),    \
                      blockbuf, e2fs->blocksize/SECTOR_SIZE)

// Get a group descriptor. Returns non-zero for success.
//
static int
e2fs_get_gdesc(e2fs_desc_t *e2fs, cyg_uint32 group_nr, e2fs_group_t *gdesc)
{
    cyg_uint32 sec_nr;

    if (group_nr < e2fs->gdesc_first ||
	group_nr >= (e2fs->gdesc_first + E2FS_GDESC_CACHE_SIZE)) {

	// cache miss
	sec_nr = E2FS_BLOCK_TO_SECTOR(e2fs, e2fs->gdesc_block);
	sec_nr += (group_nr / E2FS_GDESC_PER_SECTOR);

#if DEBUG_E2FS > 2
	diag_printf("%s: group[%d] cache miss, sec_nr[%d]\n",
                    __FUNCTION__, group_nr, sec_nr);
#endif
	if (!PARTITION_READ(e2fs->part, sec_nr, (cyg_uint32 *)e2fs->gdesc_cache,
			    sizeof(e2fs->gdesc_cache)/SECTOR_SIZE))
	    return 0;

	e2fs->gdesc_first = (group_nr / E2FS_GDESC_CACHE_SIZE) * E2FS_GDESC_CACHE_SIZE;
    }
    *gdesc = e2fs->gdesc_cache[group_nr - e2fs->gdesc_first];

    return 1;
}

// Read the requested inode from disk. Return non-zero if successful
//
static int
e2fs_get_inode(e2fs_desc_t *e2fs, int ino, e2fs_inode_t *ip)
{
    cyg_uint32 offset, sec_nr, buf[SECTOR_SIZE/sizeof(cyg_uint32)];
    e2fs_group_t gdesc;

    // get descriptor for group which this inode belongs to
    if (!e2fs_get_gdesc(e2fs, (ino - 1) / e2fs->inodes_per_group, &gdesc))
	return 0;
    if (gdesc.inode_table == 0)
	return 0;

    // byte offset within group inode table
    offset = ((ino - 1) % e2fs->inodes_per_group) * sizeof(struct e2fs_inode);

    // figure out which sector holds the inode
    sec_nr = E2FS_BLOCK_TO_SECTOR(e2fs, SWAB_LE32(gdesc.inode_table));
    sec_nr += offset / SECTOR_SIZE;

    // and the offset within that sector.
    offset %= SECTOR_SIZE;

#if DEBUG_E2FS > 0x08
    diag_printf("%s: ino[%d], sec_nr[%d] offset[%d]\n", __FUNCTION__,
                ino, sec_nr, offset);
#endif

    if (!PARTITION_READ(e2fs->part, sec_nr, buf, 1))
	return 0;

    *ip = *(e2fs_inode_t *)((char *)buf + offset);

#if DEBUG_E2FS > 0
    diag_printf("%s: inode size[%d]\n", __FUNCTION__, SWAB_LE32(ip->size));
#endif

    return 1;
}

// Mount an e2fs filesystem on the given partition.
// Return 0 if successful.
//
static int
e2fs_mount(partition_t *part, e2fs_desc_t *e2fs)
{
    int sb_block = 1;
    cyg_uint32  sb_buf[E2FS_MIN_BLOCK_SIZE/sizeof(cyg_uint32)];
    struct e2fs_super_block *sb = (struct e2fs_super_block *)sb_buf;

    e2fs->part = part;

    if (!PARTITION_READ(part, sb_block*(E2FS_MIN_BLOCK_SIZE/SECTOR_SIZE),
			(cyg_uint32 *)sb, E2FS_MIN_BLOCK_SIZE/SECTOR_SIZE))
	return -1;

    if (SWAB_LE16(sb->magic) != E2FS_SUPER_MAGIC) {
	diag_printf("ext2_mount: bad magic 0x%x\n", SWAB_LE16(sb->magic));
	return -1;
    }

    // save some stuff for easy access
    e2fs->blocksize = E2FS_BLOCK_SIZE(sb);
    e2fs->nr_ind_blocks = (e2fs)->blocksize / sizeof(cyg_uint32);
    e2fs->nr_dind_blocks = e2fs->nr_ind_blocks * ((e2fs)->blocksize / sizeof(cyg_uint32));
    e2fs->nr_tind_blocks = e2fs->nr_dind_blocks * ((e2fs)->blocksize / sizeof(cyg_uint32));
    e2fs->blocks_per_group = SWAB_LE32(sb->blocks_per_group);
    e2fs->ngroups = (SWAB_LE32(sb->blocks_count) + e2fs->blocks_per_group - 1) /
	             e2fs->blocks_per_group;
    e2fs->inodes_per_group = SWAB_LE32(sb->inodes_per_group);

    // Find the group descriptors which follow superblock
    e2fs->gdesc_block = ((sb_block * E2FS_MIN_BLOCK_SIZE) / e2fs->blocksize) + 1;
    e2fs->gdesc_first = 0; // cache group 0 initially

    if (!PARTITION_READ(part, E2FS_BLOCK_TO_SECTOR(e2fs,e2fs->gdesc_block),
			(cyg_uint32 *)e2fs->gdesc_cache, 1))
	return -1;

#if DEBUG_E2FS > 1
    diag_printf("E2FS superblock:\n");
    diag_printf("   [%d] inodes\n", SWAB_LE32(sb->inodes_count));
    diag_printf("   [%d] blocks\n", SWAB_LE32(sb->blocks_count));
    diag_printf("   [%d] blocksize\n", e2fs->blocksize);
    diag_printf("   [%d] blocks per group\n", e2fs->blocks_per_group);
    diag_printf("   [%d] ngroups\n", e2fs->ngroups);
#endif

#if DEBUG_E2FS > 4
    dump_sb(sb);
#endif

    return 0;
}

// Convert a block index into inode data into a block_nr.
// If successful, store block number in pblknr and return non-zero.
//
// NB: This needs some block/sector caching to be speedier. But
//     that takes memory and speed is not too bad now for files
//     small enough to avoid double and triple indirection.
//
static int
e2fs_inode_block(e2fs_desc_t *e2fs, e2fs_inode_t *inode,
		 cyg_uint32 bindex, cyg_uint32 *pblknr)
{
    if (bindex < E2FS_NR_DIR_BLOCKS) {
	*pblknr = SWAB_LE32(inode->block[bindex]);
	return 1;
    }
    bindex -= E2FS_NR_DIR_BLOCKS;

    if (bindex < e2fs->nr_ind_blocks) {
	// Indirect block
	if (!__READ_BLOCK(SWAB_LE32(inode->block[E2FS_IND_BLOCK])))
	    return 0;
	*pblknr = SWAB_LE32(blockbuf[bindex]);
	return 1;
    }
    bindex -= e2fs->nr_ind_blocks;

    if (bindex < e2fs->nr_dind_blocks) {
	// Double indirect block
	if (!__READ_BLOCK(SWAB_LE32(inode->block[E2FS_DIND_BLOCK])))
	    return 0;
	if (!__READ_BLOCK(SWAB_LE32(blockbuf[bindex / e2fs->nr_ind_blocks])))
	    return 0;
	*pblknr  = SWAB_LE32(blockbuf[bindex % e2fs->nr_ind_blocks]);
	return 1;
    }
    bindex -= e2fs->nr_dind_blocks;

    // Triple indirect block
    if (!__READ_BLOCK(SWAB_LE32(inode->block[E2FS_TIND_BLOCK])))
	return 0;
    if (!__READ_BLOCK(SWAB_LE32(blockbuf[bindex / e2fs->nr_dind_blocks])))
	return 0;
    bindex %= e2fs->nr_dind_blocks;
    if (!__READ_BLOCK(SWAB_LE32(blockbuf[bindex / e2fs->nr_ind_blocks])))
	return 0;
    *pblknr = SWAB_LE32(blockbuf[bindex % e2fs->nr_ind_blocks]);
    return 1;
}


// search a single directory block in memory looking for an
// entry with the given name. Return pointer to entry if
// found, NULL if not.
//
static e2fs_dir_entry_t *
search_dir_block(e2fs_desc_t *e2fs, cyg_uint32 *blkbuf,
		 const char *name, int namelen)
{
    e2fs_dir_entry_t *dir;
    cyg_uint16 reclen, len;
    cyg_uint32 offset;

#if DEBUG_E2FS > 0
    diag_dump_buf(blkbuf, e2fs->blocksize);
#endif
    offset = 0;
    while (offset < e2fs->blocksize) {
	dir = (e2fs_dir_entry_t *)((char *)blkbuf + offset);
	reclen = SWAB_LE16(dir->reclen);
	offset += reclen;
	len = dir->namelen;

	// terminate on anything which doesn't make sense
	if (reclen < 8 || (len + 8) > reclen || offset > (e2fs->blocksize + 1))
	    return NULL;

	if (dir->inode && len == namelen && !strncmp(dir->name, name, len))
	    return dir;
    }
    return NULL;
}


// Look in the given directory for an entry with the given name.
// If found, return a pointer to that entry. Return NULL if not
// found.
//
static e2fs_dir_entry_t *
e2fs_dir_lookup(e2fs_desc_t *e2fs, cyg_uint32  dir_ino,
		const char  *name, int         namelen)
{
    e2fs_inode_t inode;
    e2fs_dir_entry_t *dir;
    cyg_uint32 nblocks, last_block_size, i, block_nr, nbytes;

#if DEBUG_E2FS > 0
    diag_printf("%s: looking for %s [%d] in ino[%d]\n",
                __FUNCTION__, name, namelen, dir_ino);
#endif

    if (!e2fs_get_inode(e2fs, dir_ino, &inode)) {
#if DEBUG_E2FS > 0
	diag_printf("%s: e2fs_get_inode [%d] failed\n", __FUNCTION__, dir_ino);
#endif
	return NULL;
    }

    nbytes = SWAB_LE32(inode.size);
    nblocks = (nbytes + e2fs->blocksize - 1) / e2fs->blocksize;

    last_block_size = nbytes % e2fs->blocksize;
    if (last_block_size == 0)
	last_block_size = e2fs->blocksize;

    for (i = 0; i < nblocks; i++) {
	if (!e2fs_inode_block(e2fs, &inode, i, &block_nr))
	    return NULL;

	if (block_nr) {
	    if (!__READ_BLOCK(block_nr))
		return NULL;
	} else
	    memset(blockbuf, 0, e2fs->blocksize);

	dir = search_dir_block(e2fs, blockbuf, name, namelen);

	if (dir != NULL)
	    return dir;
    }
    return NULL;
}

typedef struct ino_info {
    cyg_uint32  ino;
    cyg_uint32  parent_ino;
    cyg_uint8   filetype;
} ino_info_t;

static int e2fs_inode_lookup(e2fs_desc_t *e2fs, cyg_uint32 dir_ino,
			     const char *pathname, ino_info_t *info);

// Starting from the given directory, find the inode number, filetype, and
// parent inode for the file pointed to by the given symbolic link inode.
// If successful, fills out ino_info_t and return true.
//
static int
e2fs_follow_symlink(e2fs_desc_t *e2fs, cyg_uint32 dir_ino, cyg_uint32 sym_ino, ino_info_t *info)
{
#define MAX_SYMLINK_NAME 255
    char symlink[MAX_SYMLINK_NAME+1];
    int  pathlen;
    cyg_uint32 block_nr;
    e2fs_inode_t inode;

    if (!e2fs_get_inode(e2fs, sym_ino, &inode)) {
#if DEBUG_E2FS > 0
	diag_printf("%s: e2fs_get_inode [%d] failed\n", __FUNCTION__, sym_ino);
#endif
	return 0;
    }

    pathlen = SWAB_LE32(inode.size);
    if (pathlen > MAX_SYMLINK_NAME)
	return 0;

    if (inode.blocks) {
	if (!e2fs_inode_block(e2fs, &inode, 0, &block_nr))
	    return 0;
	if (block_nr) {
	    if (!PARTITION_READ(e2fs->part, E2FS_BLOCK_TO_SECTOR(e2fs, block_nr),
				blockbuf, e2fs->blocksize/SECTOR_SIZE))
		return 0;
	    memcpy(symlink, blockbuf, pathlen);
	} else
	    return 0;
    } else {
	// small enough path to fit in inode struct
	memcpy(symlink, (char *)&inode.block[0], pathlen);
    }
    symlink[pathlen] = 0;

    return e2fs_inode_lookup(e2fs, dir_ino, symlink, info);
}


// Starting from the given directory, find the inode number, filetype, and
// parent inode for the given file pathname.
// If successful, fills out ino_info_t and return true.
//
static int
e2fs_inode_lookup(e2fs_desc_t *e2fs, cyg_uint32 dir_ino, const char *pathname, ino_info_t *info)
{
    int len, pathlen;
    const char *p;
    e2fs_dir_entry_t *dir = NULL;
    
    if (!pathname || (pathlen = strlen(pathname)) == 0)
	return 0;

    if (*pathname == '/') {
	if (--pathlen == 0) {
	    info->ino = info->parent_ino = E2FS_ROOT_INO;
	    info->filetype = E2FS_FTYPE_DIR;
	    return 1;
	}
	++pathname;
	dir_ino = E2FS_ROOT_INO;
    }

    while (pathlen) {
	// find next delimiter in path.
	for (p = pathname, len = 0; len < pathlen; len++, p++) {
	    // skip delimiter if found.
	    if (*p == '/') {
		++p;
		--pathlen;
		break;
	    }
	}
	dir = e2fs_dir_lookup(e2fs, dir_ino, pathname, len);
	if (dir == NULL)
	    return 0;

	pathlen -= len;
	pathname = p;

	switch (dir->filetype) {
	  case E2FS_FTYPE_SYMLINK:
	    // follow the symbolic link (this will cause recursion)
	    if (!e2fs_follow_symlink(e2fs, dir_ino, SWAB_LE32(dir->inode), info))
		return 0;
	    if (pathlen == 0)
		return 1;
	    // must be a dir if we want to continue
            if (info->filetype != E2FS_FTYPE_DIR)
		return 0;
	    dir_ino = info->ino;
	    break;

	  case E2FS_FTYPE_DIR:
	    if (pathlen)
		dir_ino = SWAB_LE32(dir->inode);
	    break;

	  case E2FS_FTYPE_REG_FILE:
	    if (pathlen)
		return 0;  // regular file embedded in middle of path
	    break;

	  case E2FS_FTYPE_UNKNOWN:
	  case E2FS_FTYPE_CHRDEV:
	  case E2FS_FTYPE_BLKDEV:
	  case E2FS_FTYPE_FIFO:
	  case E2FS_FTYPE_SOCK:
	  default:
	    return 0;
	}
    }
    info->ino = SWAB_LE32(dir->inode);
    info->parent_ino = dir_ino;
    info->filetype = dir->filetype;
    return 1;
}

struct read_info {
    e2fs_desc_t  e2fs_desc;
    e2fs_inode_t inode;
    cyg_uint32   fsize;
    cyg_uint32   fpos;
};

static void *
e2fs_open(partition_t *p, const char *filepath)
{
    static struct read_info rinfo;
    ino_info_t    ino_info;

    // mount partition
    if (e2fs_mount(p, &rinfo.e2fs_desc) != 0) {
	diag_printf("mount failed.\n");
	return NULL;
    }

    // find file inode
    if (!e2fs_inode_lookup(&rinfo.e2fs_desc, E2FS_ROOT_INO, filepath, &ino_info)) {
	diag_printf("%s: e2fs_inode_lookup failed\n", __FUNCTION__);
	return NULL;
    }

    // read inode
    if (!e2fs_get_inode(&rinfo.e2fs_desc, ino_info.ino, &rinfo.inode)) {
	diag_printf("%s: e2fs_get_inode failed for ino[%d]\n", __FUNCTION__, ino_info.ino);
	return NULL;
    }

    rinfo.fsize = SWAB_LE32(rinfo.inode.size);
    rinfo.fpos  = 0;

    return &rinfo;
}

static int
e2fs_read(void *fp, char *buf, cyg_uint32 nbytes)
{
    struct read_info *info = fp;
    e2fs_desc_t *e2fs;
    cyg_uint32 nread = 0, rem, block_nr, bindex, to_read;

    if ((info->fpos + nbytes) > info->fsize)
	nbytes = info->fsize - info->fpos;

    e2fs = &info->e2fs_desc;

    // see if we need to copy leftover data from last read call
    rem = e2fs->blocksize - (info->fpos % e2fs->blocksize);
    if (rem != e2fs->blocksize) {
	char *p = (char *)blockbuf + e2fs->blocksize - rem;

	if (rem > nbytes)
	    rem = nbytes;

	memcpy(buf, p, rem);

	nread += rem;
	buf  += rem;
	info->fpos += rem;
    }
    
    // now loop through blocks if we're not done
    bindex = info->fpos / e2fs->blocksize;
    while (nread < nbytes) {
	if (!e2fs_inode_block(e2fs, &info->inode, bindex, &block_nr))
	    return -1;

	if (block_nr) {
	    if (!PARTITION_READ(e2fs->part, E2FS_BLOCK_TO_SECTOR(e2fs, block_nr),
				blockbuf, e2fs->blocksize/SECTOR_SIZE))
		return 0;
	} else
	    memset(blockbuf, 0, e2fs->blocksize);
	
	to_read = nbytes - nread;
	if (to_read > e2fs->blocksize)
	    to_read = e2fs->blocksize;

	memcpy(buf, blockbuf, to_read);

	nread += to_read;
	buf += to_read;
	info->fpos += to_read;
	++bindex;
    }

    return nread;
}

#if DEBUG_E2FS > 4
static void dump_sb(struct e2fs_super_block *s)
{
    diag_printf("inode_count: %d\n", SWAB_LE32(s->inodes_count));
    diag_printf("blocks_count: %d\n", SWAB_LE32(s->blocks_count));
    diag_printf("r_blocks_count: %d\n", SWAB_LE32(s->r_blocks_count));
    diag_printf("free_blocks_count: %d\n", SWAB_LE32(s->free_blocks_count));
    diag_printf("free_inodes_count: %d\n", SWAB_LE32(s->free_inodes_count));
    diag_printf("first_data_block: %d\n", SWAB_LE32(s->first_data_block));
    diag_printf("log_block_size: %d\n", SWAB_LE32(s->log_block_size));
    diag_printf("log_frag_size: %d\n", SWAB_LE32(s->log_frag_size));
    diag_printf("blocks_per_group: %d\n", SWAB_LE32(s->blocks_per_group));
    diag_printf("frags_per_group: %d\n", SWAB_LE32(s->frags_per_group));
    diag_printf("inodes_per_group: %d\n", SWAB_LE32(s->inodes_per_group));
    diag_printf("mnt_count: %d\n", SWAB_LE16(s->mnt_count));
    diag_printf("max_mnt_count: %d\n", SWAB_LE16(s->max_mnt_count));
    diag_printf("magic: %d\n", SWAB_LE16(s->magic));
    diag_printf("state: %d\n", SWAB_LE16(s->state));
    diag_printf("errors: %d\n", SWAB_LE16(s->errors));
    diag_printf("minor_rev_level: %d\n", SWAB_LE16(s->minor_rev_level));
    diag_printf("lastcheck: %d\n", SWAB_LE32(s->lastcheck));
    diag_printf("checkinterval: %d\n", SWAB_LE32(s->checkinterval));
    diag_printf("creator_os: %d\n", SWAB_LE32(s->creator_os));
    diag_printf("rev_level: %d\n", SWAB_LE32(s->rev_level));
}

static void dump_inode(struct e2fs_inode *i)
{
    int j, n;

    diag_printf("mode: %o\n", SWAB_LE16(i->mode));
    diag_printf("uid: %o\n", SWAB_LE16(i->uid));
    diag_printf("size: %d\n", SWAB_LE32(i->size));
    diag_printf("gid: %o\n", SWAB_LE16(i->gid));
    diag_printf("links: %d\n", SWAB_LE16(i->links_count));
    diag_printf("blocks: %d\n", SWAB_LE32(i->blocks));

    n = i->blocks;
    if (n > E2FS_N_BLOCKS)
	n = E2FS_N_BLOCKS;

    for (j = 0; j < n; j++)
	diag_printf("  block: %d\n", SWAB_LE32(i->block[j]));
}
#endif


