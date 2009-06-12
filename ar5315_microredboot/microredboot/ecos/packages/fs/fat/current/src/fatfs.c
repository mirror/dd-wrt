//==========================================================================
//
//      fatfs.c
//
//      FAT file system
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):           Savin Zlobec <savin@elatec.si> (based on ramfs.c)
// Original data:       nickg
// Date:                2003-06-29
// Purpose:             FAT file system
// Description:         This is a FAT filesystem for eCos. 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/io_fileio.h>
#include <pkgconf/fs_fat.h>

#include <cyg/infra/cyg_type.h>   
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

#include <cyg/infra/diag.h>
#include <cyg/fileio/fileio.h>
#include <cyg/io/io.h>
#include <blib/blib.h>
#include <cyg/fs/fatfs.h>

#include "fatfs.h"

//==========================================================================
// Tracing support defines 

#ifdef FATFS_TRACE_FS_OP
# define TFS 1
#else
# define TFS 0
#endif

#ifdef FATFS_TRACE_FILE_OP
# define TFO 1
#else
# define TFO 0
#endif

//==========================================================================
// Forward definitions

// Filesystem operations
static int fatfs_mount  (cyg_fstab_entry *fste, cyg_mtab_entry *mte);
static int fatfs_umount (cyg_mtab_entry *mte);
static int fatfs_open   (cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         int mode, cyg_file *fte);
static int fatfs_unlink (cyg_mtab_entry *mte, cyg_dir dir, const char *name);
static int fatfs_mkdir  (cyg_mtab_entry *mte, cyg_dir dir, const char *name);
static int fatfs_rmdir  (cyg_mtab_entry *mte, cyg_dir dir, const char *name);
static int fatfs_rename (cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                         cyg_dir dir2, const char *name2 );
static int fatfs_link   (cyg_mtab_entry *mte, cyg_dir dir1, const char *name1,
                         cyg_dir dir2, const char *name2, int type );
static int fatfs_opendir(cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         cyg_file *fte );
static int fatfs_chdir  (cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         cyg_dir *dir_out );
static int fatfs_stat   (cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         struct stat *buf);
static int fatfs_getinfo(cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         int key, void *buf, int len );
static int fatfs_setinfo(cyg_mtab_entry *mte, cyg_dir dir, const char *name,
                         int key, void *buf, int len );

