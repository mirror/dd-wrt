
/*
 * $Id: store_dir_coss.c,v 1.66 2006/11/05 21:14:32 hno Exp $
 *
 * DEBUG: section 47    Store COSS Directory Routines
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

#define STORE_META_BUFSZ 4096
#define HITONLY_BUFS 2

int max_coss_dir_size = 0;
static int last_coss_pick_index = -1;
int coss_initialised = 0;
MemPool *coss_state_pool = NULL;
MemPool *coss_index_pool = NULL;
MemPool *coss_realloc_pool = NULL;
MemPool *coss_op_pool = NULL;

typedef struct _RebuildState RebuildState;
struct _RebuildState {
    SwapDir *sd;
    int n_read;
    FILE *log;
    int report_interval;
    int report_current;
    struct {
	unsigned int clean:1;
    } flags;
    struct _store_rebuild_data counts;
    struct {
	int new;
	int reloc;
	int fresher;
	int unknown;
    } cosscounts;
};

static char *storeCossDirSwapLogFile(SwapDir *, const char *);
static void storeCossDirRebuild(SwapDir * sd);
static void storeCossDirCloseTmpSwapLog(SwapDir * sd);
static FILE *storeCossDirOpenTmpSwapLog(SwapDir *, int *, int *);
static STLOGOPEN storeCossDirOpenSwapLog;
static STINIT storeCossDirInit;
static STLOGCLEANSTART storeCossDirWriteCleanStart;
static STLOGCLEANNEXTENTRY storeCossDirCleanLogNextEntry;
static STLOGCLEANWRITE storeCossDirWriteCleanEntry;
static STLOGCLEANDONE storeCossDirWriteCleanDone;
static STLOGCLOSE storeCossDirCloseSwapLog;
static STLOGWRITE storeCossDirSwapLog;
static STNEWFS storeCossDirNewfs;
static STCHECKOBJ storeCossDirCheckObj;
static STCHECKLOADAV storeCossDirCheckLoadAv;
static STFREE storeCossDirShutdown;
static STFSPARSE storeCossDirParse;
static STFSRECONFIGURE storeCossDirReconfigure;
static STDUMP storeCossDirDump;
static STCALLBACK storeCossDirCallback;
static void storeCossDirParseBlkSize(SwapDir *, const char *, const char *, int);
static void storeCossDirParseOverwritePct(SwapDir *, const char *, const char *, int);
static void storeCossDirParseMaxWaste(SwapDir *, const char *, const char *, int);
static void storeCossDirParseMemOnlyBufs(SwapDir *, const char *, const char *, int);
static void storeCossDirParseMaxFullBufs(SwapDir *, const char *, const char *, int);
static void storeCossDirDumpBlkSize(StoreEntry *, const char *, SwapDir *);
static void storeCossDirDumpOverwritePct(StoreEntry *, const char *, SwapDir *);
static void storeCossDirDumpMaxWaste(StoreEntry *, const char *, SwapDir *);
static void storeCossDirDumpMemOnlyBufs(StoreEntry *, const char *, SwapDir *);
static void storeCossDirDumpMaxFullBufs(StoreEntry *, const char *, SwapDir *);
static OBJH storeCossStats;

static void storeDirCoss_StartDiskRebuild(RebuildState * rb);

/* The "only" externally visible function */
STSETUP storeFsSetup_coss;

static struct cache_dir_option options[] =
{
    {"block-size", storeCossDirParseBlkSize, storeCossDirDumpBlkSize},
    {"overwrite-percent", storeCossDirParseOverwritePct, storeCossDirDumpOverwritePct},
    {"max-stripe-waste", storeCossDirParseMaxWaste, storeCossDirDumpMaxWaste},
    {"membufs", storeCossDirParseMemOnlyBufs, storeCossDirDumpMemOnlyBufs},
    {"maxfullbufs", storeCossDirParseMaxFullBufs, storeCossDirDumpMaxFullBufs},
    {NULL, NULL}
};

struct _coss_stats coss_stats;

char const *
stripePath(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    char pathtmp[SQUID_MAXPATHLEN];
    struct stat st;

    if (!cs->stripe_path) {
	strcpy(pathtmp, sd->path);
	if (stat(sd->path, &st) == 0) {
	    if (S_ISDIR(st.st_mode))
		strcat(pathtmp, "/stripe");
	} else
	    fatalf("stripePath: Cannot stat %s.", sd->path);
	cs->stripe_path = xstrdup(pathtmp);
    }
    return cs->stripe_path;
}

static char *
storeCossDirSwapLogFile(SwapDir * sd, const char *ext)
{
    LOCAL_ARRAY(char, path, SQUID_MAXPATHLEN);
    LOCAL_ARRAY(char, pathtmp, SQUID_MAXPATHLEN);
    LOCAL_ARRAY(char, digit, 32);
    char *pathtmp2;
    struct stat st;

    if (Config.Log.swap) {
	xstrncpy(pathtmp, sd->path, SQUID_MAXPATHLEN - 64);
	pathtmp2 = pathtmp;
	while ((pathtmp2 = strchr(pathtmp2, '/')) != NULL)
	    *pathtmp2 = '.';
	while (strlen(pathtmp) && pathtmp[strlen(pathtmp) - 1] == '.')
	    pathtmp[strlen(pathtmp) - 1] = '\0';
	for (pathtmp2 = pathtmp; *pathtmp2 == '.'; pathtmp2++);
	snprintf(path, SQUID_MAXPATHLEN - 64, Config.Log.swap, pathtmp2);
	if (strncmp(path, Config.Log.swap, SQUID_MAXPATHLEN - 64) == 0) {
	    strcat(path, ".");
	    snprintf(digit, 32, "%02d", sd->index);
	    strncat(path, digit, 3);
	}
    } else {
	if (stat(sd->path, &st) == 0) {
	    if (S_ISDIR(st.st_mode)) {
		xstrncpy(path, sd->path, SQUID_MAXPATHLEN - 64);
		strcat(path, "/swap.state");
	    } else
		fatal("storeCossDirSwapLogFile: 'cache_swap_log' is needed in your COSS configuration.");
	} else
	    fatalf("storeCossDirSwapLogFile: Cannot stat %s.", sd->path);
    }
    if (ext)
	strncat(path, ext, 16);
    return path;
}

static void
storeCossDirOpenSwapLog(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    char *path;
    int fd;
    path = storeCossDirSwapLogFile(sd, NULL);
    fd = file_open(path, O_WRONLY | O_CREAT | O_BINARY);
    if (fd < 0) {
	debug(79, 1) ("%s: %s\n", path, xstrerror());
	fatal("storeCossDirOpenSwapLog: Failed to open swap log.");
    }
    debug(79, 3) ("Cache COSS Dir #%d log opened on FD %d\n", sd->index, fd);
    cs->swaplog_fd = fd;
}

static void
storeCossDirCloseSwapLog(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    if (cs->swaplog_fd < 0)	/* not open */
	return;
    file_close(cs->swaplog_fd);
    debug(79, 3) ("Cache COSS Dir #%d log closed on FD %d\n",
	sd->index, cs->swaplog_fd);
    cs->swaplog_fd = -1;
}

