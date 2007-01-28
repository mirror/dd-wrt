
/*
 * $Id: store_swapout.c,v 1.96 2006/09/22 03:15:20 hno Exp $
 *
 * DEBUG: section 20    Storage Manager Swapout Functions
 * AUTHOR: Duane Wessels
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#include "squid.h"

static void storeSwapOutStart(StoreEntry * e);
static STIOCB storeSwapOutFileClosed;
static STIOCB storeSwapOutFileNotify;
static int storeSwapOutAble(const StoreEntry * e);

/* start swapping object to disk */
static void
storeSwapOutStart(StoreEntry * e)
{
    generic_cbdata *c;
    MemObject *mem = e->mem_obj;
    int swap_hdr_sz = 0;
    tlv *tlv_list;
    char *buf;
    assert(mem);
    /* Build the swap metadata, so the filesystem will know how much
     * metadata there is to store
     */
    debug(20, 5) ("storeSwapOutStart: Begin SwapOut '%s' to dirno %d, fileno %08X\n",
	storeUrl(e), e->swap_dirn, e->swap_filen);
    e->swap_status = SWAPOUT_WRITING;
    tlv_list = storeSwapMetaBuild(e);
    buf = storeSwapMetaPack(tlv_list, &swap_hdr_sz);
    storeSwapTLVFree(tlv_list);
    mem->swap_hdr_sz = (size_t) swap_hdr_sz;
    /* Create the swap file */
    c = cbdataAlloc(generic_cbdata);
    c->data = e;
    mem->swapout.sio = storeCreate(e, storeSwapOutFileNotify, storeSwapOutFileClosed, c);
    if (NULL == mem->swapout.sio) {
	e->swap_status = SWAPOUT_NONE;
	cbdataFree(c);
	xfree(buf);
	storeLog(STORE_LOG_SWAPOUTFAIL, e);
	return;
    }
    storeLockObject(e);		/* Don't lock until after create, or the replacement
				 * code might get confused */
    /* Pick up the file number if it was assigned immediately */
    e->swap_filen = mem->swapout.sio->swap_filen;
    e->swap_dirn = mem->swapout.sio->swap_dirn;
    /* write out the swap metadata */
    cbdataLock(mem->swapout.sio);
    storeWrite(mem->swapout.sio, buf, mem->swap_hdr_sz, xfree);
}

static void
storeSwapOutFileNotify(void *data, int errflag, storeIOState * sio)
{
    generic_cbdata *c = data;
    StoreEntry *e = c->data;
    MemObject *mem = e->mem_obj;
    assert(e->swap_status == SWAPOUT_WRITING);
    assert(mem);
    assert(mem->swapout.sio == sio);
    assert(errflag == 0);
    e->swap_filen = mem->swapout.sio->swap_filen;
    e->swap_dirn = mem->swapout.sio->swap_dirn;
}

