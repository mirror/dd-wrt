//==========================================================================
//
//      testfs.c
//
//      Test file system
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
// Date:                2000-05-25
// Purpose:             Test file system
// Description:         This is a very simple implementation of a RAM file system.
//                      This implementation is not "industrial strength" or suitable
//                      for production use, it is too wasteful of both memory and time.
//                      Its primary purpose is to support testing of the fileio
//                      infrastructure and API. It can, however, serve as a model
//                      and source of code fragments for the implementation
//                      of further filesystems.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/kernel.h>
#include <pkgconf/io_fileio.h>

#include <cyg/kernel/ktypes.h>         // base kernel types
#include <cyg/infra/cyg_trac.h>        // tracing macros
#include <cyg/infra/cyg_ass.h>         // assertion macros

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <cyg/fileio/fileio.h>

#include <cyg/kernel/kapi.h>
#include <cyg/infra/diag.h>

//==========================================================================
// Configuration  parameters

#define TESTFS_NFILE            10      // Max number of files/directories
#define TESTFS_NBLOCK           20      // Number of data blocks available
#define TESTFS_BLOCKSIZE        128     // Bytes stored per block
#define TESTFS_FILEBLOCKS       8       // Max blocks per file
#define TESTFS_NAMESIZE         32      // Length of file names in bytes


// Maximum file size is blocksize*blocks
#define TESTFS_FILESIZE_MAX     (TESTFS_BLOCKSIZE*TESTFS_FILEBLOCKS)

//==========================================================================
// Data structures

struct testfs_node;
typedef struct testfs_node testfs_node;

struct testfs_block;
typedef struct testfs_block testfs_block;


struct testfs_node
{
    testfs_node         *next;          // next node in list
    testfs_node         *parent;        // Back pointer to parent
    int                 refcnt;         // reference count
    char                name[TESTFS_NAMESIZE]; // file name
    struct stat         status;         // status data
    union
    {
        struct
        {
            testfs_block        *data[TESTFS_FILEBLOCKS];  // array of blocks
        } file;

        struct
        {
            testfs_node         *nodes[TESTFS_FILEBLOCKS]; // array of nodes
        } dir;
    } u;
};

struct testfs_block
{
    union
    {
        testfs_block    *next;          // next block in free list
        testfs_node     *file;          // back pointer to file
    } u;
    off_t               pos;            // position in file of first byte
    size_t              size;           // number of bytes in buffer
    char                data[TESTFS_BLOCKSIZE]; // the data
};

//==========================================================================
// Local data

// Array of nodes
static testfs_node node[TESTFS_NFILE];

// node free list.
static testfs_node *free_node = NULL;

// Array of data blocks
static testfs_block block[TESTFS_NBLOCK];

// block free list.
static testfs_block *free_block = NULL;

// Init flag
cyg_bool testfs_initialized = false;

//==========================================================================
// Forward definitions

// Filesystem operations
static int testfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte );
static int testfs_umount   ( cyg_mtab_entry *mte );
static int testfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int mode,  cyg_file *fte );
static int testfs_unlink   ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int testfs_mkdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int testfs_rmdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name );
static int testfs_rename   ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2 );
static int testfs_link     ( cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                             cyg_dir dir2, const char *name2, int type );
static int testfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_file *fte );
static int testfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             cyg_dir *dir_out );
static int testfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             struct stat *buf);
static int testfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );
static int testfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                             int key, void *buf, int len );

// File operations
static int testfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int testfs_fo_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int testfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int testfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                                CYG_ADDRWORD data);
//static int testfs_fo_select    (struct CYG_FILE_TAG *fp, int which, CYG_ADDRWORD info);
static int testfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode );        
static int testfs_fo_close     (struct CYG_FILE_TAG *fp);
static int testfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf );
static int testfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );
static int testfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len );

// Directory operations
static int testfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int testfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence );

//==========================================================================
// Filesystem table entries

