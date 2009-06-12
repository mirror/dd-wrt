//==========================================================================
//
//      ramfs.c
//
//      RAM file system
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
// Author(s):           nickg
// Contributors:        nickg
// Date:                2000-07-25
// Purpose:             RAM file system
// Description:         This is a RAM filesystem for eCos. It attempts to
//                      provide full POSIX-compatible filesystem behaviour
//                      while at the same time being efficient in terms of
//                      time and space used.
//                      
//
//####DESCRIPTIONEND####
//
//==========================================================================
// 
// General Description
// ===================
// 
// This is an implementation of a RAM filesystem for eCos. Its goal is
// to provide a working example of a filesystem that provides most of
// the required POSIX functionality. And obviously it may also be
// useful in its own right.
//
//
// Nodes
// -----
//
// All files and directories are represented by node objects. Each
// ramfs_node structure contains the following fields:
//
// mode   - Node type, file or directory.
// refcnt - Number of references to this node. For files each open counts as
//          a reference and for directories a node is referenced when it is made
//          current directory, or is opened for reading.
// nlink  - Number of links to this node. Each directory entry that references
//          this node is a link. 
// size   - Size of the data in this node in bytes.
// atime  - Last time this node was accessed.
// mtime  - Last time the data in this node was modified.
// ctime  - Last time the status information in this node was changed.
//
// The data storage in a node is controlled by the configuration and
// can take two forms. These will be described later.
//          
// Directories
// -----------
//
// A directory is a node whose data is a list of directory entries. To
// simplify management of these, long directory entries are split into
// a chain of fixed size ramfs_dirent structures. These contain the
// following fields:
//
// node    - Pointer to node referenced by this entry. This is present in
//           every directory entry fragment
// inuse   - Set to 1 if this entry is in use, zero if it is free.
// first   - Set to 1 if this is the first fragment of a directory entry.
// last    - Set to 1 if this is the last fragment of a directory entry.
// namelen - The size of the whole file name.
// fraglen - The number of bytes of the file name that are stored in this
//           fragment.
// next    - The offset of the next fragment of this directory entry.
// name    - The characters of the fragment of the file name stored in this
//           entry.
//
// Small file names are stored in a single fragment. Longer names are
// stored in a chain of fragments.
//
// Data Storage
// ------------
//
// Two data storage mechanisms may be configured, the SIMPLE and the
// BLOCKS mechanisms.
//
// SIMPLE Data Storage
// ~~~~~~~~~~~~~~~~~~~
//
// This mechanism simply uses malloc() and free() to allocate the
// memory for both nodes and file data. File data is stored in a
// single malloced vector that is realloced as necessary to acquire
// more space. 
//
// The advantage of this approach is that the RAM filesystem only uses
// as much memory as it needs, the rest is available for use by other
// components. It also requires much simpler data structures and code
// in the filesystem to manage. However, if any files get to be a
// significant proportion of the size of the heap, there is the danger
// that fragmentation will prevent any further extension of some
// files, even if there is enough memory in total. It also requires an
// implementation of malloc() to be present. If this needs to be
// present for other components,then this is not a significant
// overhead, but including it just for use by this filesystem
// represents a major addition of code and data structures.
//
//
// BLOCKS Data Storage
// ~~~~~~~~~~~~~~~~~~~
//
// This mechanism divides the memory used for file storage into fixed
// sized blocks. These blocks may either be allocated using
// malloc()/free(), or may be obtained from a array of blocks reserved
// for the purpose. Configuration allows the block size to be
// selected, as well as the allocation mechanism, and in the case of a
// block array, whether it is defined here or by an external
// component.
// 
// Data storage in nodes is arranges in three arrays of pointers to
// blocks. The first array points directly to data blocks, the second
// to blocks which themselves contain pointers to data blocks, and the
// third to blocks which contain pointers to blocks which contain
// pointers to data blocks. In the default configuration These last
// two arrays have only one element each.
// 
// The following shows how the data is arranged in a fully populated
// file with a 256 byte block size using the default configuration.
// 
//      Node
// ~            ~
// |            |
// |            |
// +------------+
// |     *------+--------> data block 0
// +------------+
// |     *------+--------> data block 1
// +------------+
// |     *------+--------> data block 2
// +------------+
// |     *------+--------> data block 3
// +------------+
// |     *------+--------> data block 4
// +------------+
// |     *------+--------> data block 5
// +------------+
// |     *------+--------> data block 6
// +------------+
// |     *------+--------> data block 7
// +------------+
// |     *------+--------> +------------+
// +------------+          |     *------+--------> data block 8
// |     *------+----+     +------------+
// +------------+    |     |            |
//                   |     ~            ~
//                   |     |            |
//                   |     +------------+
//                   |     |     *------+--------> data block 71
//                   |     +------------+
//                   |     
//                   +---->+------------+         +------------+
//                         |     *------+-------->|     *------+---->data block 72
//                         +------------+         +------------+
//                         |            |         |            |
//                         ~            ~         ~            ~
//                         |            |         |            |
//                         +------------+         +------------+
//                         |     *------+---+     |     *------+----> data block 135
//                         +------------+   |     +------------+
//                                          |
//                                          |     +------------+
//                                          +---->|     *------+----> data block 4104
//                                                +------------+
//                                                |            |
//                                                ~            ~
//                                                |            |
//                                                +------------+
//                                                |     *------+----> data block 4167
//                                                +------------+
// 
// 
//
// The advantages of this approach are that, first, memory usage is
// divided into discreet fixed size blocks which are easier to
// manage. When using malloc() to allocate them, they will fit into
// any free memory of at least the right size. Using the block array
// option removes the need to have a malloc() implementation at all.
//
// The disadvantages of this mechanism are that, first, when using
// malloc() to allocate blocks, the per-block memory allocator
// overhead is paid for each block, rather than per file. This may
// result in less memory overall being available for data
// storage. When using the block array, it is permanently reserved for
// use by the ram filesystem, and is not available for use by other
// components.
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/fs_ram.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <stdlib.h>
#include <string.h>

#include <cyg/fileio/fileio.h>

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

//==========================================================================
// Sizes derived from configuration

// -------------------------------------------------------------------------
// Simple malloc based allocator parameters

#ifdef CYGPKG_FS_RAM_SIMPLE

#define RAMFS_FILESIZE_MAX      UINT_MAX

#else

// -------------------------------------------------------------------------
// Block allocator parameters

// The number of nodes per block
#define RAMFS_NODES_PER_BLOCK (CYGNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_node))

// The number of indirect pointers that can be stored in a single data block
#define RAMFS_INDIRECT_PER_BLOCK (CYGNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_block *))

// The number of directory entries that can be stored in a single data block
#define RAMFS_DIRENT_PER_BLOCK  (CYGNUM_RAMFS_BLOCK_SIZE/sizeof(ramfs_dirent))

// Number of bytes contained in a one level indirect block
#define RAMFS_INDIRECT1_BLOCK_EXTENT (RAMFS_INDIRECT_PER_BLOCK* \
                                      CYGNUM_RAMFS_BLOCK_SIZE)

// number of bytes contained in a two level indirect block
#define RAMFS_INDIRECT2_BLOCK_EXTENT (RAMFS_INDIRECT_PER_BLOCK* \
                                      RAMFS_INDIRECT_PER_BLOCK* \
                                      CYGNUM_RAMFS_BLOCK_SIZE)