/* as sideeffect returns if the object can be cached */
int
storeSwapOutMaintainMemObject(StoreEntry * e)
{
    MemObject *mem = e->mem_obj;
    squid_off_t lowest_offset;
    squid_off_t new_mem_lo;
    int swapout_able;

    /* Don't pollute the disk with icons and other special entries */
    if (EBIT_TEST(e->flags, ENTRY_SPECIAL))
	return -1;

    /* Don't even attempt to free data from hot-cached objects */
    if (e->mem_status == IN_MEMORY)
	return 1;

    swapout_able = storeSwapOutAble(e);
    if (!swapout_able) {
	/* Stop writing to disk */
	storeReleaseRequest(e);
	if (e->mem_obj->swapout.sio != NULL)
	    storeSwapOutFileClose(e);
    }
    /* storeLowestMemReaderOffset must be after the storeSwapOutFileClose
     * call above as storeLowestMemReaderOffset needs to know if we are
     * still writing to disk or not
     */
    lowest_offset = storeLowestMemReaderOffset(e);
    debug(20, 7) ("storeSwapOut: lowest_offset = %" PRINTF_OFF_T "\n",
	lowest_offset);
    if (!swapout_able)
	new_mem_lo = lowest_offset;
    else if (mem->inmem_hi > Config.Store.maxInMemObjSize)
	new_mem_lo = lowest_offset;
    else if (mem->reply->content_length > Config.Store.maxInMemObjSize)
	new_mem_lo = lowest_offset;
    else
	new_mem_lo = mem->inmem_lo;
    /* The -1 makes sure the page isn't freed until storeSwapOut has
     * walked to the next page. (mem->swapout.memnode->next) */
    if (swapout_able && new_mem_lo) {
	squid_off_t on_disk = storeSwapOutObjectBytesOnDisk(e->mem_obj);
	if (on_disk - 1 < new_mem_lo)
	    new_mem_lo = on_disk - 1;
    }
    if (new_mem_lo < mem->inmem_lo)
	new_mem_lo = mem->inmem_lo;
    if (mem->inmem_lo != new_mem_lo) {
	mem->inmem_lo = stmemFreeDataUpto(&mem->data_hdr, new_mem_lo);

	/* If ENTRY_DEFER_READ is set, then the client side will continue to
	 * flush until it has less than READ_AHEAD_GAP bytes in memory */
	if (EBIT_TEST(e->flags, ENTRY_DEFER_READ)) {

	    if (mem->inmem_hi - mem->inmem_lo <= Config.readAheadGap) {
		storeResumeRead(e);
	    }
	}
    }
    return swapout_able;
}