FSTAB_ENTRY( testfs_fste, "testfs", 0,
             CYG_SYNCMODE_FILE_FILESYSTEM|CYG_SYNCMODE_IO_FILESYSTEM,
             testfs_mount,
             testfs_umount,
             testfs_open,
             testfs_unlink,
             testfs_mkdir,
             testfs_rmdir,
             testfs_rename,
             testfs_link,
             testfs_opendir,
             testfs_chdir,
             testfs_stat,
             testfs_getinfo,
             testfs_setinfo);

MTAB_ENTRY( testfs_mte1,
                   "/",
                   "testfs",
                   "",
                   0);

#if 0
MTAB_ENTRY( testfs_mte2,
                   "/ram",
                   "testfs",
                   "",
                   0);
#endif

static cyg_fileops testfs_fileops =
{
    testfs_fo_read,
    testfs_fo_write,
    testfs_fo_lseek,
    testfs_fo_ioctl,
    cyg_fileio_seltrue,
    testfs_fo_fsync,
    testfs_fo_close,
    testfs_fo_fstat,
    testfs_fo_getinfo,
    testfs_fo_setinfo
};

static cyg_fileops testfs_dirops =
{
    testfs_fo_dirread,
    (cyg_fileop_write *)cyg_fileio_enosys,
    testfs_fo_dirlseek,
    (cyg_fileop_ioctl *)cyg_fileio_enosys,
    cyg_fileio_seltrue,
    (cyg_fileop_fsync *)cyg_fileio_enosys,
    testfs_fo_close,
    (cyg_fileop_fstat *)cyg_fileio_enosys,
    (cyg_fileop_getinfo *)cyg_fileio_enosys,
    (cyg_fileop_setinfo *)cyg_fileio_enosys
};

//==========================================================================
// Support routines

// -------------------------------------------------------------------------
// Local strcmp() and strcpy()

static int strcmp( const char *s1, const char *s2 )
{
    while( *s1 == *s2 && *s1 != '\0' && *s2 != '\0' )
        s1++, s2++;

    return (*s2)-(*s1);
}

static char *strcpy( char *s1, const char *s2 )
{
    char *s = s1;
    while( (*s1++ = *s2++) != 0);
    return s;
}

// -------------------------------------------------------------------------
// Follow a path through the directory structure

