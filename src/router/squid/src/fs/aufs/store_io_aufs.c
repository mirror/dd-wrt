
/*
 * $Id: store_io_aufs.c,v 1.34 2006/11/05 21:14:31 hno Exp $
 *
 * DEBUG: section 79    Squid-side AUFS I/O functions.
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
#include "async_io.h"
#include "store_asyncufs.h"

#if ASYNC_READ
static AIOCB storeAufsReadDone;
#else
static DRCB storeAufsReadDone;
#endif
#if ASYNC_WRITE
static AIOCB storeAufsWriteDone;
#else
static DWCB storeAufsWriteDone;
#endif
static void storeAufsIOCallback(storeIOState * sio, int errflag);
static AIOCB storeAufsOpenDone;
static int storeAufsNeedCompletetion(storeIOState *);
static int storeAufsKickWriteQueue(storeIOState * sio);
static CBDUNL storeAufsIOFreeEntry;

CBDATA_TYPE(storeIOState);

/* === PUBLIC =========================================================== */

/* open for reading */
storeIOState *
storeAufsOpen(SwapDir * SD, StoreEntry * e, STFNCB * file_callback,
    STIOCB * callback, void *callback_data)
{
    sfileno f = e->swap_filen;
    char *path = storeAufsDirFullPath(SD, f, NULL);
    storeIOState *sio;
#if !ASYNC_OPEN
    int fd;
#endif
    debug(79, 3) ("storeAufsOpen: fileno %08X\n", f);
    /*
     * we should detect some 'too many files open' condition and return
     * NULL here.
     */
#ifdef MAGIC2
    if (aioQueueSize() > MAGIC2)
	return NULL;
#endif
#if !ASYNC_OPEN
    fd = file_open(path, O_RDONLY | O_BINARY | O_NOATIME);
    if (fd < 0) {
	debug(79, 3) ("storeAufsOpen: got failure (%d)\n", errno);
	return NULL;
    }
#endif
    CBDATA_INIT_TYPE_FREECB(storeIOState, storeAufsIOFreeEntry);
    sio = cbdataAlloc(storeIOState);
    sio->fsstate = memPoolAlloc(squidaio_state_pool);
    ((squidaiostate_t *) (sio->fsstate))->fd = -1;
    ((squidaiostate_t *) (sio->fsstate))->flags.opening = 1;
    sio->swap_filen = f;
    sio->swap_dirn = SD->index;
    sio->mode = O_RDONLY | O_BINARY;
    sio->callback = callback;
    sio->callback_data = callback_data;
    sio->e = e;
    cbdataLock(callback_data);
    Opening_FD++;
    statCounter.syscalls.disk.opens++;
#if ASYNC_OPEN
    aioOpen(path, O_RDONLY | O_BINARY | O_NOATIME, 0644, storeAufsOpenDone, sio);
#else
    storeAufsOpenDone(fd, sio, fd, 0);
#endif
    return sio;
}

/* open for creating */
storeIOState *
storeAufsCreate(SwapDir * SD, StoreEntry * e, STFNCB * file_callback, STIOCB * callback, void *callback_data)
{
    char *path;
    storeIOState *sio;
    sfileno filn;
    sdirno dirn;
#if !ASYNC_CREATE
    int fd;
#endif

    /* Allocate a number */
    dirn = SD->index;
    filn = storeAufsDirMapBitAllocate(SD);
    path = storeAufsDirFullPath(SD, filn, NULL);

    debug(79, 3) ("storeAufsCreate: fileno %08X\n", filn);
    /*
     * we should detect some 'too many files open' condition and return
     * NULL here.
     */
#ifdef MAGIC2
    if (aioQueueSize() > MAGIC2)
	return NULL;
#endif
#if !ASYNC_CREATE
    fd = file_open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    if (fd < 0) {
	debug(79, 3) ("storeAufsCreate: got failure (%d)\n", errno);
	return NULL;
    }
#endif
    CBDATA_INIT_TYPE_FREECB(storeIOState, storeAufsIOFreeEntry);
    sio = cbdataAlloc(storeIOState);
    sio->fsstate = memPoolAlloc(squidaio_state_pool);
    ((squidaiostate_t *) (sio->fsstate))->fd = -1;
    ((squidaiostate_t *) (sio->fsstate))->flags.opening = 1;
    sio->swap_filen = filn;
    sio->swap_dirn = dirn;
    sio->mode = O_WRONLY | O_BINARY;
    sio->callback = callback;
    sio->callback_data = callback_data;
    sio->e = (StoreEntry *) e;
    cbdataLock(callback_data);
    Opening_FD++;
    statCounter.syscalls.disk.opens++;
#if ASYNC_CREATE
    aioOpen(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644, storeAufsOpenDone, sio);
#else
    storeAufsOpenDone(fd, sio, fd, 0);
#endif

    /* now insert into the replacement policy */
    storeAufsDirReplAdd(SD, e);
    return sio;

}



