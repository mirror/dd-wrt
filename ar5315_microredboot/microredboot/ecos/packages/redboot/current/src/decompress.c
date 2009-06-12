//==========================================================================
//
//      decompress.c
//
//      RedBoot decompress support
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
// Author(s):    jskov
// Contributors: jskov, gthomas, tkoeller
// Date:         2001-03-08
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#ifdef CYGPKG_COMPRESS_ZLIB
#include <cyg/compress/zlib.h>
static z_stream stream;
static bool stream_end;

#define __ZLIB_MAGIC__ 0x5A4C4942   // 'ZLIB'

//
// Free memory [blocks] are stored as a linked list of "struct _block"
// The 'magic' is kept to insure that the block being freed is reasonable
//
// One of either next or size might be removable, if a sentinal block
// is placed at the end of the region at initialisation time.
struct _block {
    int            size;   // always the total length of the block, including this header
    long           magic;  // Must be __ZLIB_MAGIC__ if allocated and 0 if free
    struct _block *next;
    struct _block *prev;
};
static struct _block *memlist;

#ifdef CYGOPT_REDBOOT_FIS_ZLIB_COMMON_BUFFER
# define ZLIB_COMPRESSION_OVERHEAD CYGNUM_REDBOOT_FIS_ZLIB_COMMON_BUFFER_SIZE
#else
# define ZLIB_COMPRESSION_OVERHEAD 0xC000
#endif
static void *zlib_workspace;

//
// This function is run as part of RedBoot's initialization.
// It will allocate some memory from the "workspace" pool for
// use by the gzip/zlib routines.  This allows the memory usage
// of RedBoot to be more finely controlled than if we simply
// used the generic 'malloc() from the heap' functionality.
//
static void
_zlib_init(void)
{
    struct _block *bp;
#ifdef CYGOPT_REDBOOT_FIS_ZLIB_COMMON_BUFFER
    zlib_workspace = fis_zlib_common_buffer;
#else
    // Allocate some RAM for use by the gzip/zlib routines
    workspace_end -= ZLIB_COMPRESSION_OVERHEAD;
    zlib_workspace = workspace_end;
#endif
    bp = (struct _block *)zlib_workspace;
    memlist = bp;
    bp->next = bp->prev = 0;
    bp->size = ZLIB_COMPRESSION_OVERHEAD; 
    bp->magic = 0;
#ifdef DEBUG_ZLIB_MALLOC
    show_memlist(__FUNCTION__);
#endif
}

RedBoot_init(_zlib_init, RedBoot_INIT_FIRST);

// #define DEBUG_ZLIB_MALLOC
#ifdef DEBUG_ZLIB_MALLOC
static void
show_memlist(char *when)
{
    struct _block *bp = memlist;

    diag_printf("memory list after %s\n", when);
    diag_printf("   --START--- --END----- --SIZE---- --PREV---- --NEXT---- TYPE-----\n");
    while (bp != (struct _block *)0) {
        diag_printf("   %08p-%08p 0x%08x %08p %08p %s\n", bp, (unsigned char *)bp+bp->size, 
                    bp->size, bp->prev, bp->next, bp->magic == 0 ? "FREE" : "ALLOCATED" );
        bp = bp->next;
    }
    diag_printf("\n");
}
#endif

// Note: these have to be global and match the prototype used by the
// gzip/zlib package since we are exactly replacing them.

void 
*zcalloc(void *opaque, unsigned int items, unsigned int size)
{
    voidpf res = 0;
    int len = (items*size) + sizeof(struct _block);
    struct _block *bp = memlist;
    struct _block *nbp;

#ifdef DEBUG_ZLIB_MALLOC
    /* do this here because when int is called output is not setup yet */
    static int first_alloc = 1;
    if ( first_alloc ) {
        show_memlist("initialization");
        first_alloc = 0;
    }
#endif

    // Simple, first-fit algorithm
    while (bp) {
        if (bp->magic == 0 && bp->size > len) {
            nbp = (struct _block *)((char *)bp + len);
            /* link new block into chain */
            nbp->next = bp->next;
            bp->next = nbp;
            nbp->prev = bp;
            /* split size between the two blocks */
            nbp->size = bp->size - len;
            bp->size = len;
            /* mark the new block as free */
            nbp->magic = 0;
            /* mark allocated block as allocated */
            bp->magic = __ZLIB_MAGIC__;
            res = bp +1;
            memset(res, 0, len - sizeof(struct _block));
            break;
        }
        bp = bp->next;
    }
#ifdef DEBUG_ZLIB_MALLOC
    diag_printf("%s(0x%x,0x%x) = %p\n", __FUNCTION__, items, size, res);
    show_memlist(__FUNCTION__);
#endif
    if ( res == NULL )
        diag_printf("zcalloc: failed to allocate 0x%x items of 0x%x bytes == 0x%x bytes\n", items, size, len);
    return res;
}