static int testfs_find( testfs_node *dir,       // dir to start search in
                        const char *path,       // path to follow
                        testfs_node **found,    // return node found
                        testfs_node **parent,   // return last dir searched
                        char *name,             // name fragment buffer
                        cyg_bool *lastp)        // last name in path ?
{
    testfs_node *nd = dir;

    *lastp = false;
    *found = NULL;
    
    while( *path != '\0' )
    {
        const char *p = path;
        char *n = name;
        testfs_node *nd1;
        int i;

        // check nd is a directory
        if( !S_ISDIR(nd->status.st_mode) ) return ENOTDIR;        

        // Isolate the next element of the path name. 
        while( *p != '\0' && *p != '/' && (n-&name[0]) < TESTFS_NAMESIZE)
            *n++ = *p++;

        if( (n-&name[0]) >= TESTFS_NAMESIZE )
            return ENAMETOOLONG;

        // Step path on past the separator
        // If this is the last name element in the path,
        // set *lastp to indicate this.
        if( *(path=p) == '/' ) path++;
        else *lastp = true;

        // teminate name
        *n = '\0';

        // name now contains the next path element, search the node
        // in nd for it.

        *parent = nd;
        nd1 = NULL;
        
        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        {
            testfs_node *n = nd->u.dir.nodes[i];
            if( n == NULL )
                continue;

            if( strcmp( name, n->name ) == 0 )
            {
                nd1 = n;
                break;
            }
        }
        
        if( nd1 == NULL ) return ENOENT;

        nd = nd1;
    }

    // Return what we have found
    *found = nd;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// Get current time since epoch

static time_t testfs_time(void)
{
    // FIXME: !!!!Temporary!!!!
    return cyg_current_time();
}

// -------------------------------------------------------------------------

static int testfs_delnode( testfs_node *nd )
{

    testfs_node *parent;
    int i;

    // Non-unitary ref count means this node is either open    
    // or is a dir with entries in it.
    if( nd->refcnt > 1 )
        return EBUSY;

    // Remove from parent's node list.
    
    parent = nd->parent;

    for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        if( parent->u.dir.nodes[i] == nd )
        {
            parent->u.dir.nodes[i] = NULL;
            break;
        }

    parent->refcnt--;

    if( S_ISREG(nd->status.st_mode) )
    {
        // for a file, return blocks to free list
        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        {
            testfs_block *b = nd->u.file.data[i];
            if( b != NULL )
            {
                b->u.next = free_block;
                b->pos = -1;
                free_block = b;
            }
        }
    }

    // and finally return nd to free node list

    nd->next = free_node;
    nd->refcnt = -1;
    free_node = nd;

    return ENOERR;
}

//==========================================================================
// Filesystem operations

// -------------------------------------------------------------------------

static int testfs_mount    ( cyg_fstab_entry *fste, cyg_mtab_entry *mte )
{
    testfs_node *root;
    int i;
    
    if( !testfs_initialized )
    {
        int i;

        for( i = 0; i < TESTFS_NFILE; i++ )
        {
            node[i].next = free_node;
            node[i].refcnt = -1;
            free_node = &node[i];
        }

        for( i = 0; i < TESTFS_NBLOCK; i++ )
        {
            block[i].u.next = free_block;
            block[i].pos = -1;
            free_block = &block[i];
        }

        testfs_initialized = true;
    }

    // Allocate a node to be the root of this filesystem and
    // initialize it.

    root = free_node;
    
    if( root == NULL ) return ENOSPC;

    free_node = root->next;

    root->next                  = root;  // form circular list
    root->parent                = root;  // I'm my own parent!
    root->refcnt                = 1;     // don't want to ever lose root
    strcpy( root->name, "root");
    root->status.st_mode        = __stat_mode_DIR;
    root->status.st_ino         = root-&node[0];
    root->status.st_dev         = 0;
    root->status.st_nlink       = 1;
    root->status.st_uid         = 0;
    root->status.st_gid         = 0;
    root->status.st_size        = 0;
    root->status.st_atime       = testfs_time();
    root->status.st_mtime       = testfs_time();
    root->status.st_ctime       = testfs_time();

    for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        root->u.dir.nodes[i]    = NULL;
    
    mte->root = (cyg_dir)root;
    
    return 0;
}

// -------------------------------------------------------------------------

static int testfs_umount    ( cyg_mtab_entry *mte )
{
    testfs_node *root = (testfs_node *)mte->root;

    // Non-empty filesystem, do not unmount
    if( root->refcnt != 1 )
        return EBUSY;

    // Otherwise just return it to the free pool
    root->next = free_node;
    root->refcnt = -1;
    free_node = root;

    // Clear root pointer
    mte->root = CYG_DIR_NULL;
    
    // That's all folks.
    
    return ENOERR;
}
// -------------------------------------------------------------------------

static int testfs_open     ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                          int mode,  cyg_file *file )
{
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
    
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( lastp && err == ENOENT && (mode & O_CREAT) )
    {
        int i;
        // No node there, if the O_CREAT bit is set then we must
        // create a new one. The parent and name results will have been filled
        // in, so we know where to put it.

        // first check that there is space for it
        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
            if( parent->u.dir.nodes[i] == NULL )
                break;

        if( i == TESTFS_FILEBLOCKS ) return ENOSPC;
        
        // Allocate a new node
        nd = free_node;
        if( nd == NULL ) return ENOSPC;
        free_node = nd->next;

        // Add to directory list
        parent->u.dir.nodes[i] = nd;

        parent->refcnt++;
        
        // Fill in details
        nd->parent              = parent;
        nd->refcnt              = 1;    // 1 for directory reference
        strcpy( nd->name, name);
        nd->status.st_mode      = __stat_mode_REG;
        nd->status.st_ino       = nd-&node[0];
        nd->status.st_dev       = 0;
        nd->status.st_nlink     = 1;
        nd->status.st_uid       = 0;
        nd->status.st_gid       = 0;
        nd->status.st_size      = 0;
        nd->status.st_atime     = testfs_time();
        nd->status.st_mtime     = testfs_time();
        nd->status.st_ctime     = testfs_time();

        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
            nd->u.file.data[i]  = NULL;

        err = ENOERR;
    }

    if( err == ENOERR && (mode & O_TRUNC ) )
    {
        // Clean out any blocks in the file...

        int i;
        
        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        {
            testfs_block *b = nd->u.file.data[i];
            if( b != NULL )
            {
                b->u.next = free_block;
                b->pos = -1;
                free_block = b;
                nd->u.file.data[i] = NULL;
            }
        }

        nd->status.st_size = 0;
    }
    
    if( err != ENOERR ) return err;

    if( S_ISDIR(nd->status.st_mode) ) return EISDIR;

    nd->refcnt++;       // Count successful open as a ref
    
    // Initialize the file object
    
    file->f_flag        |= mode & CYG_FILE_MODE_MASK;
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &testfs_fileops;
    file->f_offset      = 0;
    file->f_data        = (CYG_ADDRWORD)nd;
    file->f_xops        = 0;
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_unlink   ( cyg_mtab_entry *mte, cyg_dir dir, const char *path )
{
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
   
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( err != ENOERR ) return err;

    // Cannot unlink directories, use rmdir() instead
    if( S_ISDIR(nd->status.st_mode) )
        return EPERM;

    err = testfs_delnode( nd );
    
    return err;
}

// -------------------------------------------------------------------------

static int testfs_mkdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *path )
{
    
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
    
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( lastp && err == ENOENT )
    {
        int i;
        // No node there, create a new one. The parent and name
        // results will have been filled in, so we know where to put
        // it.

        // first check that there is space for it
        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
            if( parent->u.dir.nodes[i] == NULL )
                break;

        if( i == TESTFS_FILEBLOCKS ) return ENOSPC;
        
        // Allocate a new node
        nd = free_node;
        if( nd == NULL ) return ENOSPC;
        free_node = nd->next;

        // Add to directory list
        parent->u.dir.nodes[i] = nd;

        parent->refcnt++;
 
        // Fill in details
        nd->parent              = parent;
        nd->refcnt              = 1;    // 1 for directory reference
        strcpy( nd->name, name);
        nd->status.st_mode      = __stat_mode_DIR;
        nd->status.st_ino       = nd-&node[0];
        nd->status.st_dev       = 0;
        nd->status.st_nlink     = 1;
        nd->status.st_uid       = 0;
        nd->status.st_gid       = 0;
        nd->status.st_size      = 0;
        nd->status.st_atime     = testfs_time();
        nd->status.st_mtime     = testfs_time();
        nd->status.st_ctime     = testfs_time();

        for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
            nd->u.dir.nodes[i]  = NULL;
        
        err = ENOERR;
    }

    return err;
}

