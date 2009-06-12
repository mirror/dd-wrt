#ifndef CYGONCE_FATFS_FATFS_H
#define CYGONCE_FATFS_FATFS_H
//==========================================================================
//
//      fatfs.h
//
//      FAT file system header 
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           Savin Zlobec <savin@elatec.si> 
// Date:                2003-06-29
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/fs_fat.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/io/io.h>
#include <cyg/fileio/fileio.h>
#include <blib/blib.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include <stdlib.h>
#include <string.h>

// --------------------------------------------------------------------------

#define FATFS_HASH_TABLE_SIZE CYGNUM_FS_FAT_NODE_HASH_TABLE_SIZE 

#define FATFS_NODE_POOL_SIZE  CYGNUM_FS_FAT_NODE_POOL_SIZE

#ifdef CYGDBG_FS_FAT_NODE_CACHE_EXTRA_CHECKS
# define FATFS_NODE_CACHE_EXTRA_CHECKS 1
#endif

// --------------------------------------------------------------------------

// Node cache tracing support
//#define FATFS_TRACE_NODE_CACHE 1

// FAT dir entry operations tracing support 
//#define FATFS_TRACE_DIR_ENTRY 1

// FAT clusters operations tracing support 
//#define FATFS_TRACE_CLUSTER 1

// FAT data tracing support 
//#define FATFS_TRACE_DATA 1

// FAT file operations tracing support 
//#define FATFS_TRACE_FILE_OP 1

// FAT filesystem operations tracing support 
//#define FATFS_TRACE_FS_OP 1

// --------------------------------------------------------------------------

typedef enum fatfs_type_e
{
    FATFS_FAT12 = 0,    
    FATFS_FAT16,         
    FATFS_FAT32         
} fatfs_type_t;

typedef struct fatfs_data_pos_s
{
    cyg_uint32 cluster;      // Cluster number
    cyg_uint32 cluster_snum; // Cluster file seq number 
                             // (0 - first cluster of file,
                             //  1 - second cluster of file, ...)  
    cyg_uint32 cluster_pos;  // Position inside cluster
} fatfs_data_pos_t;

typedef struct fatfs_dir_entry_s
{
    char              filename[12+1]; // File name
    mode_t            mode;           // Node type
    size_t            size;           // Size of file in bytes
    time_t            ctime;          // Creation timestamp
    time_t            atime;          // Last access timestamp
    time_t            mtime;          // Last write timestamp
    cyg_uint8         priv_data;      // Private data
    cyg_uint32        cluster;        // First cluster number
    cyg_uint32        parent_cluster; // First cluster of parent dentry
    fatfs_data_pos_t  disk_pos;       // Position of dir entry on disk
#ifdef CYGCFG_FS_FAT_USE_ATTRIBUTES
    cyg_fs_attrib_t    attrib;     // Attribute bits for DOS compatability
#endif //CYGCFG_FS_FAT_USE_ATTRIBUTES
} fatfs_dir_entry_t;

typedef struct fatfs_node_s
{
    fatfs_dir_entry_t    dentry;     // Dir entry data
    cyg_ucount32         refcnt;     // Open file/current dir references

    struct fatfs_node_s *list_prev;  // Next node in list
    struct fatfs_node_s *list_next;  // Prev node in list
    struct fatfs_node_s *hash_next;  // Next node in hash
} fatfs_node_t;

typedef struct fatfs_hash_table_s 
{
    cyg_uint32     size;                         // Number of slots
    cyg_uint32     n;                            // Number of nodes 
    fatfs_node_t  *nodes[FATFS_HASH_TABLE_SIZE]; // Nodes slots
} fatfs_hash_table_t;

typedef struct fatfs_node_list_s
{
    cyg_uint32    size;          // Number of nodes in list
    fatfs_node_t *first;         // First node in list
    fatfs_node_t *last;          // Last node in list
} fatfs_node_list_t;

typedef struct fatfs_disk_s
{
    cyg_uint32    sector_size;          // Sector size in bytes
    cyg_uint32    sector_size_log2;     // Sector size log2
    cyg_uint32    cluster_size;         // Cluster size in bytes
    cyg_uint32    cluster_size_log2;    // Cluster size log2 
    cyg_uint32    fat_tbl_pos;          // Position of the first FAT table
    cyg_uint32    fat_tbl_size;         // FAT table size in bytes
    cyg_uint32    fat_tbl_nents;        // Number of entries in FAT table
    cyg_uint32    fat_tbls_num;         // Number of FAT tables
    cyg_uint32    fat_root_dir_pos;     // Position of the root dir
    cyg_uint32    fat_root_dir_size;    // Root dir size in bytes 
    cyg_uint32    fat_root_dir_nents;   // Max number of entries in root dir
    cyg_uint32    fat_root_dir_cluster; // Cluster number of root dir (FAT32) 
    cyg_uint32    fat_data_pos;         // Position of data area
    fatfs_type_t  fat_type;             // Type of FAT - 12, 16 or 32 
    
    cyg_io_handle_t  dev_h;           // Disk device handle
    fatfs_node_t    *root;            // Root dir node

    cyg_uint8       *bcache_mem;      // Block cache memory base
    cyg_blib_t       blib;            // Block cache and access library instance

    fatfs_node_t  node_pool_base[FATFS_NODE_POOL_SIZE]; // Node pool base   
    fatfs_node_t *node_pool[FATFS_NODE_POOL_SIZE];      // Node pool 
    cyg_uint32    node_pool_free_cnt;                   // Node pool free cnt
    
    fatfs_node_list_t  live_nlist;    // List of nodes with refcnt > 0
    fatfs_node_list_t  dead_nlist;    // List of nodes with refcnt == 0
    fatfs_hash_table_t node_hash;     // Hash of nodes in live and dead lists
} fatfs_disk_t;

