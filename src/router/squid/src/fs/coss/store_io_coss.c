
/*
 * $Id: store_io_coss.c,v 1.35 2006/11/05 21:14:32 hno Exp $
 *
 * DEBUG: section 79    Storage Manager COSS Interface
 * AUTHOR: Eric Stern
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
#if HAVE_AIO_H
#include <aio.h>
#endif
#include "async_io.h"
#include "store_coss.h"
#if USE_AUFSOPS
#include "../aufs/async_io.h"
#endif

#if USE_AUFSOPS
static AIOCB storeCossWriteMemBufDone;
#else
static DWCB storeCossWriteMemBufDone;
#endif
static void storeCossIOCallback(storeIOState * sio, int errflag);
static char *storeCossMemPointerFromDiskOffset(CossInfo * cs, off_t offset, CossMemBuf ** mb);
static void storeCossMemBufLock(SwapDir * SD, storeIOState * e);
static void storeCossMemBufUnlock(SwapDir * SD, storeIOState * e);
static void storeCossWriteMemBuf(SwapDir * SD, CossMemBuf * t);
static CossMemBuf *storeCossCreateMemBuf(SwapDir * SD, int stripe, sfileno curfn, int *collision);
static CossMemBuf *storeCossCreateMemOnlyBuf(SwapDir * SD);
static CBDUNL storeCossIOFreeEntry;
static off_t storeCossFilenoToDiskOffset(sfileno f, CossInfo *);
static sfileno storeCossDiskOffsetToFileno(off_t o, CossInfo *);
static void storeCossMaybeWriteMemBuf(SwapDir * SD, CossMemBuf * t);
static void storeCossMaybeFreeBuf(CossInfo * cs, CossMemBuf * mb);
int storeCossFilenoToStripe(CossInfo * cs, sfileno filen);

static void membuf_describe(CossMemBuf * t, int level, int line);

/* Handle relocates - temporary routines until readops have been fleshed out */
void storeCossNewPendingRelocate(CossInfo * cs, storeIOState * sio, sfileno original_filen, sfileno new_filen);
CossPendingReloc *storeCossGetPendingReloc(CossInfo * cs, sfileno new_filen);
#if USE_AUFSOPS
AIOCB storeCossCompletePendingReloc;
#else
DRCB storeCossCompletePendingReloc;
#endif

/* Read operation code */
CossReadOp *storeCossCreateReadOp(CossInfo * cs, storeIOState * sio);
void storeCossCompleteReadOp(CossInfo * cs, CossReadOp * op, int error);
void storeCossKickReadOp(CossInfo * cs, CossReadOp * op);

CBDATA_TYPE(storeIOState);
CBDATA_TYPE(CossMemBuf);
CBDATA_TYPE(CossPendingReloc);

/* === PUBLIC =========================================================== */

static sfileno
storeCossMemOnlyAllocate(SwapDir * SD, const StoreEntry * e)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    CossMemBuf *newmb;
    off_t retofs;
    size_t allocsize;
    sfileno f;

    coss_stats.alloc.memalloc++;
    allocsize = e->swap_file_sz;

    if (cs->current_memonly_membuf == NULL) {
	newmb = storeCossCreateMemOnlyBuf(SD);
	cs->current_memonly_membuf = newmb;

	if (newmb == NULL) {
	    return -1;
	}
	cs->current_memonly_offset = cs->current_memonly_membuf->diskstart;
    } else if ((cs->current_memonly_offset + allocsize) >= cs->current_memonly_membuf->diskend) {
	debug(79, 3) ("storeCossMemOnlyAllocate: overflow for buffer %d (%p)\n", cs->curmemstripe, cs->current_memonly_membuf);
	cs->current_memonly_membuf->flags.full = 1;
	storeCossMaybeWriteMemBuf(SD, cs->current_memonly_membuf);
	/* cs->current_memonly_membuf may be invalid at this point */

	newmb = storeCossCreateMemOnlyBuf(SD);
	cs->current_memonly_membuf = newmb;

	if (newmb == NULL) {
	    return -1;
	}
	cs->current_memonly_offset = cs->current_memonly_membuf->diskstart;
    }
    retofs = cs->current_memonly_offset;
    cs->current_memonly_offset = retofs + allocsize;
    cs->current_memonly_membuf->numobjs++;
    cs->current_memonly_offset = ((cs->current_memonly_offset + cs->blksz_mask) >> cs->blksz_bits) << cs->blksz_bits;
    f = storeCossDiskOffsetToFileno(retofs, cs);
    assert(f >= 0 && f <= 0xffffff);
    debug(79, 3) ("storeCossMemOnlyAllocate: offset %" PRId64 ", filen: %d\n", (int64_t) retofs, f);
    return f;

}

/*
 * This routine sucks. I want to rewrite it when possible, and I also think
 * that we should check after creatmembuf() to see if the object has a
 * RELEASE_REQUEST set on it (thanks Eric!) rather than this way which seems
 * to work..
 * -- Adrian
 */