// The maximum data offset for data directly accessed from the node
#define RAMFS_DIRECT_MAX        (CYGNUM_RAMFS_BLOCKS_DIRECT*CYGNUM_RAMFS_BLOCK_SIZE)

// The maximum data offset for data accessed from the single level indirect blocks
#define RAMFS_INDIRECT1_MAX     (RAMFS_DIRECT_MAX+                      \
                                 (CYGNUM_RAMFS_BLOCKS_INDIRECT1*        \
                                  RAMFS_INDIRECT1_BLOCK_EXTENT))

// The maximum data offset for data accessed from the two level indirect blocks
#define RAMFS_INDIRECT2_MAX     (RAMFS_INDIRECT1_MAX+                   \
                                 (CYGNUM_RAMFS_BLOCKS_INDIRECT2*        \
                                  RAMFS_INDIRECT2_BLOCK_EXTENT))

// The maximum size of a file
#define RAMFS_FILESIZE_MAX      RAMFS_INDIRECT2_MAX

#endif

//==========================================================================
// Forward definitions

// Filesystem operations
static int ramfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte );
static int ramfs_umount   ( cyg_mtab_entry *mte );
static int ramfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int mode,  cyg_file *fte );
static int ramfs_unlink   ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int ramfs_mkdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int ramfs_rmdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int ramfs_rename   ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2 );
static int ramfs_link     ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2, int type );
static int ramfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_file *fte );
static int ramfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_dir *dir_out );
static int ramfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             struct stat *buf);
static int ramfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );
static int ramfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );

// File operations
static int ramfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int ramfs_fo_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int ramfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int ramfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data);
static int ramfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode );        
static int ramfs_fo_close     (struct CYG_FILE_TAG *fp);
static int ramfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf );
static int ramfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );
static int ramfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );

// Directory operations
static int ramfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int ramfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );


//==========================================================================
// Filesystem table entries

// -------------------------------------------------------------------------
// Fstab entry.
// This defines the entry in the filesystem table.
// For simplicity we use _FILESYSTEM synchronization for all accesses since
// we should never block in any filesystem operations.

FSTAB_ENTRY( ramfs_fste, "ramfs", 0,
             CYG_SYNCMODE_FILE_FILESYSTEM|CYG_SYNCMODE_IO_FILESYSTEM,
             ramfs_mount,
             ramfs_umount,
             ramfs_open,
             ramfs_unlink,
             ramfs_mkdir,
             ramfs_rmdir,
             ramfs_rename,
             ramfs_link,
             ramfs_opendir,
             ramfs_chdir,
             ramfs_stat,
             ramfs_getinfo,
             ramfs_setinfo);

// -------------------------------------------------------------------------
// File operations.
// This set of file operations are used for normal open files.

static cyg_fileops ramfs_fileops =
{
    ramfs_fo_read,
    ramfs_fo_write,
    ramfs_fo_lseek,
    ramfs_fo_ioctl,
    cyg_fileio_seltrue,
    ramfs_fo_fsync,
    ramfs_fo_close,
    ramfs_fo_fstat,
    ramfs_fo_getinfo,
    ramfs_fo_setinfo
};

// -------------------------------------------------------------------------
// Directory file operations.
// This set of operations are used for open directories. Most entries
// point to error-returning stub functions. Only the read, lseek and
// close entries are functional.

static cyg_fileops ramfs_dirops =
{
    ramfs_fo_dirread,
    (cyg_fileop_write *)cyg_fileio_enosys,
    ramfs_fo_dirlseek,
    (cyg_fileop_ioctl *)cyg_fileio_enosys,
    cyg_fileio_seltrue,
    (cyg_fileop_fsync *)cyg_fileio_enosys,
    ramfs_fo_close,
    (cyg_fileop_fstat *)cyg_fileio_enosys,
    (cyg_fileop_getinfo *)cyg_fileio_enosys,
    (cyg_fileop_setinfo *)cyg_fileio_enosys
};

//==========================================================================
// Data typedefs
// Some forward typedefs for the main data structures.

struct ramfs_node;
typedef struct ramfs_node ramfs_node;

struct ramfs_dirent;
typedef struct ramfs_dirent ramfs_dirent;

#ifndef CYGPKG_FS_RAM_SIMPLE

typedef cyg_uint8 ramfs_block[CYGNUM_RAMFS_BLOCK_SIZE];

#endif

//==========================================================================
// File and directory node
// This data structure represents a file or directory.

struct ramfs_node
{
    mode_t              mode;           // node type
    cyg_ucount32        refcnt;         // open file/current dir references
    nlink_t             nlink;          // number of links to this node
    size_t              size;           // size of file in bytes
    time_t              atime;          // last access time
    time_t              mtime;          // last modified time
    time_t              ctime;          // last changed status time

#ifdef CYGPKG_FS_RAM_SIMPLE

    // The data storage in this case consists of a single
    // malloced memory block, together with its size.
    
    size_t              datasize;       // size of data block
    cyg_uint8           *data;          // malloced data buffer

#else

    // The data storage in this case consists of arrays of pointers
    // to data blocks. 
    
#if CYGNUM_RAMFS_BLOCKS_DIRECT > 0
    // Directly accessible blocks from the inode.
    ramfs_block         *direct[CYGNUM_RAMFS_BLOCKS_DIRECT];
#endif
#if  CYGNUM_RAMFS_BLOCKS_INDIRECT1 > 0
    // Single level indirection
    ramfs_block         **indirect1[CYGNUM_RAMFS_BLOCKS_INDIRECT1];
#endif
#if  CYGNUM_RAMFS_BLOCKS_INDIRECT2 > 0
    // Two level indirection
    ramfs_block         ***indirect2[CYGNUM_RAMFS_BLOCKS_INDIRECT2];
#endif

#endif
    
};

//==========================================================================
// Directory entry.
// Fixed sized entry containing a fragment of the name of a file/directory.

struct ramfs_dirent
{
    ramfs_node          *node;          // pointer to node
    unsigned int        inuse:1,        // entry in use?
                        first:1,        // first directory entry fragment?
                        last:1,         // last directory entry fragment?
                        namelen:8,      // bytes in whole name
                        fraglen:8;      // bytes in name fragment
    off_t               next;           // offset of next dirent

    // Name fragment, fills rest of entry.
    char                name[CYGNUM_RAMFS_DIRENT_SIZE-
                             sizeof(ramfs_node *)-
                             sizeof( cyg_uint32)-
                             sizeof(off_t)];
};

//==========================================================================
// Directory search data
// Parameters for a directory search. The fields of this structure are
// updated as we follow a pathname through the directory tree.

struct ramfs_dirsearch
{
    ramfs_node          *dir;           // directory to search
    const char          *path;          // path to follow
    ramfs_node          *node;          // Node found
    const char          *name;          // last name fragment used
    int                 namelen;        // name fragment length
    cyg_bool            last;           // last name in path?
};

typedef struct ramfs_dirsearch ramfs_dirsearch;

//==========================================================================
// Forward defs

static int del_direntry( ramfs_node *dir, const char *name, int namelen );


//==========================================================================
// Block array
// This is used for block allocation when malloc is not being used.

#ifdef CYGPKG_FS_RAM_BLOCKS_ARRAY

# ifdef CYGPKG_FS_RAM_BLOCKS_ARRAY_EXTERN

