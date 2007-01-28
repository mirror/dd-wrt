#ifndef __COSS_H__
#define __COSS_H__

#ifndef COSS_MEMBUF_SZ
#define	COSS_MEMBUF_SZ	1048576
#endif

#define	COSS_REPORT_INTERVAL		20

/* Note that swap_filen in sio/e are actually disk block offsets too! */

typedef struct _cossmembuf CossMemBuf;
typedef struct _cossinfo CossInfo;
typedef struct _cossstate CossState;
typedef struct _cossindex CossIndexNode;
typedef struct _coss_pending_reloc CossPendingReloc;
typedef struct _coss_read_op CossReadOp;
typedef struct _cossstripe CossStripe;

/* What we're doing in storeCossAllocate() */
#define COSS_ALLOC_NOTIFY		0
#define COSS_ALLOC_ALLOCATE		1
#define COSS_ALLOC_REALLOC		2

#define SWAPDIR_COSS "coss"

#if USE_AUFSOPS
/* XXX a hack; the async ops should be broken out! */
typedef void AIOCB(int fd, void *cbdata, const char *buf,
    int aio_return, int aio_errno);
void aioWrite(int, off_t offset, char *, int size, AIOCB *, void *, FREE *);
void aioRead(int, off_t offset, int size, AIOCB *, void *);
void aioInit(void);
int aioCheckCallbacks(SwapDir *);
void aioSync(SwapDir *);
void squidaio_init(void);
void squidaio_shutdown(void);
extern int squidaio_magic1;
int aioQueueSize(void);
extern int squidaio_magic1;
#define MAGIC1 squidaio_magic1
#endif


struct _coss_stats {
    int stripes;
    int dead_stripes;
    struct {
	int alloc;
	int realloc;
	int memalloc;
	int collisions;
    } alloc;
    int disk_overflows;
    int stripe_overflows;
    int open_mem_hits;
    int open_mem_misses;
    struct {
	int ops;
	int success;
	int fail;
    } open, create, close, unlink, read, write, stripe_write;
};


struct _cossmembuf {
    dlink_node node;
    off_t diskstart;		/* in bytes */
    off_t diskend;		/* in bytes */
    int stripe;
    SwapDir *SD;
    int lockcount;
    char buffer[COSS_MEMBUF_SZ];
    struct _cossmembuf_flags {
	unsigned int full:1;
	unsigned int writing:1;
	unsigned int written:1;
	unsigned int dead:1;
	unsigned int memonly:1;
    } flags;
    int numobjs;
};

typedef enum {
    COSS_OP_NONE,
    COSS_OP_READ,
} coss_op_t;

struct _coss_read_op {
    /*
     * callback/callback data are part of the sio, and only one
     * read op will be scheduled at any time
     */
    coss_op_t type;
    dlink_node node;		/* per-storedir list */
    dlink_node pending_op_node;	/* children of the parent op we're blocking on */
    storeIOState *sio;
    size_t requestlen;
    size_t requestoffset;	/* in blocks */
    off_t reqdiskoffset;	/* in blocks */
    char *requestbuf;
    char completed;
    CossPendingReloc *pr;	/* NULL if we're not on a pending op list yet */
};

struct _cossstripe {
    int id;
    int numdiskobjs;
    int pending_relocs;
    struct _cossmembuf *membuf;
    dlink_list objlist;
};

struct _coss_pending_reloc {
    CossInfo *cs;
    dlink_node node;
    size_t len;
    sfileno original_filen, new_filen;	/* in blocks, not in bytes */
    dlink_list ops;
    char *p;
    struct _cossmembuf *locked_membuf;
};


/* Per-storedir info */
struct _cossinfo {
    dlink_list membufs;
    dlink_list dead_membufs;
    struct _cossmembuf *current_membuf;
    off_t current_offset;	/* in bytes */
    int fd;
    int swaplog_fd;
    int numcollisions;
    int loadcalc[2];
    int load_interval;
    dlink_list pending_relocs;
    dlink_list pending_ops;
    int pending_reloc_count;
    int count;
#if !USE_AUFSOPS
    async_queue_t aq;
#endif
    dlink_node *walk_current;
    unsigned int blksz_bits;
    unsigned int blksz_mask;	/* just 1<<blksz_bits - 1 */

    float minumum_overwrite_pct;
    int minimum_stripe_distance;
    int numstripes;
    int maxfullstripes;
    int hitonlyfullstripes;
    int numfullstripes;
    int sizerange_max;
    int sizerange_min;
    struct _cossstripe *stripes;
    int curstripe;
    struct {
	char rebuilding;
	char reading;
	int curstripe;
	char *buf;
	int buflen;
    } rebuild;
    int max_disk_nf;

    off_t current_memonly_offset;
    struct _cossmembuf *current_memonly_membuf;
    int nummemstripes;
    struct _cossstripe *memstripes;
    int curmemstripe;
    const char *stripe_path;
};

struct _cossindex {
    /* Note: the dlink_node MUST be the first member of the structure.
     * This member is later pointer typecasted to coss_index_node *.
     */
    dlink_node node;
};


/* Per-storeiostate info */
struct _cossstate {
    char *requestbuf;
    size_t requestlen;
    size_t requestoffset;	/* in blocks */
    off_t reqdiskoffset;	/* in blocks */
    struct {
	unsigned int reading:1;
	unsigned int writing:1;
	unsigned int reloc:1;
    } flags;
    struct _cossmembuf *locked_membuf;
};


/* Whether the coss system has been setup or not */
extern int coss_initialised;
extern MemPool *coss_membuf_pool;
extern MemPool *coss_state_pool;
extern MemPool *coss_index_pool;
extern MemPool *coss_realloc_pool;
extern MemPool *coss_op_pool;

/*
 * Store IO stuff
 */
extern STOBJCREATE storeCossCreate;
extern STOBJOPEN storeCossOpen;
extern STOBJCLOSE storeCossClose;
extern STOBJREAD storeCossRead;
extern STOBJWRITE storeCossWrite;
extern STOBJUNLINK storeCossUnlink;
extern STOBJRECYCLE storeCossRecycle;
extern STSYNC storeCossSync;

extern void storeCossAdd(SwapDir * sd, StoreEntry * e, int curstripe);
extern void storeCossRemove(SwapDir *, StoreEntry *);
extern void storeCossStartMembuf(SwapDir * SD);
extern void membufsDump(CossInfo * cs, StoreEntry * e);
extern void storeCossFreeDeadMemBufs(CossInfo * cs);
extern int storeCossFilenoToStripe(CossInfo * cs, sfileno filen);
extern char const *stripePath(SwapDir * sd);

extern struct _coss_stats coss_stats;

#endif