static sfileno
storeCossAllocate(SwapDir * SD, const StoreEntry * e, int which)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    CossMemBuf *newmb;
    off_t retofs;
    size_t allocsize;
    int coll = 0;
    sfileno f;
    sfileno checkf;

    /* Make sure we chcek collisions if reallocating */
    if (which == COSS_ALLOC_REALLOC) {
	checkf = e->swap_filen;
	coss_stats.alloc.realloc++;
    } else {
	checkf = -1;
	coss_stats.alloc.alloc++;
    }

    if (e->swap_file_sz > 0)
	allocsize = e->swap_file_sz;
    else
	allocsize = objectLen(e) + e->mem_obj->swap_hdr_sz;

    /* Since we're not supporting NOTIFY anymore, lets fail */
    assert(which != COSS_ALLOC_NOTIFY);

    /* Check to see if we need to allocate a membuf to start */
    if (cs->current_membuf == NULL) {
	if (cs->curstripe < (cs->numstripes - 1))
	    newmb = storeCossCreateMemBuf(SD, cs->curstripe + 1, checkf, &coll);
	else
	    newmb = storeCossCreateMemBuf(SD, 0, checkf, &coll);

	cs->current_membuf = newmb;
	if (newmb == NULL) {
	    cs->sizerange_max = SD->max_objsize;
	    return -1;
	}
	cs->current_offset = cs->current_membuf->diskstart;

	/* Check if we have overflowed the disk .. */
    } else if ((cs->current_offset + allocsize) > ((off_t) SD->max_size << 10)) {
	/*
	 * tried to allocate past the end of the disk, so wrap
	 * back to the beginning
	 */
	coss_stats.disk_overflows++;
	cs->current_membuf->flags.full = 1;
	cs->numfullstripes++;
	cs->current_membuf->diskend = cs->current_offset;
	storeCossMaybeWriteMemBuf(SD, cs->current_membuf);
	/* cs->current_membuf may be invalid at this point */
	cs->current_offset = 0;	/* wrap back to beginning */
	debug(79, 2) ("storeCossAllocate: %s: wrap to 0\n", stripePath(SD));

	newmb = storeCossCreateMemBuf(SD, 0, checkf, &coll);
	cs->current_membuf = newmb;
	if (newmb == NULL) {
	    cs->sizerange_max = SD->max_objsize;
	    return -1;
	}
	/* Check if we have overflowed the MemBuf */
    } else if ((cs->current_offset + allocsize) >= cs->current_membuf->diskend) {
	/*
	 * Skip the blank space at the end of the stripe. start over.
	 */
	coss_stats.stripe_overflows++;
	cs->current_membuf->flags.full = 1;
	cs->numfullstripes++;
	cs->current_offset = cs->current_membuf->diskend;
	storeCossMaybeWriteMemBuf(SD, cs->current_membuf);
	/* cs->current_membuf may be invalid at this point */
	debug(79, 3) ("storeCossAllocate: %s: New offset - %" PRId64 "\n", stripePath(SD),
	    (int64_t) cs->current_offset);
	assert(cs->curstripe < (cs->numstripes - 1));
	newmb = storeCossCreateMemBuf(SD, cs->curstripe + 1, checkf, &coll);
	cs->current_membuf = newmb;
	if (newmb == NULL) {
	    cs->sizerange_max = SD->max_objsize;
	    return -1;
	}
    }
    /* If we didn't get a collision, then update the current offset and return it */
    if (coll == 0) {
	retofs = cs->current_offset;
	cs->current_offset = retofs + allocsize;
	cs->current_membuf->numobjs++;
	/* round up to our blocksize */
	cs->current_offset = ((cs->current_offset + cs->blksz_mask) >> cs->blksz_bits) << cs->blksz_bits;
	f = storeCossDiskOffsetToFileno(retofs, cs);
	assert(f >= 0 && f <= 0xffffff);
	debug(79, 3) ("storeCossAllocate: offset %" PRId64 ", filen: %d\n", (int64_t) retofs, f);

	/* 
	 * Keep track of the largest object we can accept based on the
	 * max-wasted-space value
	 */
	cs->sizerange_max = cs->current_membuf->diskend - cs->current_offset;
	if (cs->sizerange_max < cs->sizerange_min)
	    cs->sizerange_max = cs->sizerange_min;

	return f;
    } else {
	/* Reset this to a safe value */
	cs->sizerange_max = SD->max_objsize;

	coss_stats.alloc.collisions++;
	debug(79, 3) ("storeCossAllocate: %s: Collision\n", stripePath(SD));
	return -1;
    }
}

void
storeCossUnlink(SwapDir * SD, StoreEntry * e)
{
    debug(79, 3) ("storeCossUnlink: %s: offset %d\n", stripePath(SD), e->swap_filen);
    coss_stats.unlink.ops++;
    coss_stats.unlink.success++;
    storeCossRemove(SD, e);
}

void
storeCossRecycle(SwapDir * SD, StoreEntry * e)
{
    debug(79, 3) ("storeCossRecycle: %s: offset %d\n", stripePath(SD), e->swap_filen);

    /* If there is a valid filen remove from COSS linked list */
    if (e->swap_filen > -1) {
	storeCossUnlink(SD, e);

	/* 
	 * Set filen and dirn to -1.  
	 * This makes storeRelease() treat the entry differently 
	 */
	e->swap_filen = -1;
	e->swap_dirn = -1;
    }
}

static int
storeCossRelocateRequired(CossInfo * cs, sfileno f)
{
    int stripes_written;
    int original_stripe = storeCossFilenoToStripe(cs, f);

    if (cs->curstripe > original_stripe)
	stripes_written = cs->curstripe - original_stripe;
    else
	stripes_written = cs->numstripes + cs->curstripe - original_stripe;

    /* Relocate if stripes_written > minimum_stripe_distance */
    return (stripes_written > cs->minimum_stripe_distance);
}

storeIOState *
storeCossCreate(SwapDir * SD, StoreEntry * e, STFNCB * file_callback, STIOCB * callback, void *callback_data)
{
    CossState *cstate;
    storeIOState *sio;
    CossInfo *cs = SD->fsdata;

    assert(cs->rebuild.rebuilding == 0);
    coss_stats.create.ops++;
    sio = cbdataAlloc(storeIOState);
    cstate = memPoolAlloc(coss_state_pool);
    sio->fsstate = cstate;
    sio->offset = 0;
    sio->mode = O_WRONLY | O_BINARY;

    /*
     * If we get handed an object with a size of -1,
     * the squid code is broken
     */
    assert(e->mem_obj->object_sz != -1);

    /*
     * this one is kinda strange - Eric called storeCossAllocate(), then
     * storeCossOpen(O_RDONLY) .. weird. Anyway, I'm allocating this now.
     */
    sio->st_size = objectLen(e) + e->mem_obj->swap_hdr_sz;
    sio->swap_dirn = SD->index;
    sio->swap_filen = storeCossAllocate(SD, e, COSS_ALLOC_ALLOCATE);
    debug(79, 3) ("storeCossCreate: %p: filen: %d\n", sio, sio->swap_filen);
    assert(-1 != sio->swap_filen);

    sio->callback = callback;
    sio->file_callback = file_callback;
    sio->callback_data = callback_data;
    cbdataLock(callback_data);
    sio->e = (StoreEntry *) e;

    cstate->flags.writing = 0;
    cstate->flags.reading = 0;
    cstate->reqdiskoffset = -1;

    /* Now add it into the index list */
    e->swap_filen = sio->swap_filen;
    e->swap_dirn = sio->swap_dirn;
    storeCossAdd(SD, e, cs->curstripe);

    storeCossMemBufLock(SD, sio);
    coss_stats.create.success++;
    return sio;
}