// Array is defined externally with a user-supplied name

__externC ramfs_block CYGPKG_FS_RAM_BLOCKS_ARRAY_NAME[CYGNUM_FS_RAM_BLOCKS_ARRAY_SIZE];

// Translate into a usable common name
#define ramfs_block_array CYGPKG_FS_RAM_BLOCKS_ARRAY_NAME

# else

// Array is defined here

static ramfs_block cyg_ramfs_block_array[CYGNUM_FS_RAM_BLOCKS_ARRAY_SIZE];

#define ramfs_block_array cyg_ramfs_block_array

# endif

// Pointer to list of free blocks
static ramfs_block *block_free_list = NULL;

#endif

//==========================================================================
// Block allocation

#ifndef CYGPKG_FS_RAM_SIMPLE

// -------------------------------------------------------------------------
// block_init()
// Initialize the block allocator by chaining them all together on
// block_free_list.

#ifdef CYGPKG_FS_RAM_BLOCKS_ARRAY

static void block_init(void)
{
    static cyg_bool initialized = false;
    int i;

    if( !initialized )
    {
        for( i = 0; i < CYGNUM_FS_RAM_BLOCKS_ARRAY_SIZE; i++ )
        {
            ramfs_block *b = &ramfs_block_array[i];
            *(ramfs_block **)b = block_free_list;
            block_free_list = b;
        }
        initialized = true;
    }
}

#endif

// -------------------------------------------------------------------------
// block_alloc()
// Allocate a block for data storage.
// If we have a block array, just pick the first off the free list.
// If we are mallocing, call malloc() to get it.

static ramfs_block *block_alloc(void)
{
    ramfs_block *b;

#ifdef CYGPKG_FS_RAM_BLOCKS_ARRAY

    block_init();       // Check blocks are initialized

    // pick first block off free list.
    b = block_free_list;

    // and advance list
    if( b != NULL )
        block_free_list = *(ramfs_block **)b;

#else

    b = malloc(CYGNUM_RAMFS_BLOCK_SIZE);

#endif

    // Clear the block to zero if it was allocated
    if( b != NULL )
        memset( b, 0, CYGNUM_RAMFS_BLOCK_SIZE );

    return b;

}

// -------------------------------------------------------------------------
// block_free()
// Free a block. Depending on the configuration send it back to the
// heap or put it back on free list.

static void block_free( ramfs_block *b )
{
#ifdef CYGPKG_FS_RAM_BLOCKS_ARRAY

    // Put the block back on the free list
    
    *(ramfs_block **)b = block_free_list;
    block_free_list = b;
    
#else    

    // Call free() to return it to the memory pool
    
    free( b );

#endif
}

#endif

//==========================================================================
// Node buffer management
// There are two versions of this, one for the _SIMPLE variant and one for
// the _BLOCKS variant. In both cases the interface to this code is via the
// findbuffer_node() and freebuffer_node() functions.

#ifdef CYGPKG_FS_RAM_SIMPLE

//==========================================================================
// SIMPLE buffer management.
// Each node has a data buffer pointer and a size. This buffer is
// realloc()ed as needed.

// -------------------------------------------------------------------------
// findbuffer_node()
// return a pointer to the data at the indicated file position, extending
// the buffer if required.

static int findbuffer_node( ramfs_node  *node,  // node pointer
                            off_t pos,          // data position to get
                            cyg_uint8 **buffer, // returned buffer pointer
                            size_t *size,       // returned buffer size
                            cyg_bool alloc)     // extend allocation?
{
    if( alloc && (pos == node->datasize || node->datasize == 0) )
    {
        // If we are allowed to alloc new data, and we are at the end of the
        // current data allocation, or there is no data present, allocate or
        // extend the data buffer.
        
        cyg_uint8 *newdata;
        
        if( node->data == NULL )
            newdata = malloc( CYGNUM_RAMFS_REALLOC_INCREMENT );
        else
            newdata = realloc( node->data, pos+CYGNUM_RAMFS_REALLOC_INCREMENT );
        
        if( newdata == NULL ) return ENOSPC;
        else memset( newdata+pos, 0, CYGNUM_RAMFS_REALLOC_INCREMENT );
        
        node->data = newdata;
        node->datasize = pos+CYGNUM_RAMFS_REALLOC_INCREMENT;
    }
    else if( pos > node->datasize )
    {
        // Indicate end of data.
        *size = 0;
        return ENOERR;
    }

    *buffer = node->data+pos;
    *size = node->datasize-pos;

    return ENOERR;
}

// -------------------------------------------------------------------------
// freebuffer_node()
// Empty out the data storage from the node.

static int freebuffer_node( ramfs_node *node )
{
    if( node->data != NULL )
    {
        free( node->data );
    }

    node->data = NULL;
    node->datasize = 0;

    return ENOERR;
}

//==========================================================================

#else

//==========================================================================
// _BLOCKS storage management.
// Data storage in the node is by means of a set of arrays of pointers to
// blocks. The first array points directly to the data blocks. Subsequent
// arrays point to single and double indirect blocks respectively. 

// -------------------------------------------------------------------------
// findbuffer_direct()
// Indexes into an array of block pointers and extracts a pointer to the
// data at offset _pos_, allocating new blocks if required.

static int findbuffer_direct( off_t pos,
                              ramfs_block **blocks,
                              int nblocks,
                              cyg_uint8 **buffer,
                              size_t *size,
                              cyg_bool alloc)
{
    int bi = pos / CYGNUM_RAMFS_BLOCK_SIZE;
    int bpos = pos % CYGNUM_RAMFS_BLOCK_SIZE;
    ramfs_block *b;
    
    *buffer = NULL;
    *size = 0;
    
    if( bi >= nblocks )
        return ENOERR;

    b = blocks[bi];

    if( b == NULL )
    {
        // There is no block there. If _alloc_ is true we can fill the
        // slot in with a new block. If it is false, we indicate end of
        // data with a zero size result.
        if( alloc )
        {
            b = block_alloc();
            if( b == NULL )
                return ENOSPC;
            blocks[bi] = b;
        }
        else return ENOERR;
    }

    *buffer = &((*b)[bpos]);
    *size = CYGNUM_RAMFS_BLOCK_SIZE - bpos;

    return ENOERR;
}

// -------------------------------------------------------------------------
// findbuffer_indirect1()
// Indexes into an array of pointers to blocks containing pointers to
// blocks and extracts a pointer to the data at offset _pos_,
// allocating new blocks if required.

#if CYGNUM_RAMFS_BLOCKS_INDIRECT1 > 0

static int findbuffer_indirect1( off_t pos,
                                 ramfs_block ***blocks,
                                 int nblocks,
                                 cyg_uint8 **buffer,
                                 size_t *size,
                                 cyg_bool alloc)
{

    int bi = pos / RAMFS_INDIRECT1_BLOCK_EXTENT;
    int bpos = pos % RAMFS_INDIRECT1_BLOCK_EXTENT;
    int err;
    cyg_uint8 *b;
    size_t sz;

    // Use findbuffer_direct() to index and allocate
    // the first level indirect block.
    
    err = findbuffer_direct( bi*CYGNUM_RAMFS_BLOCK_SIZE,
                             (ramfs_block **)blocks,
                             nblocks,
                             &b,
                             &sz,
                             alloc);

    if( err != ENOERR )
        return err;

    if( sz == 0 )
    {
        *size = 0;
        return ENOERR;
    }

    // Use findbuffer_direct() on the first level indirect
    // block to allocate and return the data pointer.
    
    return findbuffer_direct( bpos,
                              blocks[bi],
                              RAMFS_INDIRECT_PER_BLOCK,
                              buffer,
                              size,
                              alloc);
}

