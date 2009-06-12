//==========================================================================
//
//      romfs.c
//
//      ROM file system
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
// Contributors:        nickg, richard.panton@3glab.com
// Date:                2000-07-25
// Purpose:             ROM file system
// Description:         This is a ROM filesystem for eCos. It attempts to
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
// This is an implementation of a ROM filesystem for eCos. Its goal is
// to provide a working example of a filesystem that provides most of
// the required POSIX functionality. And obviously it may also be
// useful in its own right.
//
//
// Header
// ------
//
// There is a single header that describes the overall format of the ROMFS
// disk. The address of this header is used as the base for all offsets used
// in the node and directory structures. It contains the following fields:
//
// label  - A human readable label for various purposes
// fssize - The size in bytes of the entire ROMFS disk
// nodes  - A count of the nodes in the disk
//
// Immediately following thisin memory is the node table, consisting of
// 'nodes' repetitions of the node object.
//
// Nodes
// -----
//
// All files and directories are represented by node objects. Each
// romfs_node structure contains the following fields:
//
// mode   - Node type, file or directory.
// nlink  - Number of links to this node. Each directory entry that references
//          this node is a link. 
// size   - Size of the data in this node in bytes.
// ctime  - Creation time of the file (NOT the ROMFS)
// data   - Offset of the first data byte for this node from the header
//
// Directories
// -----------
//
// A directory is a node whose data is a list of directory entries.
// These contain the
// following fields:
//
// node    - Index of the node in the romfs_disk table that is referenced by
//           this entry. This is present in every directory entry fragment.
// next    - Offset of the next name entry.
// name    - The filename associated with this link to the node.
//
// Data Storage
// ------------
//
// Each file has its data stored in a single contiguous memory block
// referenced by the offset in the node.
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/fs_rom.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cyg/fileio/fileio.h>

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

//==========================================================================
// Eventually we want to eXecute In Place from the ROM in a protected
// environment, so we'll need executables to be aligned to a boundary
// suitable for MMU protection. A suitable boundary would be the 4k
// boundary in all the CPU architectures I am currently aware of.

// Forward definitions

// Filesystem operations
static int romfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte );
static int romfs_umount   ( cyg_mtab_entry *mte );
static int romfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int mode,  cyg_file *fte );
static int romfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_file *fte );
static int romfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_dir *dir_out );
static int romfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             struct stat *buf);
static int romfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );
static int romfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );

// File operations
static int romfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int romfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int romfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data);
static int romfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode );        
static int romfs_fo_close     (struct CYG_FILE_TAG *fp);
static int romfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf );
static int romfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );
static int romfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );

// Directory operations
static int romfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int romfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );


//==========================================================================
// Filesystem table entries

// -------------------------------------------------------------------------
// Fstab entry.
// This defines the entry in the filesystem table.
// For simplicity we use _FILESYSTEM synchronization for all accesses since
// we should never block in any filesystem operations.

FSTAB_ENTRY( romfs_fste, "romfs", 0,
             CYG_SYNCMODE_FILE_FILESYSTEM|CYG_SYNCMODE_IO_FILESYSTEM,
             romfs_mount,
             romfs_umount,
             romfs_open,
             (cyg_fsop_unlink *)cyg_fileio_erofs,
             (cyg_fsop_mkdir *)cyg_fileio_erofs,
             (cyg_fsop_rmdir *)cyg_fileio_erofs,
             (cyg_fsop_rename *)cyg_fileio_erofs,
             (cyg_fsop_link *)cyg_fileio_erofs,
             romfs_opendir,
             romfs_chdir,
             romfs_stat,
             romfs_getinfo,
             romfs_setinfo);

// -------------------------------------------------------------------------
// mtab entry.
// This defines a single ROMFS loaded into ROM at the configured address
//
// MTAB_ENTRY(	rom_mte,	// structure name
// 		"/rom",		// mount point
// 		"romfs",	// FIlesystem type
// 		"",		// hardware device
//  (CYG_ADDRWORD) CYGNUM_FS_ROM_BASE_ADDRESS	// Address in ROM
//           );


// -------------------------------------------------------------------------
// File operations.
// This set of file operations are used for normal open files.