storeIOState *
storeCossOpen(SwapDir * SD, StoreEntry * e, STFNCB * file_callback,
    STIOCB * callback, void *callback_data)
{
    storeIOState *sio;
    char *p;
    CossState *cstate;
    sfileno f = e->swap_filen;
    sfileno nf;
    CossInfo *cs = (CossInfo *) SD->fsdata;

    assert(cs->rebuild.rebuilding == 0);

    sio = cbdataAlloc(storeIOState);
    cstate = memPoolAlloc(coss_state_pool);

    debug(79, 3) ("storeCossOpen: %p: offset %d\n", sio, f);
    coss_stats.open.ops++;

    sio->fsstate = cstate;
    sio->swap_filen = f;
    sio->swap_dirn = SD->index;
    sio->offset = 0;
    sio->mode = O_RDONLY | O_BINARY;
    sio->callback = callback;
    sio->file_callback = file_callback;
    sio->callback_data = callback_data;
    cbdataLock(callback_data);
    sio->st_size = e->swap_file_sz;
    sio->e = e;

    cstate->flags.writing = 0;
    cstate->flags.reading = 0;
    cstate->reqdiskoffset = -1;

    /* make local copy so we don't have to lock membuf */
    p = storeCossMemPointerFromDiskOffset(cs, storeCossFilenoToDiskOffset(f, cs), NULL);
    if (p) {
	coss_stats.open_mem_hits++;
	// This seems to cause a crash: either the membuf pointer is set wrong or the membuf
	// is deallocated from underneath us.
	storeCossMemBufLock(SD, sio);
	debug(79, 3) ("storeCossOpen: %s: memory hit!\n", stripePath(SD));
    } else {
	/* Do the allocation */
	/* this is the first time we've been called on a new sio
	 * read the whole object into memory, then return the 
	 * requested amount
	 */
	coss_stats.open_mem_misses++;
	/*
	 * This bit of code actually does the LRU disk thing - we realloc
	 * a place for the object here, and the file_read() reads the object
	 * into the cossmembuf for later writing ..
	 */
	cstate->reqdiskoffset = storeCossFilenoToDiskOffset(sio->swap_filen, cs);
	assert(cstate->reqdiskoffset >= 0);

	/* If the object is allocated too recently, make a memory-only copy */
	if (storeCossRelocateRequired(cs, sio->swap_filen)) {
	    debug(79, 3) ("storeCossOpen: %s: memory miss - doing reallocation (Current stripe : %d  Object in stripe : %d)\n", stripePath(SD), cs->curstripe, storeCossFilenoToStripe(cs, sio->swap_filen));
	    nf = storeCossAllocate(SD, e, COSS_ALLOC_REALLOC);
	} else {
	    debug(79, 3) ("storeCossOpen: %s memory miss - not reallocating (Current stripe : %d  Object in stripe : %d)\n", stripePath(SD), cs->curstripe, storeCossFilenoToStripe(cs, sio->swap_filen));
	    nf = storeCossMemOnlyAllocate(SD, e);
	    if (nf == -1) {
		debug(79, 3) ("storeCossOpen: %s memory miss - reallocating because all membufs are in use\n", stripePath(SD));
		nf = storeCossAllocate(SD, e, COSS_ALLOC_REALLOC);
	    }
	}
	if (nf == -1) {
	    /* We have to clean up neatly .. */
	    coss_stats.open.fail++;
	    cbdataFree(sio);
	    cs->numcollisions++;
	    debug(79, 3) ("storeCossOpen: Reallocation of %d/%d failed\n", e->swap_dirn, e->swap_filen);
	    /* XXX XXX XXX Will squid call storeUnlink for this object? */
	    return NULL;
	}
	if (nf < cs->max_disk_nf) {
	    /* Remove the object from its currently-allocated stripe */
	    storeCossRemove(SD, e);
	    storeCossNewPendingRelocate(cs, sio, sio->swap_filen, nf);
	    sio->swap_filen = nf;
	    cstate->flags.reloc = 1;
	    /* Notify the upper levels that we've changed file number */
	    sio->file_callback(sio->callback_data, 0, sio);
	    /*
	     * lock the new buffer so it doesn't get swapped out on us
	     * this will get unlocked in storeCossClose
	     */
	    storeCossMemBufLock(SD, sio);
	    /*
	     * Do the index magic to keep the disk and memory LRUs identical
	     * by adding the object into the link list on the current stripe
	     */
	    storeCossAdd(SD, e, cs->curstripe);
	} else {
	    /* Relocate the object in COSS, but not in other layers */
	    storeCossNewPendingRelocate(cs, sio, sio->swap_filen, nf);
	    sio->swap_filen = nf;
	    cstate->flags.reloc = 1;

	    /*
	     * lock the new buffer so it doesn't get swapped out on us
	     * this will get unlocked in storeCossClose
	     */
	    storeCossMemBufLock(SD, sio);
	}
    }
    coss_stats.open.success++;
    return sio;
}

/*
 * Aha! The unlocked membuf.
 *
 * If its storeCossCreate, then it was locked. Fine.
 * If it was storeCossOpen() and we found the object in-stripe then cool,
 *   its locked.
 * If it was storeCossOpen() and we didn't find the object in-stripe then
 *   we reallocated the object into the current stripe and locked THAT.
 */
void
storeCossClose(SwapDir * SD, storeIOState * sio)
{
    debug(79, 3) ("storeCossClose: %p: offset %d\n", sio, sio->swap_filen);
    coss_stats.close.ops++;
    coss_stats.close.success++;
    storeCossMemBufUnlock(SD, sio);
    storeCossIOCallback(sio, 0);
}

void
storeCossRead(SwapDir * SD, storeIOState * sio, char *buf, size_t size, squid_off_t offset, STRCB * callback, void *callback_data)
{
    CossState *cstate = (CossState *) sio->fsstate;
    CossInfo *cs = (CossInfo *) SD->fsdata;
    CossReadOp *op;

    coss_stats.read.ops++;
    assert(sio->read.callback == NULL);
    assert(sio->read.callback_data == NULL);
    sio->read.callback = callback;
    sio->read.callback_data = callback_data;
    debug(79, 3) ("storeCossRead: %s: file number %d offset %ld\n", stripePath(SD), sio->swap_filen, (long int) offset);
    sio->offset = offset;
    cstate->flags.reading = 1;
    if ((offset + size) > sio->st_size)
	size = sio->st_size - offset;
    cstate->requestlen = size;
    cstate->requestbuf = buf;
    cstate->requestoffset = offset;
    /* All of these reads should be treated as pending ones */
    /* Ie, we create a read op; then we 'kick' the read op to see if it can be completed now */
    op = storeCossCreateReadOp(cs, sio);
    storeCossKickReadOp(cs, op);
}