#endif

// -------------------------------------------------------------------------
// findbuffer_indirect1()
// Indexes into an array of pointers to blocks containing pointers to
// blocks containing pointers to blocks (!) and extracts a pointer to
// the data at offset _pos_, allocating new blocks if required.

#if CYGNUM_RAMFS_BLOCKS_INDIRECT2 > 0

static int findbuffer_indirect2( off_t pos,
                                 ramfs_block ****blocks,
                                 int nblocks,
                                 cyg_uint8 **buffer,
                                 size_t *size,
                                 cyg_bool alloc)
{
    int bi = pos / RAMFS_INDIRECT2_BLOCK_EXTENT;
    int bpos = pos % RAMFS_INDIRECT2_BLOCK_EXTENT;
    int err;
    cyg_uint8 *b;
    size_t sz;

    // Use findbuffer_direct() to index and allocate
    // the first level indirect block.

    err = findbuffer_direct( bi*CYGNUM_RAMFS_BLOCK_SIZE,
                             (ramfs_block **)blocks,
                             nblocks,
                             &b,
                             &sz,
                             alloc);

    if( err != ENOERR )
        return err;

    if( sz == 0 )
    {
        *size = 0;
        return ENOERR;
    }

    // Use findbuffer_indirect1() on the first level indirect block to
    // index and allocate the next level indirect block and the data
    // block.
    
    return findbuffer_indirect1( bpos,
                                 blocks[bi],
                                 RAMFS_INDIRECT_PER_BLOCK,
                                 buffer,
                                 size,
                                 alloc);
}

#endif

// -------------------------------------------------------------------------
// findbuffer_node()
// Depending on the offset and configuration, call the appropriate
// function to get the buffer pointer.

static int findbuffer_node( ramfs_node  *node,
                            off_t pos,
                            cyg_uint8 **buffer,
                            size_t *size,
                            cyg_bool alloc)
{
#if CYGNUM_RAMFS_BLOCKS_DIRECT > 0    
    if( pos < RAMFS_DIRECT_MAX )
        return findbuffer_direct( pos,
                                  node->direct,
                                  CYGNUM_RAMFS_BLOCKS_DIRECT,
                                  buffer,
                                  size,
                                  alloc);
#endif        
#if CYGNUM_RAMFS_BLOCKS_INDIRECT1 > 0    
    if( pos < RAMFS_INDIRECT1_MAX )
        return findbuffer_indirect1( pos - RAMFS_DIRECT_MAX,
                                     node->indirect1,
                                     CYGNUM_RAMFS_BLOCKS_INDIRECT1,
                                     buffer,
                                     size,
                                     alloc);
#endif        
#if CYGNUM_RAMFS_BLOCKS_INDIRECT2 > 0    
    if( pos < RAMFS_INDIRECT2_MAX )
        return findbuffer_indirect2( pos - RAMFS_INDIRECT1_MAX,
                                     node->indirect2,
                                     CYGNUM_RAMFS_BLOCKS_INDIRECT2,
                                     buffer,
                                     size,
                                     alloc);
#endif

    return ENOSPC;
}

// -------------------------------------------------------------------------
// freeblock_list()
// Free a list of data blocks.

static void freeblock_list( ramfs_block *blocks[],int nblocks )
{
    int i;
    for( i = 0; i < nblocks ; i++ )
    {
        if( blocks[i] != NULL )
        {
            block_free( blocks[i] );
            blocks[i] = NULL;
        }
    }
}

// -------------------------------------------------------------------------
// freebuffer_node()
// Free all the data blocks in the node and clear the pointers.

static int freebuffer_node( ramfs_node *node )
{
#if CYGNUM_RAMFS_BLOCKS_DIRECT > 0
    freeblock_list( node->direct, CYGNUM_RAMFS_BLOCKS_DIRECT );
#endif

#if CYGNUM_RAMFS_BLOCKS_INDIRECT1 > 0
    {
        int i;
        for( i = 0; i < CYGNUM_RAMFS_BLOCKS_INDIRECT1 ; i++ )
        {
            if( node->indirect1[i] != NULL )
            {
                freeblock_list( (ramfs_block **)node->indirect1[i], RAMFS_INDIRECT_PER_BLOCK );
                block_free( (ramfs_block *)node->indirect1[i] );
                node->indirect1[i] = NULL;
            }
        }
    }
#endif    

#if CYGNUM_RAMFS_BLOCKS_INDIRECT2 > 0
    {
        int i;
        for( i = 0; i < CYGNUM_RAMFS_BLOCKS_INDIRECT2 ; i++ )
        {
            if( node->indirect2[i] != NULL )
            {
                ramfs_block ***b = node->indirect2[i];
                int j;
                for( j = 0; j < RAMFS_INDIRECT_PER_BLOCK ; j++ )
                {
                    if( b[j] != NULL )
                    {
                        freeblock_list( (ramfs_block **)b[j], RAMFS_INDIRECT_PER_BLOCK );
                        block_free( (ramfs_block *)b[j] );
                        b[j] = NULL;
                    }
                }
                block_free( (ramfs_block *)node->indirect2[i] );
                node->indirect2[i] = NULL;
            }
        }
    }
#endif

    return ENOERR;
}

//==========================================================================

#endif

//==========================================================================
// Node allocation

// -------------------------------------------------------------------------
// alloc_node()
// Allocate a node and initialize it.
// For the _SIMPLE allocation option, we just malloc it. For the
// _BLOCKS option we allocate a block and use that. In theory we could
// pack several nodes into a single block, but we don't at present due
// to sheer lazyness.

static ramfs_node *alloc_node( mode_t mode )
{
#ifdef CYGPKG_FS_RAM_SIMPLE
    ramfs_node *node = malloc( sizeof( ramfs_node ) );

    if( node == NULL )
        return NULL;
    
#else
    ramfs_block *b = block_alloc();
    ramfs_node *node;

    if( b == NULL )
        return NULL;

    node = (ramfs_node *)b;
    
#endif

    memset( node, 0, sizeof(ramfs_node) );
    
    node->mode          = mode;
    node->refcnt        = 0;
    node->nlink         = 0;
    node->size          = 0;
    node->atime         = 
    node->mtime         = 
    node->ctime         = cyg_timestamp();

#ifdef CYGPKG_FS_RAM_SIMPLE    
    node->datasize      = 0;
    node->data          = NULL;
#else

    // The node is already all zero
    
#endif    
    return node;
}

// -------------------------------------------------------------------------
// free_node()
// Release a node either back to the free pool or back into the block
// pool.

static void free_node( ramfs_node *node )
{
#ifdef CYGPKG_FS_RAM_SIMPLE    

    free( node );

#else

    block_free( (ramfs_block *)node );

#endif    
    
}


//==========================================================================
// Ref count and nlink management

// -------------------------------------------------------------------------
// dec_refcnt()
// Decrment the reference count on a node. If this makes the ref count
// zero, and the number of links is either zero for a file or one for
// a node, then this node is detached from the directory tree and can
// be freed.