// -------------------------------------------------------------------------

static int testfs_rmdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *path )
{
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
   
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( err != ENOERR ) return err;

    // Check that it is a directory
    if( !S_ISDIR(nd->status.st_mode) )
        return EPERM;

    err = testfs_delnode( nd );

    return err;
}

// -------------------------------------------------------------------------

static int testfs_rename   ( cyg_mtab_entry *mte, cyg_dir dir1, const char *path1,
                          cyg_dir dir2, const char *path2 )
{
    testfs_node *nd1, *parent1;
    testfs_node *nd2, *parent2;
    int err;
    char name1[TESTFS_NAMESIZE];
    char name2[TESTFS_NAMESIZE];
    cyg_bool lastp;
    int i,j;
    
    err = testfs_find( (testfs_node *)dir1, path1, &nd1, &parent1, name1, &lastp );

    if( err != ENOERR ) return err;

    err = testfs_find( (testfs_node *)dir2, path2, &nd2, &parent2, name2, &lastp );

    // Allow through renames to non-existent objects.
    if( lastp && err == ENOENT )
        err = ENOERR;
    
    if( err != ENOERR ) return err;

    // Null rename, just return
    if( nd1 == nd2 )
        return ENOERR;
    

    // First deal with any node that is at the destination
    if( nd2 )
    {
        // Check that we are renaming like-for-like

        if( !S_ISDIR(nd1->status.st_mode) && S_ISDIR(nd2->status.st_mode) )
            return EISDIR;

        if( S_ISDIR(nd1->status.st_mode) && !S_ISDIR(nd2->status.st_mode) )
            return ENOTDIR;

        // Now delete the destination node.
        err = testfs_delnode( nd2 );
        if( err != ENOERR ) return err;
    }

    // Now we know that there is no clashing node at the destination.
    // Move the node over and change its name.

    // first check that there is space for it
    for( i = 0; i < TESTFS_FILEBLOCKS; i++ )
        if( parent2->u.dir.nodes[i] == NULL )
            break;

    if( i == TESTFS_FILEBLOCKS ) return ENOSPC;

    // Now remove node from old parent.
    for( j = 0; j < TESTFS_FILEBLOCKS; j++ )
        if( parent1->u.dir.nodes[j] == nd1 )
        {
            parent1->u.dir.nodes[j] = NULL;
            break;
        }

    parent1->refcnt--;

    // Add to directory list
    parent2->u.dir.nodes[i] = nd1;
    parent2->refcnt++;
    nd1->parent = parent2;
    
    // And give it a new name.
    strcpy( nd1->name, name2 );
    
    return err;
}

