//==========================================================================
//
//      blib.c
//
//      Block cache and access library  
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
// Author(s):     savin 
// Date:          2003-08-29
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/io/io.h>
#include <cyg/infra/cyg_type.h>    
#include <cyg/infra/cyg_ass.h>      // assertion support
#include <cyg/infra/diag.h>         // diagnostic output
#include <blib/blib.h>

#include <string.h>  // memcpy

#include <linux/rbtree.h>
#include <linux/list.h>

// --------------------------------------------------------------------

//#define DEBUG 1

#ifdef DEBUG
# define D(_args_) diag_printf _args_
#else
# define D(_args_) CYG_EMPTY_STATEMENT
#endif

#define ALIGN(_x_) (((_x_) + (CYGARC_ALIGNMENT-1)) & ~(CYGARC_ALIGNMENT-1))

#ifdef CYGIMP_BLOCK_LIB_STATISTICS

#define STAT_INIT(_bl_) do {    \
        bl->stat.n_gets   = 0;  \
        bl->stat.n_reads  = 0;  \
        bl->stat.n_writes = 0;  \
    } while (0)    

#define STAT(_bl_, _group_) \
    ((_bl_)->stat._group_++)

#else // CYGIMP_BLOCK_LIB_STATISTICS

#define STAT_INIT(_bl_)     CYG_EMPTY_STATEMENT
#define STAT(_bl_, _group_) CYG_EMPTY_STATEMENT
    
#endif // not CYGIMP_BLOCK_LIB_STATISTICS
 
// --------------------------------------------------------------------

typedef struct {
    struct list_head  list_node; // list node
    struct rb_node    rb_node;   // red-black tree node
    cyg_uint32        num;       // block number
    cyg_bool          modified;  // is this block data modified (needs write)
    cyg_uint8         data[0];   // block data
} blib_block_t;

// --------------------------------------------------------------------

static blib_block_t *
rb_find_block(cyg_blib_t *bl, cyg_uint32 num)
{
    struct rb_node *node  = bl->rb_root.rb_node;
    blib_block_t   *block = NULL;
    
    while (NULL != node)
    {
        block = rb_entry(node, blib_block_t, rb_node);

        if (block->num == num)
            return block;

        node = (block->num > num) ? node->rb_left : node->rb_right;
    }
    return NULL;
}

static void
rb_add_block(cyg_blib_t *bl, blib_block_t *block)
{
    struct rb_node *node = bl->rb_root.rb_node;    

    if (NULL == node)
    {
        rb_link_node(&block->rb_node, NULL, &bl->rb_root.rb_node);
    }
    else
    {
        struct rb_node **link;
        blib_block_t    *b = NULL;
        
        while (NULL != node)
        {
            b = rb_entry(node, blib_block_t, rb_node);

            CYG_ASSERTC(b->num != block->num);

            link = (b->num > block->num) ? &node->rb_left : &node->rb_right;
            node = *link;
        }
        rb_link_node(&block->rb_node, &b->rb_node, link);
    }
    rb_insert_color(&block->rb_node, &bl->rb_root);
}

static __inline__ void
rb_del_block(cyg_blib_t *bl, blib_block_t *block)
{
    rb_erase(&block->rb_node, &bl->rb_root);
}

// --------------------------------------------------------------------

static __inline__ void
list_add_block(cyg_blib_t *bl, blib_block_t *block)
{
    list_add(&block->list_node, &bl->list_head);
}

static __inline__ void
list_del_block(cyg_blib_t *bl, blib_block_t *block)
{
    list_del(&block->list_node);
}

static __inline__ blib_block_t *
list_get_first_block(cyg_blib_t *bl)
{
    return(list_entry(bl->list_head.next, blib_block_t, list_node));
}

static __inline__ blib_block_t *
list_get_last_block(cyg_blib_t *bl)
{
    return(list_entry(bl->list_head.prev, blib_block_t, list_node));
}