static int dec_refcnt( ramfs_node *node )
{
    int err = ENOERR;
    node->refcnt--;

    if( node->refcnt == 0 &&
        ((S_ISREG(node->mode) && node->nlink == 0 ) ||
         (S_ISDIR(node->mode) && node->nlink == 1) )
        )
    {
        // This node it now totally detached from the directory tree,
        // so delete it.

        if( S_ISDIR(node->mode) )
        {
            del_direntry( node, ".", 1 );
            del_direntry( node, "..", 2 );
        }
        
        err = freebuffer_node( node );

        if( err == ENOERR )
            free_node( node );
    }

    return err;
}

// -------------------------------------------------------------------------
// dec_nlink()
// Decrement a node's link count. Since this has to do all the same
// work as dec_refcnt() we implement this using that function by
// essentially transferring the count to refcnt and then decrement
// that.

static int dec_nlink( ramfs_node *node )
{
    node->refcnt++;
    
    node->nlink--;

    return dec_refcnt( node );
}

//==========================================================================
// Directory operations

// -------------------------------------------------------------------------
// add_direntry()
// Add an entry to a directory. This is added as a chain of entry
// fragments until the name is exhausted.

static int add_direntry( ramfs_node *dir,       // dir to add to
                         const char *name,      // name to add
                         int namelen,           // length of name
                         ramfs_node *node       // node to reference
                       )
{
    off_t pos = 0;
    ramfs_dirent *d = NULL, *dp = NULL;
    cyg_bool isfirst = true;

    // Loop inserting fragments of the name into the directory until we
    // have found a home for them all.
    
    while( namelen > 0 )
    {
        int fraglen = namelen;

        if( fraglen > sizeof(d->name) )
            fraglen = sizeof(d->name);

        // Find a free fragment
        for(;;)
        {
            cyg_uint8 *buf;
            size_t size;
            int err = findbuffer_node( dir, pos, &buf, &size, true );
            if( err != ENOERR ) return err;

            d = (ramfs_dirent *)buf;

            if( size < sizeof(ramfs_dirent) || d->inuse )
            {
                pos += sizeof(ramfs_dirent);
                continue;
            }

            break;
        }

        // d now points to a free dirent structure

        d->node         = node;
        d->inuse        = 1;
        d->first        = isfirst;
        d->namelen      = namelen;
        d->fraglen      = fraglen;
        if( dp ) dp->next = pos;

        memcpy( d->name, name, fraglen );

        name            += fraglen;
        namelen         -= fraglen;
        pos             += sizeof(ramfs_dirent);
        dp              = d;
        isfirst         = false;
        
    }

    
    d->last = 1;        // Mark last fragment
    
    // Update directory times
    dir->mtime =
    dir->ctime = cyg_timestamp();
    
    // Extend dir size if necessary
    if( pos > dir->size )
        dir->size = pos;
    
    // Count the new link
    node->nlink++;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// find_direntry()
// Find a directory entry for the name and return a pointer to the first
// entry fragment.

static ramfs_dirent *find_direntry( ramfs_node *dir, const char *name, int namelen )
{
    ramfs_dirent *first = NULL;
    off_t pos = 0;
    int err;

    // Loop over all the entries until a match is found or we run out
    // of data.
    while( pos < dir->size )
    {
        const char *frag = name;
        ramfs_dirent *d;
        cyg_uint8 *buf;
        size_t size;
        
        // look for a first name fragment
        for(;;)
        {
            err = findbuffer_node( dir, pos, &buf, &size, false );
            if( err != ENOERR || size == 0)
                return NULL;

            d = (ramfs_dirent *)buf;

            if( size < sizeof(ramfs_dirent) || !d->inuse || !d->first )
            {
                pos += sizeof(ramfs_dirent);
                continue;
            }

            break;
        }

        // Here we have got a first fragment of a name, check it
        // against the name we are looking for. First check that they
        // are the same length.

        if( d->namelen == namelen )
        {
            // We have a potential candidate here...
            
            first = d;      // Save it for later

            // Now check that all the name fragments match
            for(;;)
            {
                int fraglen = namelen-(frag-name);

                if( fraglen > d->fraglen )
                    fraglen = d->fraglen;

                // compare strings, if different, look for another
                if( memcmp( frag, d->name, fraglen ) != 0 )
                    break;

                frag        += fraglen;
            
                // If we are at the last fragment, then the whole name string
                // has matched and we have a successful search.
                
                if( d->last )
                        return first;

                // Otherwise move on to next entry in chain
                err = findbuffer_node( dir, d->next, &buf, &size, false );
                if( err != ENOERR )
                    return NULL;

                d = (ramfs_dirent *)buf;

            }
        }

        pos += sizeof(ramfs_dirent);        
    }

    return NULL;
}

// -------------------------------------------------------------------------
// del_direntry()
// Delete a named directory entry. Find it and then follow the chain
// deleting the fragments as we go.

static int del_direntry( ramfs_node *dir, const char *name, int namelen )
{
    ramfs_dirent *d = find_direntry( dir, name, namelen );
    
    if( d == NULL )
        return ENOENT;

    for(;;)
    {
        int err;
        cyg_uint8 *buf;
        size_t size;

        d->inuse = 0;
        if( d->last ) break;

        err = findbuffer_node( dir, d->next, &buf, &size, false );
        if( err != ENOERR )
            return ENOENT;

        d = (ramfs_dirent *)buf;
    }

    dec_nlink( d->node );
    
    return ENOERR;
}

//==========================================================================
// Directory search

// -------------------------------------------------------------------------
// init_dirsearch()
// Initialize a dirsearch object to start a search

static void init_dirsearch( ramfs_dirsearch *ds,
                            ramfs_node *dir,
                            const char *name)
{
    ds->dir      = dir;
    ds->path     = name;
    ds->node     = dir;
    ds->name     = name;
    ds->namelen  = 0;
    ds->last     = false;    
}

// -------------------------------------------------------------------------
// find_entry()
// Search a single directory for the next name in a path and update the
// dirsearch object appropriately.

static int find_entry( ramfs_dirsearch *ds )
{
    ramfs_node *dir = ds->dir;
    const char *name = ds->path;
    const char *n = name;
    char namelen = 0;
    ramfs_dirent *d;
    
    // check that we really have a directory
    if( !S_ISDIR(dir->mode) )
        return ENOTDIR;

    // Isolate the next element of the path name. 
    while( *n != '\0' && *n != '/' )
        n++, namelen++;

    // Check if this is the last path element.
    while( *n == '/') n++;
    if( *n == '\0' ) 
        ds->last = true;

    // update name in dirsearch object
    ds->name = name;
    ds->namelen = namelen;
    
    // Here we have the name and its length set up.
    // Search the directory for a matching entry

    d = find_direntry( dir, name, namelen );

    if( d == NULL )
        return ENOENT;

    // pass back the node we have found
    ds->node = d->node;

    return ENOERR;

}

// -------------------------------------------------------------------------
// ramfs_find()
// Main interface to directory search code. This is used in all file
// level operations to locate the object named by the pathname.

static int ramfs_find( ramfs_dirsearch *d )
{
    int err;

    // Short circuit empty paths
    if( *(d->path) == '\0' )
        return ENOERR;

    // iterate down directory tree until we find the object
    // we want.
    for(;;)
    {
        err = find_entry( d );

        if( err != ENOERR )
            return err;

        if( d->last )
            return ENOERR;

        // Update dirsearch object to search next directory.
        d->dir = d->node;
        d->path += d->namelen;
        while( *(d->path) == '/' ) d->path++; // skip dirname separators
    }
}

//==========================================================================
// Pathconf support
// This function provides support for pathconf() and fpathconf().

static int ramfs_pathconf( ramfs_node *node, struct cyg_pathconf_info *info )
{
    int err = ENOERR;
    
    switch( info->name )
    {
    case _PC_LINK_MAX:
        info->value = LINK_MAX;
        break;
        
    case _PC_MAX_CANON:
        info->value = -1;       // not supported
        err = EINVAL;
        break;
        
    case _PC_MAX_INPUT:
        info->value = -1;       // not supported
        err = EINVAL;        
        break;
        
    case _PC_NAME_MAX:
        info->value = NAME_MAX;
        break;
        
    case _PC_PATH_MAX:
        info->value = PATH_MAX;
        break;        

    case _PC_PIPE_BUF:
        info->value = -1;       // not supported
        err = EINVAL;        
        break;        

        
    case _PC_ASYNC_IO:
        info->value = -1;       // not supported
        err = EINVAL;        
        break;
        
    case _PC_CHOWN_RESTRICTED:
        info->value = -1;       // not supported
        err = EINVAL;
        break;
        
    case _PC_NO_TRUNC:
        info->value = 0;
        break;
        
    case _PC_PRIO_IO:
        info->value = 0;
        break;
        
    case _PC_SYNC_IO:
        info->value = 0;
        break;
        
    case _PC_VDISABLE:
        info->value = -1;       // not supported
        err = EINVAL;        
        break;
        
    default:
        err = EINVAL;
        break;
    }

    return err;
}

//==========================================================================
// Filesystem operations

// -------------------------------------------------------------------------
// ramfs_mount()
// Process a mount request. This mainly creates a root for the
// filesystem.

static int ramfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte )
{
    ramfs_node *root;
    int err;
    
    // Allocate a node to be the root of this filesystem and initialize it.

    root = alloc_node(__stat_mode_DIR);

    if( root == NULL )
        return ENOSPC;

    // Add . and .. entries back to self.
        
    err = add_direntry( root, ".", 1, root );
    if( err == ENOERR )
        err = add_direntry( root, "..", 2, root );

    if( err != ENOERR )
    {
        free_node( root );
        return err;
    }
    
    mte->root = (cyg_dir)root;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_umount()
// Unmount the filesystem. This will currently only succeed if the
// filesystem is empty.

static int ramfs_umount   ( cyg_mtab_entry *mte )
{
    ramfs_node *root = (ramfs_node *)mte->root;

    // Check for open/inuse root
    if( root->refcnt != 0 )
        return EBUSY;

    // Check that root directory is clear of extra links.
    if( root->nlink != 2 )
        return EBUSY;

    // Just return it to free pool
    free_node( root );

    // Clear root pointer
    mte->root = CYG_DIR_NULL;
    
    // That's all folks.
        
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_open()
// Open a file for reading or writing.

static int ramfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int mode,  cyg_file *file )
{

    ramfs_dirsearch ds;
    ramfs_node *node = NULL;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err == ENOENT )
    {
        if( ds.last && (mode & O_CREAT) )
        {
            // No node there, if the O_CREAT bit is set then we must
            // create a new one. The dir and name fields of the dirsearch
            // object will have been updated so we know where to put it.

            node = alloc_node( __stat_mode_REG );

            if( node == NULL )
                return ENOSPC;

            err = add_direntry( ds.dir, ds.name, ds.namelen, node );

            if( err != ENOERR )
            {
                free_node( node );
                return err;
            }
        
            err = ENOERR;
        }
    }
    else if( err == ENOERR )
    {
        // The node exists. If the O_CREAT and O_EXCL bits are set, we
        // must fail the open.
        
        if( (mode & (O_CREAT|O_EXCL)) == (O_CREAT|O_EXCL) )
            err = EEXIST;
        else node = ds.node;
    }
    
    if( err == ENOERR && (mode & O_TRUNC ) )
    {
        // If the O_TRUNC bit is set we must clean out the file data.

        err = freebuffer_node( node );
        node->size = 0;

        // Update file times
        node->ctime =
        node->mtime = cyg_timestamp();
    }

    if( err != ENOERR ) return err;

    // Check that we actually have a file here
    if( S_ISDIR(node->mode) ) return EISDIR;

    node->refcnt++;       // Count successful open
    
    // Initialize the file object
    
    file->f_flag        |= mode & CYG_FILE_MODE_MASK;
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &ramfs_fileops;
    file->f_offset      = (mode&O_APPEND) ? node->size : 0;
    file->f_data        = (CYG_ADDRWORD)node;
    file->f_xops        = 0;

    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_unlink()
// Remove a file link from its directory.

static int ramfs_unlink   ( cyg_mtab_entry *mte, cyg_dir dir, const char *name )
{
    ramfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err != ENOERR ) return err;

    // Cannot unlink directories, use rmdir() instead
    if( S_ISDIR(ds.node->mode) )
        return EPERM;

    // Delete it from its directory
    err = del_direntry( ds.dir, ds.name, ds.namelen );
    
    return err;
}

