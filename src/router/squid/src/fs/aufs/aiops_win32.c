/*
 * $Id: aiops_win32.c,v 1.3 2006/09/23 10:16:40 serassio Exp $
 *
 * DEBUG: section 43    Windows AIOPS
 * AUTHOR: Stewart Forster <slf@connect.com.au>
 * AUTHOR: Robert Collins <robertc@squid-cache.org>
 * AUTHOR: Guido Serassio <serassio@squid-cache.org>
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
#include <windows.h>
#include "async_io.h"

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<dirent.h>
#include	<signal.h>

#define RIDICULOUS_LENGTH	4096

#ifdef AUFS_IO_THREADS
int squidaio_nthreads = AUFS_IO_THREADS;
#else
int squidaio_nthreads = 0;
#endif
int squidaio_magic1 = 1;	/* dummy initializer value */
int squidaio_magic2 = 1;	/* real value set in aiops.c */

enum _squidaio_thread_status {
    _THREAD_STARTING = 0,
    _THREAD_WAITING,
    _THREAD_BUSY,
    _THREAD_FAILED,
    _THREAD_DONE
};
typedef enum _squidaio_thread_status squidaio_thread_status;

enum _squidaio_request_type {
    _AIO_OP_NONE = 0,
    _AIO_OP_OPEN,
    _AIO_OP_READ,
    _AIO_OP_WRITE,
    _AIO_OP_CLOSE,
    _AIO_OP_UNLINK,
    _AIO_OP_TRUNCATE,
    _AIO_OP_OPENDIR,
    _AIO_OP_STAT
};
typedef enum _squidaio_request_type squidaio_request_type;

typedef struct squidaio_request_t {
    struct squidaio_request_t *next;
    squidaio_request_type request_type;
    int cancelled;
    char *path;
    int oflag;
    mode_t mode;
    int fd;
    char *bufferp;
    int buflen;
    off_t offset;
    int whence;
    int ret;
    int err;
    struct stat *tmpstatp;
    struct stat *statp;
    squidaio_result_t *resultp;
} squidaio_request_t;

typedef struct squidaio_request_queue_t {
    HANDLE mutex;
    HANDLE cond;		/* See Event objects */
    squidaio_request_t *volatile head;
    squidaio_request_t *volatile *volatile tailp;
    unsigned long requests;
    unsigned long blocked;	/* main failed to lock the queue */
} squidaio_request_queue_t;

typedef struct squidaio_thread_t squidaio_thread_t;
struct squidaio_thread_t {
    squidaio_thread_t *next;
    HANDLE thread;
    DWORD dwThreadId;		/* thread ID */
    squidaio_thread_status status;
    struct squidaio_request_t *current_req;
    unsigned long requests;
    int volatile exit;
};

static void squidaio_queue_request(squidaio_request_t *);
static void squidaio_cleanup_request(squidaio_request_t *);
static DWORD WINAPI squidaio_thread_loop(LPVOID lpParam);
static void squidaio_do_open(squidaio_request_t *);
static void squidaio_do_read(squidaio_request_t *);
static void squidaio_do_write(squidaio_request_t *);
static void squidaio_do_close(squidaio_request_t *);
static void squidaio_do_stat(squidaio_request_t *);
static void squidaio_do_unlink(squidaio_request_t *);
#if USE_TRUNCATE
static void squidaio_do_truncate(squidaio_request_t *);
#endif
#if AIO_OPENDIR
static void *squidaio_do_opendir(squidaio_request_t *);
#endif
static void squidaio_debug(squidaio_request_t *);
static void squidaio_poll_queues(void);

static squidaio_thread_t *threads = NULL;
static int squidaio_initialised = 0;

#define AIO_LARGE_BUFS  16384
#define AIO_MEDIUM_BUFS	AIO_LARGE_BUFS >> 1
#define AIO_SMALL_BUFS	AIO_LARGE_BUFS >> 2
#define AIO_TINY_BUFS	AIO_LARGE_BUFS >> 3
#define AIO_MICRO_BUFS	128