static void
storeCossDirInit(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;

    /* COSS is pretty useless without 64 bit file offsets */
    if (sizeof(off_t) < 8) {
	fatalf("COSS will not function without large file support (off_t is %d bytes long. Please reconsider recompiling squid with --with-large-files\n", (int) sizeof(off_t));
    }
#if USE_AUFSOPS
    aioInit();
    squidaio_init();
#else
    a_file_setupqueue(&cs->aq);
#endif
    cs->fd = file_open(stripePath(sd), O_RDWR | O_CREAT | O_BINARY);
    if (cs->fd < 0) {
	debug(79, 1) ("%s: %s\n", stripePath(sd), xstrerror());
	fatal("storeCossDirInit: Failed to open a COSS file.");
    }
    storeCossDirOpenSwapLog(sd);
    storeCossDirRebuild(sd);
    n_coss_dirs++;
    /*
     * fs.blksize is normally determined by calling statvfs() etc,
     * but we just set it here.  It is used in accounting the
     * total store size, and is reported in cachemgr 'storedir'
     * page.
     */
    sd->fs.blksize = 1 << cs->blksz_bits;
    comm_quick_poll_required();
}

void
storeCossRemove(SwapDir * sd, StoreEntry * e)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    int stripe;
#if 0
    debug(1, 1) ("storeCossRemove: %x: %d/%d\n", e, (int) e->swap_dirn, (e) e->swap_filen);
#endif
    CossIndexNode *coss_node = e->repl.data;
    /* Do what the LRU and HEAP repl policies do.. */
    if (e->repl.data == NULL) {
	return;
    }
    assert(sd->index == e->swap_dirn);
    assert(e->swap_filen >= 0);
    e->repl.data = NULL;
    stripe = storeCossFilenoToStripe(cs, e->swap_filen);
    dlinkDelete(&coss_node->node, &cs->stripes[stripe].objlist);
    memPoolFree(coss_index_pool, coss_node);
    cs->count -= 1;
}

void
storeCossAdd(SwapDir * sd, StoreEntry * e, int curstripe)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    CossStripe *cstripe = &cs->stripes[curstripe];
    CossIndexNode *coss_node = memPoolAlloc(coss_index_pool);
    assert(!e->repl.data);
    assert(sd->index == e->swap_dirn);
    /* Make sure the object exists in the current stripe, it should do! */
    assert(curstripe == storeCossFilenoToStripe(cs, e->swap_filen));
    e->repl.data = coss_node;
    dlinkAddTail(e, &coss_node->node, &cstripe->objlist);
    cs->count += 1;
}

static void
storeCossRebuildComplete(void *data)
{
    RebuildState *rb = data;
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;
    storeCossStartMembuf(SD);
    store_dirs_rebuilding--;
    storeCossDirCloseTmpSwapLog(SD);
    storeRebuildComplete(&rb->counts);
    debug(47, 1) ("COSS: %s: Rebuild Completed\n", stripePath(SD));
    cs->rebuild.rebuilding = 0;
    debug(47, 1) ("  %d objects scanned, %d objects relocated, %d objects fresher, %d objects ignored\n",
	rb->counts.scancount, rb->cosscounts.reloc, rb->cosscounts.fresher, rb->cosscounts.unknown);
    cbdataFree(rb);
}

CBDATA_TYPE(RebuildState);
static void
storeCossDirRebuild(SwapDir * sd)
{
    RebuildState *rb;
    int clean = 0;
    int zero = 0;
    FILE *fp;
    CBDATA_INIT_TYPE(RebuildState);
    rb = cbdataAlloc(RebuildState);
    rb->sd = sd;
    rb->flags.clean = (unsigned int) clean;
    fp = storeCossDirOpenTmpSwapLog(sd, &clean, &zero);
    fclose(fp);
    debug(20, 1) ("Rebuilding COSS storage in %s (DIRTY)\n", stripePath(sd));
    store_dirs_rebuilding++;
    storeDirCoss_StartDiskRebuild(rb);
}

static void
storeCossDirCloseTmpSwapLog(SwapDir * sd)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    char *swaplog_path = xstrdup(storeCossDirSwapLogFile(sd, NULL));
    char *new_path = xstrdup(storeCossDirSwapLogFile(sd, ".new"));
    int fd;
    file_close(cs->swaplog_fd);
    if (xrename(new_path, swaplog_path) < 0) {
	fatal("storeCossDirCloseTmpSwapLog: rename failed");
    }
    fd = file_open(swaplog_path, O_WRONLY | O_CREAT | O_BINARY);
    if (fd < 0) {
	debug(50, 1) ("%s: %s\n", swaplog_path, xstrerror());
	fatal("storeCossDirCloseTmpSwapLog: Failed to open swap log.");
    }
    safe_free(swaplog_path);
    safe_free(new_path);
    cs->swaplog_fd = fd;
    debug(47, 3) ("Cache COSS Dir #%d log opened on FD %d\n", sd->index, fd);
}

static FILE *
storeCossDirOpenTmpSwapLog(SwapDir * sd, int *clean_flag, int *zero_flag)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    char *swaplog_path = xstrdup(storeCossDirSwapLogFile(sd, NULL));
    char *clean_path = xstrdup(storeCossDirSwapLogFile(sd, ".last-clean"));
    char *new_path = xstrdup(storeCossDirSwapLogFile(sd, ".new"));
    struct stat log_sb;
    struct stat clean_sb;
    FILE *fp;
    int fd;
    if (stat(swaplog_path, &log_sb) < 0) {
	debug(47, 1) ("Cache COSS Dir #%d: No log file\n", sd->index);
	safe_free(swaplog_path);
	safe_free(clean_path);
	safe_free(new_path);
	return NULL;
    }
    *zero_flag = log_sb.st_size == 0 ? 1 : 0;
    /* close the existing write-only FD */
    if (cs->swaplog_fd >= 0)
	file_close(cs->swaplog_fd);
    /* open a write-only FD for the new log */
    fd = file_open(new_path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    if (fd < 0) {
	debug(50, 1) ("%s: %s\n", new_path, xstrerror());
	fatal("storeDirOpenTmpSwapLog: Failed to open swap log.");
    }
    cs->swaplog_fd = fd;
    /* open a read-only stream of the old log */
    fp = fopen(swaplog_path, "rb");
    if (fp == NULL) {
	debug(50, 0) ("%s: %s\n", swaplog_path, xstrerror());
	fatal("Failed to open swap log for reading");
    }
    memset(&clean_sb, '\0', sizeof(struct stat));
    if (stat(clean_path, &clean_sb) < 0)
	*clean_flag = 0;
    else if (clean_sb.st_mtime < log_sb.st_mtime)
	*clean_flag = 0;
    else
	*clean_flag = 1;
    safeunlink(clean_path, 1);
    safe_free(swaplog_path);
    safe_free(clean_path);
    safe_free(new_path);
    return fp;
}

struct _clean_state {
    char *cur;
    char *new;
    char *cln;
    char *outbuf;
    int outbuf_offset;
    int fd;
    dlink_node *current;
};

#define CLEAN_BUF_SZ 16384
/*
 * Begin the process to write clean cache state.  For COSS this means
 * opening some log files and allocating write buffers.  Return 0 if
 * we succeed, and assign the 'func' and 'data' return pointers.
 */