void
storeCossWrite(SwapDir * SD, storeIOState * sio, char *buf, size_t size, squid_off_t offset, FREE * free_func)
{
    char *dest;
    CossMemBuf *membuf;
    off_t diskoffset;

    /*
     * If we get handed an object with a size of -1,
     * the squid code is broken
     */
    assert(sio->e->mem_obj->object_sz != -1);
    coss_stats.write.ops++;

    if (sio->offset != offset) {
	debug(79, 1) ("storeCossWrite: Possible data corruption on fileno %d, offsets do not match (Current:%" PRINTF_OFF_T " Want:%" PRINTF_OFF_T ")\n", sio->swap_filen, sio->offset, offset);
    }
    debug(79, 3) ("storeCossWrite: %s: offset %ld, len %lu\n", stripePath(SD),
	(long int) sio->offset, (unsigned long int) size);
    diskoffset = storeCossFilenoToDiskOffset(sio->swap_filen, SD->fsdata) + sio->offset;
    dest = storeCossMemPointerFromDiskOffset(SD->fsdata, diskoffset, &membuf);
    assert(dest != NULL);
    xmemcpy(dest, buf, size);
    sio->offset += size;
    if (free_func)
	(free_func) (buf);
    coss_stats.write.success++;
}


/*  === STATIC =========================================================== */

static void
storeCossIOCallback(storeIOState * sio, int errflag)
{
    CossState *cstate = (CossState *) sio->fsstate;
    debug(79, 3) ("storeCossIOCallback: errflag=%d\n", errflag);
    assert(NULL == cstate->locked_membuf);
    if (cbdataValid(sio->callback_data))
	sio->callback(sio->callback_data, errflag, sio);
    cbdataUnlock(sio->callback_data);
    sio->callback_data = NULL;
    cbdataFree(sio);
}

static char *
storeCossMemPointerFromDiskOffset(CossInfo * cs, off_t offset, CossMemBuf ** mb)
{
    CossMemBuf *t;
    dlink_node *m;

    for (m = cs->membufs.head; m; m = m->next) {
	t = m->data;
	if ((offset >= t->diskstart) && (offset < t->diskend)) {
	    if (mb)
		*mb = t;
	    return &t->buffer[offset - t->diskstart];
	}
    }

    if (mb)
	*mb = NULL;
    return NULL;
}

static CossMemBuf *
storeCossFilenoToMembuf(SwapDir * SD, sfileno s)
{
    CossMemBuf *t = NULL;
    dlink_node *m;
    CossInfo *cs = (CossInfo *) SD->fsdata;
    off_t o = storeCossFilenoToDiskOffset(s, cs);
    for (m = cs->membufs.head; m; m = m->next) {
	t = m->data;
	if ((o >= t->diskstart) && (o < t->diskend))
	    break;
    }
    assert(t);
    return t;
}

static void
storeCossMemBufLockPending(CossPendingReloc * pr, CossMemBuf * t)
{
    assert(t->flags.dead == 0);
    assert(pr->locked_membuf == NULL);
    debug(79, 3) ("storeCossMemBufLockPending: locking %p, lockcount %d\n",
	t, t->lockcount);
    pr->locked_membuf = t;
    t->lockcount++;
}

static void
storeCossMemBufUnlockPending(CossPendingReloc * pr, CossInfo * cs)
{
    CossMemBuf *t = pr->locked_membuf;
    if (NULL == t)
	return;
    assert(t->flags.dead == 0);
    debug(79, 3) ("storeCossMemBufLockPending: unlocking %p, lockcount %d\n",
	t, t->lockcount);
    t->lockcount--;
    pr->locked_membuf = NULL;

    if (!t->flags.written) {
	storeCossMaybeWriteMemBuf(t->SD, t);
    } else {
	/* cs->current_membuf may be invalid at this point */
	storeCossMaybeFreeBuf(cs, t);
    }
}

static void
storeCossMemBufLock(SwapDir * SD, storeIOState * sio)
{
    CossMemBuf *t = storeCossFilenoToMembuf(SD, sio->swap_filen);
    CossState *cstate = (CossState *) sio->fsstate;
    assert(cstate->locked_membuf == NULL);
    assert(t->flags.dead == 0);
    debug(79, 3) ("storeCossMemBufLock: locking %p, lockcount %d\n",
	t, t->lockcount);
    cstate->locked_membuf = t;
    t->lockcount++;
}

static void
storeCossMemBufUnlock(SwapDir * SD, storeIOState * sio)
{
    CossState *cstate = (CossState *) sio->fsstate;
    CossInfo *cs = SD->fsdata;
    CossMemBuf *t = cstate->locked_membuf;
    if (NULL == t)
	return;
    assert(t->flags.dead == 0);
    debug(79, 3) ("storeCossMemBufUnlock: unlocking %p, lockcount %d\n",
	t, t->lockcount);
    t->lockcount--;
    cstate->locked_membuf = NULL;
    if (!t->flags.written) {
	storeCossMaybeWriteMemBuf(SD, t);
    } else {
	/* cs->current_membuf may be invalid at this point */
	storeCossMaybeFreeBuf(cs, t);
    }
}

static void
storeCossMaybeWriteMemBuf(SwapDir * SD, CossMemBuf * t)
{
    //CossInfo *cs = SD->fsdata;
    membuf_describe(t, 3, __LINE__);
    assert(t->flags.dead == 0);
    if (!t->flags.full)
	debug(79, 3) ("membuf %p not full\n", t);
    else if (t->flags.writing)
	debug(79, 3) ("membuf %p writing\n", t);
    else if (t->lockcount)
	debug(79, 3) ("membuf %p lockcount=%d\n", t, t->lockcount);
    else if (t->flags.written)
	debug(79, 3) ("membuf %p written\n", t);
    else
	storeCossWriteMemBuf(SD, t);
    /* t may be invalid at this point */
}

void
storeCossSync(SwapDir * SD)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    dlink_node *m;
    off_t end;

    /* First, flush pending IO ops */
#if USE_AUFSOPS
    aioSync(SD);
#else
    a_file_syncqueue(&cs->aq);