// -------------------------------------------------------------------------
// ramfs_mkdir()
// Create a new directory.

static int ramfs_mkdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name )
{
    ramfs_dirsearch ds;
    ramfs_node *node = NULL;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err == ENOENT )
    {
        if( ds.last )
        {
            // The entry does not exist, and it is the last element in
            // the pathname, so we can create it here.
            int doterr, dotdoterr, direrr;
        
            node = alloc_node( __stat_mode_DIR );

            if( node == NULL )
                return ENOSPC;

            // Add "." and ".." entries.
            doterr = add_direntry( node, ".", 1, node );
            dotdoterr = add_direntry( node, "..", 2, ds.dir );

            // And add to parent directory.
            direrr = add_direntry( ds.dir, ds.name, ds.namelen, node );

            // check for any errors in that...
            if( doterr+dotdoterr+direrr != ENOERR )
            {
                // For each of the add_direntry() calls that succeeded,
                // we must now undo it.
                
                if( doterr == ENOERR )
                    del_direntry( node, ".", 1 );
                else err = doterr;

                if( dotdoterr == ENOERR )
                    del_direntry( node, "..", 2 );
                else err = dotdoterr;

                if( direrr == ENOERR )
                    del_direntry( ds.dir, ds.name, ds.namelen );
                else err = direrr;

                // Free the data and the node itself.
                freebuffer_node( node );
                free_node( node );
            }
            else err = ENOERR;
        }
        // If this was not the last element, then and intermediate
        // directory does not exist.
    }
    else
    {
        // If there we no error, something already exists with that
        // name, so we cannot create another one.
        
        if( err == ENOERR )
            err = EEXIST;
    }

    return err;
}

// -------------------------------------------------------------------------
// ramfs_rmdir()
// Remove a directory.

static int ramfs_rmdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name )
{
    ramfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err != ENOERR ) return err;

    // Check that this is actually a directory.
    if( !S_ISDIR(ds.node->mode) )
        return EPERM;

    // Delete the entry. This will adjust the link values
    // accordingly and if the directory is now unreferenced,
    // will cause it to be deleted.
    
    err = del_direntry( ds.dir, ds.name, ds.namelen );
    
    return err;
}