void
storeSwapOut(StoreEntry * e)
{
    MemObject *mem = e->mem_obj;
    int swapout_able;
    squid_off_t swapout_size;
    size_t swap_buf_len;
    if (mem == NULL)
	return;
    /* should we swap something out to disk? */
    debug(20, 7) ("storeSwapOut: %s\n", storeUrl(e));
    debug(20, 7) ("storeSwapOut: store_status = %s\n",
	storeStatusStr[e->store_status]);
    if (EBIT_TEST(e->flags, ENTRY_ABORTED)) {
	assert(EBIT_TEST(e->flags, RELEASE_REQUEST));
	storeSwapOutFileClose(e);
	return;
    }
    if (EBIT_TEST(e->flags, ENTRY_SPECIAL)) {
	debug(20, 3) ("storeSwapOut: %s SPECIAL\n", storeUrl(e));
	return;
    }
    debug(20, 7) ("storeSwapOut: mem->inmem_lo = %" PRINTF_OFF_T "\n",
	mem->inmem_lo);
    debug(20, 7) ("storeSwapOut: mem->inmem_hi = %" PRINTF_OFF_T "\n",
	mem->inmem_hi);
    debug(20, 7) ("storeSwapOut: swapout.queue_offset = %" PRINTF_OFF_T "\n",
	mem->swapout.queue_offset);
    if (mem->swapout.sio)
	debug(20, 7) ("storeSwapOut: storeOffset() = %" PRINTF_OFF_T "\n",
	    storeOffset(mem->swapout.sio));
    assert(mem->inmem_hi >= mem->swapout.queue_offset);
    /*
     * Grab the swapout_size and check to see whether we're going to defer
     * the swapout based upon size
     */
    swapout_size = mem->inmem_hi - mem->swapout.queue_offset;
    if ((e->store_status != STORE_OK) && (swapout_size < store_maxobjsize)) {
	/*
	 * NOTE: the store_maxobjsize here is the max of optional
	 * max-size values from 'cache_dir' lines.  It is not the
	 * same as 'maximum_object_size'.  By default, store_maxobjsize
	 * will be set to -1.  However, I am worried that this
	 * deferance may consume a lot of memory in some cases.
	 * It would be good to make this decision based on reply
	 * content-length, rather than wait to accumulate huge
	 * amounts of object data in memory.
	 */
	debug(20, 5) ("storeSwapOut: Deferring starting swapping out\n");
	return;
    }
    swapout_able = storeSwapOutMaintainMemObject(e);
#if SIZEOF_SQUID_OFF_T <= 4
    if (mem->inmem_hi > 0x7FFF0000) {
	debug(20, 0) ("WARNING: preventing squid_off_t overflow for %s\n", storeUrl(e));
	storeAbort(e);
	return;
    }
#endif
    if (!swapout_able)
	return;
    debug(20, 7) ("storeSwapOut: swapout_size = %" PRINTF_OFF_T "\n",
	swapout_size);
    if (swapout_size == 0) {
	if (e->store_status == STORE_OK)
	    storeSwapOutFileClose(e);
	return;			/* Nevermore! */
    }
    if (e->store_status == STORE_PENDING) {
	/* wait for a full block to write */
	if (swapout_size < SM_PAGE_SIZE)
	    return;
	/*
	 * Wait until we are below the disk FD limit, only if the
	 * next server-side read won't be deferred.
	 */
	if (storeTooManyDiskFilesOpen() && !fwdCheckDeferRead(-1, e))
	    return;
    }
    /* Ok, we have stuff to swap out.  Is there a swapout.sio open? */
    if (e->swap_status == SWAPOUT_NONE && !EBIT_TEST(e->flags, ENTRY_FWD_HDR_WAIT)) {
	assert(mem->swapout.sio == NULL);
	assert(mem->inmem_lo == 0);
	if (storeCheckCachable(e))
	    storeSwapOutStart(e);
	else {
	    /* Now that we know the data is not cachable, free the memory
	     * to make sure the forwarding code does not defer the connection
	     */
	    storeSwapOutMaintainMemObject(e);
	    return;
	}
	/* ENTRY_CACHABLE will be cleared and we'll never get here again */
    }
    if (NULL == mem->swapout.sio)
	return;
    do {
	/*
	 * Evil hack time.
	 * We are paging out to disk in page size chunks. however, later on when
	 * we update the queue position, we might not have a page (I *think*),
	 * so we do the actual page update here.
	 */

	if (mem->swapout.memnode == NULL) {
	    /* We need to swap out the first page */
	    mem->swapout.memnode = mem->data_hdr.head;
	} else {
	    /* We need to swap out the next page */
	    mem->swapout.memnode = mem->swapout.memnode->next;
	}
	/*
	 * Get the length of this buffer. We are assuming(!) that the buffer
	 * length won't change on this buffer, or things are going to be very
	 * strange. I think that after the copy to a buffer is done, the buffer
	 * size should stay fixed regardless so that this code isn't confused,
	 * but we can look at this at a later date or whenever the code results
	 * in bad swapouts, whichever happens first. :-)
	 */
	swap_buf_len = mem->swapout.memnode->len;

	debug(20, 3) ("storeSwapOut: swap_buf_len = %d\n", (int) swap_buf_len);
	assert(swap_buf_len > 0);
	debug(20, 3) ("storeSwapOut: swapping out %d bytes from %" PRINTF_OFF_T "\n",
	    (int) swap_buf_len, mem->swapout.queue_offset);
	mem->swapout.queue_offset += swap_buf_len;
	storeWrite(mem->swapout.sio, stmemNodeGet(mem->swapout.memnode), swap_buf_len, stmemNodeFree);
	/* the storeWrite() call might generate an error */
	if (e->swap_status != SWAPOUT_WRITING)
	    break;
	swapout_size = mem->inmem_hi - mem->swapout.queue_offset;
	if (e->store_status == STORE_PENDING)
	    if (swapout_size < SM_PAGE_SIZE)
		break;
    } while (swapout_size > 0);
    if (NULL == mem->swapout.sio)
	/* oops, we're not swapping out any more */
	return;
    if (e->store_status == STORE_OK) {
	/*
	 * If the state is STORE_OK, then all data must have been given
	 * to the filesystem at this point because storeSwapOut() is
	 * not going to be called again for this entry.
	 */
	assert(mem->inmem_hi == mem->swapout.queue_offset);
	storeSwapOutFileClose(e);
    }
}