static cyg_fileops romfs_fileops =
{
    romfs_fo_read,
    (cyg_fileop_write *)cyg_fileio_erofs,
    romfs_fo_lseek,
    romfs_fo_ioctl,
    cyg_fileio_seltrue,
    romfs_fo_fsync,
    romfs_fo_close,
    romfs_fo_fstat,
    romfs_fo_getinfo,
    romfs_fo_setinfo
};

// -------------------------------------------------------------------------
// Directory file operations.
// This set of operations are used for open directories. Most entries
// point to error-returning stub functions. Only the read, lseek and
// close entries are functional.

static cyg_fileops romfs_dirops =
{
    romfs_fo_dirread,
    (cyg_fileop_write *)cyg_fileio_enosys,
    romfs_fo_dirlseek,
    (cyg_fileop_ioctl *)cyg_fileio_enosys,
    cyg_fileio_seltrue,
    (cyg_fileop_fsync *)cyg_fileio_enosys,
    romfs_fo_close,
    (cyg_fileop_fstat *)cyg_fileio_enosys,
    (cyg_fileop_getinfo *)cyg_fileio_enosys,
    (cyg_fileop_setinfo *)cyg_fileio_enosys
};

//==========================================================================
// Data typedefs
// Some forward typedefs for the main data structures.

struct romfs_disk;
typedef struct romfs_disk romfs_disk;

struct romfs_node;
typedef struct romfs_node romfs_node;

struct romfs_dirent;
typedef struct romfs_dirent romfs_dirent;

//==========================================================================
// File and directory node
// This data structure represents a file or directory. 

struct romfs_node
{
    cyg_uint32          mode;           // 0-3   node type
    cyg_ucount32        nlink;          // 4-7   number of links to this node
    cyg_uint16		uid;		// 8-9   Owner id
    cyg_uint16		gid;		// 10-11 Group id
    cyg_uint32          size;           // 12-15 size of file in bytes
    cyg_uint32          ctime;          // 16-19 creation status time
    cyg_uint32		offset;		// 20-23 offset of data from start of ROMFS
    cyg_uint32		pad[2];		// 24-31 padding to align to 32byte boundary
};

//==========================================================================
// Directory entry.
// Variable sized entry containing the name and node of a directory entry

struct romfs_dirent
{
    cyg_ucount32        node;           // Index of node in romfs_disk structure
    cyg_uint32		next;		// Offset from start of directory of
					// a) the next entry, or 
    					// b) the end of the directory data
    char                name[0];	// The name, NUL terminated
};

//==========================================================================
// ROMFS header
// This data structure contains overall information on the ROMFS

struct romfs_disk
{
    cyg_uint32		magic;		// 0-3   Marks a valid ROMFS entry
    cyg_ucount32	nodecount;	// 4-7   Count of nodes in this filesystem
    cyg_ucount32	disksize;	// 8-11  Count of bytes in this filesystem
    cyg_uint32		dev_id;		// 12-15 ID of disk (put into stat.st_dev)
    char		name[16];	// 16-31 Name - pads to 32 bytes
    romfs_node		node[0];
};

#define ROMFS_MAGIC	0x526f6d2e	// The magic signature word for a romfs
#define ROMFS_CIGAM	0x2e6d6f52	// The byte sex is wrong if you see this

//==========================================================================
// Directory search data
// Parameters for a directory search. The fields of this structure are
// updated as we follow a pathname through the directory tree.

struct romfs_dirsearch
{
    romfs_disk		*disk;		// disk structure
    romfs_node          *dir;           // directory to search
    const char          *path;          // path to follow
    romfs_node          *node;          // Node found
    const char          *name;          // last name used
    int			namelen;	// name fragment length
    cyg_bool		last;		// last name in path?
};

typedef struct romfs_dirsearch romfs_dirsearch;

//==========================================================================
// This seems to be the only string function referenced. Define as static
// here to avoid having to load the string library

static bool match( const char *a, const char *b, int len )
{
    for ( ; len > 0 && *a && *b && *a == *b ; a++, b++, len-- )
	;
    return ( len == 0 );
}

		
//==========================================================================
// SIMPLE buffer management.
// Each node has a data buffer pointer and a size.