// -------------------------------------------------------------------------
// ramfs_rename()
// Rename a file/dir.

static int ramfs_rename   ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2 )
{
    ramfs_dirsearch ds1, ds2;
    int err;

    init_dirsearch( &ds1, (ramfs_node *)dir1, name1 );
    
    err = ramfs_find( &ds1 );

    if( err != ENOERR ) return err;

    init_dirsearch( &ds2, (ramfs_node *)dir2, name2 );
    
    err = ramfs_find( &ds2 );

    // Allow through renames to non-existent objects.
    if( ds2.last && err == ENOENT )
        ds2.node = NULL, err = ENOERR;
    
    if( err != ENOERR ) return err;

    // Null rename, just return
    if( ds1.node == ds2.node )
        return ENOERR;

    // First deal with any entry that is at the destination
    if( ds2.node )
    {
        // Check that we are renaming like-for-like

        if( !S_ISDIR(ds1.node->mode) && S_ISDIR(ds2.node->mode) )
            return EISDIR;

        if( S_ISDIR(ds1.node->mode) && !S_ISDIR(ds2.node->mode) )
            return ENOTDIR;

        // Now delete the destination directory entry
        
        err = del_direntry( ds2.dir, ds2.name, ds2.namelen );
        
        if( err != ENOERR ) return err;

    }

    // Now we know that there is no clashing node at the destination,
    // make a new direntry at the destination and delete the old entry
    // at the source.

    err = add_direntry( ds2.dir, ds2.name, ds2.namelen, ds1.node );

    if( err == ENOERR )
        err = del_direntry( ds1.dir, ds1.name, ds1.namelen );

    // Update directory times
    if( err == ENOERR )
        ds1.dir->ctime =
        ds1.dir->mtime =
        ds2.dir->ctime =
        ds2.dir->mtime = cyg_timestamp();
            
    return err;
}

// -------------------------------------------------------------------------
// ramfs_link()
// Make a new directory entry for a file.

static int ramfs_link     ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2, int type )
{
    ramfs_dirsearch ds1, ds2;
    int err;

    // Only do hard links for now in this filesystem
    if( type != CYG_FSLINK_HARD )
        return EINVAL;
    
    init_dirsearch( &ds1, (ramfs_node *)dir1, name1 );
    
    err = ramfs_find( &ds1 );

    if( err != ENOERR ) return err;

    init_dirsearch( &ds2, (ramfs_node *)dir2, name2 );
    
    err = ramfs_find( &ds2 );

    // Don't allow links to existing objects
    if( err == ENOERR ) return EEXIST;
    
    // Allow through links to non-existing terminal objects
    if( ds2.last && err == ENOENT )
        ds2.node = NULL, err = ENOERR;

    if( err != ENOERR ) return err;
    
    // Now we know that there is no existing node at the destination,
    // make a new direntry at the destination.

    err = add_direntry( ds2.dir, ds2.name, ds2.namelen, ds1.node );

    if( err == ENOERR )
        ds1.node->ctime =
        ds2.dir->ctime =
        ds2.dir->mtime = cyg_timestamp();
            
    return err;
}

// -------------------------------------------------------------------------
// ramfs_opendir()
// Open a directory for reading.

static int ramfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_file *file )
{
    ramfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err != ENOERR ) return err;

    // check it is really a directory.
    if( !S_ISDIR(ds.node->mode) ) return ENOTDIR;

    ds.node->refcnt++;       // Count successful open
    
    // Initialize the file object, setting the f_ops field to a
    // special set of file ops.
    
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &ramfs_dirops;
    file->f_offset      = 0;
    file->f_data        = (CYG_ADDRWORD)ds.node;
    file->f_xops        = 0;
        
    return ENOERR;

}

// -------------------------------------------------------------------------
// ramfs_chdir()
// Change directory support.

static int ramfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_dir *dir_out )
{
    if( dir_out != NULL )
    {
        // This is a request to get a new directory pointer in
        // *dir_out.

        ramfs_dirsearch ds;
        int err;
    
        init_dirsearch( &ds, (ramfs_node *)dir, name );
    
        err = ramfs_find( &ds );

        if( err != ENOERR ) return err;

        // check it is a directory
        if( !S_ISDIR(ds.node->mode) )
            return ENOTDIR;
        
        // Increment ref count to keep this directory in existent
        // while it is the current cdir.
        ds.node->refcnt++;

        // Pass it out
        *dir_out = (cyg_dir)ds.node;
    }
    else
    {
        // If no output dir is required, this means that the mte and
        // dir arguments are the current cdir setting and we should
        // forget this fact.

        ramfs_node *node = (ramfs_node *)dir;

        // Just decrement directory reference count.
        dec_refcnt( node );
    }
        
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_stat()
// Get struct stat info for named object.

static int ramfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             struct stat *buf)
{
    ramfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err != ENOERR ) return err;

    // Fill in the status
    buf->st_mode        = ds.node->mode;
    buf->st_ino         = (ino_t)ds.node;
    buf->st_dev         = 0;
    buf->st_nlink       = ds.node->nlink;
    buf->st_uid         = 0;
    buf->st_gid         = 0;
    buf->st_size        = ds.node->size;
    buf->st_atime       = ds.node->atime;
    buf->st_mtime       = ds.node->mtime;
    buf->st_ctime       = ds.node->ctime;
    
    return err;
}

// -------------------------------------------------------------------------
// ramfs_getinfo()
// Getinfo. Currently only support pathconf().

static int ramfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len )
{
    ramfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (ramfs_node *)dir, name );
    
    err = ramfs_find( &ds );

    if( err != ENOERR ) return err;

    switch( key )
    {
    case FS_INFO_CONF:
        err = ramfs_pathconf( ds.node, (struct cyg_pathconf_info *)buf );
        break;
        
    default:
        err = EINVAL;
    }
    return err;
}

// -------------------------------------------------------------------------
// ramfs_setinfo()
// Setinfo. Nothing to support here at present.

static int ramfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len )
{
    // No setinfo keys supported at present
    
    return EINVAL;
}


//==========================================================================
// File operations

// -------------------------------------------------------------------------
// ramfs_fo_read()
// Read data from the file.

static int ramfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    ramfs_node *node = (ramfs_node *)fp->f_data;
    int i;
    off_t pos = fp->f_offset;
    ssize_t resid = uio->uio_resid;

    // Loop over the io vectors until there are none left
    for( i = 0; i < uio->uio_iovcnt; i++ )
    {
        cyg_iovec *iov = &uio->uio_iov[i];
        char *buf = (char *)iov->iov_base;
        off_t len = iov->iov_len;

        // Loop over each vector filling it with data from the file.
        while( len > 0 && pos < node->size )
        {
            cyg_uint8 *fbuf;
            size_t bsize;
            off_t l = len;
            int err;

            // Get a pointer to the data at offset _pos_.
            err = findbuffer_node( node, pos, &fbuf, &bsize, false );

            if( err != ENOERR )
                return err;

            // adjust size to end of file if necessary
            if( l > node->size-pos )
                l = node->size-pos;
            
            // adjust size to the amount of contiguous data we can see
            // at present.
            if( l > bsize )
                l = bsize;

            // copy data out
            memcpy( buf, fbuf, l );

            // Update working vars
            len -= l;
            buf += l;
            pos += l;
            resid -= l;
        }
    }

    // We successfully read some data, update the node's access time
    // and update the file offset and transfer residue.
    
    node->atime = cyg_timestamp();

    uio->uio_resid = resid;
    fp->f_offset = pos;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_fo_write()