static MemPool *squidaio_large_bufs = NULL;	/* 16K */
static MemPool *squidaio_medium_bufs = NULL;	/* 8K */
static MemPool *squidaio_small_bufs = NULL;	/* 4K */
static MemPool *squidaio_tiny_bufs = NULL;	/* 2K */
static MemPool *squidaio_micro_bufs = NULL;	/* 128K */

static int request_queue_len = 0;
static MemPool *squidaio_request_pool = NULL;
static MemPool *squidaio_thread_pool = NULL;
static squidaio_request_queue_t request_queue;
static struct {
    squidaio_request_t *head, **tailp;
} request_queue2 = {

    NULL, &request_queue2.head
};
static squidaio_request_queue_t done_queue;
static struct {
    squidaio_request_t *head, **tailp;
} done_requests = {

    NULL, &done_requests.head
};
static int done_fd = 0;
static int done_fd_read = 0;
static int done_signalled = 0;
static HANDLE main_thread;

static MemPool *
squidaio_get_pool(int size)
{
    MemPool *p;
    if (size <= AIO_LARGE_BUFS) {
	if (size <= AIO_MICRO_BUFS)
	    p = squidaio_micro_bufs;
	else if (size <= AIO_TINY_BUFS)
	    p = squidaio_tiny_bufs;
	else if (size <= AIO_SMALL_BUFS)
	    p = squidaio_small_bufs;
	else if (size <= AIO_MEDIUM_BUFS)
	    p = squidaio_medium_bufs;
	else
	    p = squidaio_large_bufs;
    } else
	p = NULL;
    return p;
}

void *
squidaio_xmalloc(int size)
{
    void *p;
    MemPool *pool;

    if ((pool = squidaio_get_pool(size)) != NULL) {
	p = memPoolAlloc(pool);
    } else
	p = xmalloc(size);

    return p;
}

static char *
squidaio_xstrdup(const char *str)
{
    char *p;
    int len = strlen(str) + 1;

    p = squidaio_xmalloc(len);
    strncpy(p, str, len);

    return p;
}

void
squidaio_xfree(void *p, int size)
{
    MemPool *pool;

    if ((pool = squidaio_get_pool(size)) != NULL) {
	memPoolFree(pool, p);
    } else
	xfree(p);
}

static void
squidaio_xstrfree(char *str)
{
    MemPool *pool;
    int len = strlen(str) + 1;

    if ((pool = squidaio_get_pool(len)) != NULL) {
	memPoolFree(pool, str);
    } else
	xfree(str);
}

static void
squidaio_fdhandler(int fd, void *data)
{
    char junk[256];
    FD_READ_METHOD(done_fd_read, junk, sizeof(junk));
    commSetSelect(fd, COMM_SELECT_READ, squidaio_fdhandler, NULL, 0);
}

