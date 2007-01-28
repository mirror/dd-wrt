#include "squid.h"

static struct {
    struct {
	int calls;
	int select_fail;
	int create_fail;
	int success;
    } create;
    struct {
	int calls;
	int success;
	int open_fail;
	int loadav_fail;
    } open;
} store_io_stats;

OBJH storeIOStats;

/*
 * submit a request to create a cache object for writing.
 * The StoreEntry structure is sent as a hint to the filesystem
 * to what will be stored in this object, to allow the filesystem
 * to select different polices depending on object size or type.
 */
storeIOState *
storeCreate(StoreEntry * e, STIOCB * file_callback, STIOCB * close_callback, void *callback_data)
{
    squid_off_t objsize;
    sdirno dirn;
    SwapDir *SD;
    storeIOState *sio;

    store_io_stats.create.calls++;
    /* This is just done for logging purposes */
    objsize = objectLen(e);
    if (objsize != -1)
	objsize += e->mem_obj->swap_hdr_sz;

    /*
     * Pick the swapdir
     * We assume that the header has been packed by now ..
     */
    dirn = storeDirSelectSwapDir(e);
    if (dirn == -1) {
	debug(20, 2) ("storeCreate: no valid swapdirs for this object\n");
	store_io_stats.create.select_fail++;
	return NULL;
    }
    debug(20, 2) ("storeCreate: Selected dir '%d' for obj size '%" PRINTF_OFF_T "'\n", dirn, objsize);
    SD = &Config.cacheSwap.swapDirs[dirn];

    /* Now that we have a fs to use, call its storeCreate function */
    sio = SD->obj.create(SD, e, file_callback, close_callback, callback_data);
    if (NULL == sio)
	store_io_stats.create.create_fail++;
    else
	store_io_stats.create.success++;
    return sio;
}


/*
 * storeOpen() is purely for reading ..
 */
storeIOState *
storeOpen(StoreEntry * e, STFNCB * file_callback, STIOCB * callback,
    void *callback_data)
{
    int load;
    storeIOState *sio;

    SwapDir *SD = &Config.cacheSwap.swapDirs[e->swap_dirn];
    store_io_stats.open.calls++;
    load = SD->checkload(SD, ST_OP_OPEN);
    if (load < 0 || load > 1000) {
	store_io_stats.open.loadav_fail++;
	return NULL;
    }
    sio = SD->obj.open(SD, e, file_callback, callback, callback_data);
    if (sio == NULL) {
	store_io_stats.open.open_fail++;
    } else {
	store_io_stats.open.success++;
    }
    return sio;
}

void
storeClose(storeIOState * sio)
{
    SwapDir *SD = &Config.cacheSwap.swapDirs[sio->swap_dirn];
    if (sio->flags.closing)
	return;
    sio->flags.closing = 1;
    SD->obj.close(SD, sio);
}

void
storeRead(storeIOState * sio, char *buf, size_t size, squid_off_t offset, STRCB * callback, void *callback_data)
{
    SwapDir *SD = &Config.cacheSwap.swapDirs[sio->swap_dirn];
    (SD->obj.read) (SD, sio, buf, size, offset, callback, callback_data);
}

void
storeWrite(storeIOState * sio, char *buf, size_t size, FREE * free_func)
{
    SwapDir *SD = &Config.cacheSwap.swapDirs[sio->swap_dirn];
    squid_off_t offset = sio->write_offset;
    sio->write_offset += size;
    (SD->obj.write) (SD, sio, buf, size, offset, free_func);
}

void
storeUnlink(StoreEntry * e)
{
    SwapDir *SD = INDEXSD(e->swap_dirn);
    SD->obj.unlink(SD, e);
}

void
storeRecycle(StoreEntry * e)
{
    if (e->swap_dirn >= 0 && !storeEntryLocked(e)) {
	SwapDir *SD = INDEXSD(e->swap_dirn);

	/* Expire the object */
	storeExpireNow(e);
	storeReleaseRequest(e);

	/* Make the cache_dir forget about it */
	SD->obj.recycle(SD, e);
    }
    /* Finally make the store layer forget about this object */
    storeRelease(e);
}


squid_off_t
storeOffset(storeIOState * sio)
{
    return sio->offset;
}

/*
 * Make this non-static so we can register
 * it from storeInit();
 */
void
storeIOStats(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "Store IO Interface Stats\n");
    storeAppendPrintf(sentry, "create.calls %d\n", store_io_stats.create.calls);
    storeAppendPrintf(sentry, "create.select_fail %d\n", store_io_stats.create.select_fail);
    storeAppendPrintf(sentry, "create.create_fail %d\n", store_io_stats.create.create_fail);
    storeAppendPrintf(sentry, "create.success %d\n", store_io_stats.create.success);
    storeAppendPrintf(sentry, "open.calls %d\n", store_io_stats.open.calls);
    storeAppendPrintf(sentry, "open.success %d\n", store_io_stats.open.success);
    storeAppendPrintf(sentry, "open.loadav_fail %d\n", store_io_stats.open.loadav_fail);
    storeAppendPrintf(sentry, "open.open_fail %d\n", store_io_stats.open.open_fail);
}