static int
storeCossDirWriteCleanStart(SwapDir * sd)
{
    //CossInfo *cs = (CossInfo *) sd->fsdata;
    struct _clean_state *state = xcalloc(1, sizeof(*state));
#if HAVE_FCHMOD
    struct stat sb;
#endif
    state->new = xstrdup(storeCossDirSwapLogFile(sd, ".clean"));
    state->fd = file_open(state->new, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
    if (state->fd < 0) {
	xfree(state->new);
	xfree(state);
	return -1;
    }
    sd->log.clean.write = NULL;
    sd->log.clean.state = NULL;
    state->cur = xstrdup(storeCossDirSwapLogFile(sd, NULL));
    state->cln = xstrdup(storeCossDirSwapLogFile(sd, ".last-clean"));
    state->outbuf = xcalloc(CLEAN_BUF_SZ, 1);
    state->outbuf_offset = 0;
    unlink(state->cln);
    debug(20, 3) ("storeCOssDirWriteCleanLogs: opened %s, FD %d\n",
	state->new, state->fd);
#if HAVE_FCHMOD
    if (stat(state->cur, &sb) == 0)
	fchmod(state->fd, sb.st_mode);
#endif
    sd->log.clean.write = storeCossDirWriteCleanEntry;
    sd->log.clean.state = state;

    return 0;
}

static const StoreEntry *
storeCossDirCleanLogNextEntry(SwapDir * sd)
{
    struct _clean_state *state = sd->log.clean.state;
    const StoreEntry *entry;
    if (!state)
	return NULL;
    if (!state->current)
	return NULL;
    entry = (const StoreEntry *) state->current->data;
    state->current = state->current->prev;
    return entry;
}

/*
 * "write" an entry to the clean log file.
 */
static void
storeCossDirWriteCleanEntry(SwapDir * sd, const StoreEntry * e)
{
    storeSwapLogData s;
    static size_t ss = sizeof(storeSwapLogData);
    struct _clean_state *state = sd->log.clean.state;
    memset(&s, '\0', ss);
    s.op = (char) SWAP_LOG_ADD;
    s.swap_filen = e->swap_filen;
    s.timestamp = e->timestamp;
    s.lastref = e->lastref;
    s.expires = e->expires;
    s.lastmod = e->lastmod;
    s.swap_file_sz = e->swap_file_sz;
    s.refcount = e->refcount;
    s.flags = e->flags;
    xmemcpy(&s.key, e->hash.key, MD5_DIGEST_CHARS);
    xmemcpy(state->outbuf + state->outbuf_offset, &s, ss);
    state->outbuf_offset += ss;
    /* buffered write */
    if (state->outbuf_offset + ss > CLEAN_BUF_SZ) {
	if (FD_WRITE_METHOD(state->fd, state->outbuf, state->outbuf_offset) < 0) {
	    debug(50, 0) ("storeCossDirWriteCleanLogs: %s: write: %s\n",
		state->new, xstrerror());
	    debug(20, 0) ("storeCossDirWriteCleanLogs: Current swap logfile not replaced.\n");
	    file_close(state->fd);
	    state->fd = -1;
	    unlink(state->new);
	    safe_free(state);
	    sd->log.clean.state = NULL;
	    sd->log.clean.write = NULL;
	    return;
	}
	state->outbuf_offset = 0;
    }
}

static void
storeCossDirWriteCleanDone(SwapDir * sd)
{
    struct _clean_state *state = sd->log.clean.state;
    if (NULL == state)
	return;
    if (state->fd < 0)
	return;
    if (FD_WRITE_METHOD(state->fd, state->outbuf, state->outbuf_offset) < 0) {
	debug(50, 0) ("storeCossDirWriteCleanLogs: %s: write: %s\n",
	    state->new, xstrerror());
	debug(20, 0) ("storeCossDirWriteCleanLogs: Current swap logfile "
	    "not replaced.\n");
	file_close(state->fd);
	state->fd = -1;
	unlink(state->new);
    }
    safe_free(state->outbuf);
    /*
     * You can't rename open files on Microsoft "operating systems"
     * so we have to close before renaming.
     */
    storeCossDirCloseSwapLog(sd);
    /* rename */
    if (state->fd >= 0) {
#if defined(_SQUID_OS2_) || defined(_SQUID_WIN32_)
	file_close(state->fd);
	state->fd = -1;
#endif
	xrename(state->new, state->cur);
    }
    /* touch a timestamp file if we're not still validating */
    if (store_dirs_rebuilding)
	(void) 0;
    else if (state->fd < 0)
	(void) 0;
    else
	file_close(file_open(state->cln, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY));
    /* close */
    safe_free(state->cur);
    safe_free(state->new);
    safe_free(state->cln);
    if (state->fd >= 0)
	file_close(state->fd);
    state->fd = -1;
    safe_free(state);
    sd->log.clean.state = NULL;
    sd->log.clean.write = NULL;
}

static void
storeSwapLogDataFree(void *s)
{
    memFree(s, MEM_SWAP_LOG_DATA);
}

static void
storeCossDirSwapLog(const SwapDir * sd, const StoreEntry * e, int op)
{
    CossInfo *cs = (CossInfo *) sd->fsdata;
    storeSwapLogData *s = memAllocate(MEM_SWAP_LOG_DATA);
    s->op = (char) op;
    s->swap_filen = e->swap_filen;
    s->timestamp = e->timestamp;
    s->lastref = e->lastref;
    s->expires = e->expires;
    s->lastmod = e->lastmod;
    s->swap_file_sz = e->swap_file_sz;
    s->refcount = e->refcount;
    s->flags = e->flags;
    xmemcpy(s->key, e->hash.key, MD5_DIGEST_CHARS);
    file_write(cs->swaplog_fd,
	-1,
	s,
	sizeof(storeSwapLogData),
	NULL,
	NULL,
	(FREE *) storeSwapLogDataFree);
}

static void
storeCossCreateStripe(SwapDir * SD, const char *path)
{
    char *block;
    int swap;
    int i;
    CossInfo *cs = (CossInfo *) SD->fsdata;

    debug(47, 1) ("Creating COSS stripe %s\n", path);
    swap = open(path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0600);
    block = (char *) xcalloc(COSS_MEMBUF_SZ, 1);
    for (i = 0; i < cs->numstripes; ++i) {
	if (write(swap, block, COSS_MEMBUF_SZ) < COSS_MEMBUF_SZ) {
	    fatalf("Failed to create COSS stripe %s\n", path);
	}
    }
    close(swap);
    xfree(block);
}

static void
storeCossDirNewfs(SwapDir * SD)
{
    struct stat st;

    if (stat(SD->path, &st) == 0) {
	if (S_ISDIR(st.st_mode)) {
	    if (stat(stripePath(SD), &st) != 0)
		storeCossCreateStripe(SD, stripePath(SD));
	}
    } else
	storeCossCreateStripe(SD, (const char *) SD->path);
}

/*
 * Only "free" the filesystem specific stuff here
 */
static void
storeCossDirFree(SwapDir * SD)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    if (cs->swaplog_fd > -1) {
	file_close(cs->swaplog_fd);
	cs->swaplog_fd = -1;
    }
    xfree(cs->stripes);
    xfree(cs->memstripes);
    xfree(cs);
    SD->fsdata = NULL;		/* Will aid debugging... */

}