#endif

    /* Then, flush any in-memory partial membufs */
    if (!cs->membufs.head)
	return;
    for (m = cs->membufs.head; m; m = m->next) {
	CossMemBuf *t = m->data;
	if (t->flags.writing) {
	    debug(79, 1) ("WARNING: sleeping for 5 seconds in storeCossSync()\n");
	    sleep(5);		/* XXX EEEWWW! */
	}
	lseek(cs->fd, t->diskstart, SEEK_SET);
	end = (t == cs->current_membuf) ? cs->current_offset : t->diskend;
	FD_WRITE_METHOD(cs->fd, t->buffer, end - t->diskstart);
    }
}

static void
storeCossWriteMemBuf(SwapDir * SD, CossMemBuf * t)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    int cur_load_interval = (squid_curtime / cs->load_interval) % 2;
    int prev_load_interval = ((squid_curtime + cs->load_interval) / cs->load_interval) % 2;
    assert(t->flags.dead == 0);
    debug(79, 3) ("storeCossWriteMemBuf: %p: offset %ld, len %ld\n", t,
	(long int) t->diskstart, (long int) (t->diskend - t->diskstart));
    t->flags.writing = 1;
    /* Check to see whether anything has a pending relocate (ie, a disk read)
     * scheduled from the disk data we're about to overwrite.
     * According to the specification this should never, ever happen - all the
     * objects underneath this stripe were deallocated before we started
     * using them - but there is a possibility that an object was opened
     * before the objects underneath the membufs stripe were purged and there
     * is still a pending relocate for it. Its a slim chance but it might happen.
     */
    if (!(t->flags.memonly)) {
	coss_stats.stripe_write.ops++;
	assert(t->stripe < cs->numstripes);
	if (cs->stripes[t->stripe].pending_relocs > 0) {
	    debug(79, 1) ("WARNING: %s: One or more pending relocate (reads) from stripe %d are queued - and I'm now writing over that part of the disk. This may result in object data corruption!\n", stripePath(SD), t->stripe);
	}
	/* Update load stats */
	cs->loadcalc[cur_load_interval] += 1;
	cs->loadcalc[prev_load_interval] = 0;

	/*
	 * normally nothing should have this node locked here - but between the time
	 * we call a_file_write and the IO completes someone might have snuck in and
	 * attached itself somehow. This is why there's a distinction between "written"
	 * and "writing". Read the rest of the code for more details.
	 */
#if USE_AUFSOPS
	/* XXX The last stripe, for now, ain't the coss stripe size for some reason */
	/* XXX This may cause problems later on; worry about figuring it out later on */
	//assert(t->diskend - t->diskstart == COSS_MEMBUF_SZ);
	debug(79, 3) ("aioWrite: FD %d: disk start: %" PRIu64 ", size %" PRIu64 "\n", cs->fd, (uint64_t) t->diskstart, (uint64_t) t->diskend - t->diskstart);
	aioWrite(cs->fd, t->diskstart, &(t->buffer[0]), COSS_MEMBUF_SZ, storeCossWriteMemBufDone, t, NULL);
#else
	a_file_write(&cs->aq, cs->fd, t->diskstart, &t->buffer,
	    COSS_MEMBUF_SZ, storeCossWriteMemBufDone, t, NULL);
#endif
    } else {
	/* No need to write, just mark as written and free */
	t->flags.written = 1;
	t->flags.writing = 0;
	storeCossMaybeFreeBuf(cs, t);
    }
}

/*
 * Check if a memory buffer can be freed.
 * Memory buffers can be freed if their refcount is 0 and they've been written.
 */
static void
storeCossMaybeFreeBuf(CossInfo * cs, CossMemBuf * mb)
{
    assert(mb->lockcount >= 0);
    /* It'd be nice if we could walk all the pending sio's somehow to see if some has this membuf locked .. */
    if (mb->flags.dead == 1) {
	debug(79, 1) ("storeCossMaybeFreeBuf: %p: dead; it'll be freed soon enough\n", mb);
	return;
    }
    /* Place on dead list rather than free
     * the asyncio code fails over to a 'sync' path; which may mean a membuf is
     * deallocated somewhere deep in the stack level. This way we just mark them
     * as dead and deallocate membufs early in the stack frame (ie, before we
     * call the asyncio disk completion handler.)
     */
    if (mb->lockcount == 0 && mb->flags.written == 1) {
	/* We need to wait until here to mark the membuf as
	 * free so we can re-alocate it
	 */
	if (mb->flags.memonly) {
	    assert(cs->memstripes[mb->stripe].membuf == mb);
	    cs->memstripes[mb->stripe].membuf = NULL;
	} else {
	    cs->numfullstripes--;
	}
	debug(79, 3) ("storeCossMaybeFreeBuf: %p: lockcount = 0, written = 1: marking dead\n", mb);
	mb->flags.dead = 1;
	dlinkDelete(&mb->node, &cs->membufs);
	dlinkAddTail(mb, &mb->node, &cs->dead_membufs);
	coss_stats.dead_stripes++;
	coss_stats.stripes--;
    }
}

void
storeCossFreeDeadMemBufs(CossInfo * cs)
{
    CossMemBuf *mb;
    while (cs->dead_membufs.head != NULL) {
	mb = cs->dead_membufs.head->data;
	assert(mb->flags.dead == 1);
	debug(79, 3) ("storeCossFreeDeadMemBufs: %p: freeing\n", mb);
	dlinkDelete(&mb->node, &cs->dead_membufs);
	cbdataFree(mb);
	coss_stats.dead_stripes--;
    }
}

/*
 * Writing a membuf has completed. Set the written flag to 1; membufs might have been
 * locked for read between the initial membuf write and the completion of the disk
 * write.
 */
#if USE_AUFSOPS
static void
storeCossWriteMemBufDone(int fd, void *my_data, const char *buf, int aio_return, int aio_errno)
#else
static void
storeCossWriteMemBufDone(int fd, int r_errflag, size_t r_len, void *my_data)
#endif
{
    CossMemBuf *t = my_data;
    CossInfo *cs = (CossInfo *) t->SD->fsdata;
    int errflag;
    int len;
#if USE_AUFSOPS
    len = aio_return;
    if (aio_errno)
	errflag = aio_errno == ENOSPC ? DISK_NO_SPACE_LEFT : DISK_ERROR;
    else
	errflag = DISK_OK;
#else
    len = r_len;
    errflag = r_errflag;
#endif

    debug(79, 3) ("storeCossWriteMemBufDone: stripe %d, buf %p, len %ld\n", t->stripe, t, (long int) len);
    if (errflag) {
	coss_stats.stripe_write.fail++;
	debug(79, 1) ("storeCossWriteMemBufDone: got failure (%d)\n", errflag);
	debug(79, 1) ("FD %d, size=%d\n", fd, (int) (t->diskend - t->diskstart));
    } else {
	coss_stats.stripe_write.success++;
    }
    assert(cs->stripes[t->stripe].membuf == t);
    debug(79, 2) ("storeCossWriteMemBufDone: %s: stripe %d: numobjs written: %d, lockcount %d\n", stripePath(t->SD), t->stripe, t->numobjs, t->lockcount);
    cs->stripes[t->stripe].numdiskobjs = t->numobjs;
    cs->stripes[t->stripe].membuf = NULL;
    t->flags.written = 1;
    t->flags.writing = 0;
    storeCossMaybeFreeBuf(cs, t);
}