void
storeSwapOutFileClose(StoreEntry * e)
{
    MemObject *mem = e->mem_obj;
    storeIOState *sio = mem->swapout.sio;
    assert(mem != NULL);
    debug(20, 3) ("storeSwapOutFileClose: %s\n", storeKeyText(e->hash.key));
    debug(20, 3) ("storeSwapOutFileClose: sio = %p\n", mem->swapout.sio);
    if (sio == NULL)
	return;
    mem->swapout.sio = NULL;
    storeClose(sio);
}

static void
storeSwapOutFileClosed(void *data, int errflag, storeIOState * sio)
{
    generic_cbdata *c = data;
    StoreEntry *e = c->data;
    MemObject *mem = e->mem_obj;
    assert(e->swap_status == SWAPOUT_WRITING);
    cbdataFree(c);
    if (errflag) {
	debug(20, 1) ("storeSwapOutFileClosed: dirno %d, swapfile %08X, errflag=%d\n\t%s\n",
	    e->swap_dirn, e->swap_filen, errflag, xstrerror());
	if (errflag == DISK_NO_SPACE_LEFT) {
	    storeDirDiskFull(e->swap_dirn);
	    storeDirConfigure();
	    storeConfigure();
	}
	if (e->swap_filen > 0)
	    storeUnlink(e);
	e->swap_filen = -1;
	e->swap_dirn = -1;
	e->swap_status = SWAPOUT_NONE;
	storeReleaseRequest(e);
    } else {
	/* swapping complete */
	debug(20, 3) ("storeSwapOutFileClosed: SwapOut complete: '%s' to %d, %08X\n",
	    storeUrl(e), e->swap_dirn, e->swap_filen);
	e->swap_file_sz = objectLen(e) + mem->swap_hdr_sz;
	e->swap_status = SWAPOUT_DONE;
	storeDirUpdateSwapSize(&Config.cacheSwap.swapDirs[e->swap_dirn], e->swap_file_sz, 1);
	if (storeCheckCachable(e)) {
	    storeLog(STORE_LOG_SWAPOUT, e);
	    storeDirSwapLog(e, SWAP_LOG_ADD);
	}
	statCounter.swap.outs++;
    }
    debug(20, 3) ("storeSwapOutFileClosed: %s:%d\n", __FILE__, __LINE__);
    mem->swapout.sio = NULL;
    cbdataUnlock(sio);
    storeSwapOutMaintainMemObject(e);
    storeUnlockObject(e);
}

/*
 * How much of the object data is on the disk?
 */
squid_off_t
storeSwapOutObjectBytesOnDisk(const MemObject * mem)
{
    /*
     * NOTE: storeOffset() represents the disk file size,
     * not the amount of object data on disk.
     * 
     * If we don't have at least 'swap_hdr_sz' bytes
     * then none of the object data is on disk.
     *
     * This should still be safe if swap_hdr_sz == 0,
     * meaning we haven't even opened the swapout file
     * yet.
     */
    off_t nwritten;
    if (mem->swapout.sio == NULL)
	return mem->swapout.queue_offset;
    nwritten = storeOffset(mem->swapout.sio);
    if (nwritten <= mem->swap_hdr_sz)
	return 0;
    return nwritten - mem->swap_hdr_sz;
}

/*
 * Is this entry a candidate for writing to disk?
 */
static int
storeSwapOutAble(const StoreEntry * e)
{
    if (e->mem_obj->inmem_hi > Config.Store.maxObjectSize)
	return 0;
    if (!EBIT_TEST(e->flags, ENTRY_CACHABLE))
	return 0;
    if (e->mem_obj->swapout.sio != NULL)
	return 1;
    if (e->mem_obj->swapout.queue_offset)
	if (e->mem_obj->swapout.queue_offset == e->mem_obj->inmem_hi)
	    return 1;
    if (e->mem_obj->inmem_lo > 0)
	return 0;
    return 1;
}