/* we are shutting down, flush all membufs to disk */
static void
storeCossDirShutdown(SwapDir * SD)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    debug(47, 1) ("COSS: %s: syncing\n", stripePath(SD));

    storeCossSync(SD);		/* This'll call a_file_syncqueue() or a aioSync() */
#if !USE_AUFSOPS
    a_file_closequeue(&cs->aq);
#endif
    file_close(cs->fd);
    cs->fd = -1;
    xfree((void *) cs->stripe_path);

    if (cs->swaplog_fd > -1) {
	file_close(cs->swaplog_fd);
	cs->swaplog_fd = -1;
    }
    n_coss_dirs--;
}

/*
 * storeCossDirCheckObj
 *
 * This routine is called by storeDirSelectSwapDir to see if the given
 * object is able to be stored on this filesystem. COSS filesystems will
 * not store everything. We don't check for maxobjsize here since its
 * done by the upper layers.
 */
int
storeCossDirCheckObj(SwapDir * SD, const StoreEntry * e)
{
    CossInfo *cs = SD->fsdata;
    int objsize = objectLen(e) + e->mem_obj->swap_hdr_sz;
    /* Check if the object is a special object, we can't cache these */
    if (EBIT_TEST(e->flags, ENTRY_SPECIAL))
	return 0;
    if (cs->rebuild.rebuilding == 1)
	return 0;
    /* Check to see if the object is going to waste too much disk space */
    if (objsize > cs->sizerange_max)
	return 0;

    return 1;
}

int
storeCossDirCheckLoadAv(SwapDir * SD, store_op_t op)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
#if USE_AUFSOPS
    float disk_size_weight, current_write_weight;
    int cur_load_interval = (squid_curtime / cs->load_interval) % 2;
    int ql = 0;
#endif
    int loadav;

    /* Return load, cs->aq.aq_numpending out of MAX_ASYNCOP */
#if USE_AUFSOPS
    ql = aioQueueSize();
    if (ql == 0)
	loadav = COSS_LOAD_BASE;
    else
	loadav = COSS_LOAD_BASE + (ql * COSS_LOAD_QUEUE_WEIGHT / MAGIC1);

    /* We want to try an keep the disks at a similar write rate 
     * otherwise the LRU algorithm breaks
     *
     * The queue length has a 10% weight on the load
     * The number of stripes written has a 90% weight
     */
    disk_size_weight = (float) max_coss_dir_size / SD->max_size;
    current_write_weight = (float) cs->loadcalc[cur_load_interval] * COSS_LOAD_STRIPE_WEIGHT / MAX_LOAD_VALUE;

    loadav += disk_size_weight * current_write_weight;

    /* Remove the folowing check if we want to allow COSS partitions to get
     * too busy to accept new objects
     */
    if (loadav > MAX_LOAD_VALUE)
	loadav = MAX_LOAD_VALUE;

    /* Finally, we want to reject all new obects if the number of full stripes
     * is too large
     */
    if (cs->numfullstripes > cs->hitonlyfullstripes)
	loadav += MAX_LOAD_VALUE;

    debug(47, 9) ("storeAufsDirCheckObj: load=%d\n", loadav);
    return loadav;
#else
    loadav = cs->aq.aq_numpending * MAX_LOAD_VALUE / MAX_ASYNCOP;
    return loadav;
#endif
}


/*
 * storeCossDirCallback - do the IO completions
 */
static int
storeCossDirCallback(SwapDir * SD)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;
    storeCossFreeDeadMemBufs(cs);
#if USE_AUFSOPS
    /* I believe this call, at the present, checks all callbacks for all SDs, not just ours */
    return aioCheckCallbacks(SD);
#else
    return a_file_callback(&cs->aq);
#endif
}

/* ========== LOCAL FUNCTIONS ABOVE, GLOBAL FUNCTIONS BELOW ========== */

static void
storeCossDirStats(SwapDir * SD, StoreEntry * sentry)
{
    CossInfo *cs = (CossInfo *) SD->fsdata;

    storeAppendPrintf(sentry, "\n");
    storeAppendPrintf(sentry, "Maximum Size: %d KB\n", SD->max_size);
    storeAppendPrintf(sentry, "Current Size: %d KB\n", SD->cur_size);
    storeAppendPrintf(sentry, "Percent Used: %0.2f%%\n",
	100.0 * SD->cur_size / SD->max_size);
    storeAppendPrintf(sentry, "Current load metric: %d / %d\n", storeCossDirCheckLoadAv(SD, ST_OP_CREATE), MAX_LOAD_VALUE);
    storeAppendPrintf(sentry, "Number of object collisions: %d\n", (int) cs->numcollisions);
#if 0
    /* is this applicable? I Hope not .. */
    storeAppendPrintf(sentry, "Filemap bits in use: %d of %d (%d%%)\n",
	SD->map->n_files_in_map, SD->map->max_n_files,
	percent(SD->map->n_files_in_map, SD->map->max_n_files));
#endif
#if !USE_AUFSOPS
    storeAppendPrintf(sentry, "Pending operations: %d out of %d\n", cs->aq.aq_numpending, MAX_ASYNCOP);
#endif
    storeAppendPrintf(sentry, "Flags:");
    if (SD->flags.selected)
	storeAppendPrintf(sentry, " SELECTED");
    if (SD->flags.read_only)
	storeAppendPrintf(sentry, " READ-ONLY");
    storeAppendPrintf(sentry, "\n");
    storeAppendPrintf(sentry, "Pending Relocations: %d\n", cs->pending_reloc_count);
    membufsDump(cs, sentry);
}