/* Close */
void
storeAufsClose(SwapDir * SD, storeIOState * sio)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    debug(79, 3) ("storeAufsClose: dirno %d, fileno %08X, FD %d\n",
	sio->swap_dirn, sio->swap_filen, aiostate->fd);
    if (storeAufsNeedCompletetion(sio)) {
	aiostate->flags.close_request = 1;
	return;
    }
    storeAufsIOCallback(sio, DISK_OK);
}


/* Read */
void
storeAufsRead(SwapDir * SD, storeIOState * sio, char *buf, size_t size, squid_off_t offset, STRCB * callback, void *callback_data)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    assert(sio->read.callback == NULL);
    assert(sio->read.callback_data == NULL);
    assert(!aiostate->flags.reading);
    if (aiostate->fd < 0) {
	struct _queued_read *q;
	debug(79, 3) ("storeAufsRead: queueing read because FD < 0\n");
	assert(aiostate->flags.opening);
	assert(aiostate->pending_reads == NULL);
	q = memPoolAlloc(aufs_qread_pool);
	q->buf = buf;
	q->size = size;
	q->offset = (off_t) offset;
	q->callback = callback;
	q->callback_data = callback_data;
	cbdataLock(q->callback_data);
	linklistPush(&(aiostate->pending_reads), q);
	return;
    }
    sio->read.callback = callback;
    sio->read.callback_data = callback_data;
    aiostate->read_buf = buf;
    cbdataLock(callback_data);
    debug(79, 3) ("storeAufsRead: dirno %d, fileno %08X, FD %d\n",
	sio->swap_dirn, sio->swap_filen, aiostate->fd);
    sio->offset = offset;
    aiostate->flags.reading = 1;
#if ASYNC_READ
    aioRead(aiostate->fd, (off_t) offset, size, storeAufsReadDone, sio);
    statCounter.syscalls.disk.reads++;
#else
    file_read(aiostate->fd, buf, size, (off_t) offset, storeAufsReadDone, sio);
    /* file_read() increments syscalls.disk.reads */
#endif
}


/* Write */
void
storeAufsWrite(SwapDir * SD, storeIOState * sio, char *buf, size_t size, squid_off_t offset, FREE * free_func)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    debug(79, 3) ("storeAufsWrite: dirno %d, fileno %08X, FD %d\n",
	sio->swap_dirn, sio->swap_filen, aiostate->fd);
    if (aiostate->fd < 0) {
	/* disk file not opened yet */
	struct _queued_write *q;
	assert(aiostate->flags.opening);
	q = memPoolAlloc(aufs_qwrite_pool);
	q->buf = buf;
	q->size = size;
	q->offset = (off_t) offset;
	q->free_func = free_func;
	linklistPush(&(aiostate->pending_writes), q);
	return;
    }
#if ASYNC_WRITE
    if (aiostate->flags.writing) {
	struct _queued_write *q;
	debug(79, 3) ("storeAufsWrite: queuing write\n");
	q = memPoolAlloc(aufs_qwrite_pool);
	q->buf = buf;
	q->size = size;
	q->offset = (off_t) offset;
	q->free_func = free_func;
	linklistPush(&(aiostate->pending_writes), q);
	return;
    }
    aiostate->flags.writing = 1;
    aioWrite(aiostate->fd, (off_t) offset, buf, size, storeAufsWriteDone, sio,
	free_func);
    statCounter.syscalls.disk.writes++;
#else
    file_write(aiostate->fd, (off_t) offset, buf, size, storeAufsWriteDone, sio,
	free_func);
    /* file_write() increments syscalls.disk.writes */
#endif
}

/* Unlink */
void
storeAufsUnlink(SwapDir * SD, StoreEntry * e)
{
    debug(79, 3) ("storeAufsUnlink: dirno %d, fileno %08X\n", SD->index, e->swap_filen);
    storeAufsDirReplRemove(e);
    storeAufsDirMapBitReset(SD, e->swap_filen);
    storeAufsDirUnlinkFile(SD, e->swap_filen);
    statCounter.syscalls.disk.unlinks++;
}

void
storeAufsRecycle(SwapDir * SD, StoreEntry * e)
{
    debug(79, 3) ("storeAufsRecycle: fileno %08X\n", e->swap_filen);

    /* detach from the underlying physical object */
    if (e->swap_filen > -1) {
	storeAufsDirReplRemove(e);
	storeAufsDirMapBitReset(SD, e->swap_filen);
	e->swap_filen = -1;
	e->swap_dirn = -1;
    }
}