static void
list_move_block_to_head(cyg_blib_t *bl, blib_block_t *block)
{
    list_del(&block->list_node);
    list_add(&block->list_node, &bl->list_head);
}

// --------------------------------------------------------------------

static __inline__ void
free_block(cyg_blib_t *bl, blib_block_t *block)
{
    list_add(&block->list_node, &bl->free_list_head);
}

static __inline__ blib_block_t *
alloc_block(cyg_blib_t *bl)
{
    if ( !list_empty(&bl->free_list_head) )
    {
        blib_block_t *new;

        new = list_entry(bl->free_list_head.next, blib_block_t, list_node);
        list_del(bl->free_list_head.next);
       
        return new; 
    }
    else
        return NULL;
}

static void
init_block_mem_pool(cyg_blib_t *bl)
{
    cyg_uint8  *block_mem;
    cyg_uint32  avail_mem_size, block_mem_size;
    
    INIT_LIST_HEAD(&bl->free_list_head);    

    block_mem      = bl->mem_base;
    avail_mem_size = bl->mem_size;
    block_mem_size = ALIGN(sizeof(blib_block_t) + bl->block_size);
    
    while (avail_mem_size >= block_mem_size)
    {
        blib_block_t *block = (blib_block_t *)block_mem;
        
        list_add(&block->list_node, &bl->free_list_head);
        
        block_mem      += block_mem_size;
        avail_mem_size -= block_mem_size;
    }
}

// --------------------------------------------------------------------

static cyg_uint32
get_val_log2(cyg_uint32 val)
{
    cyg_uint32 i, log2;

    i = val;
    log2 = 0;
    while (0 == (i & 1))
    {
        i >>= 1;
        log2++;
    }
    if (i != 1)
        return 0;
    else
        return log2;
}

static int 
blib_sync_block(cyg_blib_t *bl, blib_block_t *block)
{
    int ret = ENOERR;

    if (block->modified)
    {
        int len = 1;

        D(("blib writting block=%d\n", block->num)); 
 
        STAT(bl, n_writes);

        ret = bl->bwrite_fn(bl->priv, (void *)block->data, &len, block->num);

        if (ENOERR == ret)
            block->modified = false;
    }
    return ret;
}

static int 
blib_sync(cyg_blib_t *bl)
{
    struct list_head *node = bl->list_head.next;
    blib_block_t     *block;
    int ret = ENOERR;
 
    D(("blib cache sync\n")); 
    
    while (node != &bl->list_head)
    {
        block = list_entry(node, blib_block_t, list_node);

        ret = blib_sync_block(bl, block);
        if (ENOERR != ret)
           break;
        
        node = node->next;
    }
    return ret;
}

static int 
blib_get_block(cyg_blib_t    *bl, 
               cyg_uint32     num, 
               cyg_bool       read_data,
               blib_block_t **dblock)
{
    blib_block_t *block = NULL;
    int ret = ENOERR;
    int len;
 
    D(("blib get block=%d\n", num)); 
    
    STAT(bl, n_gets);

    // first check if the most recently used block is the requested block,
    // this can improve performance when using byte access functions
    if (!list_empty(&bl->list_head))
    {
        blib_block_t *first_block = list_get_first_block(bl);
        if (first_block->num == num)
            block = first_block;
        else 
            block = rb_find_block(bl, num);
    }
    
    if (NULL != block)
    {
        D(("blib block=%d found in cache\n", num)); 

        list_move_block_to_head(bl, block);
        *dblock = block;
        return ret;
    }

    D(("blib block=%d NOT found in cache\n", num)); 

    block = alloc_block(bl);

    if (NULL == block)
    {
        CYG_ASSERTC(!list_empty(&bl->list_head));
        
        block = list_get_last_block(bl);

        D(("blib reusing block=%d space\n", block->num)); 

        ret = blib_sync_block(bl, block);
        if (ENOERR != ret)
            return ret;
        
        list_del_block(bl, block);
        rb_del_block(bl, block);
    }

    block->num      = num;
    block->modified = false;

    if (read_data)
    {
        D(("blib reading block=%d\n", block->num)); 

        STAT(bl, n_reads);

        len = 1;
        ret = bl->bread_fn(bl->priv, (void *)block->data, &len, block->num);
        if (ENOERR != ret)
        {
            free_block(bl, block);
            return ret;
        }
    }
    rb_add_block(bl, block);
    list_add_block(bl, block);
    
    *dblock = block;
    return ret;
}