void
squidaio_init(void)
{
    int i;
    int done_pipe[2];
    squidaio_thread_t *threadp;

    if (squidaio_initialised)
	return;

    if (!DuplicateHandle(GetCurrentProcess(),	/* pseudo handle, don't close */
	    GetCurrentThread(),	/* pseudo handle to copy */
	    GetCurrentProcess(),	/* pseudo handle, don't close */
	    &main_thread,
	    0,			/* required access */
	    FALSE,		/* child process's don't inherit the handle */
	    DUPLICATE_SAME_ACCESS)) {
	/* spit errors */
	fatal("couldn't get current thread handle\n");
    }
    /* Initialize request queue */
    if ((request_queue.mutex = CreateMutex(NULL,	/* no inheritance */
		FALSE,		/* start unowned (as per mutex_init) */
		NULL)		/* no name */
	) == NULL) {
	fatal("failed to create mutex\n");
    }
    if ((request_queue.cond = CreateEvent(NULL,		/* no inheritance */
		FALSE,		/* auto signal reset - which I think is pthreads like ? */
		FALSE,		/* start non signaled */
		NULL)		/* no name */
	) == NULL) {
	fatal("failed to create condition event variable.\n");
    }
    request_queue.head = NULL;
    request_queue.tailp = &request_queue.head;
    request_queue.requests = 0;
    request_queue.blocked = 0;

    /* Initialize done queue */
    if ((done_queue.mutex = CreateMutex(NULL,	/* no inheritance */
		FALSE,		/* start unowned (as per mutex_init) */
		NULL)		/* no name */
	) == NULL) {
	fatal("failed to create mutex\n");
    }
    if ((done_queue.cond = CreateEvent(NULL,	/* no inheritance */
		TRUE,		/* manually signaled - which I think is pthreads like ? */
		FALSE,		/* start non signaled */
		NULL)		/* no name */
	) == NULL) {
	fatal("failed to create condition event variable.\n");
    }
    done_queue.head = NULL;
    done_queue.tailp = &done_queue.head;
    done_queue.requests = 0;
    done_queue.blocked = 0;

    /* Initialize done pipe signal */
    pipe(done_pipe);
    done_fd = done_pipe[1];
    done_fd_read = done_pipe[0];
    fd_open(done_fd_read, FD_PIPE, "async-io completion event: main");
    fd_open(done_fd, FD_PIPE, "async-io completion event: threads");
    commSetNonBlocking(done_pipe[0]);
    commSetNonBlocking(done_pipe[1]);
    commSetCloseOnExec(done_pipe[0]);
    commSetCloseOnExec(done_pipe[1]);
    commSetSelect(done_pipe[0], COMM_SELECT_READ, squidaio_fdhandler, NULL, 0);

    /* Create threads and get them to sit in their wait loop */
    squidaio_thread_pool = memPoolCreate("aio_thread", sizeof(squidaio_thread_t));
    if (squidaio_nthreads == 0) {
	int j = 16;
	for (i = 0; i < n_asyncufs_dirs; i++) {
	    squidaio_nthreads += j;
	    j = j * 2 / 3;
	    if (j < 4)
		j = 4;
	}
#if USE_AUFSOPS
	j = 6;
	for (i = 0; i < n_coss_dirs; i++) {
	    squidaio_nthreads += j;
	    j = 3;
	}
#endif
    }
    if (squidaio_nthreads == 0)
	squidaio_nthreads = 16;
    squidaio_magic1 = squidaio_nthreads * MAGIC1_FACTOR;
    squidaio_magic2 = squidaio_nthreads * MAGIC2_FACTOR;
    for (i = 0; i < squidaio_nthreads; i++) {
	threadp = memPoolAlloc(squidaio_thread_pool);
	threadp->status = _THREAD_STARTING;
	threadp->current_req = NULL;
	threadp->requests = 0;
	threadp->next = threads;
	threads = threadp;
	if ((threadp->thread = CreateThread(NULL,	/* no security attributes */
		    0,		/* use default stack size */
		    squidaio_thread_loop,	/* thread function */
		    threadp,	/* argument to thread function */
		    0,		/* use default creation flags */
		    &(threadp->dwThreadId))	/* returns the thread identifier */
	    ) == NULL) {
	    fprintf(stderr, "Thread creation failed\n");
	    threadp->status = _THREAD_FAILED;
	    continue;
	}
	/* Set the new thread priority above parent process */
	SetThreadPriority(threadp->thread, THREAD_PRIORITY_ABOVE_NORMAL);
    }

    /* Create request pool */
    squidaio_request_pool = memPoolCreate("aio_request", sizeof(squidaio_request_t));
    squidaio_large_bufs = memPoolCreate("squidaio_large_bufs", AIO_LARGE_BUFS);
    squidaio_medium_bufs = memPoolCreate("squidaio_medium_bufs", AIO_MEDIUM_BUFS);
    squidaio_small_bufs = memPoolCreate("squidaio_small_bufs", AIO_SMALL_BUFS);
    squidaio_tiny_bufs = memPoolCreate("squidaio_tiny_bufs", AIO_TINY_BUFS);
    squidaio_micro_bufs = memPoolCreate("squidaio_micro_bufs", AIO_MICRO_BUFS);

    squidaio_initialised = 1;
}

