/*
 * store_aufs.h
 *
 * Internal declarations for the aufs routines
 */

#ifndef __ASYNC_IO_H__
#define __ASYNC_IO_H__

extern int n_asyncufs_dirs;
extern int squidaio_nthreads;
extern int squidaio_magic1;
extern int squidaio_magic2;

/* Base number of threads if not specified to configure.
 * Weighted by number of directories (see aiops.c) */
#define THREAD_FACTOR 16

/* Queue limit where swapouts are deferred (load calculation) */
#define MAGIC1_FACTOR 10
#define MAGIC1 squidaio_magic1
/* Queue limit where swapins are deferred (open/create fails) */
#define MAGIC2_FACTOR 20
#define MAGIC2 squidaio_magic2

struct _squidaio_result_t {
    int aio_return;
    int aio_errno;
    void *_data;		/* Internal housekeeping */
    void *data;			/* Available to the caller */
};

typedef struct _squidaio_result_t squidaio_result_t;

typedef void AIOCB(int fd, void *cbdata, const char *buf, int aio_return, int aio_errno);

void squidaio_init(void);
void squidaio_shutdown(void);
int squidaio_cancel(squidaio_result_t *);
int squidaio_open(const char *, int, mode_t, squidaio_result_t *);
int squidaio_read(int, char *, int, off_t, int, squidaio_result_t *);
int squidaio_write(int, char *, int, off_t, int, squidaio_result_t *);
int squidaio_close(int, squidaio_result_t *);
int squidaio_stat(const char *, struct stat *, squidaio_result_t *);
int squidaio_unlink(const char *, squidaio_result_t *);
int squidaio_truncate(const char *, off_t length, squidaio_result_t *);
int squidaio_opendir(const char *, squidaio_result_t *);
squidaio_result_t *squidaio_poll_done(void);
int squidaio_operations_pending(void);
int squidaio_sync(void);
int squidaio_get_queue_len(void);
void *squidaio_xmalloc(int size);
void squidaio_xfree(void *p, int size);
void squidaio_stats(StoreEntry *);

void aioInit(void);
void aioDone(void);
void aioCancel(int);
void aioOpen(const char *, int, mode_t, AIOCB *, void *);
void aioClose(int);
void aioWrite(int, off_t offset, char *, int size, AIOCB *, void *, FREE *);
void aioRead(int, off_t offset, int size, AIOCB *, void *);
void aioStat(char *, struct stat *, AIOCB *, void *);
void aioUnlink(const char *, AIOCB *, void *);
void aioTruncate(const char *, off_t length, AIOCB *, void *);
int aioCheckCallbacks(SwapDir *);
void aioSync(SwapDir *);
int aioQueueSize(void);

#endif