// Write data to file.

static int ramfs_fo_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    ramfs_node *node = (ramfs_node *)fp->f_data;
    off_t pos = fp->f_offset;
    ssize_t resid = uio->uio_resid;    
    int err = ENOERR;
    int i;

    // If the APPEND mode bit was supplied, force all writes to
    // the end of the file.
    if( fp->f_flag & CYG_FAPPEND )
        pos = fp->f_offset = node->size;
    
    // Check that pos is within current file size, or at the very end.
    if( pos < 0 || pos > node->size )
        return EINVAL;

    // Now loop over the iovecs until they are all done, or
    // we get an error.
    for( i = 0; i < uio->uio_iovcnt; i++ )
    {
        cyg_iovec *iov = &uio->uio_iov[i];
        char *buf = (char *)iov->iov_base;
        off_t len = iov->iov_len;

        // loop over the vector writing it to the file until it has
        // all been done.
        while( len > 0 )
        {
            cyg_uint8 *fbuf;
            size_t bsize;
            off_t l = len;
            
            err = findbuffer_node( node, pos, &fbuf, &bsize, true );

            // Stop writing if there is no more space in the file and
            // indicate end of data.
            if( err == ENOSPC )
                break;
            
            if( err != ENOERR )
                return err;
            
            // adjust size to this block
            if( l > bsize )
                l = bsize;

            // copy data in
            memcpy( fbuf, buf, l );

            // Update working vars
            len -= l;
            buf += l;
            pos += l;
            resid -= l;
        }
    }

    // We wrote some data successfully, update the modified and access
    // times of the node, increase its size appropriately, and update
    // the file offset and transfer residue.
    node->mtime =
    node->ctime = cyg_timestamp();
    if( pos > node->size )
        node->size = pos;    

    uio->uio_resid = resid;
    fp->f_offset = pos;
    
    return err;
}

// -------------------------------------------------------------------------
// ramfs_fo_lseek()
// Seek to a new file position.

static int ramfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *apos, int whence )
{
    ramfs_node *node = (ramfs_node *)fp->f_data;
    off_t pos = *apos;

    switch( whence )
    {
    case SEEK_SET:
        // Pos is already where we want to be.
        break;

    case SEEK_CUR:
        // Add pos to current offset.
        pos += fp->f_offset;
        break;

    case SEEK_END:
        // Add pos to file size.
        pos += node->size;
        break;

    default:
        return EINVAL;
    }
    
    // Check that pos is still within current file size, or at the
    // very end.
    if( pos < 0 || pos > node->size )
        return EINVAL;

    // All OK, set fp offset and return new position.
    *apos = fp->f_offset = pos;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_fo_ioctl()
// Handle ioctls. Currently none are defined.

static int ramfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data)
{
    // No Ioctls currenly defined.

    return EINVAL;
}

// -------------------------------------------------------------------------
// ramfs_fo_fsync().
// Force the file out to data storage.

static int ramfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode )
{
    // Data is always permanently where it belongs, nothing to do
    // here.
  
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_fo_close()
// Close a file. We just decrement the refcnt and let it go away if
// that is all that is keeping it here.

static int ramfs_fo_close     (struct CYG_FILE_TAG *fp)
{
    ramfs_node *node = (ramfs_node *)fp->f_data;

    dec_refcnt( node );

    fp->f_data = 0;     // zero data pointer
    
    return ENOERR;
}

// -------------------------------------------------------------------------
//ramfs_fo_fstat()
// Get file status.

static int ramfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf )
{
    ramfs_node *node = (ramfs_node *)fp->f_data;

    // Fill in the status
    buf->st_mode        = node->mode;
    buf->st_ino         = (ino_t)node;
    buf->st_dev         = 0;
    buf->st_nlink       = node->nlink;
    buf->st_uid         = 0;
    buf->st_gid         = 0;
    buf->st_size        = node->size;
    buf->st_atime       = node->atime;
    buf->st_mtime       = node->mtime;
    buf->st_ctime       = node->ctime;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_fo_getinfo()
// Get info. Currently only supports fpathconf().

static int ramfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    ramfs_node *node = (ramfs_node *)fp->f_data;    
    int err;

    switch( key )
    {
    case FS_INFO_CONF:
        err = ramfs_pathconf( node, (struct cyg_pathconf_info *)buf );
        break;
        
    default:
        err = EINVAL;
    }
    return err;
}

// -------------------------------------------------------------------------
// ramfs_fo_setinfo()
// Set info. Nothing supported here.

static int ramfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    // No setinfo key supported at present
    
    return ENOERR;
}


//==========================================================================
// Directory operations

// -------------------------------------------------------------------------
// ramfs_fo_dirread()
// Read a single directory entry from a file.

static int ramfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    ramfs_node *dir = (ramfs_node *)fp->f_data;
    off_t pos = fp->f_offset;
    int err = ENOERR;
    struct dirent *ent = (struct dirent *)uio->uio_iov[0].iov_base;
    char *nbuf = ent->d_name;
    int nlen = sizeof(ent->d_name)-1;
    off_t len = uio->uio_iov[0].iov_len;
    ramfs_dirent *d = NULL;
    cyg_uint8 *buf;
    size_t size;
        
    if( len < sizeof(struct dirent) )
        return EINVAL;    
    
    // look for a first name fragment

    while( pos < dir->size )
    {
        err = findbuffer_node( dir, pos, &buf, &size, false );
        if( err != ENOERR || size == 0)
            break;

        d = (ramfs_dirent *)buf;

        if( size < sizeof(ramfs_dirent) || !d->inuse || !d->first )
        {
            pos += sizeof(ramfs_dirent);
            continue;
        }

        break;
    }

    // Check we have not exceeded the size of the directory.
    if( pos == dir->size )
        return err;
    
    // Here we  have the first fragment of a directory entry.

    for(;;)
    {
        int fraglen = d->fraglen;

        // adjust to allow for remaining space in dirent struct
        if( fraglen > nlen )
            fraglen = nlen;
        
        memcpy( nbuf, d->name, fraglen);
        nbuf += fraglen;
        nlen -= fraglen;
            
        // if we hit the last entry, we have a successful transfer
        if( d->last || nlen == 0)
            break;

        // Otherwise move on to next entry in chain
        err = findbuffer_node( dir, d->next, &buf, &size, false );
        if( err != ENOERR )
            return err;

        d = (ramfs_dirent *)buf;
    }

    // A successful read. Terminate the entry name with a NUL, set the
    // residue and set the file offset to restart at the next
    // directory entry.
    
    *nbuf = '\0';
    uio->uio_resid -= sizeof(struct dirent);
    fp->f_offset = pos+sizeof(ramfs_dirent);
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// ramfs_fo_dirlseek()
// Seek directory to start.

static int ramfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence )
{
    // Only allow SEEK_SET to zero
    
    if( whence != SEEK_SET || *pos != 0)
        return EINVAL;

    *pos = fp->f_offset = 0;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// EOF ramfs.c