// -------------------------------------------------------------------------
// findbuffer_node()
// return a pointer to the data at the indicated file position.

static int findbuffer_node( romfs_disk *disk,	// header pointer
			    romfs_node  *node,  // node pointer
                            off_t pos,          // data position to get
                            cyg_uint8 **buffer, // returned buffer pointer
                            size_t *size)       // returned buffer size
{
    if ( pos >= node->size || node->size == 0 )
    {
        // Indicate end of data.
        *size = 0;
    } else {

	// Calculate the buffer position
	*buffer = (cyg_uint8*)disk + node->offset + pos;
	*size = node->size-pos;
    }

    return ENOERR;
}

//==========================================================================
// Directory operations


// -------------------------------------------------------------------------
// find_direntry()
// Find a directory entry for the name and return a pointer to the first
// entry fragment.

static romfs_dirent *find_direntry( romfs_disk *disk, romfs_node *dir, const char *name, int namelen )
{
    off_t pos = 0;
    int err;

    // Loop over all the entries until a match is found or we run out
    // of data.
    while( pos < dir->size )
    {
        romfs_dirent *d;
        cyg_uint8 *buf;
        size_t size;
        
	err = findbuffer_node( disk, dir, pos, &buf, &size );
	if( err != ENOERR || size == 0)
	    return NULL;

	d = (romfs_dirent *)buf;

	// Is this the directory entry we're looking for?
	if ( match( d->name, name, namelen ) )
	    return d;

	// Otherwise move on to next entry in chain
        pos = d->next;
    }

    return NULL;
}

//==========================================================================
// Directory search

// -------------------------------------------------------------------------
// init_dirsearch()
// Initialize a dirsearch object to start a search

static void init_dirsearch( romfs_dirsearch *ds,
                            romfs_disk *disk,
			    romfs_node *dir,
                            const char *name)
{
    ds->disk     = disk;
    ds->dir      = dir;
    ds->path     = name;
    ds->node     = dir;
    ds->name     = name;
    ds->namelen  = 0;
    ds->last	 = false;
}

// -------------------------------------------------------------------------
// find_entry()
// Search a single directory for the next name in a path and update the
// dirsearch object appropriately.

static int find_entry( romfs_dirsearch *ds )
{
    romfs_node *dir = ds->dir;
    const char *name = ds->path;
    const char *n = name;
    int namelen = 0;
    romfs_dirent *d;
    
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

    d = find_direntry( ds->disk, dir, name, namelen );

    if( d == NULL )
        return ENOENT;

    // pass back the node we have found
    ds->node = &ds->disk->node[d->node];

    return ENOERR;

}

// -------------------------------------------------------------------------
// romfs_find()
// Main interface to directory search code. This is used in all file
// level operations to locate the object named by the pathname.

static int romfs_find( romfs_dirsearch *d )
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

static int romfs_pathconf( romfs_node *node, struct cyg_pathconf_info *info )
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
// romfs_mount()
// Process a mount request. This mainly finds root for the
// filesystem.