/*  === STATIC =========================================================== */

static int
storeAufsKickWriteQueue(storeIOState * sio)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    struct _queued_write *q = linklistShift(&aiostate->pending_writes);
    if (NULL == q)
	return 0;
    debug(79, 3) ("storeAufsKickWriteQueue: writing queued chunk of %ld bytes\n",
	(long int) q->size);
    storeAufsWrite(INDEXSD(sio->swap_dirn), sio, q->buf, q->size, q->offset, q->free_func);
    memPoolFree(aufs_qwrite_pool, q);
    return 1;
}

static int
storeAufsKickReadQueue(storeIOState * sio)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    struct _queued_read *q = linklistShift(&(aiostate->pending_reads));
    if (NULL == q)
	return 0;
    debug(79, 3) ("storeAufsKickReadQueue: reading queued request of %ld bytes\n",
	(long int) q->size);
    if (cbdataValid(q->callback_data))
	storeAufsRead(INDEXSD(sio->swap_dirn), sio, q->buf, q->size, q->offset, q->callback, q->callback_data);
    cbdataUnlock(q->callback_data);
    memPoolFree(aufs_qread_pool, q);
    return 1;
}

static void
storeAufsOpenDone(int unused, void *my_data, const char *unused2, int fd, int errflag)
{
    storeIOState *sio = my_data;
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    debug(79, 3) ("storeAufsOpenDone: FD %d, errflag %d\n", fd, errflag);
    Opening_FD--;
    aiostate->flags.opening = 0;
    if (errflag || fd < 0) {
	errno = errflag;
	debug(79, 0) ("storeAufsOpenDone: %s\n", xstrerror());
	debug(79, 1) ("\t%s\n", storeAufsDirFullPath(INDEXSD(sio->swap_dirn), sio->swap_filen, NULL));
	storeAufsIOCallback(sio, DISK_ERROR);
	return;
    }
    store_open_disk_fd++;
    aiostate->fd = fd;
    commSetCloseOnExec(fd);
    fd_open(fd, FD_FILE, storeAufsDirFullPath(INDEXSD(sio->swap_dirn), sio->swap_filen, NULL));
    if (FILE_MODE(sio->mode) == O_WRONLY) {
	if (storeAufsKickWriteQueue(sio))
	    return;
    } else if ((FILE_MODE(sio->mode) == O_RDONLY) && !aiostate->flags.close_request) {
	if (storeAufsKickReadQueue(sio))
	    return;
    }
    if (aiostate->flags.close_request)
	storeAufsIOCallback(sio, errflag);
    debug(79, 3) ("storeAufsOpenDone: exiting\n");
}

#if ASYNC_READ
static void
storeAufsReadDone(int fd, void *my_data, const char *buf, int len, int errflag)
#else
static void
storeAufsReadDone(int fd, const char *buf, int len, int errflag, void *my_data)
#endif
{
    storeIOState *sio = my_data;
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    STRCB *callback = sio->read.callback;
    void *their_data = sio->read.callback_data;
    ssize_t rlen;
    int inreaddone = aiostate->flags.inreaddone;	/* Protect from callback loops */
    debug(79, 3) ("storeAufsReadDone: dirno %d, fileno %08X, FD %d, len %d\n",
	sio->swap_dirn, sio->swap_filen, fd, len);
    aiostate->flags.inreaddone = 1;
    aiostate->flags.reading = 0;
    if (errflag) {
	debug(79, 3) ("storeAufsReadDone: got failure (%d)\n", errflag);
	rlen = -1;
    } else {
	rlen = len;
	sio->offset += len;
    }
#if ASYNC_READ
    /* translate errflag from errno to Squid disk error */
    errno = errflag;
    if (errflag)
	errflag = DISK_ERROR;
    else
	errflag = DISK_OK;
#else
    if (errflag == DISK_EOF)
	errflag = DISK_OK;	/* EOF is signalled by len == 0, not errors... */
#endif
    assert(callback);
    assert(their_data);
    sio->read.callback = NULL;
    sio->read.callback_data = NULL;
    if (!aiostate->flags.close_request && cbdataValid(their_data)) {
#if ASYNC_READ
	if (rlen > 0)
	    memcpy(aiostate->read_buf, buf, rlen);
#endif
	callback(their_data, aiostate->read_buf, rlen);
    }
    cbdataUnlock(their_data);
    aiostate->flags.inreaddone = 0;
    if (aiostate->flags.close_request && !inreaddone)
	storeAufsIOCallback(sio, errflag);
}