static CossMemBuf *
storeCossCreateMemOnlyBuf(SwapDir * SD)
{
    CossMemBuf *newmb;
    CossInfo *cs = (CossInfo *) SD->fsdata;
    off_t start;
    int stripe;
    static time_t last_warn = 0;

    /* TODO: Maybe make this a simple search for a free membuf */
    for (stripe = 0; stripe < cs->nummemstripes; stripe++) {
	if (cs->memstripes[stripe].membuf == NULL)
	    break;
    }
    if (stripe >= cs->nummemstripes) {
	if (last_warn + 15 < squid_curtime) {
	    debug(79, 1) ("storeCossCreateMemOnlyBuf: no free membufs.  You may need to increase the value of membufs on the %s cache_dir\n", stripePath(SD));
	    last_warn = squid_curtime;
	}
	return NULL;
    }
    cs->curmemstripe = stripe;

    start = (off_t) stripe *COSS_MEMBUF_SZ;
    newmb = cbdataAlloc(CossMemBuf);

    cs->memstripes[stripe].membuf = newmb;
    newmb->diskstart = ((off_t) SD->max_size << 10) + start;
    newmb->stripe = stripe;
    newmb->diskend = newmb->diskstart + COSS_MEMBUF_SZ;
    newmb->flags.full = 0;
    newmb->flags.writing = 0;
    newmb->flags.memonly = 1;
    newmb->lockcount = 0;
    newmb->numobjs = 0;
    newmb->SD = SD;

    dlinkAdd(newmb, &newmb->node, &cs->membufs);

    coss_stats.stripes++;
    return newmb;
}

/*
 * This creates a memory buffer but assumes its going to be at the end
 * of the "LRU" and thusly will delete expire objects which appear under
 * it.
 */
static CossMemBuf *
storeCossCreateMemBuf(SwapDir * SD, int stripe, sfileno curfn, int *collision)
{
    CossMemBuf *newmb, *t;
    StoreEntry *e;
    dlink_node *m, *n;
    int numreleased = 0;
    CossInfo *cs = (CossInfo *) SD->fsdata;
    off_t start = (off_t) stripe * COSS_MEMBUF_SZ;
    off_t o;
    static time_t last_warn = 0;
    assert(start >= 0);

    if (cs->numfullstripes >= cs->maxfullstripes) {
	if (last_warn + 15 < squid_curtime) {
	    debug(79, 1) ("storeCossCreateMemBuf: Maximum number of full buffers reached on %s. You may need to increase the maxfullbuffers option for this cache_dir\n", stripePath(SD));
	    last_warn = squid_curtime;
	}
	return NULL;
    }
    /* No, we shouldn't ever try to create a membuf if we haven't freed the one on
     * this stripe. Grr */
    assert(cs->stripes[stripe].membuf == NULL);
    cs->curstripe = stripe;

    newmb = cbdataAlloc(CossMemBuf);
    cs->stripes[stripe].membuf = newmb;
    newmb->diskstart = start;
    newmb->stripe = stripe;
    debug(79, 2) ("storeCossCreateMemBuf: %s: creating new membuf at stripe %d,  %" PRId64 " (%p)\n", stripePath(SD), stripe, (int64_t) newmb->diskstart, newmb);
    newmb->diskend = newmb->diskstart + COSS_MEMBUF_SZ;
    newmb->flags.full = 0;
    newmb->flags.writing = 0;
    newmb->lockcount = 0;
    newmb->numobjs = 0;
    newmb->SD = SD;
    /* XXX This should be reversed, with the new buffer last in the chain */
    dlinkAdd(newmb, &newmb->node, &cs->membufs);
    assert(newmb->diskstart >= 0);
    assert(newmb->diskend >= 0);

    /* Print out the list of membufs */
    debug(79, 3) ("storeCossCreateMemBuf: %s: membuflist:\n", stripePath(SD));
    for (m = cs->membufs.head; m; m = m->next) {
	t = m->data;
	membuf_describe(t, 3, __LINE__);
    }

    /*
     * Kill objects from the tail to make space for a new chunk
     */
    m = cs->stripes[stripe].objlist.head;
    while (m != NULL) {
	n = m->next;
	e = m->data;
	o = storeCossFilenoToDiskOffset(e->swap_filen, cs);
	if (curfn > -1 && curfn == e->swap_filen)
	    *collision = 1;	/* Mark an object alloc collision */
	assert((o >= newmb->diskstart) && (o < newmb->diskend));
	debug(79, 3) ("COSS: %s: stripe %d, releasing filen %d (offset %" PRINTF_OFF_T ")\n", stripePath(SD), stripe, e->swap_filen, (squid_off_t) o);
	storeRelease(e);
	numreleased++;
	m = n;
    }
    if (numreleased > 0)
	debug(79, 3) ("storeCossCreateMemBuf: this allocation released %d storeEntries\n", numreleased);
    coss_stats.stripes++;
    return newmb;
}

/*
 * Creates the initial membuf after rebuild
 */
void
storeCossStartMembuf(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    CossMemBuf *newmb;
    CBDATA_INIT_TYPE_FREECB(storeIOState, storeCossIOFreeEntry);
    CBDATA_INIT_TYPE_FREECB(CossMemBuf, NULL);
    CBDATA_INIT_TYPE_FREECB(storeIOState, storeCossIOFreeEntry);
    CBDATA_INIT_TYPE_FREECB(CossPendingReloc, NULL);
    /*
     * XXX for now we start at the beginning of the disk;
     * The rebuild logic doesn't 'know' to pad out the current
     * offset to make it a multiple of COSS_MEMBUF_SZ.
     */
    newmb = storeCossCreateMemBuf(sd, 0, -1, NULL);
    assert(!cs->current_membuf);
    cs->current_membuf = newmb;

    newmb = storeCossCreateMemOnlyBuf(sd);
    assert(!cs->current_memonly_membuf);
    cs->current_memonly_membuf = newmb;

    cs->current_memonly_offset = cs->current_memonly_membuf->diskstart;
}

