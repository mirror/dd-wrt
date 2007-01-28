/*
 * store_aufs.h
 *
 * Internal declarations for the aufs routines
 */

#ifndef __STORE_ASYNCUFS_H__
#define __STORE_ASYNCUFS_H__

/* Which operations to run async */
#define ASYNC_OPEN 1
#define ASYNC_CLOSE 0
#define ASYNC_CREATE 1
#define ASYNC_WRITE 0
#define ASYNC_READ 1

struct _squidaioinfo_t {
    int swaplog_fd;
    int l1;
    int l2;
    fileMap *map;
    int suggest;
};

struct _squidaiostate_t {
    int fd;
    struct {
	unsigned int close_request:1;
	unsigned int reading:1;
	unsigned int writing:1;
	unsigned int opening:1;
#if !ASYNC_WRITE
	unsigned int write_kicking:1;
#endif
	unsigned int inreaddone:1;
    } flags;
    char *read_buf;
    link_list *pending_writes;
    link_list *pending_reads;
};

struct _queued_write {
    char *buf;
    size_t size;
    off_t offset;
    FREE *free_func;
};

struct _queued_read {
    char *buf;
    size_t size;
    off_t offset;
    STRCB *callback;
    void *callback_data;
};

typedef struct _squidaioinfo_t squidaioinfo_t;
typedef struct _squidaiostate_t squidaiostate_t;

/* The squidaio_state memory pools */
extern MemPool *squidaio_state_pool;
extern MemPool *aufs_qread_pool;
extern MemPool *aufs_qwrite_pool;

extern void storeAufsDirMapBitReset(SwapDir *, sfileno);
extern int storeAufsDirMapBitAllocate(SwapDir *);

extern char *storeAufsDirFullPath(SwapDir * SD, sfileno filn, char *fullpath);
extern void storeAufsDirUnlinkFile(SwapDir *, sfileno);
extern void storeAufsDirReplAdd(SwapDir * SD, StoreEntry *);
extern void storeAufsDirReplRemove(StoreEntry *);

/*
 * Store IO stuff
 */
extern STOBJCREATE storeAufsCreate;
extern STOBJOPEN storeAufsOpen;
extern STOBJCLOSE storeAufsClose;
extern STOBJREAD storeAufsRead;
extern STOBJWRITE storeAufsWrite;
extern STOBJUNLINK storeAufsUnlink;
extern STOBJRECYCLE storeAufsRecycle;

#endif