static void
storeCossDirParse(SwapDir * sd, int index, char *path)
{
    unsigned int i;
    unsigned int size;
    CossInfo *cs;
    off_t max_offset;

    i = GetInteger();
    size = i << 10;		/* Mbytes to Kbytes */
    if (size <= 0)
	fatal("storeCossDirParse: invalid size value");

    cs = xcalloc(1, sizeof(CossInfo));
    if (cs == NULL)
	fatal("storeCossDirParse: couldn't xmalloc() CossInfo!\n");

    sd->index = index;
    sd->path = xstrdup(path);
    sd->max_size = size;
    sd->fsdata = cs;

    cs->fd = -1;
    cs->swaplog_fd = -1;

    sd->init = storeCossDirInit;
    sd->newfs = storeCossDirNewfs;
    sd->dump = storeCossDirDump;
    sd->freefs = storeCossDirFree;
    sd->dblcheck = NULL;
    sd->statfs = storeCossDirStats;
    sd->maintainfs = NULL;
    sd->checkobj = storeCossDirCheckObj;
    sd->checkload = storeCossDirCheckLoadAv;
    sd->refobj = NULL;		/* LRU is done in storeCossRead */
    sd->unrefobj = NULL;
    sd->callback = storeCossDirCallback;
    sd->sync = storeCossSync;

    sd->obj.create = storeCossCreate;
    sd->obj.open = storeCossOpen;
    sd->obj.close = storeCossClose;
    sd->obj.read = storeCossRead;
    sd->obj.write = storeCossWrite;
    sd->obj.unlink = storeCossUnlink;
    sd->obj.recycle = storeCossRecycle;

    sd->log.open = storeCossDirOpenSwapLog;
    sd->log.close = storeCossDirCloseSwapLog;
    sd->log.write = storeCossDirSwapLog;
    sd->log.clean.start = storeCossDirWriteCleanStart;
    sd->log.clean.write = storeCossDirWriteCleanEntry;
    sd->log.clean.nextentry = storeCossDirCleanLogNextEntry;
    sd->log.clean.done = storeCossDirWriteCleanDone;

    cs->current_offset = 0;
    cs->fd = -1;
    cs->swaplog_fd = -1;
    cs->numcollisions = 0;
    cs->membufs.head = cs->membufs.tail = NULL;		/* set when the rebuild completes */
    cs->current_membuf = NULL;
    cs->blksz_bits = 9;		/* default block size = 512 */
    cs->blksz_mask = (1 << cs->blksz_bits) - 1;

    /* By default, only overwrite objects that were written mor ethan 50% of the disk ago
     * and use a maximum of 10 in-memory stripes
     */
    cs->minumum_overwrite_pct = 0.5;
    cs->nummemstripes = 10;

    /* Calculate load in 60 second incremenets */
    /* This could be made configurable */
    cs->load_interval = 60;

    parse_cachedir_options(sd, options, 0);

    cs->sizerange_max = sd->max_objsize;
    cs->sizerange_min = sd->max_objsize;

    /* Enforce maxobjsize being set to something */
    if (sd->max_objsize == -1)
	fatal("COSS requires max-size to be set to something other than -1!\n");
    if (sd->max_objsize > COSS_MEMBUF_SZ)
	fatalf("COSS max-size option must be less than COSS_MEMBUF_SZ (%d)\n", COSS_MEMBUF_SZ);
    /*
     * check that we won't overflow sfileno later.  0xFFFFFF is the
     * largest possible sfileno, assuming sfileno is a 25-bit
     * signed integer, as defined in structs.h.
     */
    max_offset = (off_t) 0xFFFFFF << cs->blksz_bits;
    if ((sd->max_size + (cs->nummemstripes * (COSS_MEMBUF_SZ >> 10))) > (unsigned long) (max_offset >> 10)) {
	debug(47, 1) ("COSS block-size = %d bytes\n", 1 << cs->blksz_bits);
	debug(47, 1) ("COSS largest file offset = %lu KB\n", (unsigned long) max_offset >> 10);
	debug(47, 1) ("COSS cache_dir size = %d KB\n", sd->max_size);
	fatal("COSS cache_dir size exceeds largest offset\n");
    }
    cs->max_disk_nf = ((off_t) sd->max_size << 10) >> cs->blksz_bits;
    debug(47, 2) ("COSS: max disk fileno is %d\n", cs->max_disk_nf);

    /* XXX todo checks */

    /* Ensure that off_t range can cover the max_size */

    /* Ensure that the max size IS a multiple of the membuf size, or things
     * will get very fruity near the end of the disk. */
    cs->numstripes = (off_t) (((off_t) sd->max_size) << 10) / COSS_MEMBUF_SZ;
    debug(47, 2) ("COSS: number of stripes: %d of %d bytes each\n", cs->numstripes, COSS_MEMBUF_SZ);
    cs->stripes = xcalloc(cs->numstripes, sizeof(struct _cossstripe));
    for (i = 0; i < cs->numstripes; i++) {
	cs->stripes[i].id = i;
	cs->stripes[i].membuf = NULL;
	cs->stripes[i].numdiskobjs = -1;
    }
    cs->minimum_stripe_distance = cs->numstripes * cs->minumum_overwrite_pct;

    /* Make sure cs->maxfull has a default value */
    if (cs->maxfullstripes == 0)
	cs->maxfullstripes = cs->numstripes;

    /* We will reject new objects (ie go into hit-only mode)
     * if there are <= 2 stripes available
     */
    cs->hitonlyfullstripes = cs->maxfullstripes - HITONLY_BUFS;

    debug(47, 2) ("COSS: number of memory-only stripes %d of %d bytes each\n", cs->nummemstripes, COSS_MEMBUF_SZ);
    cs->memstripes = xcalloc(cs->nummemstripes, sizeof(struct _cossstripe));
    for (i = 0; i < cs->nummemstripes; i++) {
	cs->memstripes[i].id = i;
	cs->memstripes[i].membuf = NULL;
	cs->memstripes[i].numdiskobjs = -1;
    }

    /* Update the max size (used for load calculations) */
    if (sd->max_size > max_coss_dir_size)
	max_coss_dir_size = sd->max_size;
}

static void
storeCossDirReconfigure(SwapDir * sd, int index, char *path)
{
    unsigned int i;
    unsigned int size;

    i = GetInteger();
    size = i << 10;		/* Mbytes to Kbytes */
    if (size <= 0)
	fatal("storeCossDirParse: invalid size value");

    if (size == sd->max_size)
	debug(3, 1) ("Cache COSS dir '%s' size remains unchanged at %d KB\n", path, size);
    else {
	debug(3, 1) ("Cache COSS dir '%s' size changed to %d KB\n", path, size);
	sd->max_size = size;
    }
    parse_cachedir_options(sd, options, 1);
    /* Enforce maxobjsize being set to something */
    if (sd->max_objsize == -1)
	fatal("COSS requires max-size to be set to something other than -1!\n");
}

void
storeCossDirDump(StoreEntry * entry, SwapDir * s)
{
    storeAppendPrintf(entry, " %d", s->max_size >> 10);
    dump_cachedir_options(entry, options, s);
}

static void
storeCossDirParseMaxFullBufs(SwapDir * sd, const char *name, const char *value, int reconfiguring)
{
    CossInfo *cs = sd->fsdata;
    int maxfull = atoi(value);
    if (maxfull <= HITONLY_BUFS)
	fatalf("COSS ERROR: There must be more than %d maxfullbufs\n", HITONLY_BUFS);
    if (maxfull > 500)
	fatal("COSS ERROR: Squid will likely use too much memory if it ever used 500MB worth of full buffers\n");
    cs->maxfullstripes = maxfull;
}

static void
storeCossDirParseMemOnlyBufs(SwapDir * sd, const char *name, const char *value, int reconfiguring)
{
    CossInfo *cs = sd->fsdata;
    int membufs = atoi(value);
    if (reconfiguring) {
	debug(47, 0) ("WARNING: cannot change COSS memory bufs Squid is running\n");
	return;
    }
    if (membufs < 2)
	fatal("COSS ERROR: There must be at least 2 membufs\n");
    if (membufs > 500)
	fatal("COSS ERROR: Squid will likely use too much memory if it ever used 500MB worth of buffers\n");
    cs->nummemstripes = membufs;
}

static void
storeCossDirParseMaxWaste(SwapDir * sd, const char *name, const char *value, int reconfiguring)
{
    CossInfo *cs = sd->fsdata;
    int waste = atoi(value);

    if (waste < 8192)
	fatal("COSS max-stripe-waste must be > 8192\n");
    if (waste > sd->max_objsize)
	debug(47, 1) ("storeCossDirParseMaxWaste: COSS max-stripe-waste can not be bigger than the max object size (%" PRINTF_OFF_T ")\n", sd->max_objsize);
    cs->sizerange_min = waste;
}