#if ASYNC_WRITE
static void
storeAufsWriteDone(int fd, void *my_data, const char *buf, int aio_return, int aio_errno)
#else
static void
storeAufsWriteDone(int fd, int errflag, size_t len, void *my_data)
#endif
{
    static int loop_detect = 0;
    storeIOState *sio = my_data;
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
#if ASYNC_WRITE
    int errflag;
    int len = aio_return;
    /* Translate from errno to Squid disk error */
    if (aio_errno)
	errflag = aio_errno == ENOSPC ? DISK_NO_SPACE_LEFT : DISK_ERROR;
    else
	errflag = DISK_OK;
#endif
    debug(79, 3) ("storeAufsWriteDone: dirno %d, fileno %08X, FD %d, len %ld, err=%d\n",
	sio->swap_dirn, sio->swap_filen, fd, (long int) len, errflag);
    assert(++loop_detect < 10);
    aiostate->flags.writing = 0;
    if (errflag) {
	debug(79, 0) ("storeAufsWriteDone: got failure (%d)\n", errflag);
	storeAufsIOCallback(sio, errflag);
	loop_detect--;
	return;
    }
    sio->offset += len;
#if ASYNC_WRITE
    if (storeAufsKickWriteQueue(sio))
	(void) 0;
    else if (aiostate->flags.close_request)
	storeAufsIOCallback(sio, errflag);
#else
    /* loop around storeAufsKickWriteQueue to break recursion stack
     * overflow when large amounts of data has been queued for write.
     * As writes are blocking here we immediately get called again
     * without going via the I/O event loop..
     */
    if (!aiostate->flags.write_kicking) {
	/* cbdataLock to protect us from the storeAufsIOCallback on error above */
	cbdataLock(sio);
	aiostate->flags.write_kicking = 1;
	while (storeAufsKickWriteQueue(sio))
	    if (!cbdataValid(sio))
		break;
	if (cbdataValid(sio)) {
	    aiostate->flags.write_kicking = 0;
	    if (aiostate->flags.close_request)
		storeAufsIOCallback(sio, errflag);
	}
	cbdataUnlock(sio);
    }
#endif
    loop_detect--;
}

static void
storeAufsIOCallback(storeIOState * sio, int errflag)
{
    STIOCB *callback = sio->callback;
    void *their_data = sio->callback_data;
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    int fd = aiostate->fd;
    debug(79, 3) ("storeAufsIOCallback: errflag=%d\n", errflag);
    sio->callback = NULL;
    sio->callback_data = NULL;
    debug(79, 9) ("%s:%d\n", __FILE__, __LINE__);
    if (callback)
	if (NULL == their_data || cbdataValid(their_data))
	    callback(their_data, errflag, sio);
    debug(79, 9) ("%s:%d\n", __FILE__, __LINE__);
    cbdataUnlock(their_data);
    aiostate->fd = -1;
    if (aiostate->flags.opening)
	Opening_FD--;
    cbdataFree(sio);
    if (fd < 0)
	return;
    debug(79, 9) ("%s:%d\n", __FILE__, __LINE__);
#if ASYNC_CLOSE
    fd_close(fd);
    aioClose(fd);
#else
    aioCancel(fd);
    file_close(fd);
#endif
    store_open_disk_fd--;
    statCounter.syscalls.disk.closes++;
    debug(79, 9) ("%s:%d\n", __FILE__, __LINE__);
}


static int
storeAufsNeedCompletetion(storeIOState * sio)
{
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;

    if (aiostate->flags.writing)
	return 1;
    if (aiostate->flags.opening && FILE_MODE(sio->mode) == O_WRONLY)
	return 1;
    if (aiostate->flags.reading)
	return 1;
    if (aiostate->flags.inreaddone)
	return 1;

    return 0;
}


/*      
 * Clean up references from the SIO before it gets released.
 * The actuall SIO is managed by cbdata so we do not need
 * to bother with that.
 */
static void
storeAufsIOFreeEntry(void *siop)
{
    storeIOState *sio = (storeIOState *) siop;
    squidaiostate_t *aiostate = (squidaiostate_t *) sio->fsstate;
    struct _queued_write *qw;
    struct _queued_read *qr;
    while ((qw = linklistShift(&aiostate->pending_writes))) {
	if (qw->free_func)
	    qw->free_func(qw->buf);
	memPoolFree(aufs_qwrite_pool, qw);
    }
    while ((qr = linklistShift(&aiostate->pending_reads))) {
	cbdataUnlock(qr->callback_data);
	memPoolFree(aufs_qread_pool, qr);
    }
    if (sio->read.callback_data)
	cbdataUnlock(sio->read.callback_data);
    if (sio->callback_data)
	cbdataUnlock(sio->callback_data);
    memPoolFree(squidaio_state_pool, aiostate);
}