// File operations
static int fatfs_fo_read   (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatfs_fo_write  (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatfs_fo_lseek  (struct CYG_FILE_TAG *fp, off_t *pos, int whence );
static int fatfs_fo_ioctl  (struct CYG_FILE_TAG *fp, CYG_ADDRWORD com,
                            CYG_ADDRWORD data);
static int fatfs_fo_fsync  (struct CYG_FILE_TAG *fp, int mode );        
static int fatfs_fo_close  (struct CYG_FILE_TAG *fp);
static int fatfs_fo_fstat  (struct CYG_FILE_TAG *fp, struct stat *buf );
static int fatfs_fo_getinfo(struct CYG_FILE_TAG *fp, int key, 
                            void *buf, int len );
static int fatfs_fo_setinfo(struct CYG_FILE_TAG *fp, int key, 
                            void *buf, int len );

// Directory operations
static int fatfs_fo_dirread (struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio);
static int fatfs_fo_dirlseek(struct CYG_FILE_TAG *fp, off_t *pos, int whence);

//==========================================================================
// Filesystem table entries

// -------------------------------------------------------------------------
// Fstab entry.

FSTAB_ENTRY(fatfs_fste, "fatfs", 0,
            CYG_SYNCMODE_FILE_FILESYSTEM|CYG_SYNCMODE_IO_FILESYSTEM,
            fatfs_mount,
            fatfs_umount,
            fatfs_open,
            fatfs_unlink,
            fatfs_mkdir,
            fatfs_rmdir,
            fatfs_rename,
            fatfs_link,
            fatfs_opendir,
            fatfs_chdir,
            fatfs_stat,
            fatfs_getinfo,
            fatfs_setinfo);

// -------------------------------------------------------------------------
// File operations.

static cyg_fileops fatfs_fileops =
{
    fatfs_fo_read,
    fatfs_fo_write,
    fatfs_fo_lseek,
    fatfs_fo_ioctl,
    cyg_fileio_seltrue,
    fatfs_fo_fsync,
    fatfs_fo_close,
    fatfs_fo_fstat,
    fatfs_fo_getinfo,
    fatfs_fo_setinfo
};

// -------------------------------------------------------------------------
// Directory file operations.

static cyg_fileops fatfs_dirops =
{
    fatfs_fo_dirread,
    (cyg_fileop_write *)cyg_fileio_enosys,
    fatfs_fo_dirlseek,
    (cyg_fileop_ioctl *)cyg_fileio_enosys,
    cyg_fileio_seltrue,
    (cyg_fileop_fsync *)cyg_fileio_enosys,
    fatfs_fo_close,
    (cyg_fileop_fstat *)cyg_fileio_enosys,
    (cyg_fileop_getinfo *)cyg_fileio_enosys,
    (cyg_fileop_setinfo *)cyg_fileio_enosys
};

// -------------------------------------------------------------------------
// Directory search data
// Parameters for a directory search. The fields of this structure are
// updated as we follow a pathname through the directory tree.

typedef struct fatfs_dirsearch_s
{
    fatfs_disk_t        *disk;     // Disk info 
    fatfs_node_t        *dir;      // Directory to search
    const char          *path;     // Path to follow
    fatfs_node_t        *node;     // Node found
    const char          *name;     // Last name fragment used
    int                  namelen;  // Name fragment length
    cyg_bool             last;     // Last name in path?
} fatfs_dirsearch_t;

// -------------------------------------------------------------------------
// FATFS file descriptor data

typedef struct fatfs_fd_s
{
    fatfs_node_t      *node;
    fatfs_data_pos_t   pos;
} fatfs_fd_t;

static fatfs_fd_t  fatfs_fds_base[CYGNUM_FILEIO_NFD];
static fatfs_fd_t *fatfs_fds_pool[CYGNUM_FILEIO_NFD];
static cyg_uint32  fatfs_fds_free_cnt;

//==========================================================================

#if TFS
static void 
print_disk_info(fatfs_disk_t *disk)
{
    diag_printf("FAT: sector size:        %u\n", disk->sector_size);
    diag_printf("FAT: cluster size:       %u\n", disk->cluster_size);
    diag_printf("FAT: FAT table position: %u\n", disk->fat_tbl_pos);
    diag_printf("FAT: FAT table num ent:  %u\n", disk->fat_tbl_nents);
    diag_printf("FAT: FAT table size:     %u\n", disk->fat_tbl_size);
    diag_printf("FAT: FAT tables num:     %u\n", disk->fat_tbls_num);
    diag_printf("FAT: FAT root dir pos:   %u\n", disk->fat_root_dir_pos);
    diag_printf("FAT: FAT root dir nents: %u\n", disk->fat_root_dir_nents);
    diag_printf("FAT: FAT root dir size:  %u\n", disk->fat_root_dir_size);
    diag_printf("FAT: FAT data position:  %u\n", disk->fat_data_pos);
}
#endif

static void
init_fatfs_fds(void)
{
    static bool initialized = false;

    int i;

    if (initialized)
        return;
    
    initialized = true;
    
    for (i = 0; i < CYGNUM_FILEIO_NFD; i++)
    {
        fatfs_fds_pool[i] = &fatfs_fds_base[i];    
    }
    fatfs_fds_free_cnt = i;
}

static fatfs_fd_t *
alloc_fatfs_fd(fatfs_disk_t *disk, fatfs_node_t *node)
{
    fatfs_fd_t *fd = NULL;

    if (fatfs_fds_free_cnt > 0)
    {
        fd = fatfs_fds_pool[--fatfs_fds_free_cnt];

        fd->node = node;
        fatfs_initpos(disk, &node->dentry, &fd->pos);
    }
    
    return fd;
}

static void
free_fatfs_fd(fatfs_fd_t *fd)
{
    fatfs_fds_pool[fatfs_fds_free_cnt++] = fd;
}

static void
init_dirsearch(fatfs_dirsearch_t *ds,
               fatfs_disk_t      *disk, 
               fatfs_node_t      *dir,
               const char        *name)
{
    ds->disk = disk;
    
    if (NULL == dir)
        ds->dir = disk->root;
    else
        ds->dir = dir;
    
    ds->path    = name;
    ds->node    = ds->dir;
    ds->namelen = 0;
    ds->last    = false;
}

static int
find_direntry(fatfs_dirsearch_t *ds)
{
    fatfs_dir_entry_t  dentry;
    fatfs_data_pos_t   pos;
    int                err;

    CYG_TRACE1(TFS, "searching for dir entry '%s'", ds->name);

    // First check the cache
    
    ds->node = fatfs_node_find(ds->disk, 
                               ds->name, 
                               ds->namelen, 
                               ds->dir->dentry.cluster);

    if (ds->node != NULL)
    {
        // Dir entry found in cache
        
        CYG_TRACE0(TFS, "dir entry found in cache");

        fatfs_node_touch(ds->disk, ds->node);
        return ENOERR;
    }

    // Dir entry not in cache - search the current dir

    fatfs_initpos(ds->disk, &ds->dir->dentry, &pos);

    while (true)
    {  
        // Read next dir entry 
        
        err = fatfs_read_dir_entry(ds->disk, &ds->dir->dentry, &pos, &dentry);
        if (err != ENOERR)
            return (err == EEOF ? ENOERR : err);

        // Compare filenames
        
        if ('\0' == dentry.filename[ds->namelen] &&
               0 == strncasecmp(dentry.filename, ds->name, ds->namelen))
        {
            // Dir entry found - allocate new node and return

            CYG_TRACE0(TFS, "dir entry found");

            ds->node = fatfs_node_alloc(ds->disk, &dentry);
            if (NULL == ds->node)
                return EMFILE;

            return ENOERR;
        }
    }
}

static int 
find_entry(fatfs_dirsearch_t *ds)
{
    const char  *name     = ds->path;
    const char  *n        = name;
    char         namelen  = 0;
    int          err;
    
    if( !S_ISDIR(ds->dir->dentry.mode) )
    {
        CYG_TRACE1(TFS, "entry '%s' not dir", ds->dir->dentry.filename);
        return ENOTDIR;
    }
    
    // Isolate the next element of the path name
    while (*n != '\0' && *n != '/')
        n++, namelen++;

    // If we terminated on a NUL, set last flag
    if (*n == '\0')
        ds->last = true;

    // Update name in dirsearch object
    ds->name    = name;
    ds->namelen = namelen;

    err = find_direntry(ds);
    if (err != ENOERR)
        return err;

    CYG_TRACE2(TFS, "entry '%s' %s", name, (ds->node ? "found" : "not found"));
    
    if (ds->node != NULL)
       return ENOERR;
    else
       return ENOENT; 
}

static int
fatfs_find(fatfs_dirsearch_t *ds)
{
    int err;

    CYG_TRACE1(TFS, "find path='%s'", ds->path);
    
    // Short circuit empty paths
    if (*(ds->path) == '\0')
        return ENOERR;
    
    // Iterate down directory tree until we find the object we want
    for(;;)
    {
        err = find_entry(ds);
        
        if (err != ENOERR)
            return err;
        
        if (ds->last)
        {
            CYG_TRACE0(TFS, "entry found");
            return ENOERR;
        }

        // Update dirsearch object to search next directory
        ds->dir   = ds->node;
        ds->path += ds->namelen;
        
        // Skip dirname separators
        if (*(ds->path) == '/') ds->path++;
        
        CYG_TRACE1(TFS, "find path to go='%s'", ds->path);
    }
}

//==========================================================================
// Filesystem operations

// -------------------------------------------------------------------------
// fatfs_mount()
// Process a mount request. This mainly creates a root for the
// filesystem.

static int 
fatfs_mount(cyg_fstab_entry *fste, cyg_mtab_entry *mte)
{
    cyg_io_handle_t     dev_h;
    fatfs_disk_t       *disk;
    fatfs_dir_entry_t   root_dentry;
    Cyg_ErrNo           err;

    CYG_TRACE2(TFS, "mount fste=%p mte=%p", fste, mte);

    init_fatfs_fds();
    
    CYG_TRACE1(TFS, "looking up disk device '%s'", mte->devname);
    
    err = cyg_io_lookup(mte->devname, &dev_h);
    if (err != ENOERR)
        return err;

    disk = (fatfs_disk_t *)malloc(sizeof(fatfs_disk_t));
    if (NULL == disk)
        return ENOMEM;
        
    CYG_TRACE0(TFS, "initializing block cache"); 

    disk->bcache_mem = (cyg_uint8 *)malloc(CYGNUM_FS_FAT_BLOCK_CACHE_MEMSIZE);
    if (NULL == disk->bcache_mem)
    {
        free(disk);
        return ENOMEM;
    }
    // FIXME: get block size from disk device
    err = cyg_blib_io_create(dev_h, disk->bcache_mem, 
            CYGNUM_FS_FAT_BLOCK_CACHE_MEMSIZE, 512, &disk->blib);
    if (err != ENOERR)
    {
        free(disk->bcache_mem);
        free(disk);
        return err;
    }
    
    disk->dev_h = dev_h;

    CYG_TRACE0(TFS, "initializing disk");
    
    err = fatfs_init(disk);
    if (err != ENOERR)
    {
        cyg_blib_delete(&disk->blib);
        free(disk->bcache_mem);
        free(disk);
        return err;
    }
   
#if TFS    
    print_disk_info(disk);
#endif

    CYG_TRACE0(TFS, "initializing node cache");

    fatfs_node_cache_init(disk);
    
    CYG_TRACE0(TFS, "initializing root node");
    
    fatfs_get_root_dir_entry(disk, &root_dentry);
    
    disk->root = fatfs_node_alloc(disk, &root_dentry);

    fatfs_node_ref(disk, disk->root);
    
    mte->root = (cyg_dir)disk->root;
    mte->data = (CYG_ADDRWORD)disk;

    CYG_TRACE0(TFS, "disk mounted");

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_umount()
// Unmount the filesystem. This will currently only succeed if the
// filesystem is empty.

static int
fatfs_umount(cyg_mtab_entry *mte)
{
    fatfs_disk_t  *disk  = (fatfs_disk_t *) mte->data;
    fatfs_node_t  *root  = (fatfs_node_t *) mte->root;

    CYG_TRACE3(TFS, "umount mte=%p %d live nodes %d dead nodes", 
                    mte, fatfs_get_live_node_count(disk), 
                    fatfs_get_dead_node_count(disk));

    if (root->refcnt > 1)
        return EBUSY;
    
    if (fatfs_get_live_node_count(disk) != 1)
        return EBUSY;

    fatfs_node_unref(disk, root);
    fatfs_node_cache_flush(disk);
    // FIXME: cache delete can fail if cache can't be synced
    cyg_blib_delete(&disk->blib); 
    free(disk->bcache_mem);
    free(disk);
    
    mte->root = CYG_DIR_NULL;
    mte->data = (CYG_ADDRWORD) NULL;
    
    CYG_TRACE0(TFS, "disk umounted");
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_open()
// Open a file for reading or writing.

static int 
fatfs_open(cyg_mtab_entry *mte,
           cyg_dir         dir, 
           const char     *name,
           int             mode,  
           cyg_file       *file)
{
    fatfs_disk_t       *disk = (fatfs_disk_t *) mte->data;
    fatfs_node_t       *node = NULL;
    fatfs_fd_t         *fd;
    fatfs_dirsearch_t   ds;
    int                 err;

    CYG_TRACE5(TFS, "open mte=%p dir=%p name='%s' mode=%d file=%p", 
                    mte, dir, name, mode, file);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);

    if (err == ENOENT)
    {
        if (ds.last && (mode & O_CREAT))
        {
            fatfs_dir_entry_t new_file_dentry;
            
            // No node there, if the O_CREAT bit is set then we must
            // create a new one. The dir and name fields of the dirsearch
            // object will have been updated so we know where to put it.

            CYG_TRACE1(TFS, "creating new file '%s'", name); 

            err = fatfs_create_file(disk, 
                                    &ds.dir->dentry, 
                                    ds.name, 
                                    ds.namelen, 
                                    &new_file_dentry);
            if (err != ENOERR)
                return err;

            node = fatfs_node_alloc(disk, &new_file_dentry);
            if (NULL == node)
                return EMFILE;
           
            // Update directory times
            ds.dir->dentry.atime =
            ds.dir->dentry.mtime = cyg_timestamp();
            
            err = ENOERR;
        }
    }
    else if (err == ENOERR)
    {
        // The node exists. If the O_CREAT and O_EXCL bits are set, we
        // must fail the open

        if ((mode & (O_CREAT|O_EXCL)) == (O_CREAT|O_EXCL))
            err = EEXIST;
        else
            node = ds.node;
    }

    if (err == ENOERR && (mode & O_TRUNC))
    {
        // If the O_TRUNC bit is set we must clean out the file data
        CYG_TRACE0(TFS, "truncating file"); 
        fatfs_trunc_file(disk, &node->dentry);
    }

    if (err != ENOERR)
        return err;
    
    if (S_ISDIR(node->dentry.mode))
        return EISDIR;

#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    // if the file is read only and is opened for writing
    // fail with permission error
    if (S_FATFS_ISRDONLY(node->dentry.attrib) && (mode & O_WRONLY))
        return EACCES;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    // Allocate file object private data and
    // make a reference to this file node

    fd = alloc_fatfs_fd(disk, node);
    if (NULL == fd)
        return EMFILE;

    fatfs_node_ref(disk, node);

    // Initialize the file object

    if (mode & O_APPEND)
        fatfs_setpos(disk, &node->dentry, &fd->pos, node->dentry.size);  
    
    file->f_flag   |= mode & CYG_FILE_MODE_MASK;
    file->f_type    = CYG_FILE_TYPE_FILE;
    file->f_ops     = &fatfs_fileops;
    file->f_offset  = (mode & O_APPEND) ? node->dentry.size : 0;
    file->f_data    = (CYG_ADDRWORD) fd;
    file->f_xops    = 0;

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_unlink()
// Remove a file link from its directory.

static int 
fatfs_unlink(cyg_mtab_entry *mte, 
             cyg_dir         dir, 
             const char     *name)
{
    fatfs_disk_t       *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t   ds;
    int                 err;

    CYG_TRACE3(TFS, "unlink mte=%p dir=%p name='%s'", mte, dir, name);

    init_dirsearch(&ds, disk, (fatfs_node_t *)dir, name);

    err = fatfs_find(&ds);

    if (err != ENOERR)
        return err;

    if (ds.node->refcnt > 0)
        return EBUSY;
    
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    // if the file is read only fail with permission error
    if (S_FATFS_ISRDONLY(ds.node->dentry.attrib))
        return EPERM;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    err = fatfs_delete_file(disk, &ds.node->dentry);
    if (err == ENOERR)
        fatfs_node_free(disk, ds.node);
    
    return err;
}

// -------------------------------------------------------------------------
// fatfs_mkdir()
// Create a new directory.

static int
fatfs_mkdir(cyg_mtab_entry *mte, cyg_dir dir, const char *name)
{
    fatfs_disk_t       *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t   ds;
    int                 err;
    
    CYG_TRACE3(TFS, "mkdir mte=%p dir=%p name='%s'", mte, dir, name);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);

    if (err == ENOENT)
    {
        if (ds.last)
        {
            fatfs_dir_entry_t new_dir_dentry;
            
            // The entry does not exist, and it is the last element in
            // the pathname, so we can create it here

            err = fatfs_create_dir(disk, 
                                   &ds.dir->dentry, 
                                   ds.name, 
                                   ds.namelen, 
                                   &new_dir_dentry);
            if (err != ENOERR)
                return err;
            
            fatfs_node_alloc(disk, &new_dir_dentry);

            return ENOERR;
        }
    } 
    else if (err == ENOERR)
    {
        return EEXIST;
    }
    
    return err;
}

// -------------------------------------------------------------------------
// fatfs_rmdir()
// Remove a directory.

static int 
fatfs_rmdir(cyg_mtab_entry *mte, cyg_dir dir, const char *name)
{
    fatfs_disk_t      *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t  ds;
    int                err;

    CYG_TRACE3(TFS, "rmdir mte=%p dir=%p name='%s'", mte, dir, name);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);

    if (err != ENOERR)
        return err;
  
    if (!S_ISDIR(ds.node->dentry.mode))
        return EPERM;
 
    if (ds.node->refcnt > 0)
        return EBUSY;
    
    err = fatfs_delete_file(disk, &ds.node->dentry);
    if (err == ENOERR)
        fatfs_node_free(disk, ds.node);
    
    return err;
}

// -------------------------------------------------------------------------
// fatfs_rename()
// Rename a file/dir.

static int 
fatfs_rename(cyg_mtab_entry *mte, 
             cyg_dir         dir1, 
             const char     *name1,
             cyg_dir         dir2, 
             const char     *name2)
{
    fatfs_disk_t       *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t   ds1, ds2;
    int                 err;
 
    CYG_TRACE5(TFS, "rename mte=%p dir1=%p name1='%s' dir2=%p name2='%s'", 
                    mte, dir1, name1, dir2, name2);

    init_dirsearch(&ds1, disk, (fatfs_node_t *)dir1, name1);

    err = fatfs_find(&ds1);
    if (err != ENOERR)
        return err;

#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    // if the file is read only fail with permission error
    if (S_FATFS_ISRDONLY(ds1.node->dentry.attrib))
        return EPERM;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

    // Protect the found nodes from being reused 
    // by the search for the ds2 dir/node pair 
    fatfs_node_ref(disk, ds1.dir);
    fatfs_node_ref(disk, ds1.node);
    
    init_dirsearch(&ds2, disk, (fatfs_node_t *) dir2, name2);
   
    err = fatfs_find(&ds2);

    // Check if the target name already exists
    if (err == ENOERR && ds2.last)
    {
        err = EEXIST;  
        goto out;  
    }
    
    // Check if the target dir doesn't exist
    if (err == ENOENT && !ds2.last)
        goto out;
    
    // Check if the target and the source are the same
    if (ds1.node == ds2.node)
    {
        err = ENOERR;
        goto out;
    }
    
    err = fatfs_rename_file(disk, 
                            &ds1.dir->dentry, 
                            &ds1.node->dentry, 
                            &ds2.dir->dentry, 
                            ds2.name, 
                            ds2.namelen);  

    fatfs_node_rehash(disk, ds1.node);

out:
    // Unreference previousely protected nodes
    fatfs_node_unref(disk, ds1.dir);
    fatfs_node_unref(disk, ds1.node);
   
    if (err == ENOERR)
    {
        ds1.dir->dentry.atime =
        ds1.dir->dentry.mtime = 
        ds2.dir->dentry.atime = 
        ds2.dir->dentry.mtime = cyg_timestamp();    
    } 
    return err;
}

// -------------------------------------------------------------------------
// fatfs_link()
// Make a new directory entry for a file.

static int 
fatfs_link(cyg_mtab_entry *mte, 
           cyg_dir         dir1, 
           const char     *name1,
           cyg_dir         dir2, 
           const char     *name2, 
           int             type)
{
    CYG_TRACE6(TFS, "link mte=%p dir1=%p name1='%s' dir2=%p name2='%s' type=%d",
                    mte, dir1, name1, dir2, name2, type);

    // Linking not supported
    return EINVAL;
}

// -------------------------------------------------------------------------
// fatfs_opendir()
// Open a directory for reading.

static int
fatfs_opendir(cyg_mtab_entry *mte,
              cyg_dir         dir,
              const char     *name,
              cyg_file       *file)
{
    fatfs_disk_t       *disk = (fatfs_disk_t *) mte->data;
    fatfs_fd_t         *fd;
    fatfs_dirsearch_t   ds;
    int                 err;

    CYG_TRACE4(TFS, "opendir mte=%p dir=%p name='%s' file=%p", 
                    mte, dir, name, file);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);
    if (err != ENOERR)
        return err;

    if (!S_ISDIR(ds.node->dentry.mode)) 
        return ENOTDIR;

    // Allocate file object private data and
    // make a reference to this file node

    fd = alloc_fatfs_fd(disk, ds.node);
    if (NULL == fd)
        return EMFILE;
    
    fatfs_node_ref(disk, ds.node);
    
    // Initialize the file object

    file->f_type    = CYG_FILE_TYPE_FILE;
    file->f_ops     = &fatfs_dirops;
    file->f_data    = (CYG_ADDRWORD) fd;
    file->f_xops    = 0;
    file->f_offset  = 0;

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_chdir()
// Change directory support.

static int
fatfs_chdir(cyg_mtab_entry *mte,
            cyg_dir         dir,
            const char     *name,
            cyg_dir        *dir_out)
{
    fatfs_disk_t *disk = (fatfs_disk_t *) mte->data;

    CYG_TRACE4(TFS, "chdir mte=%p dir=%p dir_out=%p name=%d", 
                    mte, dir, dir_out, name);

    if (dir_out != NULL)
    {
        // This is a request to get a new directory pointer in *dir_out
        
        fatfs_dirsearch_t ds;
        int               err;

        init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

        err = fatfs_find(&ds);
        if (err != ENOERR)
            return err;

        if (!S_ISDIR(ds.node->dentry.mode))
            return ENOTDIR;

        if (ds.node != disk->root)
            fatfs_node_ref(disk, ds.node);
        
        *dir_out = (cyg_dir) ds.node;
    }
    else
    {
        // If no output dir is required, this means that the mte and
        // dir arguments are the current cdir setting and we should
        // forget this fact.

        fatfs_node_t *node = (fatfs_node_t *) dir;

        if (node != disk->root)
            fatfs_node_unref(disk, node);
    }

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_stat()
// Get struct stat info for named object.

static int
fatfs_stat(cyg_mtab_entry *mte,
           cyg_dir         dir,
           const char     *name,
           struct stat    *buf)
{
    fatfs_disk_t      *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t  ds;
    int                err;

    CYG_TRACE4(TFS, "stat mte=%p dir=%p name='%s' buf=%p", 
                    mte, dir, name, buf);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);
    if (err != ENOERR)
        return err;

    // Fill in the status
    
    buf->st_mode   = ds.node->dentry.mode;
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    if (!S_FATFS_ISRDONLY(ds.node->dentry.attrib))
        buf->st_mode |= (S_IWUSR | S_IWGRP | S_IWOTH);
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES
    buf->st_ino    = (ino_t) ds.node->dentry.cluster;
    buf->st_dev    = 0;
    buf->st_nlink  = 1;
    buf->st_uid    = 0;
    buf->st_gid    = 0;
    buf->st_size   = ds.node->dentry.size;
    buf->st_atime  = ds.node->dentry.atime;
    buf->st_mtime  = ds.node->dentry.mtime;
    buf->st_ctime  = ds.node->dentry.ctime;

    return ENOERR;
}

#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
// -------------------------------------------------------------------------
// fatfs_set_attrib()
// Set FAT file system attributes for specified file

static int
fatfs_set_attrib(cyg_mtab_entry        *mte,
                 cyg_dir                dir,
                 const char            *name,
                 const cyg_fs_attrib_t  new_attrib)
{
    fatfs_disk_t      *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t  ds;
    int                err;

    CYG_TRACE4(TFS, "chmod mte=%p dir=%p name='%s' buf=%x", 
                    mte, dir, name, new_attrib);

    // Verify new_mode is valid
    if ((new_attrib & S_FATFS_ATTRIB) != new_attrib)
        return EINVAL;
    
    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);
    if (err != ENOERR)
        return err;

    // Change the "changeable" mode bits for the file.
    ds.node->dentry.attrib = 
      (ds.node->dentry.attrib & (~S_FATFS_ATTRIB)) | new_attrib;

    return fatfs_write_dir_entry(disk,&ds.node->dentry);
}

// -------------------------------------------------------------------------
// fatfs_get_attrib()
// Set FAT file system attributes for specified file

static int
fatfs_get_attrib(cyg_mtab_entry  *mte,
                 cyg_dir          dir,
                 const char      *name,
                 cyg_fs_attrib_t * const file_attrib)
{
    fatfs_disk_t      *disk = (fatfs_disk_t *) mte->data;
    fatfs_dirsearch_t  ds;
    int                err;

    CYG_TRACE4(TFS, "chmod mte=%p dir=%p name='%s' buf=%x", 
                    mte, dir, name, new_attrib);

    init_dirsearch(&ds, disk, (fatfs_node_t *) dir, name);

    err = fatfs_find(&ds);
    if (err != ENOERR)
        return err;

    // Get the attribute field
    CYG_CHECK_DATA_PTR(file_attrib,"Invalid destination attribute pointer");
    *file_attrib = ds.node->dentry.attrib;

    return ENOERR;
}
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES

// -------------------------------------------------------------------------
// fatfs_getinfo()
// Getinfo. Support for attrib

static int 
fatfs_getinfo(cyg_mtab_entry *mte, 
              cyg_dir         dir, 
              const char     *name,
              int             key, 
              void           *buf, 
              int             len)
{
    int err = EINVAL;

    CYG_TRACE6(TFS, "getinfo mte=%p dir=%p name='%s' key=%d buf=%p len=%d",
                    mte, dir, name, key, buf, len);
    switch( key )
    {
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
        case FS_INFO_ATTRIB:
            err = fatfs_get_attrib(mte, dir, name, (cyg_fs_attrib_t*)buf);
            break;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES
        default:
            err = EINVAL;
            break;
    }
    return err;
}

// -------------------------------------------------------------------------
// fatfs_setinfo()
// Setinfo. Support for fssync and attrib

static int 
fatfs_setinfo(cyg_mtab_entry *mte, 
              cyg_dir         dir, 
              const char     *name,
              int             key, 
              void           *buf, 
              int             len)
{
    int err = EINVAL;

    CYG_TRACE6(TFS, "setinfo mte=%p dir=%p name='%s' key=%d buf=%p len=%d",
                    mte, dir, name, key, buf, len);

    switch( key )
    {
        case FS_INFO_SYNC:
            err = cyg_blib_sync(&(((fatfs_disk_t *) mte->data)->blib));
            break;
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
        case FS_INFO_ATTRIB:
            err = fatfs_set_attrib(mte, dir, name, *(cyg_fs_attrib_t *)buf);
            break;
#endif // CYGCFG_FS_FAT_USE_ATTRIBUTES
        default:
            err = EINVAL;
            break;
    }
    return err;
}

//==========================================================================
// File operations

// -------------------------------------------------------------------------
// fatfs_fo_read()
// Read data from the file.

static int 
fatfs_fo_read(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    fatfs_disk_t  *disk   = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd     = (fatfs_fd_t *)   fp->f_data;
    fatfs_node_t  *node   = fd->node;
    cyg_uint32     pos    = fp->f_offset;
    ssize_t        resid  = uio->uio_resid;
    int            i;

    CYG_TRACE3(TFO, "read fp=%p uio=%p pos=%d", fp, uio, pos);

    // Loop over the io vectors until there are none left

    for (i = 0; i < uio->uio_iovcnt; i++)
    {
        cyg_iovec  *iov  = &uio->uio_iov[i];
        char       *buf  = (char *) iov->iov_base;
        off_t       len  = iov->iov_len;

        // Loop over each vector filling it with data from the file
        
        while (len > 0 && pos < node->dentry.size)
        {
            cyg_uint32 l = len;
            int        err;

            // Adjust size to end of file if necessary
            if (l > node->dentry.size-pos)
                l = node->dentry.size-pos;

            err = fatfs_read_data(disk, &node->dentry, &fd->pos, buf, &l);
            if (err != ENOERR)
                return err;

            // Update working vars

            len   -= l;
            buf   += l;
            pos   += l;
            resid -= l;
        }
    }

    // We successfully read some data, update the access time, 
    // file offset and transfer residue

    node->dentry.atime = cyg_timestamp(); 
    uio->uio_resid     = resid;
    fp->f_offset       = (off_t) pos;

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_fo_write()
// Write data to file.

static int 
fatfs_fo_write(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    fatfs_disk_t  *disk   = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd     = (fatfs_fd_t *)   fp->f_data;
    fatfs_node_t  *node   = fd->node;
    cyg_uint32     pos    = fp->f_offset;
    ssize_t        resid  = uio->uio_resid;
    int            err    = ENOERR;
    int            i;

    CYG_TRACE3(TFO, "write fp=%p uio=%p pos=%d", fp, uio, pos);
    
    // If the APPEND mode bit was supplied, force all writes to
    // the end of the file
    if (fp->f_flag & CYG_FAPPEND)
    {
        fatfs_setpos(disk, &node->dentry, &fd->pos, node->dentry.size);
        pos = fp->f_offset = node->dentry.size;
    }
    
    // Check that pos is within current file size, or at the very end
    if (pos < 0 || pos > node->dentry.size)
        return EINVAL;
   
    // Now loop over the iovecs until they are all done, or we get an error
    
    for (i = 0; i < uio->uio_iovcnt; i++)
    {
        cyg_iovec  *iov  = &uio->uio_iov[i];
        char       *buf  = (char *) iov->iov_base;
        off_t       len  = iov->iov_len;
 
        // Loop over the vector writing it to the file 
        // until it has all been done

        while (len > 0)
        {
            cyg_uint32 l = len;

            err = fatfs_write_data(disk, &node->dentry, &fd->pos, buf, &l);

            // Update working vars

            len   -= l;
            buf   += l;
            pos   += l;
            resid -= l;

            // Stop writing if there is no more space in the file
            if (err == ENOSPC)
                break;
            
            if (err != ENOERR)
                return err;
        }
    }        

    // We wrote some data successfully, update the modified and access
    // times of the node, increase its size appropriately, and update
    // the file offset and transfer residue.

    node->dentry.mtime = 
    node->dentry.atime = cyg_timestamp();

    if (pos > node->dentry.size)
        node->dentry.size = pos;

    uio->uio_resid = resid;
    fp->f_offset   = (off_t) pos;

    return err;
}

// -------------------------------------------------------------------------
// fatfs_fo_lseek()
// Seek to a new file position.

static int 
fatfs_fo_lseek(struct CYG_FILE_TAG *fp, off_t *apos, int whence)
{
    fatfs_disk_t  *disk  = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd    = (fatfs_fd_t *)   fp->f_data;
    off_t          pos   = *apos;
    int            err;
    
    CYG_TRACE3(TFO, "lseek fp=%p pos=%d whence=%d", fp, fp->f_offset, whence);

    switch (whence)
    {
        case SEEK_SET:
            // Pos is already where we want to be
            break;
         case SEEK_CUR:
            // Add pos to current offset
            pos += fp->f_offset;
            break;
         case SEEK_END:
            // Add pos to file size
            pos += fd->node->dentry.size;
            break;
         default:
            return EINVAL;
    }

    // Check that pos is still within current file size, 
    // or at the very end
    if (pos < 0 || pos > fd->node->dentry.size)
        return EINVAL;
  
    // All OK, set fp offset and return new position
    
    err = fatfs_setpos(disk, &fd->node->dentry, &fd->pos, pos);

    if (ENOERR == err)
        *apos = fp->f_offset = pos;
    
    CYG_TRACE2(TFO, "lseek fp=%p new pos=%d", fp, *apos);

    return err;
}

// -------------------------------------------------------------------------
// fatfs_fo_ioctl()
// Handle ioctls. Currently none are defined.

static int 
fatfs_fo_ioctl(struct CYG_FILE_TAG *fp, CYG_ADDRWORD com, CYG_ADDRWORD data)
{
    CYG_TRACE3(TFO, "ioctl fp=%p com=%x data=%x", fp, com, data);
    return EINVAL;
}

// -------------------------------------------------------------------------
// fatfs_fo_fsync().
// Force the file out to data storage.

static int 
fatfs_fo_fsync(struct CYG_FILE_TAG *fp, int mode)
{
    fatfs_disk_t  *disk  = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd    = (fatfs_fd_t *)   fp->f_data;
    fatfs_node_t  *node  = fd->node;
    int            err;
    
    CYG_TRACE2(TFO, "fsync fp=%p mode=%d", fp, mode);

    err = fatfs_write_dir_entry(disk, &node->dentry);

    if (ENOERR == err)
        err = cyg_blib_sync(&disk->blib);
    
    return err;
}

// -------------------------------------------------------------------------
// fatfs_fo_close()
// Close a file.

static int
fatfs_fo_close(struct CYG_FILE_TAG *fp)
{    
    fatfs_disk_t  *disk  = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd    = (fatfs_fd_t *)   fp->f_data;
    fatfs_node_t  *node  = fd->node;
    int            err   = ENOERR;

    CYG_TRACE1(TFO, "close fp=%p", fp);

    // Write file attributes to disk, unreference 
    // the file node and free its private data

    if (node != disk->root)
        err = fatfs_write_dir_entry(disk, &node->dentry);

    fatfs_node_unref(disk, node);    

    free_fatfs_fd(fd);
    
    return err;
}

// -------------------------------------------------------------------------
// fatfs_fo_fstat()
// Get file status.

static int
fatfs_fo_fstat(struct CYG_FILE_TAG *fp, struct stat *buf)
{
    fatfs_fd_t    *fd    = (fatfs_fd_t *) fp->f_data;
    fatfs_node_t  *node  = fd->node;
    
    CYG_TRACE2(TFO, "fstat fp=%p buf=%p", fp, buf);

    // Fill in the status

    buf->st_mode   = node->dentry.mode;
    buf->st_ino    = (ino_t) node->dentry.cluster;
    buf->st_dev    = 0;
    buf->st_nlink  = 1;
    buf->st_uid    = 0;
    buf->st_gid    = 0;
    buf->st_size   = node->dentry.size;
    buf->st_atime  = node->dentry.atime;
    buf->st_mtime  = node->dentry.mtime;
    buf->st_ctime  = node->dentry.ctime;

    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_fo_getinfo()
// Get info.

static int
fatfs_fo_getinfo(struct CYG_FILE_TAG *fp, int key, void *buf, int len)
{
    CYG_TRACE4(TFO, "getinfo fp=%p key=%d buf=%p len=%d", fp, key, buf, len);
    return EINVAL;
}

// -------------------------------------------------------------------------
// fatfs_fo_setinfo()
// Set info.

static int
fatfs_fo_setinfo(struct CYG_FILE_TAG *fp, int key, void *buf, int len)
{
    CYG_TRACE4(TFO, "setinfo fp=%p key=%d buf=%p len=%d", fp, key, buf, len);
    return EINVAL;
}

//==========================================================================
// Directory operations

// -------------------------------------------------------------------------
// fatfs_fo_dirread()
// Read a single directory entry from a file.

static int 
fatfs_fo_dirread(struct CYG_FILE_TAG *fp, struct CYG_UIO_TAG *uio)
{
    fatfs_disk_t      *disk    = (fatfs_disk_t *)  fp->f_mte->data;
    fatfs_fd_t        *fd      = (fatfs_fd_t *)    fp->f_data;
    struct dirent     *ent     = (struct dirent *) uio->uio_iov[0].iov_base;
    char              *nbuf    = ent->d_name;
    off_t              len     = uio->uio_iov[0].iov_len;
    fatfs_dir_entry_t  dentry;
    int                err;
    
    CYG_TRACE3(TFO, "dirread fp=%p uio=%p pos=%d", fp, uio, fp->f_offset);

    if (len < sizeof(struct dirent))
        return EINVAL;

    err = fatfs_read_dir_entry(disk, &fd->node->dentry, &fd->pos, &dentry);

    if (err != ENOERR)
        return (err == EEOF ? ENOERR : err);

    strcpy(nbuf, dentry.filename);

    fd->node->dentry.atime  = cyg_timestamp();
    uio->uio_resid         -= sizeof(struct dirent);
    fp->f_offset++;
    
    return ENOERR;
}

// -------------------------------------------------------------------------
// fatfs_fo_dirlseek()
// Seek directory to start.

static int 
fatfs_fo_dirlseek(struct CYG_FILE_TAG *fp, off_t *pos, int whence)
{
    fatfs_disk_t  *disk  = (fatfs_disk_t *) fp->f_mte->data;
    fatfs_fd_t    *fd    = (fatfs_fd_t *)   fp->f_data;
    int            err;
    
    CYG_TRACE2(TFO, "dirlseek fp=%p whence=%d", fp, whence);

    // Only allow SEEK_SET to zero
    
    if (whence != SEEK_SET || *pos != 0)
        return EINVAL;
   
    err = fatfs_setpos(disk, &fd->node->dentry, &fd->pos, 0);

    if (ENOERR == err)
        *pos = fp->f_offset = 0;
    
    return err;
}

// -------------------------------------------------------------------------
// EOF fatfs.c
