//==========================================================================
//
//      fatfs_ncache.c
//
//      FAT file system node cache functions
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
// Date:                2003-06-26
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/fs_fat.h>
#include <pkgconf/infra.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>
#include <sys/types.h>
#include <ctype.h>

#include "fatfs.h"

//==========================================================================
// Defines & macros 

#ifdef CYGDBG_USE_ASSERTS
# define USE_ASSERTS 1
#endif

#ifdef FATFS_NODE_CACHE_EXTRA_CHECKS
# define USE_XCHECKS 1
#endif

#ifdef FATFS_TRACE_NODE_CACHE
# define TNC 1
#else
# define TNC 0
#endif

// This defines how many nodes should always be kept in dead list - 
// it should be >= 2 or the file finding code may not work correctly!
#define DLIST_KEEP_NUM 2 

//==========================================================================
// Node list functions 

static void
node_list_init(fatfs_node_list_t *list)
{
    list->size  = 0;
    list->first = list->last = NULL;
}

static __inline__ int 
node_list_get_size(fatfs_node_list_t *list)
{
    return (list->size);
}

static __inline__ fatfs_node_t*
node_list_get_head(fatfs_node_list_t *list)
{
    return (list->first);
}

static __inline__ fatfs_node_t*
node_list_get_tail(fatfs_node_list_t *list)
{
    return (list->last);
}

static __inline__ fatfs_node_t*
node_list_get_next(fatfs_node_t *node)
{
    return (node->list_next);
}

static __inline__ fatfs_node_t*
node_list_get_prev(fatfs_node_t *node)
{
    return (node->list_prev);
}

static __inline__ bool
node_list_is_last(fatfs_node_t *node)
{
    return (NULL == node->list_next);
}

static __inline__ bool
node_list_is_first(fatfs_node_t *node)
{
    return (NULL == node->list_prev);
}

static void
node_list_head_add(fatfs_node_list_t *list, fatfs_node_t *node)
{
    node->list_prev = NULL;
    node->list_next = list->first;
    
    if (NULL == list->first)
    {
        CYG_ASSERTC(list->size == 0);
        list->last = node;
    }
    else
    {
        list->first->list_prev = node;
    }
    
    list->first = node;
    list->size++;
}

#if 0
static __inline__ fatfs_node_t*
node_list_head_get(fatfs_node_list_t *list)
{
    return list->first;
}

static void
node_list_tail_add(fatfs_node_list_t *list, fatfs_node_t *node)
{
    node->list_prev = list->last;
    node->list_next = NULL;
    
    if (NULL == list->last)
    {
        CYG_ASSERTC(list->size == 0);
        list->first = node;
    }
    else
    {
        list->last->list_next = node;
    }
    
    list->last = node;
    list->size++;
}
#endif

static __inline__ fatfs_node_t*
node_list_tail_get(fatfs_node_list_t *list)
{
    return list->last;
}

static void
node_list_remove(fatfs_node_list_t *list, fatfs_node_t *node)
{
    if (list->first == list->last)
    {
        if (list->first == node)
        {
            CYG_ASSERTC(list->size == 1);
            list->first = list->last = NULL;
        }
        else
        {
            CYG_ASSERT(false, "chain node not in list!");
        }
    }
    else if (list->first == node)
    {
        CYG_ASSERTC(node->list_prev == NULL); 
        list->first = node->list_next;
        list->first->list_prev = NULL;
    }
    else if (list->last == node)
    {
        CYG_ASSERTC(node->list_next == NULL); 
        list->last = node->list_prev;
        list->last->list_next = NULL;
    }
    else
    {
        CYG_ASSERTC(node->list_prev != NULL && node->list_next != NULL); 
        node->list_prev->list_next = node->list_next;
        node->list_next->list_prev = node->list_prev;        
    }
    list->size--;
}

#ifdef USE_XCHECKS
static void
node_list_check(fatfs_node_list_t *list, int min_ref, int max_ref)
{
    int i;
    fatfs_node_t *node, *pnode;

    CYG_ASSERT((list->last == NULL && list->first == NULL) ||
               (list->last != NULL && list->first != NULL), 
               "list->first and list->last broken!");

    if (list->first == NULL)
    {
        CYG_ASSERTC(list->size == 0);
        return;
    }
    
    CYG_ASSERTC(list->first->list_prev == NULL);
    CYG_ASSERTC(list->last->list_next == NULL);

    CYG_ASSERTC(list->first->refcnt >= min_ref && 
                list->first->refcnt <= max_ref);
    CYG_ASSERTC(list->last->refcnt >= min_ref && 
                list->last->refcnt <= max_ref);
    
    i = 1;
    node = list->first; 
    pnode = NULL;
    while (node->list_next != NULL)
    {
        i++;
        CYG_ASSERTC(node->list_prev == pnode);
        CYG_ASSERTC(node->refcnt >= min_ref && node->refcnt <= max_ref);
        pnode = node;
        node = node->list_next;
    }
    CYG_ASSERTC(node->list_prev == pnode);
    CYG_ASSERTC(list->size == i);
    CYG_ASSERTC(node == list->last);
}