/*
 * Clean up any references from the SIO before it get's released.
 */
static void
storeCossIOFreeEntry(void *sio)
{
    memPoolFree(coss_state_pool, ((storeIOState *) sio)->fsstate);
}

static off_t
storeCossFilenoToDiskOffset(sfileno f, CossInfo * cs)
{
    off_t doff;

    doff = (off_t) f;
    doff <<= cs->blksz_bits;
    assert(doff >= 0);
    return doff;
}

static sfileno
storeCossDiskOffsetToFileno(off_t o, CossInfo * cs)
{
    assert(0 == (o & cs->blksz_mask));
    return o >> cs->blksz_bits;
}

static void
membuf_describe(CossMemBuf * t, int level, int line)
{
    assert(t->lockcount >= 0);
    debug(79, level) ("membuf id:%d (%p), LC:%02d, ST:%010lu, FL:%c%c%c\n",
	t->stripe,
	t,
	t->lockcount,
	(unsigned long) t->diskstart,
	t->flags.full ? 'F' : '.',
	t->flags.writing ? 'W' : '.',
	t->flags.written ? 'T' : '.');
}

int
storeCossFilenoToStripe(CossInfo * cs, sfileno filen)
{
    off_t o;
    /* Calculate sfileno to disk offset */
    o = ((off_t) filen) << cs->blksz_bits;
    /* Now, divide by COSS_MEMBUF_SZ to get which stripe it is in */
    return (int) (o / (off_t) COSS_MEMBUF_SZ);
}

/*
 * New stuff
 */
void
storeCossNewPendingRelocate(CossInfo * cs, storeIOState * sio, sfileno original_filen, sfileno new_filen)
{
    CossPendingReloc *pr;
    CossMemBuf *membuf;
    char *p;
    off_t disk_offset;
    int stripe;

    pr = cbdataAlloc(CossPendingReloc);
    cbdataLock(pr);
    pr->cs = cs;
    pr->original_filen = original_filen;
    pr->new_filen = new_filen;
    pr->len = sio->e->swap_file_sz;
    debug(79, 3) ("COSS Pending Relocate: %d -> %d: beginning\n", pr->original_filen, pr->new_filen);
    cs->pending_reloc_count++;
    dlinkAddTail(pr, &pr->node, &cs->pending_relocs);

    /* Update the stripe count */
    stripe = storeCossFilenoToStripe(cs, original_filen);
    assert(stripe >= 0);
    assert(stripe < cs->numstripes);
    assert(cs->stripes[stripe].pending_relocs >= 0);
    cs->stripes[stripe].pending_relocs++;

    /* And now; we begin the IO */
    p = storeCossMemPointerFromDiskOffset(cs, storeCossFilenoToDiskOffset(new_filen, cs), &membuf);
    pr->p = p;

    /* Lock the destination membuf */
    storeCossMemBufLockPending(pr, membuf);

    disk_offset = storeCossFilenoToDiskOffset(original_filen, cs);
    debug(79, 3) ("COSS Pending Relocate: size %" PRINTF_OFF_T ", disk_offset %" PRIu64 "\n", (squid_off_t) sio->e->swap_file_sz, (int64_t) disk_offset);
#if USE_AUFSOPS
    /* NOTE: the damned buffer isn't passed into aioRead! */
    debug(79, 3) ("COSS: aioRead: FD %d, from %d -> %d, offset %" PRIu64 ", len: %ld\n", cs->fd, pr->original_filen, pr->new_filen, (int64_t) disk_offset, (long int) pr->len);
    aioRead(cs->fd, (off_t) disk_offset, pr->len, storeCossCompletePendingReloc, pr);
#else
    a_file_read(&cs->aq, cs->fd,
	p,
	pr->len,
	disk_offset,
	storeCossCompletePendingReloc,
	pr);
#endif
}

CossPendingReloc *
storeCossGetPendingReloc(CossInfo * cs, sfileno new_filen)
{
    dlink_node *n;
    CossPendingReloc *pr;

    n = cs->pending_relocs.head;
    while (n != NULL) {
	pr = n->data;
	if (pr->new_filen == new_filen) {
	    return pr;
	}
	n = n->next;
    }
    return NULL;
}
#if USE_AUFSOPS
void
storeCossCompletePendingReloc(int fd, void *my_data, const char *buf, int aio_return, int aio_errno)
#else
void
storeCossCompletePendingReloc(int fd, const char *buf, int r_len, int r_errflag, void *my_data)
#endif
{
    CossPendingReloc *pr = my_data;
    CossReadOp *op;
    CossInfo *cs = pr->cs;
    int stripe;
    int errflag, len;
#if USE_AUFSOPS
    char *p;
#endif

#if USE_AUFSOPS
    len = aio_return;
    if (aio_errno)
	errflag = aio_errno == ENOSPC ? DISK_NO_SPACE_LEFT : DISK_ERROR;
    else
	errflag = DISK_OK;
#else
    errflag = r_errflag;
    len = r_len;
#endif

    debug(79, 3) ("storeCossCompletePendingReloc: %p\n", pr);
    assert(cbdataValid(pr));
    if (errflag != 0) {
	coss_stats.read.fail++;
	if (errflag > 0) {
	    errno = errflag;
	    debug(79, 1) ("storeCossCompletePendingReloc: error: %s\n", xstrerror());
	} else {
	    debug(79, 1) ("storeCossCompletePendingReloc: got failure (%d)\n", errflag);
	}
    } else {
	debug(79, 3) ("COSS Pending Relocate: %d -> %d: completed\n", pr->original_filen, pr->new_filen);
	coss_stats.read.success++;
    }
    /* aufs aioRead() doesn't take a buffer, it reads into its own. Grr */
#if USE_AUFSOPS
    p = storeCossMemPointerFromDiskOffset(cs, storeCossFilenoToDiskOffset(pr->new_filen, cs), NULL);
    assert(p != NULL);
    assert(p == pr->p);
    xmemcpy(p, buf, len);
#endif

    /* Nope, we're not a pending relocate anymore! */
    dlinkDelete(&pr->node, &cs->pending_relocs);

    /* Update the stripe count */
    stripe = storeCossFilenoToStripe(cs, pr->original_filen);
    assert(stripe >= 0);
    assert(stripe < cs->numstripes);
    assert(cs->stripes[stripe].pending_relocs >= 1);
    cs->stripes[stripe].pending_relocs--;

    /* Relocate has completed; we can now complete pending read ops on this particular entry */
    while (pr->ops.head != NULL) {
	op = pr->ops.head->data;
	debug(79, 3) ("storeCossCompletePendingReloc: %p: dequeueing op %p\n", pr, op);
	op->pr = NULL;
	dlinkDelete(&op->pending_op_node, &pr->ops);
	storeCossCompleteReadOp(cs, op, errflag);
	/* XXX again, this shouldn't be here (find the dlinkAddTail() in storeCossKickReadOp); these should
	 * be abstracted out. */
    }
    /* Unlock (and possibly write/free) the destination membuf */
    storeCossMemBufUnlockPending(pr, cs);
    /* Good, now we can delete it */
    cbdataUnlock(pr);
    cbdataFree(pr);
    assert(cs->pending_reloc_count != 0);
    cs->pending_reloc_count--;
}