void
squidaio_shutdown(void)
{
    squidaio_thread_t *threadp;
    int i;
    HANDLE *hthreads;

    if (!squidaio_initialised)
	return;

    /* This is the same as in squidaio_sync */
    do {
	squidaio_poll_queues();
    } while (request_queue_len > 0);

    hthreads = (HANDLE *) xcalloc(squidaio_nthreads, sizeof(HANDLE));
    threadp = threads;
    for (i = 0; i < squidaio_nthreads; i++) {
	threadp->exit = 1;
	hthreads[i] = threadp->thread;
	threadp = threadp->next;
    }
    ReleaseMutex(request_queue.mutex);
    ResetEvent(request_queue.cond);
    ReleaseMutex(done_queue.mutex);
    ResetEvent(done_queue.cond);
    Sleep(0);

    WaitForMultipleObjects(squidaio_nthreads, hthreads, TRUE, 2000);
    for (i = 0; i < squidaio_nthreads; i++) {
	CloseHandle(hthreads[i]);
    }
    CloseHandle(main_thread);
    xfree(hthreads);

    fd_close(done_fd);
    fd_close(done_fd_read);
    close(done_fd);
    close(done_fd_read);
}

static DWORD WINAPI
squidaio_thread_loop(LPVOID lpParam)
{
    squidaio_thread_t *threadp = lpParam;
    squidaio_request_t *request;
    HANDLE cond;		/* local copy of the event queue because win32 event handles
				 * don't atomically release the mutex as cond variables do. */

    /* lock the thread info */
    if (WAIT_FAILED == WaitForSingleObject(request_queue.mutex, INFINITE)) {
	fatal("Can't get ownership of mutex\n");
    }
    /* duplicate the handle */
    if (!DuplicateHandle(GetCurrentProcess(),	/* pseudo handle, don't close */
	    request_queue.cond,	/* handle to copy */
	    GetCurrentProcess(),	/* pseudo handle, don't close */
	    &cond,
	    0,			/* required access */
	    FALSE,		/* child process's don't inherit the handle */
	    DUPLICATE_SAME_ACCESS))
	fatal("Can't duplicate mutex handle\n");
    if (!ReleaseMutex(request_queue.mutex)) {
	CloseHandle(cond);
	fatal("Can't release mutex\n");
    }
    while (1) {
	DWORD rv;
	threadp->current_req = request = NULL;
	request = NULL;
	/* Get a request to process */
	threadp->status = _THREAD_WAITING;
	if (threadp->exit) {
	    CloseHandle(request_queue.mutex);
	    CloseHandle(cond);
	    return 0;
	}
	rv = WaitForSingleObject(request_queue.mutex, INFINITE);
	if (rv == WAIT_FAILED) {
	    CloseHandle(cond);
	    return 1;
	}
	while (!request_queue.head) {
	    if (!ReleaseMutex(request_queue.mutex)) {
		CloseHandle(cond);
		threadp->status = _THREAD_FAILED;
		return 1;
	    }
	    rv = WaitForSingleObject(cond, INFINITE);
	    if (rv == WAIT_FAILED) {
		CloseHandle(cond);
		return 1;
	    }
	    rv = WaitForSingleObject(request_queue.mutex, INFINITE);
	    if (rv == WAIT_FAILED) {
		CloseHandle(cond);
		return 1;
	    }
	}
	request = request_queue.head;
	if (request)
	    request_queue.head = request->next;
	if (!request_queue.head)
	    request_queue.tailp = &request_queue.head;
	if (!ReleaseMutex(request_queue.mutex)) {
	    CloseHandle(cond);
	    return 1;
	}
	/* process the request */
	threadp->status = _THREAD_BUSY;
	request->next = NULL;
	threadp->current_req = request;
	errno = 0;
	if (!request->cancelled) {
	    switch (request->request_type) {
	    case _AIO_OP_OPEN:
		squidaio_do_open(request);
		break;
	    case _AIO_OP_READ:
		squidaio_do_read(request);
		break;
	    case _AIO_OP_WRITE:
		squidaio_do_write(request);
		break;
	    case _AIO_OP_CLOSE:
		squidaio_do_close(request);
		break;
	    case _AIO_OP_UNLINK:
		squidaio_do_unlink(request);
		break;
#if USE_TRUNCATE
	    case _AIO_OP_TRUNCATE:
		squidaio_do_truncate(request);
		break;
#endif
#if AIO_OPENDIR			/* Opendir not implemented yet */
	    case _AIO_OP_OPENDIR:
		squidaio_do_opendir(request);
		break;
#endif
	    case _AIO_OP_STAT:
		squidaio_do_stat(request);
		break;
	    default:
		request->ret = -1;
		request->err = EINVAL;
		break;
	    }
	} else {		/* cancelled */
	    request->ret = -1;
	    request->err = EINTR;
	}
	threadp->status = _THREAD_DONE;
	/* put the request in the done queue */
	rv = WaitForSingleObject(done_queue.mutex, INFINITE);
	if (rv == WAIT_FAILED) {
	    CloseHandle(cond);
	    return 1;
	}
	*done_queue.tailp = request;
	done_queue.tailp = &request->next;
	if (!ReleaseMutex(done_queue.mutex)) {
	    CloseHandle(cond);
	    return 1;
	}
	if (!done_signalled) {
	    done_signalled = 1;
	    FD_WRITE_METHOD(done_fd, "!", 1);
	}
	threadp->requests++;
/* Relinquish the remainder of thread time slice to any other thread
 * of equal priority that is ready to run. 
 */
	Sleep(0);
    }				/* while forever */
    CloseHandle(cond);
    return 0;
}				/* squidaio_thread_loop */