void 
zcfree(void *opaque, void *ptr)
{
    struct _block *bp;

    if (!ptr) return;  // Safety
    bp = (struct _block *)((char *)ptr - sizeof(struct _block));
    if (bp->magic != __ZLIB_MAGIC__) {
        diag_printf("%s(%p) - invalid block\n", __FUNCTION__, ptr);
        return;
    }

    /* mark as free */
    bp->magic = 0;

#ifdef DEBUG_ZLIB_MALLOC
    diag_printf("%s(%p) = 0x%x bytes\n", __FUNCTION__, ptr, bp->size);
#endif

    bp = (struct _block *)((char *)ptr - sizeof(struct _block));
    while(bp->next && bp->next->magic == 0) {
#ifdef DEBUG_ZLIB_MALLOC
        diag_printf("  merging %08p and %08p (after)\n", bp, bp->next);
#endif
        bp->size += bp->next->size;
        bp->next = bp->next->next;
    }

    while(bp->prev && bp->prev->magic == 0) {
#ifdef DEBUG_ZLIB_MALLOC
        diag_printf("  merging %08p and %08p (before)\n", bp->prev, bp);
#endif
        bp->prev->size += bp->size;
        bp->prev->next = bp->next;
        bp = bp->prev;
    }

#ifdef DEBUG_ZLIB_MALLOC
    show_memlist(__FUNCTION__);
#endif
}

//
// This function is called to initialize a gzip/zlib stream.
//
static int
gzip_init(_pipe_t* p)
{
    int err;

    // Note: this code used to [re]initialize the memory pool used
    // by zlib.  This is now done in _zlib_init(), but only once.
    stream.zalloc = zcalloc;
    stream.zfree = zcfree;
    stream.next_in = NULL;
    stream.avail_in = 0;
    stream.next_out = NULL;
    stream.avail_out = 0;
    err = inflateInit(&stream);
    stream_end = false;

    return err;
}

//
// This function is called during the decompression cycle to
// actually cause a buffer to be filled with uncompressed data.
//
static int
gzip_inflate(_pipe_t* p)
{
    int err, bytes_out;

    if (stream_end)
	return Z_STREAM_END;
	
    stream.next_in = p->in_buf;
    stream.avail_in = p->in_avail;
    stream.next_out = p->out_buf;
    stream.avail_out = p->out_max;
    err = inflate(&stream, Z_SYNC_FLUSH);
    bytes_out = stream.next_out - p->out_buf;
    p->out_size += bytes_out;
    p->out_buf = stream.next_out;
    p->msg = stream.msg;
    p->in_avail = stream.avail_in;
    p->in_buf = stream.next_in;

    // Let upper layers process any inflated bytes at
    // end of stream.
    if (err == Z_STREAM_END && bytes_out) {
	stream_end = true;
	err = Z_OK;
    }

    return err;
}

//
// Called when the input data is completed or an error has
// occured.  This allows for clean up as well as passing error
// information up.
//
static int
gzip_close(_pipe_t* p, int err)
{
    switch (err) {
    case Z_STREAM_END:
        err = 0;
        break;
    case Z_OK:
        if (stream_end) {
          break;
        }
        // Decompression didn't complete
        p->msg = "premature end of input";
        // fall-through
    default:
        err = -1;
        break;
    }

    inflateEnd(&stream);

    return err;
}

//
// Exported interfaces
//
_decompress_fun_init* _dc_init = gzip_init;
_decompress_fun_inflate* _dc_inflate = gzip_inflate;
_decompress_fun_close* _dc_close = gzip_close;
#endif // CYGPKG_COMPRESS_ZLIB