// -------------------------------------------------------------------------

static int testfs_link   ( cyg_mtab_entry *mte, cyg_dir dir1, const char *path1,
                          cyg_dir dir2, const char *path2, int type )
{
    // The data structures of this file system do not support the
    // creation of links.
    
    return ENOSYS;
}

// -------------------------------------------------------------------------

static int testfs_opendir  ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                          cyg_file *file )
{
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
    
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( err != ENOERR ) return err;

    if( !S_ISDIR(nd->status.st_mode) )
        return ENOTDIR;

    nd->refcnt++;       // Count successful open as a ref
    
    // Initialize the file object
    
    file->f_type        = CYG_FILE_TYPE_FILE;
    file->f_ops         = &testfs_dirops;
    file->f_offset      = 0;
    file->f_data        = (CYG_ADDRWORD)nd;
    file->f_xops        = 0;
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_chdir    ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                             cyg_dir *dir_out )
{
    if( dir_out != NULL )
    {
        // This is a request to get a new directory pointer in
        // *dir_out.

        testfs_node *nd, *parent;
        int err;
        char name[TESTFS_NAMESIZE];
        cyg_bool lastp;
    
        err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

        if( err != ENOERR ) return err;

        if( !S_ISDIR(nd->status.st_mode) )
            return ENOTDIR;
        
        // Increment ref count to keep this directory in existent
        // while it is the current cdir.
        nd->refcnt++;

        // Pass it out
        *dir_out = (cyg_dir)nd;
    }
    else
    {
        // If no output dir is required, this means that the mte and
        // dir arguments are the current cdir setting and we should
        // forget this fact.

        testfs_node *nd = (testfs_node *)dir;

        // Just decrement reference count.
        nd->refcnt--;
    }
        
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_stat     ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                          struct stat *buf)
{
    testfs_node *nd, *parent;
    int err;
    char name[TESTFS_NAMESIZE];
    cyg_bool lastp;
    
    err = testfs_find( (testfs_node *)dir, path, &nd, &parent, name, &lastp );

    if( err != ENOERR ) return err;

    *buf = nd->status;
    
    return err;
}

// -------------------------------------------------------------------------

static int testfs_getinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                          int key, void *buf, int len )
{
    return ENOSYS;
}

// -------------------------------------------------------------------------

static int testfs_setinfo  ( cyg_mtab_entry *mte, cyg_dir dir, const char *path,
                          int key, void *buf, int len )
{
    return ENOSYS;
}


//==========================================================================
// File operations


// -------------------------------------------------------------------------