static int 
blib_init_cache(cyg_blib_t *bl, 
                void       *mem_base,
                cyg_uint32  mem_size,
                cyg_uint32  block_size, 
                cyg_bool    reinit)
{
    int ret = ENOERR;
    
    if (reinit)
    {
        ret = blib_sync(bl);
        if (ENOERR != ret)
            return ret;
    }
    
    bl->rb_root = RB_ROOT;
    INIT_LIST_HEAD(&bl->list_head);

    bl->mem_base   = mem_base;
    bl->mem_size   = mem_size;    
    bl->block_size = block_size;

    bl->block_size_log2 = get_val_log2(block_size);
    if (0 == bl->block_size_log2)
        return -EINVAL;
    
    init_block_mem_pool(bl);

    STAT_INIT(bl);

    return ret;
}

// --------------------------------------------------------------------

static int
io_bread(void *priv, void *buf, cyg_uint32 *len, cyg_uint32 pos)
{
    return cyg_io_bread((cyg_io_handle_t)priv, buf, len, pos);
}

static int
io_bwrite(void *priv, const void *buf, cyg_uint32 *len, cyg_uint32 pos)
{
    return cyg_io_bwrite((cyg_io_handle_t)priv, buf, len, pos);
}

// --------------------------------------------------------------------

int
cyg_blib_create(void               *priv_data,
                void               *mem_base,
                cyg_uint32          mem_size,
                cyg_uint32          block_size,
                cyg_blib_bread_fn   bread_fn,
                cyg_blib_bwrite_fn  bwrite_fn,
                cyg_blib_t         *bl)
{
    int ret;
    
    bl->priv      = priv_data;
    bl->bread_fn  = bread_fn;
    bl->bwrite_fn = bwrite_fn;

    ret = blib_init_cache(bl, mem_base, mem_size, block_size, false); 
    return ret; 
}

int 
cyg_blib_io_create(cyg_io_handle_t     handle,
                   void               *mem_base,
                   cyg_uint32          mem_size,
                   cyg_uint32          block_size,
                   cyg_blib_t         *bl)
{
    return cyg_blib_create((void *)handle, mem_base, mem_size, block_size,
                           io_bread, io_bwrite, bl);
}

int
cyg_blib_delete(cyg_blib_t *bl)
{
    return blib_sync(bl);
}

int 
cyg_blib_bread(cyg_blib_t *bl,
               void       *buf,
               cyg_uint32 *len,
               cyg_uint32  pos)
{
    blib_block_t *block;
    cyg_int32  size = *len;
    cyg_int32  bnum = pos;
    cyg_uint8 *bbuf = buf;
    Cyg_ErrNo  ret  = ENOERR;

    D(("blib bread block=%d len=%d buf=%p\n", pos, *len, buf)); 

    while (size > 0)
    {
        ret = blib_get_block(bl, bnum, true, &block); 
        if (ENOERR != ret)
            break;
        
        memcpy((void *)bbuf, (void *)block->data, bl->block_size);

        bbuf += bl->block_size;
        bnum++;
        size--;
    }
    *len -= size;
    return ret;
}

int 
cyg_blib_bwrite(cyg_blib_t *bl,
                const void *buf,
                cyg_uint32 *len,
                cyg_uint32  pos)
{
    blib_block_t *block;
    cyg_int32  size = *len;
    cyg_int32  bnum = pos;
    cyg_uint8 *bbuf = (cyg_uint8 * const) buf;
    Cyg_ErrNo  ret  = ENOERR;

    D(("blib bwrite block=%d len=%d buf=%p\n", pos, *len, buf)); 

    while (size > 0)
    {
        ret = blib_get_block(bl, bnum, false, &block); 
        if (ENOERR != ret)
            break;
        
        memcpy((void *)block->data, (void *)bbuf, bl->block_size);
        block->modified = true;

        bbuf += bl->block_size;
        bnum++;
        size--;
    }
    *len -= size;
    return ret;
}