// --------------------------------------------------------------------------

int  fatfs_init(fatfs_disk_t *disk);

void fatfs_get_root_dir_entry(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry);

bool fatfs_is_root_dir_dentry(fatfs_dir_entry_t *dentry);

int  fatfs_get_disk_usage(fatfs_disk_t *disk,
                          cyg_uint32   *total_clusters,
                          cyg_uint32   *free_clusters);

int  fatfs_initpos(fatfs_disk_t      *disk,
                   fatfs_dir_entry_t *file,
                   fatfs_data_pos_t  *pos);

int  fatfs_setpos(fatfs_disk_t      *disk,
                  fatfs_dir_entry_t *file,
                  fatfs_data_pos_t  *pos,
                  cyg_uint32         offset);

cyg_uint32 fatfs_getpos(fatfs_disk_t      *disk, 
                        fatfs_dir_entry_t *file,
                        fatfs_data_pos_t  *pos);

int  fatfs_read_dir_entry(fatfs_disk_t      *disk,
                          fatfs_dir_entry_t *dir,
                          fatfs_data_pos_t  *pos,
                          fatfs_dir_entry_t *dentry);

int  fatfs_write_dir_entry(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry);

int  fatfs_delete_file(fatfs_disk_t *disk, fatfs_dir_entry_t *file);

int  fatfs_create_file(fatfs_disk_t      *disk, 
                       fatfs_dir_entry_t *dir, 
                       const char        *name,
                       int                namelen,
                       fatfs_dir_entry_t *dentry);

int  fatfs_create_dir(fatfs_disk_t      *disk, 
                      fatfs_dir_entry_t *dir, 
                      const char        *name,
                      int                namelen,
                      fatfs_dir_entry_t *dentry);

int  fatfs_trunc_file(fatfs_disk_t *disk, fatfs_dir_entry_t *file);

int  fatfs_rename_file(fatfs_disk_t      *disk,
                       fatfs_dir_entry_t *dir1,
                       fatfs_dir_entry_t *target,
                       fatfs_dir_entry_t *dir2,
                       const char        *name,
                       int                namelen);

int  fatfs_read_data(fatfs_disk_t      *disk,
                     fatfs_dir_entry_t *file,
                     fatfs_data_pos_t  *pos,
                     void              *data,
                     cyg_uint32        *len);

int  fatfs_write_data(fatfs_disk_t      *disk,
                      fatfs_dir_entry_t *file,
                      fatfs_data_pos_t  *pos,
                      void              *data,
                      cyg_uint32        *len);

// --------------------------------------------------------------------------

void fatfs_node_cache_init(fatfs_disk_t *disk);

void fatfs_node_cache_flush(fatfs_disk_t *disk);

fatfs_node_t *fatfs_node_alloc(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry);

void fatfs_node_ref(fatfs_disk_t *disk, fatfs_node_t *node);

void fatfs_node_unref(fatfs_disk_t *disk, fatfs_node_t *node);

void fatfs_node_touch(fatfs_disk_t *disk, fatfs_node_t *node);

void fatfs_node_rehash(fatfs_disk_t *disk, fatfs_node_t *node);

void fatfs_node_free(fatfs_disk_t *disk, fatfs_node_t *node);

fatfs_node_t* fatfs_node_find(fatfs_disk_t *disk, 
                              const char   *name, 
                              unsigned int  namelen, 
                              cyg_uint32    parent_cluster);

int  fatfs_get_live_node_count(fatfs_disk_t *disk);

int  fatfs_get_dead_node_count(fatfs_disk_t *disk);

// --------------------------------------------------------------------------
// Support routines
// These enable the definition of local versions of certain routines
// if the given packages are not present.

#ifndef CYGPKG_LIBC_I18N

__externC int toupper( int c );

#endif

#ifndef CYGFUN_LIBC_STRING_BSD_FUNCS

__externC int strcasecmp( const char *s1, const char *s2 );

__externC int strncasecmp( const char *s1, const char *s2, size_t n );

#endif

// --------------------------------------------------------------------------

#endif // CYGONCE_FATFS_FATFS_H

// --------------------------------------------------------------------------
// EOF fatfs.h
