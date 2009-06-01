
/*
 * $Id: MemPool.c,v 1.39 2006/09/18 22:54:39 hno Exp $
 *
 * DEBUG: section 63    Low Level Memory Pool Management
 * AUTHOR: Alex Rousskov
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
#include "Stack.h"

#define MB ((size_t)1024*1024)

/* exported */
unsigned int mem_pool_alloc_calls = 0;
unsigned int mem_pool_free_calls = 0;

/* module globals */

/* huge constant to set mem_idle_limit to "unlimited" */
static const size_t mem_unlimited_size = 2 * 1024 * MB - 1;

/* we cannot keep idle more than this limit */
static size_t mem_idle_limit = 0;

/* memory pool accounting */
static MemPoolMeter TheMeter;
static gb_t mem_traffic_volume =
{0, 0};
static Stack Pools;

/* local prototypes */
static void memShrink(size_t new_limit);
static void memPoolDescribe(const MemPool * pool);
static void memPoolShrink(MemPool * pool, size_t new_limit);


static double
toMB(size_t size)
{
    return ((double) size) / MB;
}

static size_t
toKB(size_t size)
{
    return (size + 1024 - 1) / 1024;
}


/* Initialization */

void
memConfigure(void)
{
    size_t new_pool_limit = mem_idle_limit;
    /* set to configured value first */
#if LEAK_CHECK_MODE
#if PURIFY
    if (1) {
#else
    if (RUNNING_ON_VALGRIND) {
#endif
	debug(63, 1) ("Disabling Memory pools for accurate leak checks\n");
	Config.onoff.mem_pools = 0;
    }
#endif
    if (!Config.onoff.mem_pools)
	new_pool_limit = 0;
    else if (Config.MemPools.limit > 0)
	new_pool_limit = Config.MemPools.limit;
    else
	new_pool_limit = mem_unlimited_size;
    /* shrink memory pools if needed */
    if (TheMeter.idle.level > new_pool_limit) {
	debug(63, 1) ("Shrinking idle mem pools to %.2f MB\n", toMB(new_pool_limit));
	memShrink(new_pool_limit);
    }
    assert(TheMeter.idle.level <= new_pool_limit);
    mem_idle_limit = new_pool_limit;
}

void
memInitModule(void)
{
    memset(&TheMeter, 0, sizeof(TheMeter));
    stackInit(&Pools);
    debug(63, 1) ("Memory pools are '%s'; limit: %.2f MB\n",
	(Config.onoff.mem_pools ? "on" : "off"), toMB(mem_idle_limit));
}

void
memCleanModule(void)
{
    int i;
    int dirty_count = 0;
    for (i = 0; i < Pools.count; i++) {
	MemPool *pool = Pools.items[i];
	if (!pool)
	    continue;
	if (memPoolInUseCount(pool)) {
	    memPoolDescribe(pool);
	    dirty_count++;
	} else {
	    memPoolDestroy(pool);
	}
    }
    if (dirty_count)
	debug(63, 2) ("memCleanModule: %d pools are left dirty\n", dirty_count);
    /* we clean the stack anyway */
    stackClean(&Pools);
}


static void
memShrink(size_t new_limit)
{
    size_t start_limit = TheMeter.idle.level;
    int i;
    debug(63, 1) ("memShrink: started with %ld KB goal: %ld KB\n",
	(long int) toKB(TheMeter.idle.level), (long int) toKB(new_limit));
    /* first phase: cut proportionally to the pool idle size */
    for (i = 0; i < Pools.count && TheMeter.idle.level > new_limit; ++i) {
	MemPool *pool = Pools.items[i];
	const size_t target_pool_size = (size_t) ((double) pool->meter.idle.level * new_limit) / start_limit;
	memPoolShrink(pool, target_pool_size);
    }
    debug(63, 1) ("memShrink: 1st phase done with %ld KB left\n", (long int) toKB(TheMeter.idle.level));
    /* second phase: cut to 0 */
    for (i = 0; i < Pools.count && TheMeter.idle.level > new_limit; ++i)
	memPoolShrink(Pools.items[i], 0);
    debug(63, 1) ("memShrink: 2nd phase done with %ld KB left\n", (long int) toKB(TheMeter.idle.level));
    assert(TheMeter.idle.level <= new_limit);	/* paranoid */
}

/* MemPoolMeter */

static void
memPoolMeterReport(const MemPoolMeter * pm, size_t obj_size,
    int alloc_count, int inuse_count, int idle_count, StoreEntry * e)
{
    assert(pm);
    storeAppendPrintf(e, "%d\t %ld\t %ld\t %.2f\t %d\t %d\t %ld\t %ld\t %d\t %d\t %ld\t %ld\t %ld\t %.2f\t %.2f\t %.2f\t %ld\n",
    /* alloc */
	alloc_count,
	(long int) toKB(obj_size * pm->alloc.level),
	(long int) toKB(obj_size * pm->alloc.hwater_level),
	(double) ((squid_curtime - pm->alloc.hwater_stamp) / 3600.),
	xpercentInt(obj_size * pm->alloc.level, TheMeter.alloc.level),
    /* in use */
	inuse_count,
	(long int) toKB(obj_size * pm->inuse.level),
	(long int) toKB(obj_size * pm->inuse.hwater_level),
	xpercentInt(pm->inuse.level, pm->alloc.level),
    /* idle */
	idle_count,
	(long int) toKB(obj_size * pm->idle.level),
	(long int) toKB(obj_size * pm->idle.hwater_level),
    /* (int)rint(xpercent(pm->idle.level, pm->alloc.level)), */
    /* saved */
	(long int) pm->saved.count,
	xpercent(pm->saved.count, mem_traffic_volume.count),
	xpercent(obj_size * gb_to_double(&pm->saved), gb_to_double(&mem_traffic_volume)),
	xpercent(pm->saved.count, pm->total.count),
	(long int) pm->total.count);
}

/* MemMeter */

void
memMeterSyncHWater(MemMeter * m)
{
    assert(m);
    if (m->hwater_level < m->level) {
	m->hwater_level = m->level;
	m->hwater_stamp = squid_curtime;
    }
}

/* MemPool */

MemPool *
memPoolCreate(const char *label, size_t obj_size)
{
    MemPool *pool = xcalloc(1, sizeof(MemPool));
    assert(label && obj_size);
    pool->label = label;
    pool->obj_size = obj_size;
#if DEBUG_MEMPOOL
    pool->real_obj_size = (obj_size & 7) ? (obj_size | 7) + 1 : obj_size;
#endif
    stackInit(&pool->pstack);
    /* other members are set to 0 */
    stackPush(&Pools, pool);
    return pool;
}

void
memPoolDestroy(MemPool * pool)
{
    int i;
    assert(pool);
    for (i = 0; i < Pools.count; i++) {
	if (Pools.items[i] == pool) {
	    Pools.items[i] = NULL;
	    break;
	}
    }
    stackClean(&pool->pstack);
    xfree(pool);
}

#if DEBUG_MEMPOOL
#define MEMPOOL_COOKIE(p) ((void *)((unsigned long)(p) ^ 0xDEADBEEF))
struct mempool_cookie {
    MemPool *pool;
    void *cookie;
};

#endif

void *
memPoolAlloc(MemPool * pool)
{
    void *obj;
    assert(pool);
    memMeterInc(pool->meter.inuse);
    gb_inc(&pool->meter.total, 1);
    gb_inc(&TheMeter.total, pool->obj_size);
    memMeterAdd(TheMeter.inuse, pool->obj_size);
    gb_inc(&mem_traffic_volume, pool->obj_size);
    mem_pool_alloc_calls++;
    if (pool->pstack.count) {
	assert(pool->meter.idle.level);
	memMeterDec(pool->meter.idle);
	memMeterDel(TheMeter.idle, pool->obj_size);
	gb_inc(&pool->meter.saved, 1);
	gb_inc(&TheMeter.saved, pool->obj_size);
	obj = stackPop(&pool->pstack);
#if DEBUG_MEMPOOL
	(void) VALGRIND_MAKE_READABLE(obj, pool->real_obj_size + sizeof(struct mempool_cookie));
#else
	(void) VALGRIND_MAKE_READABLE(obj, pool->obj_size);
#endif
#if DEBUG_MEMPOOL
	{
	    struct mempool_cookie *cookie = (void *) (((unsigned char *) obj) + pool->real_obj_size);
	    assert(cookie->cookie == MEMPOOL_COOKIE(obj));
	    assert(cookie->pool == pool);
	    (void) VALGRIND_MAKE_NOACCESS(cookie, sizeof(cookie));
	}
#endif
    } else {
	assert(!pool->meter.idle.level);
	memMeterInc(pool->meter.alloc);
	memMeterAdd(TheMeter.alloc, pool->obj_size);
#if DEBUG_MEMPOOL
	{
	    struct mempool_cookie *cookie;
	    obj = xcalloc(1, pool->real_obj_size + sizeof(struct mempool_cookie));
	    cookie = (struct mempool_cookie *) (((unsigned char *) obj) + pool->real_obj_size);
	    cookie->cookie = MEMPOOL_COOKIE(obj);
	    cookie->pool = pool;
	    (void) VALGRIND_MAKE_NOACCESS(cookie, sizeof(cookie));
	}
#else
	obj = xcalloc(1, pool->obj_size);
#endif
    }
    return obj;
}

void
memPoolFree(MemPool * pool, void *obj)
{
    assert(pool && obj);
    memMeterDec(pool->meter.inuse);
    memMeterDel(TheMeter.inuse, pool->obj_size);
    mem_pool_free_calls++;
    (void) VALGRIND_CHECK_WRITABLE(obj, pool->obj_size);
#if DEBUG_MEMPOOL
    {
	struct mempool_cookie *cookie = (void *) (((unsigned char *) obj) + pool->real_obj_size);
	(void) VALGRIND_MAKE_READABLE(cookie, sizeof(cookie));
	assert(cookie->cookie == MEMPOOL_COOKIE(obj));
	assert(cookie->pool == pool);
    }
#endif
    if (TheMeter.idle.level + pool->obj_size <= mem_idle_limit) {
	memMeterInc(pool->meter.idle);
	memMeterAdd(TheMeter.idle, pool->obj_size);
	memset(obj, 0, pool->obj_size);
#if DEBUG_MEMPOOL
	(void) VALGRIND_MAKE_NOACCESS(obj, pool->real_obj_size + sizeof(struct mempool_cookie));
#else
	(void) VALGRIND_MAKE_NOACCESS(obj, pool->obj_size);
#endif
	stackPush(&pool->pstack, obj);
    } else {
	memMeterDec(pool->meter.alloc);
	memMeterDel(TheMeter.alloc, pool->obj_size);
	xfree(obj);
    }
    assert(pool->meter.idle.level <= pool->meter.alloc.level);
}

static void
memPoolShrink(MemPool * pool, size_t new_limit)
{
    assert(pool);
    while (pool->meter.idle.level > new_limit && pool->pstack.count > 0) {
	memMeterDec(pool->meter.alloc);
	memMeterDec(pool->meter.idle);
	memMeterDel(TheMeter.idle, pool->obj_size);
	memMeterDel(TheMeter.alloc, pool->obj_size);
	xfree(stackPop(&pool->pstack));
    }
    assert(pool->meter.idle.level <= new_limit);	/* paranoid */
}

int
memPoolWasUsed(const MemPool * pool)
{
    assert(pool);
    return pool->meter.alloc.hwater_level > 0;
}

int
memPoolInUseCount(const MemPool * pool)
{
    assert(pool);
    return pool->meter.inuse.level;
}

size_t
memPoolInUseSize(const MemPool * pool)
{
    assert(pool);
    return (pool->obj_size * pool->meter.inuse.level);
}

/* to-do: make debug level a parameter? */
static void
memPoolDescribe(const MemPool * pool)
{
    assert(pool);
    debug(63, 2) ("%-20s: %6d x %4d bytes = %5ld KB\n",
	pool->label, memPoolInUseCount(pool), (int) pool->obj_size,
	(long int) toKB(memPoolInUseSize(pool)));
}

size_t
memTotalAllocated(void)
{
    return TheMeter.alloc.level;
}

#if DEBUG_MEMPOOL
static void
memPoolDiffReport(const MemPool * pool, StoreEntry * e)
{
    assert(pool);
    MemPoolMeter diff = pool->meter;
    diff.alloc.level -= pool->diff_meter.alloc.level;
    diff.inuse.level -= pool->diff_meter.inuse.level;
    diff.idle.level -= pool->diff_meter.idle.level;
    if (diff.alloc.level == 0 && diff.inuse.level == 0)
	return;
    storeAppendPrintf(e, " \t \t ");
    memPoolMeterReport(&diff, pool->obj_size,
	diff.alloc.level, pool->meter.inuse.level, pool->meter.idle.level,
	e);
}
#endif

static void
memPoolReport(MemPool * pool, StoreEntry * e, int diff)
{
    assert(pool);
    storeAppendPrintf(e, "%-20s\t %4d\t ",
	pool->label, (int) pool->obj_size);
    memPoolMeterReport(&pool->meter, pool->obj_size,
	pool->meter.alloc.level, pool->meter.inuse.level, pool->meter.idle.level,
	e);
#if DEBUG_MEMPOOL
    if (diff)
	memPoolDiffReport(pool, e);
    if (diff < 0)
	pool->diff_meter = pool->meter;
#endif
}

void
memReport(StoreEntry * e)
{
    size_t overhd_size = 0;
    int alloc_count = 0;
    int inuse_count = 0;
    int idle_count = 0;
    int i;
    int diff = 0;
#if DEBUG_MEMPOOL
    char *arg = strrchr(e->mem_obj->url, '/');
    if (arg) {
	arg++;
	if (strcmp(arg, "reset") == 0)
	    diff = -1;
	else if (strcmp(arg, "diff") == 0)
	    diff = 1;
    }
    storeAppendPrintf(e, "action:mem/diff\tView diff\n");
    storeAppendPrintf(e, "action:mem/reset\tReset diff\n");
#endif
    /* caption */
    storeAppendPrintf(e, "Current memory usage:\n");
    /* heading */
    storeAppendPrintf(e, "Pool\t Obj Size\t"
	"Allocated\t\t\t\t\t In Use\t\t\t\t Idle\t\t\t Allocations Saved\t\t\t Hit Rate\t\n"
	" \t (bytes)\t"
	"(#)\t (KB)\t high (KB)\t high (hrs)\t impact (%%total)\t"
	"(#)\t (KB)\t high (KB)\t portion (%%alloc)\t"
	"(#)\t (KB)\t high (KB)\t"
	"(number)\t (%%num)\t (%%vol)\t"
	"(%%num)\t"
	"(number)"
	"\n");
    /* main table */
    for (i = 0; i < Pools.count; i++) {
	MemPool *pool = Pools.items[i];
	if (memPoolWasUsed(pool)) {
	    memPoolReport(pool, e, diff);
	    alloc_count += pool->meter.alloc.level;
	    inuse_count += pool->meter.inuse.level;
	    idle_count += pool->meter.idle.level;
	}
	overhd_size += sizeof(MemPool) + sizeof(MemPool *) +
	    strlen(pool->label) + 1 +
	    pool->pstack.capacity * sizeof(void *);
    }
    overhd_size += sizeof(Pools) + Pools.capacity * sizeof(MemPool *);
    /* totals */
    storeAppendPrintf(e, "%-20s\t %-4s\t ", "Total", "-");
    memPoolMeterReport(&TheMeter, 1, alloc_count, inuse_count, idle_count, e);
    storeAppendPrintf(e, "Cumulative allocated volume: %s\n", gb_to_str(&mem_traffic_volume));
    /* overhead */
    storeAppendPrintf(e, "Current overhead: %ld bytes (%.3f%%)\n",
	(long int) overhd_size, xpercent(overhd_size, TheMeter.inuse.level));
    /* limits */
    storeAppendPrintf(e, "Idle pool limit: %.2f MB\n", toMB(mem_idle_limit));
    storeAppendPrintf(e, "memPoolAlloc calls: %d\n", mem_pool_alloc_calls);
    storeAppendPrintf(e, "memPoolFree calls: %d\n", mem_pool_free_calls);
}
