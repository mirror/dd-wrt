#ifndef CYGONCE_BLIB_H
#define CYGONCE_BLIB_H
//==========================================================================
//
//      blib.h
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

#include <cyg/infra/cyg_type.h> 
#include <cyg/io/io.h> 

#include <linux/rbtree.h>
#include <linux/list.h>

// --------------------------------------------------------------------

typedef int (*cyg_blib_bread_fn) (
    void*,       // private data
    void*,       // block buffer
    cyg_uint32*, // number of blocks to read
    cyg_uint32   // starting block number
);

typedef int (*cyg_blib_bwrite_fn) (
    void*,       // private data
    const void*, // block buffer
    cyg_uint32*, // number of blocks to write 
    cyg_uint32   // starting block number
);

typedef struct {
    cyg_uint32  n_gets;     // number of block gets
    cyg_uint32  n_reads;    // number of block reads
    cyg_uint32  n_writes;   // number of block writes
} cyg_blib_stat_t;

typedef struct {
    void                 *priv;            // private data
    struct list_head      list_head;       // head of block list 
    struct rb_root        rb_root;         // block red-black tree root
    cyg_uint32            block_size;      // block size
    cyg_uint32            block_size_log2; // block size log2
    cyg_uint8            *mem_base;        // memory base
    cyg_uint32            mem_size;        // memory size
    struct list_head      free_list_head;  // list of free blocks
    cyg_blib_bread_fn     bread_fn;        // block read function
    cyg_blib_bwrite_fn    bwrite_fn;       // block write function
#ifdef CYGIMP_BLOCK_LIB_STATISTICS
    cyg_blib_stat_t       stat;            // statistics
#endif
} cyg_blib_t;

// --------------------------------------------------------------------

// --------------------------------------------------------------------
// Creates a block lib instance
// 
//   priv_data  - private data to pass to bread_fn and bwrite_fn
//   mem_base   - block cache memory base
//   mem_size   - block cache memory size
//   block_size - block size
//   bread_fn   - function which reads blocks
//   bwrite_fn  - function which writes blocks
//   bl         - block lib instance space holder
//   
//   returns ENOERR if create succeded
//

int cyg_blib_create(void               *priv_data,
                    void               *mem_base,
                    cyg_uint32          mem_size,
                    cyg_uint32          block_size,
                    cyg_blib_bread_fn   bread_fn,
                    cyg_blib_bwrite_fn  bwrite_fn,
                    cyg_blib_t         *bl);

// --------------------------------------------------------------------
// Creates a block lib instance on top of IO system 
//   (cyg_io_bread and cyg_io_bwrite)
// 
//   handle     - cyg_io_handle_t
//   mem_base   - block cache memory base
//   mem_size   - block cache memory size
//   block_size - block size
//   bl         - block lib instance space holder
//   
//   returns ENOERR if create succeded
//

int cyg_blib_io_create(cyg_io_handle_t     handle,
                       void               *mem_base,
                       cyg_uint32          mem_size,
                       cyg_uint32          block_size,
                       cyg_blib_t         *bl);

// --------------------------------------------------------------------
// Deletes a block lib instance
//   
//   bl - block lib instance
//
//   The block cache is synced before
//
//   returns ENOERR if delete succeded
//

int cyg_blib_delete(cyg_blib_t *bl);

// --------------------------------------------------------------------
// Reads a number of blocks
//
//   bl  - block lib instance
//   buf - block buffer ptr 
//   len - number of blocks to read
//   pos - starting block number
//       
//   returns ENOERR if read succeded
//   
        
int cyg_blib_bread(cyg_blib_t *bl,
                   void       *buf,
                   cyg_uint32 *len,
                   cyg_uint32  pos);

// --------------------------------------------------------------------
// Writes a number of blocks
//
//   bl  - block lib instance
//   buf - block buffer ptr 
//   len - number of blocks to write 
//   pos - starting block number
//       
//   returns ENOERR if write succeded
//   
 
int cyg_blib_bwrite(cyg_blib_t *bl,
                    const void *buf,
                    cyg_uint32 *len,
                    cyg_uint32  pos);

// --------------------------------------------------------------------
// Reads data
//
//   bl   - block lib instance
//   buf  - data buffer ptr 
//   len  - number of bytes to read
//   bnum - starting block number 
//   pos  - starting position inside starting block
//       
//   returns ENOERR if read succeded
//   
//   The block number is automatically adjusted if
//   position is greater than block size
//
 
int cyg_blib_read(cyg_blib_t *bl,
                  void       *buf,
                  cyg_uint32 *len,
                  cyg_uint32  bnum,
                  cyg_uint32  pos);

// --------------------------------------------------------------------
// Writes data
//
//   bl   - block lib instance
//   buf  - data buffer ptr 
//   len  - number of bytes to write 
//   bnum - starting block number 
//   pos  - starting position inside starting block
//       
//   returns ENOERR if write succeded
//
//   The block number is automatically adjusted if
//   position is greater than block size
//    
 
int cyg_blib_write(cyg_blib_t *bl,
                   const void *buf,
                   cyg_uint32 *len,
                   cyg_uint32  bnum,
                   cyg_uint32  pos);

// --------------------------------------------------------------------
// Syncs block cache - ie write modified blocks
//
//   bl - block lib instance
//
//   returns ENOERR if sync succeded
//

int cyg_blib_sync(cyg_blib_t *bl);

// --------------------------------------------------------------------
// Syncs block - ie write if modified
//
//   bl  - block lib instance
//   num - block number to sync
//
//   returns ENOERR if sync succeded
//

int cyg_blib_sync_block(cyg_blib_t *bl, cyg_uint32 num);

// --------------------------------------------------------------------
// Flushes block cache 
//
//   bl  - block lib instance
//
//   returns ENOERR if flush succeded
//
//   The block cache is synced before
//

int cyg_blib_flush(cyg_blib_t *bl);

// --------------------------------------------------------------------
// Sets block size 
//
//   bl         - block lib instance
//   block_size - new block size
//
//   returns ENOERR if set succeded
//
//   The block cache is synced before
//

int cyg_blib_set_block_size(cyg_blib_t *bl, cyg_uint32 block_size);

// --------------------------------------------------------------------
// Gets block size 
//
//   bl  - block lib instance
//
//   returns the current block size

static inline cyg_uint32 cyg_blib_get_block_size(cyg_blib_t *bl)
{
    return bl->block_size;
}

// --------------------------------------------------------------------
// Gets log2 of block size 
//
//   bl  - block lib instance
//
//   returns log2 of the current block size 

static inline cyg_uint32 cyg_blib_get_block_size_log2(cyg_blib_t *bl)
{
    return bl->block_size_log2;
}

// --------------------------------------------------------------------
// Gets block cache statistics 
//
//   bl - block lib instance
//
//   returns ENOERR if get succeded
//

int cyg_blib_get_stat(cyg_blib_t *bl, cyg_blib_stat_t *stat);

#endif // CYGONCE_BLIB_H

// --------------------------------------------------------------------
// EOF blib.h