static void
node_lists_check(fatfs_disk_t* disk)
{
    node_list_check(&disk->live_nlist, 1, 99999);                        
    node_list_check(&disk->dead_nlist, 0, 0);                           
}
#endif // USE_XCHECKS

//==========================================================================
// Node hash functions 

static unsigned int 
hash_fn(const char *str, unsigned int strlen)
{
    unsigned int i = 0, val = 0;

    while (i++ < strlen)
        val = (val ^ (int)toupper(*str++)) << 1;
    return(val);
}

static void
node_hash_init(fatfs_hash_table_t *tbl)
{
    int i;

    // Set size and clear all slots
    tbl->size = FATFS_HASH_TABLE_SIZE;
    for (i = 0; i < tbl->size; i++)
        tbl->nodes[i] = NULL;
    tbl->n = 0;
}

static bool
node_hash_add(fatfs_hash_table_t *tbl, fatfs_node_t *node)
{
    unsigned int hval;
   
    // Calculate hash of given node filename
    hval = hash_fn(node->dentry.filename, 
                   strlen(node->dentry.filename)) % tbl->size;

    CYG_TRACE2(TNC, "name='%s' hval=%d", node->dentry.filename, hval);

    if (tbl->nodes[hval] == NULL)
    {
        // First node in this slot
        node->hash_next  = NULL;
        tbl->nodes[hval] = node;
        tbl->n++; 
        return true;
    }
    else
    { 
        // More nodes in this slot
        fatfs_node_t *lnode, *pnode;
       
        pnode = NULL;
        lnode = tbl->nodes[hval];

        // Insert node into list so that it is sorted by filename        
        while (lnode != NULL)
        {
            if (lnode == node)
                return false;
            
            if (strcasecmp(lnode->dentry.filename, node->dentry.filename) > 0)
            {
                if (pnode != NULL)
                    pnode->hash_next = node; // Insert in the middle
                else
                    tbl->nodes[hval] = node; // Insert at the beginning
                node->hash_next = lnode;
                tbl->n++; 
                return true;
            }    
            
            pnode = lnode;    
            lnode = lnode->hash_next;
        }
        // Insert at the end
        pnode->hash_next = node;
        node->hash_next  = NULL;
        tbl->n++; 
        return true;
    }
}

static fatfs_node_t*
node_hash_find(fatfs_hash_table_t *tbl, 
               const char         *name,
               unsigned int        namelen,
               unsigned int        parent_cluster)
{   
    unsigned int  hval; 
    fatfs_node_t *node;

    // Calculate hash of name and get the first node in slot
    hval = hash_fn(name, namelen) % tbl->size;
    node = tbl->nodes[hval];

    CYG_TRACE2(TNC, "name='%s' hval=%d\n", name, hval);
 
    // Find the node in list wich matches the 
    // given name and parent_cluster
    while (node != NULL)
    {
        // First compare the parent cluster number and 
        // check filename length since it is faster than 
        // comparing filenames
        if (parent_cluster == node->dentry.parent_cluster &&
            '\0' == node->dentry.filename[namelen])
        {
            int i = strncasecmp(node->dentry.filename, name, namelen);

            if (i == 0)
                return node;
            else if (i > 0)
                return NULL; // Stop searching - we have a 
                             // sorted list so there can't be
                             // any matching filename further on
                             // if i > 0 - look at node_hash_add
        }
        node = node->hash_next;
    }
    
    // No such node found
    return NULL; 
}

static bool
node_hash_remove_keyless(fatfs_hash_table_t *tbl, fatfs_node_t *node)
{
    int i;
    fatfs_node_t *lnode, *pnode;

    // Look in all slots, since we have no key
    for (i = 0; i < tbl->size; i++)
    {
        // Look in list and remove it if there
        lnode = tbl->nodes[i];
        pnode = NULL;
        while (lnode != NULL)
        {
            if (lnode == node)
            {
                if (pnode == NULL)
                    tbl->nodes[i]    = lnode->hash_next;
                else
                    pnode->hash_next = lnode->hash_next;
                node->hash_next = NULL;
                tbl->n--;
            
                // All done
                return true;
            }
            pnode = lnode;
            lnode = lnode->hash_next;
        }    
    }
    return false;
}

