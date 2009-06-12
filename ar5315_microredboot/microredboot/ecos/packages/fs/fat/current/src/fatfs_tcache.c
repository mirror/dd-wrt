//==========================================================================
//
//      fatfs_tcache.c
//
//      FAT file system FAT table cache functions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Savin Zlobec 
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
// Author(s):           savin 
// Date:                2003-06-27
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/fs_fat.h>
#include <pkgconf/infra.h>
#include <cyg/kernel/kapi.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <sys/types.h>

#include "fatfs.h"

//==========================================================================
// Defines & macros 

#define TC_INC FATFS_FAT_TABLE_CACHE_INCREMENT

#ifdef CYGDBG_USE_ASSERTS
# define USE_ASSERTS 1
#endif

#ifdef FATFS_TRACE_FAT_TABLE_CACHE
# define TTC 1
#else
# define TTC 0
#endif

//==========================================================================
// Private functions 

//--------------------------------------------------------------------------
// tcache_increment()
// Allocates or reallocates memory for FAT table cache.
// Returns true is allocation succeded. 

static bool 
tcache_increment(fatfs_disk_t *disk, fatfs_tcache_t *tcache)
{
    CYG_TRACE2(TTC, "max_size=%d size=%d", tcache->max_size, tcache->size);

    if (NULL == tcache->clusters)
    {
        tcache->clusters = 
            (cyg_uint32 *)cyg_mempool_var_try_alloc(disk->tcache_mpool_h, 
                                                    TC_INC*sizeof(cyg_uint32));

        if (NULL == tcache->clusters)
            return false;

        tcache->max_size = TC_INC;
    }
    else
    {
        cyg_uint32 *newc;

        newc = (cyg_uint32 *)cyg_mempool_var_try_alloc(disk->tcache_mpool_h,
            (tcache->max_size+TC_INC)*sizeof(cyg_uint32));

        if (NULL == newc)
            return false;

        memcpy(newc, tcache->clusters, (tcache->max_size*sizeof(cyg_uint32)));
        cyg_mempool_var_free(disk->tcache_mpool_h, tcache->clusters);
        
        tcache->clusters = newc;
        tcache->max_size += TC_INC;
    }

    CYG_TRACE2(TTC, "max_size=%d size=%d", tcache->max_size, tcache->size);

    return true;
}

//==========================================================================
// Exported functions 

//--------------------------------------------------------------------------
// fatfs_tcache_create()
// Creates FAT table caches memory pool.

int  
fatfs_tcache_create(fatfs_disk_t *disk, cyg_uint32 mem_size)
{
    disk->tcache_mem = (cyg_uint8 *)malloc(mem_size);
    if (NULL == disk->tcache_mem)
        return ENOMEM;
    
    cyg_mempool_var_create(disk->tcache_mem, mem_size, 
                           &disk->tcache_mpool_h, &disk->tcache_mpool);
    return ENOERR;
}

//--------------------------------------------------------------------------
// fatfs_tcache_delete()
// Deletes FAT table caches memory pool.

void 
fatfs_tcache_delete(fatfs_disk_t *disk)
{
    cyg_mempool_var_delete(disk->tcache_mpool_h);
    free(disk->tcache_mem);
}

//--------------------------------------------------------------------------
// fatfs_tcache_init()
// Initializes FAT table cache structure.

void
fatfs_tcache_init(fatfs_disk_t *disk, fatfs_tcache_t *tcache)
{
    CYG_CHECK_DATA_PTRC(tcache);

    tcache->clusters = NULL;
    tcache->size     = 0;
    tcache->max_size = 0;
}

//--------------------------------------------------------------------------
// fatfs_tcache_flush()
// Frees given tcache.

void
fatfs_tcache_flush(fatfs_disk_t *disk, fatfs_tcache_t *tcache)
{
    CYG_CHECK_DATA_PTRC(tcache);

    if (tcache->clusters != NULL)
    {
        cyg_mempool_var_free(disk->tcache_mpool_h, tcache->clusters);
        tcache->clusters = NULL;    
        tcache->size     = 0;
        tcache->max_size = 0;
    }
}

//--------------------------------------------------------------------------
// fatfs_tcache_get()
// Gets the cluster from cache at given position.
// Returns true if cluster in cache. 

bool
fatfs_tcache_get(fatfs_disk_t   *disk,
                 fatfs_tcache_t *tcache, 
                 cyg_uint32      num, 
                 cyg_uint32     *cluster)
{
    CYG_CHECK_DATA_PTRC(tcache);
    CYG_TRACE2(TTC, "size=%d max_size=%d", tcache->size, tcache->max_size);
    
    // Check if requested cluster is cached
    if (num >= tcache->size)
    {
        CYG_TRACE1(TTC, "cluster num=%d not in tcache", num);
        return false;
    }
    
    *cluster = tcache->clusters[num];  
    CYG_TRACE2(TTC, "got cluster=%d num=%d from tcache", *cluster, num);
    return true;
}

//--------------------------------------------------------------------------
// fatfs_tcache_get()
// Gets last cluster in cache.
// Returns false if cache is empty.

bool
fatfs_tcache_get_last(fatfs_disk_t   *disk,
                      fatfs_tcache_t *tcache, 
                      cyg_uint32     *num, 
                      cyg_uint32     *cluster)
{
    CYG_CHECK_DATA_PTRC(tcache);
    CYG_TRACE2(TTC, "size=%d max_size=%d", tcache->size, tcache->max_size);
    
    // Check if we have something in cache
    if (tcache->size == 0)
        return false;
   
    *num = tcache->size - 1; 
    *cluster = tcache->clusters[*num];  
    CYG_TRACE2(TTC, "got last cluster=%d num=%d from tcache", *cluster, *num);
    return true;
}

//--------------------------------------------------------------------------
// fatfs_tcache_set()
// Sets given cluster in cache at given position.
// There can't be any holes in cache (ie in order to set cluster
// at position N all clusters from 0 to N-1 must be set before)
// Returns true if set succeded.

bool
fatfs_tcache_set(fatfs_disk_t   *disk,
                 fatfs_tcache_t *tcache, 
                 cyg_uint32      num, 
                 cyg_uint32      cluster)
{
    CYG_CHECK_DATA_PTRC(tcache);
    CYG_TRACE4(TTC, "num=%d cluster=%d tcache size=%d max_size=%d", 
        num, cluster, tcache->size, tcache->max_size);

    if (num > tcache->size)
    {
        CYG_TRACE0(TTC, "won't make a hole in tcache");
        return false;
    }
    
    if (num < tcache->size)
    {
        CYG_TRACE2(TTC, "set cluster=%d num=%d in tcache", cluster, num);
        tcache->clusters[num] = cluster;
        return true;
    }

    // Here num is equal to size - check if we need to increment cache 
    if (tcache->size == tcache->max_size)
    {
        // If we can't get memory to increment the cache
        // try to flush dead nodes fat table cache
        if (!tcache_increment(disk, tcache))
        {
            fatfs_node_flush_dead_tcache(disk); 
            if (!tcache_increment(disk, tcache))
                return false;
        }
    }
    
    tcache->size++;
    tcache->clusters[num] = cluster;

    CYG_TRACE2(TTC, "added cluster=%d num=%d to tcache", cluster, num);

    return true;
}

// -------------------------------------------------------------------------
// EOF fatfs_tcache.c