static void
squidaio_queue_request(squidaio_request_t * request)
{
    static int high_start = 0;
    debug(43, 9) ("squidaio_queue_request: %p type=%d result=%p\n",
	request, request->request_type, request->resultp);
    /* Mark it as not executed (failing result, no error) */
    request->ret = -1;
    request->err = 0;
    /* Internal housekeeping */
    request_queue_len += 1;
    request->resultp->_data = request;
    /* Play some tricks with the request_queue2 queue */
    request->next = NULL;
    if (!request_queue2.head) {
	if (WaitForSingleObject(request_queue.mutex, 0) == WAIT_OBJECT_0) {
	    /* Normal path */
	    *request_queue.tailp = request;
	    request_queue.tailp = &request->next;
	    if (!SetEvent(request_queue.cond))
		fatal("couldn't push queue\n");
	    if (!ReleaseMutex(request_queue.mutex)) {
		/* unexpected error */
		fatal("couldn't push queue\n");
	    }
	} else {
	    /* Oops, the request queue is blocked, use request_queue2 */
	    *request_queue2.tailp = request;
	    request_queue2.tailp = &request->next;
	}
    } else {
	/* Secondary path. We have blocked requests to deal with */
	/* add the request to the chain */
	*request_queue2.tailp = request;
	if (WaitForSingleObject(request_queue.mutex, 0) == WAIT_OBJECT_0) {
	    /* Ok, the queue is no longer blocked */
	    *request_queue.tailp = request_queue2.head;
	    request_queue.tailp = &request->next;
	    if (!SetEvent(request_queue.cond))
		fatal("couldn't push queue\n");
	    if (!ReleaseMutex(request_queue.mutex)) {
		/* unexpected error */
		fatal("couldn't push queue\n");
	    }
	    request_queue2.head = NULL;
	    request_queue2.tailp = &request_queue2.head;
	} else {
	    /* still blocked, bump the blocked request chain */
	    request_queue2.tailp = &request->next;
	}
    }
    if (request_queue2.head) {
	static int filter = 0;
	static int filter_limit = 8;
	if (++filter >= filter_limit) {
	    filter_limit += filter;
	    filter = 0;
	    debug(43, 1) ("squidaio_queue_request: WARNING - Queue congestion\n");
	}
    }
    /* Warn if out of threads */
    if (request_queue_len > MAGIC1) {
	static int last_warn = 0;
	static int queue_high, queue_low;
	if (high_start == 0) {
	    high_start = squid_curtime;
	    queue_high = request_queue_len;
	    queue_low = request_queue_len;
	}
	if (request_queue_len > queue_high)
	    queue_high = request_queue_len;
	if (request_queue_len < queue_low)
	    queue_low = request_queue_len;
	if (squid_curtime >= (last_warn + 15) &&
	    squid_curtime >= (high_start + 5)) {
	    debug(43, 1) ("squidaio_queue_request: WARNING - Disk I/O overloading\n");
	    if (squid_curtime >= (high_start + 15))
		debug(43, 1) ("squidaio_queue_request: Queue Length: current=%d, high=%d, low=%d, duration=%ld\n",
		    request_queue_len, queue_high, queue_low, (long int) (squid_curtime - high_start));
	    last_warn = squid_curtime;
	}
    } else {
	high_start = 0;
    }
    /* Warn if seriously overloaded */
    if (request_queue_len > RIDICULOUS_LENGTH) {
	debug(43, 0) ("squidaio_queue_request: Async request queue growing uncontrollably!\n");
	debug(43, 0) ("squidaio_queue_request: Syncing pending I/O operations.. (blocking)\n");
	squidaio_sync();
	debug(43, 0) ("squidaio_queue_request: Synced\n");
    }
}				/* squidaio_queue_request */