/*
 * Begin a read operation
 *
 * the current 'state' of the read operation has already been set in storeIOState.
 *
 * We assume that the read operation will be from a currently in-memory MemBuf.
 */
CossReadOp *
storeCossCreateReadOp(CossInfo * cs, storeIOState * sio)
{
    CossReadOp *op;
    CossState *cstate = sio->fsstate;

    /* Create entry */
    op = memPoolAlloc(coss_op_pool);

    debug(79, 3) ("COSS: Creating Read operation: %p: filen %d, offset %" PRId64 ", size %" PRId64 "\n", op, sio->swap_filen, (int64_t) cstate->requestoffset, (int64_t) cstate->requestlen);

    /* Fill in details */
    op->type = COSS_OP_READ;
    op->sio = sio;
    cbdataLock(op->sio);
    op->requestlen = cstate->requestlen;
    op->requestoffset = cstate->requestoffset;
    op->reqdiskoffset = cstate->reqdiskoffset;
    op->requestbuf = cstate->requestbuf;

    /* Add to list */
    dlinkAddTail(op, &op->node, &cs->pending_ops);
    return op;
}

void
storeCossCompleteReadOp(CossInfo * cs, CossReadOp * op, int error)
{
    storeIOState *sio = op->sio;
    STRCB *callback = NULL;
    void *callback_data = NULL;
    CossState *cstate = sio->fsstate;
    ssize_t rlen = -1;
    char *p;
    SwapDir *SD = INDEXSD(sio->swap_dirn);

    debug(79, 3) ("storeCossCompleteReadOp: op %p, op dependencies satisfied, completing\n", op);

    assert(storeCossGetPendingReloc(cs, sio->swap_filen) == NULL);
    /* and make sure we aren't on a pending op list! */
    assert(op->pr == NULL);
    /* Is the callback still valid? If so; copy the data and callback */
    if (cbdataValid(sio) && cbdataValid(sio->read.callback_data)) {
	callback = sio->read.callback;
	callback_data = sio->read.callback_data;
	assert(callback);
	assert(callback_data);
	sio->read.callback = NULL;
	sio->read.callback_data = NULL;
	if (error == 0) {
	    /* P is the beginning of the object data we're interested in */
	    p = storeCossMemPointerFromDiskOffset(cs, storeCossFilenoToDiskOffset(sio->swap_filen, SD->fsdata), NULL);
	    assert(p != NULL);
	    /* cstate->requestlen contains the current copy length */
	    assert(cstate->requestlen == op->requestlen);
	    assert(cstate->requestbuf == op->requestbuf);
	    assert(cstate->requestoffset == op->requestoffset);
	    xmemcpy(cstate->requestbuf, &p[cstate->requestoffset], cstate->requestlen);
	    rlen = cstate->requestlen;
	}
	callback(callback_data, cstate->requestbuf, rlen);
    }
    cbdataUnlock(sio);		/* sio could have been freed here */
    op->sio = NULL;
    /* Remove from the operation list */
    dlinkDelete(&op->node, &cs->pending_ops);

    /* Completed! */
    memPoolFree(coss_op_pool, op);
}

/* See if the read op can be satisfied now */
void
storeCossKickReadOp(CossInfo * cs, CossReadOp * op)
{
    CossPendingReloc *pr;

    debug(79, 3) ("storeCossKickReadOp: op %p\n", op);

    if ((pr = storeCossGetPendingReloc(cs, op->sio->swap_filen)) == NULL) {
	debug(79, 3) ("COSS: filen: %d, tis already in memory; serving.\n", op->sio->swap_filen);
	storeCossCompleteReadOp(cs, op, 0);
    } else {
	debug(79, 3) ("COSS: filen: %d, not in memory, she'll have to wait.\n", op->sio->swap_filen);
	/* XXX Eww, hack! It has to be done; but doing it here is yuck */
	if (op->pr == NULL) {
	    debug(79, 3) ("storeCossKickReadOp: %p: op not bound to a pending read %p; binding\n", op, pr);
	    dlinkAddTail(op, &op->pending_op_node, &pr->ops);
	    op->pr = pr;
	}
    }
}

static void
membufsPrint(StoreEntry * e, CossMemBuf * t, const char *prefix)
{
    storeAppendPrintf(e, "%s: %d, lockcount: %d, numobjects %d, flags: %s,%s,%s,%s\n",
	prefix, t->stripe, t->lockcount, t->numobjs,
	t->flags.full ? "FULL" : "NOTFULL",
	t->flags.writing ? "WRITING" : "NOTWRITING",
	t->flags.written ? "WRITTEN" : "NOTWRITTEN",
	t->flags.memonly ? "MEMONLY" : "DISK");
}

void
membufsDump(CossInfo * cs, StoreEntry * e)
{
    dlink_node *m;
    int i;
    m = cs->membufs.head;
    while (m != NULL) {
	CossMemBuf *t = m->data;
	membufsPrint(e, t, "Stripe");
	m = m->next;
    }
    m = cs->dead_membufs.head;
    while (m != NULL) {
	CossMemBuf *t = m->data;
	membufsPrint(e, t, "Dead Stripe");
	m = m->next;
    }
    storeAppendPrintf(e, "Pending Relocations:\n");
    for (i = 0; i < cs->numstripes; i++) {
	if (cs->stripes[i].pending_relocs > 0) {
	    storeAppendPrintf(e, "  Stripe: %d   Number: %d\n", i, cs->stripes[i].pending_relocs);
	}
    }
}