static void
storeCossDirParseOverwritePct(SwapDir * sd, const char *name, const char *value, int reconfiguring)
{
    CossInfo *cs = sd->fsdata;
    int pct = atoi(value);

    if (pct < 0)
	fatal("COSS overwrite percent must be > 0\n");
    if (pct > 100)
	fatal("COSS overwrite percent must be < 100\n");
    cs->minumum_overwrite_pct = (float) pct / 100;
    cs->minimum_stripe_distance = cs->numstripes * cs->minumum_overwrite_pct;
}

static void
storeCossDirParseBlkSize(SwapDir * sd, const char *name, const char *value, int reconfiguring)
{
    CossInfo *cs = sd->fsdata;
    int blksz = atoi(value);
    int check;
    int nbits;
    if (blksz == (1 << cs->blksz_bits))
	/* no change */
	return;
    if (reconfiguring) {
	debug(47, 0) ("WARNING: cannot change COSS block-size while Squid is running\n");
	return;
    }
    nbits = 0;
    check = blksz;
    while (check > 1) {
	nbits++;
	check >>= 1;
    }
    check = 1 << nbits;
    if (check != blksz)
	fatal("COSS block-size must be a power of 2\n");
    if (nbits > 13)
	fatal("COSS block-size must be 8192 or smaller\n");
    cs->blksz_bits = nbits;
    cs->blksz_mask = (1 << cs->blksz_bits) - 1;
}

static void
storeCossDirDumpMaxFullBufs(StoreEntry * e, const char *option, SwapDir * sd)
{
    CossInfo *cs = sd->fsdata;
    storeAppendPrintf(e, " maxfullbufs=%d MB", cs->maxfullstripes);
}

static void
storeCossDirDumpMemOnlyBufs(StoreEntry * e, const char *option, SwapDir * sd)
{
    CossInfo *cs = sd->fsdata;
    storeAppendPrintf(e, " membufs=%d MB", cs->nummemstripes);
}

static void
storeCossDirDumpMaxWaste(StoreEntry * e, const char *option, SwapDir * sd)
{
    CossInfo *cs = sd->fsdata;
    storeAppendPrintf(e, " max-stripe-waste=%d", cs->sizerange_min);
}

static void
storeCossDirDumpOverwritePct(StoreEntry * e, const char *option, SwapDir * sd)
{
    CossInfo *cs = sd->fsdata;
    storeAppendPrintf(e, " overwrite-percent=%d%%", (int) cs->minumum_overwrite_pct * 100);
}

static void
storeCossDirDumpBlkSize(StoreEntry * e, const char *option, SwapDir * sd)
{
    CossInfo *cs = sd->fsdata;
    storeAppendPrintf(e, " block-size=%d", 1 << cs->blksz_bits);
}

static SwapDir *
storeCossDirPick(void)
{
    int i, choosenext = 0;
    SwapDir *SD;

    if (n_coss_dirs == 0)
	return NULL;
    for (i = 0; i < Config.cacheSwap.n_configured; i++) {
	SD = &Config.cacheSwap.swapDirs[i];
	if (strcmp(SD->type, SWAPDIR_COSS) == 0) {
	    if ((last_coss_pick_index == -1) || (n_coss_dirs == 1)) {
		last_coss_pick_index = i;
		return SD;
	    } else if (choosenext) {
		last_coss_pick_index = i;
		return SD;
	    } else if (last_coss_pick_index == i) {
		choosenext = 1;
	    }
	}
    }
    for (i = 0; i < Config.cacheSwap.n_configured; i++) {
	SD = &Config.cacheSwap.swapDirs[i];
	if (strcmp(SD->type, SWAPDIR_COSS) == 0) {
	    if ((last_coss_pick_index == -1) || (n_coss_dirs == 1)) {
		last_coss_pick_index = i;
		return SD;
	    } else if (choosenext) {
		last_coss_pick_index = i;
		return SD;
	    } else if (last_coss_pick_index == i) {
		choosenext = 1;
	    }
	}
    }
    return NULL;
}

/*
 * initial setup/done code
 */
static void
storeCossDirDone(void)
{
    int i, n_dirs = n_coss_dirs;

    for (i = 0; i < n_dirs; i++)
	storeCossDirShutdown(storeCossDirPick());
/* 
 * TODO : check if others memPoolDestroy() of COSS objects are needed here
 */
    memPoolDestroy(coss_state_pool);
    coss_initialised = 0;
}

static void
storeCossStats(StoreEntry * sentry)
{
    const char *tbl_fmt = "%10s %10d %10d %10d\n";
    storeAppendPrintf(sentry, "\n                   OPS     SUCCESS        FAIL\n");
    storeAppendPrintf(sentry, tbl_fmt,
	"open", coss_stats.open.ops, coss_stats.open.success, coss_stats.open.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"create", coss_stats.create.ops, coss_stats.create.success, coss_stats.create.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"close", coss_stats.close.ops, coss_stats.close.success, coss_stats.close.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"unlink", coss_stats.unlink.ops, coss_stats.unlink.success, coss_stats.unlink.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"read", coss_stats.read.ops, coss_stats.read.success, coss_stats.read.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"write", coss_stats.write.ops, coss_stats.write.success, coss_stats.write.fail);
    storeAppendPrintf(sentry, tbl_fmt,
	"s_write", coss_stats.stripe_write.ops, coss_stats.stripe_write.success, coss_stats.stripe_write.fail);
    storeAppendPrintf(sentry, "\n");
    storeAppendPrintf(sentry, "stripes:          %d\n", coss_stats.stripes);
    storeAppendPrintf(sentry, "dead_stripes:     %d\n", coss_stats.dead_stripes);
    storeAppendPrintf(sentry, "alloc.alloc:      %d\n", coss_stats.alloc.alloc);
    storeAppendPrintf(sentry, "alloc.realloc:    %d\n", coss_stats.alloc.realloc);
    storeAppendPrintf(sentry, "alloc.memalloc:   %d\n", coss_stats.alloc.memalloc);
    storeAppendPrintf(sentry, "alloc.collisions: %d\n", coss_stats.alloc.collisions);
    storeAppendPrintf(sentry, "disk_overflows:   %d\n", coss_stats.disk_overflows);
    storeAppendPrintf(sentry, "stripe_overflows: %d\n", coss_stats.stripe_overflows);
    storeAppendPrintf(sentry, "open_mem_hits:    %d\n", coss_stats.open_mem_hits);
    storeAppendPrintf(sentry, "open_mem_misses:  %d\n", coss_stats.open_mem_misses);
}

void
storeFsSetup_coss(storefs_entry_t * storefs)
{
    assert(!coss_initialised);

    storefs->parsefunc = storeCossDirParse;
    storefs->reconfigurefunc = storeCossDirReconfigure;
    storefs->donefunc = storeCossDirDone;
    coss_state_pool = memPoolCreate("COSS IO State data", sizeof(CossState));
    coss_index_pool = memPoolCreate("COSS index data", sizeof(CossIndexNode));
    coss_realloc_pool = memPoolCreate("COSS pending realloc", sizeof(CossPendingReloc));
    coss_op_pool = memPoolCreate("COSS pending operation", sizeof(CossReadOp));
    cachemgrRegister(SWAPDIR_COSS, "COSS Stats", storeCossStats, 0, 1);
    coss_initialised = 1;
}

/* New storedir rebuilding code! */

static void storeDirCoss_ReadStripe(RebuildState * rb);
static void storeDirCoss_ParseStripeBuffer(RebuildState * rb);
static void storeCoss_ConsiderStoreEntry(RebuildState * rb, const cache_key * key, StoreEntry * e);