static void
squidaio_cleanup_request(squidaio_request_t * requestp)
{
    squidaio_result_t *resultp = requestp->resultp;
    int cancelled = requestp->cancelled;

    /* Free allocated structures and copy data back to user space if the */
    /* request hasn't been cancelled */
    switch (requestp->request_type) {
    case _AIO_OP_STAT:
	if (!cancelled && requestp->ret == 0)
	    xmemcpy(requestp->statp, requestp->tmpstatp, sizeof(struct stat));
	squidaio_xfree(requestp->tmpstatp, sizeof(struct stat));
	squidaio_xstrfree(requestp->path);
	break;
    case _AIO_OP_OPEN:
	if (cancelled && requestp->ret >= 0)
	    /* The open() was cancelled but completed */
	    close(requestp->ret);
	squidaio_xstrfree(requestp->path);
	break;
    case _AIO_OP_CLOSE:
	if (cancelled && requestp->ret < 0)
	    /* The close() was cancelled and never got executed */
	    close(requestp->fd);
	break;
    case _AIO_OP_UNLINK:
    case _AIO_OP_TRUNCATE:
    case _AIO_OP_OPENDIR:
	squidaio_xstrfree(requestp->path);
	break;
    case _AIO_OP_READ:
	break;
    case _AIO_OP_WRITE:
	break;
    default:
	break;
    }
    if (resultp != NULL && !cancelled) {
	resultp->aio_return = requestp->ret;
	resultp->aio_errno = requestp->err;
    }
    memPoolFree(squidaio_request_pool, requestp);
}				/* squidaio_cleanup_request */


int
squidaio_cancel(squidaio_result_t * resultp)
{
    squidaio_request_t *request = resultp->_data;

    if (request && request->resultp == resultp) {
	debug(43, 9) ("squidaio_cancel: %p type=%d result=%p\n",
	    request, request->request_type, request->resultp);
	request->cancelled = 1;
	request->resultp = NULL;
	resultp->_data = NULL;
	return 0;
    }
    return 1;
}				/* squidaio_cancel */