static int romfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte )
{
    romfs_disk *disk=NULL;
    
    if ( !mte->data ) {
	// If the image address was not in the MTE data word,
	if ( mte->devname && mte->devname[0] ) {
            char *addr;
            // And there's something in the 'hardware device' field,
	    // then read the address from there.
	    sscanf( mte->devname, "%p", &addr );
            disk = (romfs_disk *) addr;
        }
    } else {
        disk = (romfs_disk *)mte->data;
    }

    if ( !disk ) {
	// If still no address, try the FSTAB entry data word
	disk = (romfs_disk *)fste->data;
    }

    if ( !disk ) {
	// If still no address, give up...
	return ENOENT;
    }


    
    // Check the ROMFS magic number to ensure that there's really an fs.
    if ( disk->magic == ROMFS_CIGAM ) {
	// The disk image has the wrong byte sex!!!
	return EIO;
    } else if ( disk->magic != ROMFS_MAGIC || disk->nodecount == 0 ) {
	// No image found
	return ENOENT;
    }

    mte->root = (cyg_dir)&disk->node[0];
    
    mte->data = (CYG_ADDRWORD)disk;
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_umount()
// Unmount the filesystem. This will currently always succeed.

static int romfs_umount   ( cyg_mtab_entry *mte )
{
    // Clear root pointer
    mte->root = CYG_DIR_NULL;
    
    // That's all folks.
        
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_open()
// Open a file for reading

static int romfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int mode,  cyg_file *file )
{

    romfs_dirsearch ds;
    romfs_node *node = NULL;
    int err;

    init_dirsearch( &ds, (romfs_disk *)mte->data, (romfs_node *)dir, name );
    
    err = romfs_find( &ds );

    if( err == ENOENT )
    {
	return ENOENT;
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
        // If the O_TRUNC bit is set we must fail the open

        err = EPERM;
    }

    if( err != ENOERR ) return err;

    // Check that we actually have a file here
    if( S_ISDIR(node->mode) ) return EISDIR;

    // Initialize the file object
    
    file->f_flag        |= mode & CYG_FILE_MODE_MASK;
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &romfs_fileops;
    file->f_offset      = 0;
    file->f_data        = (CYG_ADDRWORD)node;
    file->f_xops        = 0;

    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_opendir()
// Open a directory for reading.

static int romfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_file *file )
{
    romfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (romfs_disk *)mte->data, (romfs_node *)dir, name );
    
    err = romfs_find( &ds );

    if( err != ENOERR ) return err;

    // check it is really a directory.
    if( !S_ISDIR(ds.node->mode) ) return ENOTDIR;

    // Initialize the file object, setting the f_ops field to a
    // special set of file ops.
    
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &romfs_dirops;
    file->f_offset      = 0;
    file->f_data        = (CYG_ADDRWORD)ds.node;
    file->f_xops        = 0;
        
    return ENOERR;

}

// -------------------------------------------------------------------------
// romfs_chdir()
// Change directory support.

static int romfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_dir *dir_out )
{
    if( dir_out != NULL )
    {
        // This is a request to get a new directory pointer in
        // *dir_out.

        romfs_dirsearch ds;
        int err;
    
        init_dirsearch( &ds, (romfs_disk *)mte->data, (romfs_node *)dir, name );
    
        err = romfs_find( &ds );

        if( err != ENOERR ) return err;

        // check it is a directory
        if( !S_ISDIR(ds.node->mode) )
            return ENOTDIR;
        
        // Pass it out
        *dir_out = (cyg_dir)ds.node;
    }
    // If no output dir is required, this means that the mte and
    // dir arguments are the current cdir setting and we should
    // forget this fact. Do nothing in ROMFS.
        
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_stat()
// Get struct stat info for named object.

static int romfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             struct stat *buf)
{
    romfs_dirsearch ds;
    int err;
    romfs_disk *disk = (romfs_disk *)mte->data;

    init_dirsearch( &ds, disk, (romfs_node *)dir, name );
    
    err = romfs_find( &ds );

    if( err != ENOERR ) return err;

    // Fill in the status
    buf->st_mode        = ds.node->mode;
    buf->st_ino         = (ino_t)(ds.node - &disk->node[0]);
    buf->st_dev         = (dev_t)disk->dev_id;
    buf->st_nlink       = ds.node->nlink;
    buf->st_uid         = ds.node->uid;
    buf->st_gid         = ds.node->gid;
    buf->st_size        = ds.node->size;
    buf->st_atime       = ds.node->ctime;
    buf->st_mtime       = ds.node->ctime;
    buf->st_ctime       = ds.node->ctime;
    
    return err;
}

// -------------------------------------------------------------------------
// romfs_getinfo()
// Getinfo. Currently only support pathconf().

static int romfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len )
{
    romfs_dirsearch ds;
    int err;

    init_dirsearch( &ds, (romfs_disk *)mte->data, (romfs_node *)dir, name );
    
    err = romfs_find( &ds );

    if( err != ENOERR ) return err;

    switch( key )
    {
    case FS_INFO_CONF:
        err = romfs_pathconf( ds.node, (struct cyg_pathconf_info *)buf );
        break;
        
    default:
        err = EINVAL;
    }
    return err;
}

// -------------------------------------------------------------------------
// romfs_setinfo()
// Setinfo. Nothing to support here at present.

static int romfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len )
{
    // No setinfo keys supported at present
    
    return EINVAL;
}