static bool 
node_hash_remove(fatfs_hash_table_t *tbl, fatfs_node_t *node)
{
    unsigned int hval;
    fatfs_node_t *lnode, *pnode;
   
    // Calculate hash of name and get the first node in slot
    hval = hash_fn(node->dentry.filename, 
                   strlen(node->dentry.filename)) % tbl->size;
    lnode = tbl->nodes[hval];

    // Now find the node in list and remove it
    pnode = NULL;
    while (lnode != NULL)
    {
        if (lnode == node)
        {
            if (pnode == NULL)
                tbl->nodes[hval] = lnode->hash_next;
            else
                pnode->hash_next = lnode->hash_next;
            node->hash_next = NULL;
            tbl->n--;
            
            // All done
            return true;
        }
        pnode = lnode;
        lnode = lnode->hash_next;
    } 

    // Node not in list 
    return false;
}

#ifdef USE_XCHECKS
static void
node_hash_check(fatfs_hash_table_t *tbl)
{
    int i, n;

    n = 0;
    for (i = 0; i < tbl->size; i++)
    {
        fatfs_node_t *lnode, *pnode;

        pnode = NULL;
        lnode = tbl->nodes[i];

        while (lnode != NULL)
        {
            if (pnode != NULL)
            {
                int c = strcasecmp(pnode->dentry.filename, lnode->dentry.filename);
                CYG_ASSERT(c <= 0, "hash table not sorted");
                CYG_ASSERT(pnode->dentry.parent_cluster != lnode->dentry.parent_cluster ||
                           0 != c, "duplicated node in hash table");
            }
            n++;
            pnode = lnode;
            lnode = lnode->hash_next;
        }
    }
    CYG_ASSERTC(tbl->n == n);
}

static void
node_hash_not_found_check(fatfs_disk_t *disk, 
                          const char   *name,
                          unsigned int  namelen,
                          unsigned int  parent_cluster)
{
    fatfs_node_t* node;

    node = node_list_get_head(&disk->live_nlist);
    while (NULL != node)
    {
        if (node->dentry.parent_cluster == parent_cluster          &&
            namelen == strlen(node->dentry.filename)               &&
            0 == strncasecmp(name, node->dentry.filename, namelen))
            CYG_ASSERT(false, "node not found in hash, "
                              "but exists in live list");
        node = node_list_get_next(node);
    }

    node = node_list_get_head(&disk->dead_nlist);
    while (NULL != node)
    {
        if (node->dentry.parent_cluster == parent_cluster          &&
            namelen == strlen(node->dentry.filename)               &&
            0 == strncasecmp(name, node->dentry.filename, namelen))
            CYG_ASSERT(false, "node not found in hash, "
                              "but exists in dead list");
        node = node_list_get_next(node);
    }   
}

static void
node_hash_found_check(fatfs_disk_t *disk, 
                      const char   *name,
                      unsigned int  namelen,
                      unsigned int  parent_cluster,
                      fatfs_node_t* node)
{
    fatfs_node_t *n;

    n = node_list_get_head(&disk->live_nlist);
    while (NULL != n)
    {
        if (n == node) 
        {
            if (node->dentry.parent_cluster != parent_cluster ||
                namelen != strlen(node->dentry.filename)      ||
                0 != strncasecmp(name, node->dentry.filename, namelen))
                CYG_ASSERT(false, "node_hash_find returned wrong node");
            return;
        }
        n = node_list_get_next(n);
    }

    n = node_list_get_head(&disk->dead_nlist);
    while (NULL != n)
    {
        if (n == node)
        {
            if (node->dentry.parent_cluster != parent_cluster ||
                namelen != strlen(node->dentry.filename)      ||
                0 != strncasecmp(name, node->dentry.filename, namelen))
                CYG_ASSERT(false, "node_hash_find returned wrong node");
            return;
        } 
        n = node_list_get_next(n);
    }

    CYG_ASSERT(false, "node not found in dead or live lists, "
                      "but exists in hash.");
}
#endif // USE_XCHECKS
    
//--------------------------------------------------------------------------

#ifdef USE_XCHECKS