int
cyg_blib_read(cyg_blib_t *bl,
              void       *buf,
              cyg_uint32 *len,
              cyg_uint32  bnum,
              cyg_uint32  pos)
{
    blib_block_t *block;
    cyg_uint8 *bbuf = buf;
    cyg_int32  size = *len;
    Cyg_ErrNo  ret  = ENOERR;

    size = *len;

    if (pos >= bl->block_size)
    {
        bnum += pos >> bl->block_size_log2;
        pos   = pos & (bl->block_size - 1);
    }
        
    D(("blib read len=%d pos=%d bnum=%d\n", *len, pos, bnum));

    while (size > 0)
    {
        int csize;

        if ((size + pos) > bl->block_size)
            csize = bl->block_size - pos;
        else
            csize = size;

        ret = blib_get_block(bl, bnum, true, &block);
        if (ENOERR != ret)
            break;
 
        memcpy((void *)bbuf, (void *)(block->data+pos), csize);

        bbuf += csize;
        size -= csize;
        pos   = 0;
        bnum++;
    }
    *len -= size;
    return ret;
}

int
cyg_blib_write(cyg_blib_t *bl,
               const void *buf,
               cyg_uint32 *len,
               cyg_uint32  bnum,
               cyg_uint32  pos)
{
    blib_block_t *block;
    cyg_uint8 *bbuf = (cyg_uint8 * const) buf;
    cyg_int32  size = *len;
    Cyg_ErrNo  ret  = ENOERR;

    size = *len;

    if (pos >= bl->block_size)
    {
        bnum += pos >> bl->block_size_log2;
        pos   = pos & (bl->block_size - 1);
    }
        
    D(("blib write len=%d pos=%d bnum=%d\n", *len, pos, bnum));

    while (size > 0)
    {
        int csize;

        if ((size + pos) > bl->block_size)
            csize = bl->block_size - pos;
        else
            csize = size;

        if (0 == pos && csize == bl->block_size)
            ret = blib_get_block(bl, bnum, false, &block);
        else
            ret = blib_get_block(bl, bnum, true, &block);
        if (ENOERR != ret)
            break;

        memcpy((void *)(block->data+pos), (void *)bbuf, csize);
        block->modified = true;

        bbuf += csize;
        size -= csize;
        pos   = 0;
        bnum++;
    }
    *len -= size;
    return ret;
}

int
cyg_blib_sync(cyg_blib_t *bl)
{
    return blib_sync(bl);
}

int
cyg_blib_sync_block(cyg_blib_t *bl, cyg_uint32 num)
{
    blib_block_t *block = rb_find_block(bl, num);
    
    if (NULL != block)
        return blib_sync_block(bl, block);
    else
        return -EINVAL;
}

int
cyg_blib_flush(cyg_blib_t *bl)
{
    return blib_init_cache(bl, bl->mem_base,  bl->mem_size,
                           bl->block_size, true);
}

int
cyg_blib_set_block_size(cyg_blib_t *bl, cyg_uint32 block_size)
{
    return blib_init_cache(bl, bl->mem_base, bl->mem_size, 
                           block_size, true); 
}    

int
cyg_blib_get_stat(cyg_blib_t *bl, cyg_blib_stat_t *stat)
{
#ifdef CYGIMP_BLOCK_LIB_STATISTICS
    *stat = bl->stat;
    return ENOERR;
#else
    stat->n_gets   = 0;
    stat->n_reads  = 0;
    stat->n_writes = 0;
    return -EINVAL;
#endif
}

// --------------------------------------------------------------------
// EOF blib.c