//==========================================================================
// File operations

// -------------------------------------------------------------------------
// romfs_fo_read()
// Read data from the file.

static int romfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    romfs_node *node = (romfs_node *)fp->f_data;
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
            err = findbuffer_node( (romfs_disk *)fp->f_mte->data, node, pos, &fbuf, &bsize );

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

    // We successfully read some data
    // Update the file offset and transfer residue.
    
    uio->uio_resid = resid;
    fp->f_offset = pos;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_fo_lseek()
// Seek to a new file position.

static int romfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *apos, int whence )
{
    romfs_node *node = (romfs_node *)fp->f_data;
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
// romfs_fo_ioctl()
// Handle ioctls. Currently none are defined.

static int romfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data)
{
    // No Ioctls currenly defined.

    return EINVAL;
}

// -------------------------------------------------------------------------
// romfs_fo_fsync().
// Force the file out to data storage.

static int romfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode )
{
    // Data is always permanently where it belongs, nothing to do
    // here.
  
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_fo_close()
// Close a file. We just clear out the data pointer.

static int romfs_fo_close     (struct CYG_FILE_TAG *fp)
{
    fp->f_data = 0;     // zero data pointer
    
    return ENOERR;
}

// -------------------------------------------------------------------------
//romfs_fo_fstat()
// Get file status.

static int romfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf )
{
    romfs_node *node = (romfs_node *)fp->f_data;
    romfs_disk *disk = (romfs_disk*)(fp->f_mte->data);

    // Fill in the status
    buf->st_mode        = node->mode;
    buf->st_ino         = (ino_t)(node - &disk->node[0]);
    buf->st_dev         = disk->dev_id;
    buf->st_nlink       = node->nlink;
    buf->st_uid         = node->uid;
    buf->st_gid         = node->gid;
    buf->st_size        = node->size;
    buf->st_atime       = node->ctime;
    buf->st_mtime       = node->ctime;
    buf->st_ctime       = node->ctime;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_fo_getinfo()
// Get info. Currently only supports fpathconf().

static int romfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    romfs_node *node = (romfs_node *)fp->f_data;    
    int err;

    switch( key )
    {
    case FS_INFO_CONF:
        err = romfs_pathconf( node, (struct cyg_pathconf_info *)buf );
        break;
        
    default:
        err = EINVAL;
    }
    return err;
}

// -------------------------------------------------------------------------
// romfs_fo_setinfo()
// Set info. Nothing supported here.

static int romfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    // No setinfo key supported at present
    
    return ENOERR;
}


//==========================================================================
// Directory operations

// -------------------------------------------------------------------------
// romfs_fo_dirread()
// Read a single directory entry from a file.

static int romfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    romfs_node *dir = (romfs_node *)fp->f_data;
    off_t pos = fp->f_offset;
    int err = ENOERR;
    struct dirent *ent = (struct dirent *)uio->uio_iov[0].iov_base;
    char *nbuf = ent->d_name;
    int nlen = sizeof(ent->d_name)-1;
    off_t len = uio->uio_iov[0].iov_len;
    romfs_dirent *d = NULL;
    cyg_uint8 *buf;
    size_t size;
    int i;
        
    if( len < sizeof(struct dirent) )
        return EINVAL;    
    
    // Get the next entry
    err = findbuffer_node( (romfs_disk *)fp->f_mte->data, dir, pos, &buf, &size );
    if( err != ENOERR || size == 0 || pos >= dir->size )
	return err;

    d = (romfs_dirent *)buf;

    for ( i = 0 ; i < nlen && d->name[i] ; i++, nbuf++ )
	*nbuf = d->name[i];

    // A successful read. Terminate the entry name with a NUL, set the
    // residue and set the file offset to restart at the next
    // directory entry.
    
    *nbuf = '\0';
    uio->uio_resid -= sizeof(struct dirent);
    fp->f_offset = d->next;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// romfs_fo_dirlseek()
// Seek directory to start.

static int romfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence )
{
    // Only allow SEEK_SET to zero
    
    if( whence != SEEK_SET || *pos != 0)
        return EINVAL;

    *pos = fp->f_offset = 0;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// EOF romfs.c