# define SANITY_CHECK()                                                     \
    CYG_MACRO_START                                                         \
        node_lists_check(disk);                                             \
        node_hash_check(&disk->node_hash);                                  \
        CYG_ASSERTC((disk->live_nlist.size + disk->dead_nlist.size) ==      \
                    disk->node_hash.n);                                     \
    CYG_MACRO_END    

# define NODE_FIND_CHECK()                                                    \
    CYG_MACRO_START                                                           \
        if (NULL == node)                                                     \
            node_hash_not_found_check(disk, name, namelen, parent_cluster);   \
        else                                                                  \
            node_hash_found_check(disk, name, namelen, parent_cluster, node); \
    CYG_MACRO_END

#else // USE_XCHECKS
# define SANITY_CHECK()    CYG_EMPTY_STATEMENT
# define NODE_FIND_CHECK() CYG_EMPTY_STATEMENT
#endif // not USE_XCHECKS

//==========================================================================
// Node pool allocation functions

static void
node_pool_init(fatfs_disk_t *disk)
{
    int i;

    for (i = 0; i < FATFS_NODE_POOL_SIZE; i++)
        disk->node_pool[i] = &disk->node_pool_base[i];

    disk->node_pool_free_cnt = i;
}

static fatfs_node_t *
node_pool_alloc(fatfs_disk_t *disk)
{
    fatfs_node_t *node = NULL;

    if (disk->node_pool_free_cnt > 0)
        node = disk->node_pool[--disk->node_pool_free_cnt];

    return node;
}

static void
node_pool_free(fatfs_disk_t *disk, fatfs_node_t *node)
{
    disk->node_pool[disk->node_pool_free_cnt++] = node;
}
    
//==========================================================================
//==========================================================================
// Exported functions 

//--------------------------------------------------------------------------
// fatfs_node_cache_init()
// Initializes node cache of given disk.