static int testfs_fo_read      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    testfs_node *nd = (testfs_node *)fp->f_data;
    int i;
    off_t pos = fp->f_offset;

    for( i = 0; i < uio->uio_iovcnt; i++ )
    {
        cyg_iovec *iov = &uio->uio_iov[i];
        char *buf = (char *)iov->iov_base;
        off_t len = iov->iov_len;

        while( len > 0 && pos < nd->status.st_size )
        {
            testfs_block *b = nd->u.file.data[pos/TESTFS_BLOCKSIZE];
            off_t l = len;
            off_t bpos = pos%TESTFS_BLOCKSIZE;
            
            // If there is no block in that pos, we have reached
            // the end of the file.
            if( b == NULL ) return ENOERR;

            // adjust size to this block
            if( l > (b->size-bpos) )
                l = (b->size-bpos);

            // copy data out
            memcpy( buf, &b->data[bpos], l );

            uio->uio_resid -= l;
            len -= l;
            buf += l;
            pos += l;

            // keep offset up to date incase of errors
            fp->f_offset = pos;
        }
    }
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_write     (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    testfs_node *nd = (testfs_node *)fp->f_data;
    int i;
    off_t pos = fp->f_offset;
    
    // Check we are not at end of allowed max file size
    if( pos >= TESTFS_FILESIZE_MAX )
        return EFBIG;

    // Check that pos is within current file size, or at the very end.
    if( pos < 0 || pos > nd->status.st_size )
        return EINVAL;

    // Now loop over the iovecs until they are all done, or
    // we get an error.
    for( i = 0; i < uio->uio_iovcnt; i++ )
    {
        cyg_iovec *iov = &uio->uio_iov[i];
        char *buf = (char *)iov->iov_base;
        off_t len = iov->iov_len;

        while( len > 0 )
        {
            testfs_block *b = nd->u.file.data[pos/TESTFS_BLOCKSIZE];
            off_t l = len;
            off_t bpos = pos%TESTFS_BLOCKSIZE;
            
            // If there is no block in that pos, allocate one
            // and initialize it
            if( b == NULL )
            {
                b = free_block;
                if( b == NULL ) return ENOSPC;
                free_block = b->u.next;
                nd->u.file.data[pos/TESTFS_BLOCKSIZE] = b;
                b->u.file = nd;
                b->pos = pos;
                b->size = 0;
            }

            // adjust size to this block
            if( l > (TESTFS_BLOCKSIZE-bpos) )
                l = (TESTFS_BLOCKSIZE-bpos);

            // copy data in
            memcpy( &b->data[bpos], buf, l );

            // adjust buffer info
            if( b->size < bpos+l )
                b->size = bpos+l;
            
            uio->uio_resid -= l;
            len -= l;
            buf += l;
            pos += l;

            // keep node size and file offset up to date
            //in case of an error.
            if( pos > nd->status.st_size )
                nd->status.st_size = pos;
            fp->f_offset = pos;

            if( pos >= TESTFS_FILESIZE_MAX )
                return EFBIG;
        }
    }
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_lseek     (struct CYG_FILE_TAG *fp, off_t *apos, int whence )
{
    testfs_node *nd = (testfs_node *)fp->f_data;
    off_t pos = *apos;

    switch( whence )
    {
    case SEEK_SET:
        // we are already where we want to be.
        break;

    case SEEK_CUR:
        pos += fp->f_offset;
        break;

    case SEEK_END:
        pos += nd->status.st_size;
        break;

    default:
        return EINVAL;
    }
    
    // Check that pos is within current file size, or at the very end.
    if( pos < 0 || pos > nd->status.st_size )
        return EINVAL;

    // All OK, set fp offset.
    *apos = fp->f_offset = pos;
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_ioctl     (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                             CYG_ADDRWORD data)
{
    return ENOSYS;
}

// -------------------------------------------------------------------------

static int testfs_fo_fsync     (struct CYG_FILE_TAG *fp, int mode )
{
    // Nothing to do
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_close     (struct CYG_FILE_TAG *fp)
{
    testfs_node *nd = (testfs_node *)fp->f_data;

    nd->refcnt--;       // remove open count

    fp->f_data = 0;     // clear data pointer
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_fstat     (struct CYG_FILE_TAG *fp, struct stat *buf )
{
    testfs_node *nd = (testfs_node *)fp->f_data;

    *buf = nd->status;    
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_getinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_setinfo   (struct CYG_FILE_TAG *fp, int key, void *buf, int len )
{
    return ENOERR;
}


//==========================================================================
// Directory operations

static int testfs_fo_dirread      (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    testfs_node *nd = (testfs_node *)fp->f_data;
    off_t pos = fp->f_offset;
    cyg_iovec *iov = &uio->uio_iov[0];
    char *buf = (char *)iov->iov_base;
    off_t len = iov->iov_len;

    // End of directory
    if( pos >= TESTFS_FILEBLOCKS )
        return ENOERR;

    if( len < sizeof(struct dirent) )
        return EINVAL;
    
    for( ; pos < TESTFS_FILEBLOCKS; pos++ )
        if( nd->u.dir.nodes[pos] != NULL )
        {
            struct dirent *ent = (struct dirent *)buf;
            strcpy( ent->d_name, nd->u.dir.nodes[pos]->name );
            uio->uio_resid -= sizeof(struct dirent);
            break;
        }

    fp->f_offset = pos+1;
    
    return ENOERR;
}

// -------------------------------------------------------------------------

static int testfs_fo_dirlseek     (struct CYG_FILE_TAG *fp, off_t *pos, int whence )
{
    if( whence != SEEK_SET || *pos != 0)
        return EINVAL;

    *pos = fp->f_offset = 0;
    
    return ENOERR;
}

//==========================================================================
// Filesystem dump
// Dumps out the node and block arrays in a readable format, and does
// a little consistency checking as it goes.

void testfs_dump(void)
{
    int errors = 0;
    int i;
    char *indent = "\n                      |";

    diag_printf("Nodes:\n");
    for( i = 0; i < TESTFS_NFILE; i++ )
    {
        testfs_node *nd = &node[i];

        diag_printf("%3d : ",i);
        if( nd->refcnt < 0 )
            diag_printf("<free>");
        else if( !S_ISDIR(nd->status.st_mode) )
        {
            // Regular file
            int j;
            diag_printf("f %8s %4d |",nd->name,nd->status.st_size);
            for( j = 0; j < TESTFS_FILEBLOCKS; j++ )
            {
                testfs_block *b = nd->u.file.data[j];
                if( b != NULL )
                {
                    if( j > 0 && (j%4) == 0 )
                        diag_printf(indent);
                    diag_printf(" %3d[%3d,%3d]",b-block,b->pos,b->size);
                    if( b->u.file != nd )
                    {
                        errors++;
                        diag_printf("!");
                    }
                }
            }
        }
        else
        {
            // Directory
            int j;
            int rc = 1;
            diag_printf("d %8s      |",nd->name);

            for( j = 0; j < TESTFS_FILEBLOCKS; j++ )
            {
                testfs_node *n = nd->u.dir.nodes[j];
                if( n != NULL )
                {
                    if( j > 0 && (j%4) == 0 )
                        diag_printf(indent);
                    diag_printf(" %3d[%7s]",n-node,n->name);
                    rc++;
                }
            }

            if( nd->refcnt != rc )
            {
                diag_printf("%s refcount is %d should be %d",indent,nd->refcnt,rc);
                if( nd->refcnt == rc+1 )
                    diag_printf(" (but may be current dir)");
            }
        }

        diag_printf("\n");
    }

    diag_printf("Blocks:\n");

    for( i = 0; i < TESTFS_NBLOCK; i++ )
    {
        testfs_block *b = &block[i];

        diag_printf("%3d :",i);
        if( b->pos == -1 )
            diag_printf(" <free>");
        else
        {
            int j;
            testfs_node *nd = b->u.file;
            diag_printf(" %3d %3d %d[%7s]",b->pos,b->size,nd-node,nd->name);
            for( j = 0; j < TESTFS_FILEBLOCKS; j++ )
            {
                if( nd->u.file.data[j] == b )
                    break;
            }
            if( j == TESTFS_FILEBLOCKS )
            {
                errors++;
                diag_printf(" block not in file!");
            }
        }
        diag_printf("\n");
    }

    if( errors != 0 )
        diag_printf("%d errors detected\n",errors);
}

// -------------------------------------------------------------------------
// EOF testfs.c