#if USE_AUFSOPS
static void
storeDirCoss_ReadStripeComplete(int fd, void *my_data, const char *buf, int aio_return, int aio_errno)
#else
static void
storeDirCoss_ReadStripeComplete(int fd, const char *buf, int r_len, int r_errflag, void *my_data)
#endif
{
    RebuildState *rb = my_data;
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;
#if USE_AUFSOPS
    int r_errflag;
    int r_len;
    r_len = aio_return;
    if (aio_errno)
	r_errflag = aio_errno == ENOSPC ? DISK_NO_SPACE_LEFT : DISK_ERROR;
    else
	r_errflag = DISK_OK;
    xmemcpy(cs->rebuild.buf, buf, r_len);
#endif

    debug(47, 2) ("COSS: %s: stripe %d, read %d bytes, status %d\n", stripePath(SD), cs->rebuild.curstripe, r_len, r_errflag);
    cs->rebuild.reading = 0;
    if (r_errflag != DISK_OK) {
	debug(47, 2) ("COSS: %s: stripe %d: error! Ignoring objects in this stripe.\n", stripePath(SD), cs->rebuild.curstripe);
	goto nextstripe;
    }
    cs->rebuild.buflen = r_len;
    /* parse the stripe contents */
    /* 
     * XXX note: the read should be put before the parsing so they can happen
     * simultaneously. This'll require some code-shifting so the read buffer
     * and parse buffer are different. This might speed up the read speed;
     * the disk throughput isn't being reached at the present.
     */
    storeDirCoss_ParseStripeBuffer(rb);

  nextstripe:
    cs->rebuild.curstripe++;
    if (cs->rebuild.curstripe >= cs->numstripes) {
	/* Completed the rebuild - move onto the next phase */
	debug(47, 2) ("COSS: %s: completed reading the stripes.\n", stripePath(SD));
	storeCossRebuildComplete(rb);
	return;
    } else {
	/* Next stripe */
	storeDirCoss_ReadStripe(rb);
    }
}

static void
storeDirCoss_ReadStripe(RebuildState * rb)
{
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;

    assert(cs->rebuild.reading == 0);
    cs->rebuild.reading = 1;
    /* Use POSIX AIO for now */
    debug(47, 2) ("COSS: %s: reading stripe %d\n", stripePath(SD), cs->rebuild.curstripe);
    if (cs->rebuild.curstripe > rb->report_current) {
	debug(47, 1) ("COSS: %s: Rebuilding (%d %% completed - %d/%d stripes)\n", stripePath(SD),
	    cs->rebuild.curstripe * 100 / cs->numstripes, cs->rebuild.curstripe, cs->numstripes);
	rb->report_current += rb->report_interval;
    }
#if USE_AUFSOPS
    /* XXX this should be a prime candidate to use a modified aioRead which doesn't malloc a damned buffer */
    aioRead(cs->fd, (off_t) cs->rebuild.curstripe * COSS_MEMBUF_SZ, COSS_MEMBUF_SZ, storeDirCoss_ReadStripeComplete, rb);
#else
    a_file_read(&cs->aq, cs->fd, cs->rebuild.buf, COSS_MEMBUF_SZ, (off_t) cs->rebuild.curstripe * COSS_MEMBUF_SZ, storeDirCoss_ReadStripeComplete, rb);
#endif
}

static void
storeDirCoss_StartDiskRebuild(RebuildState * rb)
{
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;
    assert(cs->rebuild.rebuilding == 0);
    assert(cs->numstripes > 0);
    assert(cs->rebuild.buf == NULL);
    assert(cs->fd >= 0);
    cs->rebuild.rebuilding = 1;
    cs->rebuild.curstripe = 0;
    cs->rebuild.buf = xmalloc(COSS_MEMBUF_SZ);
    rb->report_interval = cs->numstripes / COSS_REPORT_INTERVAL;
    rb->report_current = 0;
    debug(47, 2) ("COSS: %s: Beginning disk rebuild.\n", stripePath(SD));
    storeDirCoss_ReadStripe(rb);
}

/*
 * Take a stripe and attempt to place objects into it
 */
static void
storeDirCoss_ParseStripeBuffer(RebuildState * rb)
{
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;
    tlv *t, *tlv_list;
    int j = 0;
    int bl = 0;
    int tmp;
    squid_off_t *l, len;
    int blocksize = cs->blksz_mask + 1;
    StoreEntry tmpe;
    cache_key key[MD5_DIGEST_CHARS];
    sfileno filen;

    assert(cs->rebuild.rebuilding == 1);
    assert(cs->numstripes > 0);
    assert(cs->rebuild.buf != NULL);

    if (cs->rebuild.buflen == 0) {
	debug(47, 3) ("COSS: %s: stripe %d: read 0 bytes, skipping stripe\n", stripePath(SD), cs->rebuild.curstripe);
	return;
    }
    while (j < cs->rebuild.buflen) {
	l = NULL;
	bl = 0;
	/* XXX there's no bounds checking on the buffer being passed into storeSwapMetaUnpack! */
	tlv_list = storeSwapMetaUnpack(cs->rebuild.buf + j, &bl);
	if (tlv_list == NULL) {
	    debug(47, 3) ("COSS: %s: stripe %d: offset %d gives NULL swapmeta data; end of stripe\n", stripePath(SD), cs->rebuild.curstripe, j);
	    return;
	}
	filen = (off_t) j / (off_t) blocksize + (off_t) ((off_t) cs->rebuild.curstripe * (off_t) COSS_MEMBUF_SZ / (off_t) blocksize);
	debug(47, 3) ("COSS: %s: stripe %d: filen %d: header size %d\n", stripePath(SD), cs->rebuild.curstripe, filen, bl);

	/* COSS objects will have an object size written into the metadata */
	memset(&tmpe, 0, sizeof(tmpe));
	memset(key, 0, sizeof(key));
	for (t = tlv_list; t; t = t->next) {
	    switch (t->type) {
	    case STORE_META_URL:
		debug(47, 3) ("    URL: %s\n", (char *) t->value);
		break;
	    case STORE_META_OBJSIZE:
		l = t->value;
		debug(47, 3) ("Size: %" PRINTF_OFF_T " (len %d)\n", *l, t->length);
		break;
	    case STORE_META_KEY:
		assert(t->length == MD5_DIGEST_CHARS);
		xmemcpy(key, t->value, MD5_DIGEST_CHARS);
		break;
#if SIZEOF_SQUID_FILE_SZ == SIZEOF_SIZE_T
	    case STORE_META_STD:
		assert(t->length == STORE_HDR_METASIZE);
		xmemcpy(&tmpe.timestamp, t->value, STORE_HDR_METASIZE);
		break;
#else
	    case STORE_META_STD_LFS:
		assert(t->length == STORE_HDR_METASIZE);
		xmemcpy(&tmpe.timestamp, t->value, STORE_HDR_METASIZE);
		break;
	    case STORE_META_STD:
		assert(t->length == STORE_HDR_METASIZE_OLD);
		{
		    struct {
			time_t timestamp;
			time_t lastref;
			time_t expires;
			time_t lastmod;
			size_t swap_file_sz;
			u_short refcount;
			u_short flags;
		    }     *tmp = t->value;
		    assert(sizeof(*tmp) == STORE_HDR_METASIZE_OLD);
		    tmpe.timestamp = tmp->timestamp;
		    tmpe.lastref = tmp->lastref;
		    tmpe.expires = tmp->expires;
		    tmpe.lastmod = tmp->lastmod;
		    tmpe.swap_file_sz = tmp->swap_file_sz;
		    tmpe.refcount = tmp->refcount;
		    tmpe.flags = tmp->flags;
		}
		break;
#endif
	    }
	}
	/* Make sure we have an object; if we don't then it may be an indication of trouble */
	if (l == NULL) {
	    debug(47, 3) ("COSS: %s: stripe %d: Object with no size; end of stripe\n", stripePath(SD), cs->rebuild.curstripe);
	    storeSwapTLVFree(tlv_list);
	    return;
	}
	len = *l;
	/* Finally, make sure there's enough data left in this stripe to satisfy the object
	 * we've just been informed about
	 */
	if ((cs->rebuild.buflen - j) < (len + bl)) {
	    debug(47, 3) ("COSS: %s: stripe %d: Not enough data in this stripe for this object, bye bye.\n", stripePath(SD), cs->rebuild.curstripe);
	    storeSwapTLVFree(tlv_list);
	    return;
	}
	/* Houston, we have an object */
	if (storeKeyNull(key)) {
	    debug(47, 3) ("COSS: %s: stripe %d: null data, next!\n", stripePath(SD), cs->rebuild.curstripe);
	    goto nextobject;
	}
	rb->counts.scancount++;
	tmpe.hash.key = key;
	/* Check sizes */
	if (tmpe.swap_file_sz == 0) {
	    tmpe.swap_file_sz = len + bl;
	}
	if (tmpe.swap_file_sz != (len + bl)) {
	    debug(47, 3) ("COSS: %s: stripe %d: file size mismatch (%" PRINTF_OFF_T " != %" PRINTF_OFF_T ")\n", stripePath(SD), cs->rebuild.curstripe, tmpe.swap_file_sz, len);
	    goto nextobject;
	}
	if (EBIT_TEST(tmpe.flags, KEY_PRIVATE)) {
	    debug(47, 3) ("COSS: %s: stripe %d: private key flag set, ignoring.\n", stripePath(SD), cs->rebuild.curstripe);
	    rb->counts.badflags++;
	    goto nextobject;
	}
	/* Time to consider the object! */
	tmpe.swap_filen = filen;
	tmpe.swap_dirn = SD->index;

	debug(47, 3) ("COSS: %s Considering filneumber %d\n", stripePath(SD), tmpe.swap_filen);
	storeCoss_ConsiderStoreEntry(rb, key, &tmpe);

      nextobject:
	/* Free the TLV data */
	storeSwapTLVFree(tlv_list);
	tlv_list = NULL;

	/* Now, advance to the next block-aligned offset after this object */
	j = j + len + bl;
	/* And now, the blocksize! */
	tmp = j / blocksize;
	tmp = (tmp + 1) * blocksize;
	j = tmp;
    }
}