void
fatfs_node_cache_init(fatfs_disk_t *disk)
{
    CYG_CHECK_DATA_PTRC(disk);
    
    node_list_init(&disk->live_nlist);
    node_list_init(&disk->dead_nlist);
    node_hash_init(&disk->node_hash);
    node_pool_init(disk);
    
    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_cache_flush()
// Frees all node cache of given disk.

void
fatfs_node_cache_flush(fatfs_disk_t *disk)
{
    fatfs_node_t *node, *next_node;
 
    node = node_list_get_head(&disk->live_nlist);

    while (NULL != node)
    {
        next_node = node_list_get_next(node);
        node_list_remove(&disk->live_nlist, node);
        if (!node_hash_remove(&disk->node_hash, node))
            CYG_ASSERT(false, "Node not in hash");
        node_pool_free(disk, node);
        node = next_node;
    }

    node = node_list_get_head(&disk->dead_nlist);

    while (NULL != node)
    {
        next_node = node_list_get_next(node);
        node_list_remove(&disk->dead_nlist, node);
        if (!node_hash_remove(&disk->node_hash, node))
            CYG_ASSERT(false, "Node not in hash");
        node_pool_free(disk, node);
        node = next_node;
    }

    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_alloc()
// Allocates a new node.  

fatfs_node_t*
fatfs_node_alloc(fatfs_disk_t *disk, fatfs_dir_entry_t *dentry)
{
    fatfs_node_t *node;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(dentry);
 
    node = node_pool_alloc(disk);
        
    if (NULL == node)
    {
        CYG_TRACE2(TNC, "getting node from dead list (size=%d keep=%d)",
                        node_list_get_size(&disk->dead_nlist), DLIST_KEEP_NUM);
        
        if (node_list_get_size(&disk->dead_nlist) <= DLIST_KEEP_NUM)
            return NULL;
        
        node = node_list_tail_get(&disk->dead_nlist);
        if (NULL == node)
            return NULL;
        
        CYG_TRACE1(TNC, "recycling node='%s'", node->dentry.filename); 

        node_list_remove(&disk->dead_nlist, node);
        if (!node_hash_remove(&disk->node_hash, node))
            CYG_ASSERT(false, "node not in hash");
    }     

    // Init new node    
    node->dentry = *dentry;
    node->refcnt = 0;

    node_list_head_add(&disk->dead_nlist, node);
    if (!node_hash_add(&disk->node_hash, node))
        CYG_ASSERT(false, "node already in hash");

    SANITY_CHECK();

    return node;
}

//--------------------------------------------------------------------------
// fatfs_node_touch()
// Moves given node to top of its list.
// (When reusing dead node it is always taken from the bottom of list) 

void
fatfs_node_touch(fatfs_disk_t *disk, fatfs_node_t *node)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(node);
    CYG_TRACE2(TNC, "node='%s' refcnt=%d", node->dentry.filename, node->refcnt);

    if (node->refcnt == 0)
    {
        node_list_remove(&disk->dead_nlist, node);
        node_list_head_add(&disk->dead_nlist, node);
    }
    else
    {
        node_list_remove(&disk->live_nlist, node);
        node_list_head_add(&disk->live_nlist, node);
    } 

    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_ref()
// Increases the given node reference count.
// If the reference goes from 0 to 1, than the node
// is moved from dead list to live list.

void
fatfs_node_ref(fatfs_disk_t *disk, fatfs_node_t *node)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(node);
    CYG_TRACE2(TNC, "node='%s' refcnt=%d", node->dentry.filename, node->refcnt);
    
    node->refcnt++;    
    if (1 == node->refcnt)
    {
        // First reference - move node from dead to live list

        CYG_TRACE1(TNC, "node='%s' to live list", node->dentry.filename);
        
        node_list_remove(&disk->dead_nlist, node);
        node_list_head_add(&disk->live_nlist, node);
    }

    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_unref()
// Decreases the given node reference count.
// If the reference goes from 1 to 0, than the node
// is moved from live list to top of dead list.
// (When reusing dead node it is always taken from the bottom of list)

void
fatfs_node_unref(fatfs_disk_t *disk, fatfs_node_t *node)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(node);
    CYG_TRACE2(TNC, "node='%s' refcnt=%d", node->dentry.filename, node->refcnt);
    CYG_ASSERT(node->refcnt > 0, "node->refcnt <= 0");

    node->refcnt--;
    if (0 == node->refcnt)
    {
        // No more references - move node from live to dead list

        CYG_TRACE1(TNC, "node='%s' to dead list", node->dentry.filename);

        node_list_remove(&disk->live_nlist, node);
        node_list_head_add(&disk->dead_nlist, node);
    }

    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_free()
// Frees node memory and removes it from its list and hash. 
// This function must not be called on a referenced node.

void
fatfs_node_free(fatfs_disk_t *disk, fatfs_node_t *node)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(node);
    CYG_TRACE2(TNC, "node='%s' refcnt=%d", node->dentry.filename, node->refcnt);
    CYG_ASSERTC(node->refcnt == 0);
    CYG_ASSERTC(node != disk->root);

    // Remove from dead list, from hash and free ptr

    node_list_remove(&disk->dead_nlist, node);
    if (!node_hash_remove(&disk->node_hash, node))
        CYG_ASSERT(false, "node not in hash");   
    
    node_pool_free(disk, node);

    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_rehash()
// Recalculates given node hash key. 

void
fatfs_node_rehash(fatfs_disk_t *disk, fatfs_node_t *node)
{
    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(node);
    
    if (!node_hash_remove_keyless(&disk->node_hash, node))
        CYG_ASSERT(false, "node not in hash");
    
    if (!node_hash_add(&disk->node_hash, node))
        CYG_ASSERT(false, "node already in hash");
    
    SANITY_CHECK();
}

//--------------------------------------------------------------------------
// fatfs_node_find()
// Finds node in hash by its name and parent cluster. 
// If no node found NULL is returned.

fatfs_node_t*
fatfs_node_find(fatfs_disk_t *disk, 
                const char   *name, 
                unsigned int  namelen,
                unsigned int  parent_cluster)
{
    fatfs_node_t *node;

    CYG_CHECK_DATA_PTRC(disk);
    CYG_CHECK_DATA_PTRC(name);
 
    node = node_hash_find(&disk->node_hash, name, namelen, parent_cluster);

    NODE_FIND_CHECK();
    
    CYG_TRACE3(TNC, "node name=%s pcluster=%d %s found in cache\n",
        name, parent_cluster, ((node != NULL) ? "" : "not"));
    
    return node;
}

//--------------------------------------------------------------------------
// fatfs_get_live_node_count()
// Gets the number of live nodes.

int
fatfs_get_live_node_count(fatfs_disk_t *disk)
{
    return node_list_get_size(&disk->live_nlist);
}

//--------------------------------------------------------------------------
// fatfs_get_dead_node_count()
// Gets the number of dead nodes.

int
fatfs_get_dead_node_count(fatfs_disk_t *disk)
{
    return node_list_get_size(&disk->dead_nlist);
}

// -------------------------------------------------------------------------
// EOF fatfs_ncache.c