int
squidaio_open(const char *path, int oflag, mode_t mode, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->path = (char *) squidaio_xstrdup(path);
    requestp->oflag = oflag;
    requestp->mode = mode;
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_OPEN;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_open(squidaio_request_t * requestp)
{
    requestp->ret = open(requestp->path, requestp->oflag, requestp->mode);
    requestp->err = errno;
}


int
squidaio_read(int fd, char *bufp, int bufs, off_t offset, int whence, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->fd = fd;
    requestp->bufferp = bufp;
    requestp->buflen = bufs;
    requestp->offset = offset;
    requestp->whence = whence;
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_READ;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_read(squidaio_request_t * requestp)
{
    lseek(requestp->fd, requestp->offset, requestp->whence);
    if (!ReadFile((HANDLE) _get_osfhandle(requestp->fd), requestp->bufferp,
	    requestp->buflen, (LPDWORD) & requestp->ret, NULL)) {
	WIN32_maperror(GetLastError());
	requestp->ret = -1;
    }
    requestp->err = errno;
}


int
squidaio_write(int fd, char *bufp, int bufs, off_t offset, int whence, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->fd = fd;
    requestp->bufferp = bufp;
    requestp->buflen = bufs;
    requestp->offset = offset;
    requestp->whence = whence;
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_WRITE;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_write(squidaio_request_t * requestp)
{
    assert(requestp->offset >= 0);
    if (!WriteFile((HANDLE) _get_osfhandle(requestp->fd), requestp->bufferp,
	    requestp->buflen, (LPDWORD) & requestp->ret, NULL)) {
	WIN32_maperror(GetLastError());
	requestp->ret = -1;
    }
    requestp->err = errno;
}


int
squidaio_close(int fd, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->fd = fd;
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_CLOSE;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_close(squidaio_request_t * requestp)
{
    if ((requestp->ret = close(requestp->fd)) < 0)
	debug(43, 0) ("squidaio_do_close: FD %d, errno %d\n", requestp->fd, errno);
    requestp->err = errno;
}


int
squidaio_stat(const char *path, struct stat *sb, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->path = (char *) squidaio_xstrdup(path);
    requestp->statp = sb;
    requestp->tmpstatp = (struct stat *) squidaio_xmalloc(sizeof(struct stat));
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_STAT;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_stat(squidaio_request_t * requestp)
{
    requestp->ret = stat(requestp->path, requestp->tmpstatp);
    requestp->err = errno;
}


int
squidaio_unlink(const char *path, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->path = squidaio_xstrdup(path);
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_UNLINK;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_unlink(squidaio_request_t * requestp)
{
    requestp->ret = unlink(requestp->path);
    requestp->err = errno;
}


#if USE_TRUNCATE
int
squidaio_truncate(const char *path, off_t length, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;

    requestp = memPoolAlloc(squidaio_request_pool);
    requestp->path = (char *) squidaio_xstrdup(path);
    requestp->offset = length;
    requestp->resultp = resultp;
    requestp->request_type = _AIO_OP_TRUNCATE;
    requestp->cancelled = 0;

    squidaio_queue_request(requestp);
    return 0;
}


static void
squidaio_do_truncate(squidaio_request_t * requestp)
{
    requestp->ret = truncate(requestp->path, requestp->offset);
    requestp->err = errno;
}

#endif


#if AIO_OPENDIR
/* XXX squidaio_opendir NOT implemented yet.. */

int
squidaio_opendir(const char *path, squidaio_result_t * resultp)
{
    squidaio_request_t *requestp;
    int len;

    requestp = memPoolAlloc(squidaio_request_pool);
    return -1;
}

static void
squidaio_do_opendir(squidaio_request_t * requestp)
{
    /* NOT IMPLEMENTED */
}

#endif

static void
squidaio_poll_queues(void)
{
    /* kick "overflow" request queue */
    if (request_queue2.head &&
	(WaitForSingleObject(request_queue.mutex, 0) == WAIT_OBJECT_0)) {
	*request_queue.tailp = request_queue2.head;
	request_queue.tailp = request_queue2.tailp;
	if (!SetEvent(request_queue.cond))
	    fatal("couldn't push queue\n");
	if (!ReleaseMutex(request_queue.mutex)) {
	    /* unexpected error */
	    fatal("couldn't push queue\n");
	}
	request_queue2.head = NULL;
	request_queue2.tailp = &request_queue2.head;
    }
    /* Give up the CPU to allow the threads to do their work */
    if (done_queue.head || request_queue.head)
	Sleep(0);
    /* poll done queue */
    if (done_queue.head &&
	(WaitForSingleObject(done_queue.mutex, 0) == WAIT_OBJECT_0)) {
	struct squidaio_request_t *requests = done_queue.head;
	done_queue.head = NULL;
	done_queue.tailp = &done_queue.head;
	if (!ReleaseMutex(done_queue.mutex)) {
	    /* unexpected error */
	    fatal("couldn't poll queue\n");
	}
	*done_requests.tailp = requests;
	request_queue_len -= 1;
	while (requests->next) {
	    requests = requests->next;
	    request_queue_len -= 1;
	}
	done_requests.tailp = &requests->next;
    }
}

squidaio_result_t *
squidaio_poll_done(void)
{
    squidaio_request_t *request;
    squidaio_result_t *resultp;
    int cancelled;
    int polled = 0;

  AIO_REPOLL:
    request = done_requests.head;
    if (request == NULL && !polled) {
	if (done_signalled) {
	    char junk[256];
	    FD_READ_METHOD(done_fd_read, junk, sizeof(junk));
	    done_signalled = 0;
	}
	squidaio_poll_queues();
	polled = 1;
	request = done_requests.head;
    }
    if (!request) {
	return NULL;
    }
    debug(43, 9) ("squidaio_poll_done: %p type=%d result=%p\n",
	request, request->request_type, request->resultp);
    done_requests.head = request->next;
    if (!done_requests.head)
	done_requests.tailp = &done_requests.head;
    resultp = request->resultp;
    cancelled = request->cancelled;
    squidaio_debug(request);
    debug(43, 5) ("DONE: %d -> %d\n", request->ret, request->err);
    squidaio_cleanup_request(request);
    if (cancelled)
	goto AIO_REPOLL;
    return resultp;
}				/* squidaio_poll_done */

int
squidaio_operations_pending(void)
{
    return request_queue_len + (done_requests.head ? 1 : 0);
}

int
squidaio_sync(void)
{
    /* XXX This might take a while if the queue is large.. */
    do {
	squidaio_poll_queues();
    } while (request_queue_len > 0);
    return squidaio_operations_pending();
}

int
squidaio_get_queue_len(void)
{
    return request_queue_len;
}

static void
squidaio_debug(squidaio_request_t * request)
{
    switch (request->request_type) {
    case _AIO_OP_OPEN:
	debug(43, 5) ("OPEN of %s to FD %d\n", request->path, request->ret);
	break;
    case _AIO_OP_READ:
	debug(43, 5) ("READ on fd: %d\n", request->fd);
	break;
    case _AIO_OP_WRITE:
	debug(43, 5) ("WRITE on fd: %d\n", request->fd);
	break;
    case _AIO_OP_CLOSE:
	debug(43, 5) ("CLOSE of fd: %d\n", request->fd);
	break;
    case _AIO_OP_UNLINK:
	debug(43, 5) ("UNLINK of %s\n", request->path);
	break;
    case _AIO_OP_TRUNCATE:
	debug(43, 5) ("UNLINK of %s\n", request->path);
	break;
    default:
	break;
    }
}

void
squidaio_stats(StoreEntry * sentry)
{
    squidaio_thread_t *threadp;
    int i;

    if (!squidaio_initialised)
	return;

    storeAppendPrintf(sentry, "\n\nThreads Status:\n");
    storeAppendPrintf(sentry, "#\tID\t# Requests\n");

    threadp = threads;
    for (i = 0; i < squidaio_nthreads; i++) {
	storeAppendPrintf(sentry, "%i\t0x%lx\t%ld\n", i + 1, threadp->dwThreadId, threadp->requests);
	threadp = threadp->next;
    }
}