static void
storeCoss_AddStoreEntry(RebuildState * rb, const cache_key * key, StoreEntry * e)
{
    StoreEntry *ne;
    SwapDir *SD = rb->sd;
    CossInfo *cs = SD->fsdata;
    rb->counts.objcount++;
    /* The Passed-in store entry is temporary; don't bloody use it directly! */
    assert(e->swap_dirn == SD->index);
    ne = new_StoreEntry(STORE_ENTRY_WITHOUT_MEMOBJ, NULL, NULL);
    ne->store_status = STORE_OK;
    storeSetMemStatus(ne, NOT_IN_MEMORY);
    ne->swap_status = SWAPOUT_DONE;
    ne->swap_filen = e->swap_filen;
    ne->swap_dirn = SD->index;
    ne->swap_file_sz = e->swap_file_sz;
    ne->lock_count = 0;
    ne->lastref = e->lastref;
    ne->timestamp = e->timestamp;
    ne->expires = e->expires;
    ne->lastmod = e->lastmod;
    ne->refcount = e->refcount;
    ne->flags = e->flags;
    EBIT_SET(ne->flags, ENTRY_CACHABLE);
    EBIT_CLR(ne->flags, RELEASE_REQUEST);
    EBIT_CLR(ne->flags, KEY_PRIVATE);
    ne->ping_status = PING_NONE;
    EBIT_CLR(ne->flags, ENTRY_VALIDATED);
    storeHashInsert(ne, key);	/* do it after we clear KEY_PRIVATE */
    storeCossAdd(SD, ne, cs->rebuild.curstripe);
    storeEntryDump(ne, 5);
    assert(ne->repl.data != NULL);
    assert(e->repl.data == NULL);
}

static void
storeCoss_DeleteStoreEntry(RebuildState * rb, const cache_key * key, StoreEntry * e)
{
    storeRecycle(e);
}

/*
 * Consider inserting the given StoreEntry into the given
 * COSS directory.
 *
 * The rules for doing this is reasonably simple:
 *
 * If the object doesn't exist in the cache then we simply
 * add it to the current stripe list
 *
 * If the object does exist in the cache then we compare
 * "freshness"; if the newer object is fresher then we
 * remove it from its stripe and re-add it to the current
 * stripe.
 */
static void
storeCoss_ConsiderStoreEntry(RebuildState * rb, const cache_key * key, StoreEntry * e)
{
    StoreEntry *oe;

    /* Check for clashes */
    oe = storeGet(key);
    if (oe == NULL) {
	rb->cosscounts.new++;
	debug(47, 3) ("COSS: Adding filen %d\n", e->swap_filen);
	/* no clash! woo, can add and forget */
	storeCoss_AddStoreEntry(rb, key, e);
	return;
    }
    /* This isn't valid - its possible we have a fresher object in another store */
    /* unlike the UFS-based stores we don't "delete" the disk object when we
     * have deleted the object; its one of the annoying things about COSS. */
    //assert(oe->swap_dirn == SD->index);
    /* Dang, its a clash. See if its fresher */

    /* Fresher? Its a new object: deallocate the old one, reallocate the new one */
    if (e->lastref > oe->lastref) {
	debug(47, 3) ("COSS: fresher object for filen %d found (%ld -> %ld)\n", oe->swap_filen, (long int) oe->timestamp, (long int) e->timestamp);
	rb->cosscounts.fresher++;
	storeCoss_DeleteStoreEntry(rb, key, oe);
	oe = NULL;
	storeCoss_AddStoreEntry(rb, key, e);
	return;
    }
    /*
     * Not fresher? Its the same object then we /should/ probably relocate it; I'm
     * not sure what should be done here.
     */
    if (oe->timestamp == e->timestamp && oe->expires == e->expires) {
	debug(47, 3) ("COSS: filen %d -> %d (since they're the same!)\n", oe->swap_filen, e->swap_filen);
	rb->cosscounts.reloc++;
	storeCoss_DeleteStoreEntry(rb, key, oe);
	oe = NULL;
	storeCoss_AddStoreEntry(rb, key, e);
	return;
    }
    debug(47, 3) ("COSS: filen %d: ignoring this one for some reason\n", e->swap_filen);
    rb->cosscounts.unknown++;
}
