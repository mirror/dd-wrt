/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 * Simple memory debug layer to track any unallocated memory as well as to
 * catch any other common mistakes, such as double free or freeing of
 * unallocated memory. Our attitude here is "fail with core dump early" if
 * some error of inconsistency is found to aid debugging. Some extra smarts
 * can be added, such as guard area to detect any buffer overflows.
 */

#include <sys/types.h>
#include <pthread.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rtpp_log.h"
#include "rtpp_cfg_stable.h"
#include "rtpp_defines.h"
#include "rtpp_memdeb.h"
#include "rtpp_memdeb_internal.h"
#include "rtpp_memdeb_stats.h"

#undef malloc
#undef free
#undef realloc
#undef strdup
#undef asprintf
#undef vasprintf

#define UNUSED(x) (void)(x)

#define MEMDEB_SIGNATURE 0x8b26e00041dfdec6UL

struct memdeb_node
{
    uint64_t magic;
    const char *fname;
    int linen;
    const char *funcn;
    struct memdeb_stats mstats;
    struct memdeb_node *next;
};

struct memdeb_pfx
{
    struct memdeb_node *mnp;
    size_t asize;
    char real_data[0];
};

static struct {
    const char *funcn;
    int max_nunalloc;
    const char *why;
} approved_unallocated[] = {
    {.funcn = "addr2bindaddr", .max_nunalloc = 100, .why = "Too busy to fix now"},
    {.funcn = NULL}
};

static struct memdeb_node *nodes;
static pthread_mutex_t *memdeb_mutex;

static struct memdeb_node *
rtpp_memdeb_nget(const char *fname, int linen, const char *funcn, int doalloc)
{
    struct memdeb_node *rval, *mnp, *lastnode;

    if (memdeb_mutex == NULL) {
        memdeb_mutex = malloc(sizeof(pthread_mutex_t));
        if (memdeb_mutex == NULL)
            abort();
        pthread_mutex_init(memdeb_mutex, NULL);
    }
    pthread_mutex_lock(memdeb_mutex);
    lastnode = NULL;
    for (mnp = nodes; mnp != NULL; mnp = mnp->next) {
        if (mnp->magic != MEMDEB_SIGNATURE) {
            /* nodelist is corrupt */
            abort();
        }
        if (mnp->fname == fname && mnp->linen == linen && mnp->funcn == funcn)
            return (mnp);
        lastnode = mnp;
    }
    if (doalloc == 0) {
        pthread_mutex_unlock(memdeb_mutex);
        return (NULL);
    }
    rval = malloc(sizeof(struct memdeb_node));
    if (rval == NULL) {
        abort();
    }
    memset(rval, '\0', sizeof(struct memdeb_node));
    rval->magic = MEMDEB_SIGNATURE;
    rval->fname = fname;
    rval->linen = linen;
    rval->funcn = funcn;
    if (nodes == NULL) {
        nodes = rval;
    } else {
        lastnode->next = rval;
    }
    return (rval);
}

void *
rtpp_memdeb_malloc(size_t size, const char *fname, int linen, const char *funcn)
{
    struct memdeb_node *mnp;
    struct memdeb_pfx *mpf;

    mpf = malloc(offsetof(struct memdeb_pfx, real_data) + size);
    mnp = rtpp_memdeb_nget(fname, linen, funcn, 1);
    if (mpf == NULL) {
        mnp->mstats.afails++;
        pthread_mutex_unlock(memdeb_mutex);
        return (NULL);
    }
    mnp->mstats.nalloc++;
    mnp->mstats.balloc += size;
    pthread_mutex_unlock(memdeb_mutex);
    mpf->asize = size;
    mpf->mnp = mnp;
    return (mpf->real_data);
}

static struct memdeb_pfx *
ptr2mpf(void *ptr)
{
    char *cp;
    struct memdeb_pfx *mpf;

    cp = ptr;
    cp -= offsetof(struct memdeb_pfx, real_data);
    mpf = (struct memdeb_pfx *)cp;
    if (mpf->mnp->magic != MEMDEB_SIGNATURE) {
        /* Free of unallicated pointer or nodelist is corrupt */
        abort();
    }
    return (mpf);
}

void
rtpp_memdeb_free(void *ptr, const char *fname, int linen, const char *funcn)
{
    UNUSED(fname);
    UNUSED(linen);
    UNUSED(funcn);
    struct memdeb_pfx *mpf;

    mpf = ptr2mpf(ptr);
    pthread_mutex_lock(memdeb_mutex);
    mpf->mnp->mstats.nfree++;
    mpf->mnp->mstats.bfree += mpf->asize;
    pthread_mutex_unlock(memdeb_mutex);
    return free(mpf);
}

void *
rtpp_memdeb_realloc(void *ptr, size_t size,  const char *fname, int linen, const char *funcn)
{
    UNUSED(fname);
    UNUSED(linen);
    UNUSED(funcn);
    struct memdeb_pfx *mpf;
    char *cp;

    mpf = ptr2mpf(ptr);
    cp = realloc(mpf, size + offsetof(struct memdeb_pfx, real_data));
    if (cp == NULL) {
        pthread_mutex_lock(memdeb_mutex);
        mpf->mnp->mstats.afails++;
        pthread_mutex_unlock(memdeb_mutex);
        return (cp);
    }
    mpf = (struct memdeb_pfx *)cp;
    pthread_mutex_lock(memdeb_mutex);
    mpf->mnp->mstats.nrealloc++;
    mpf->mnp->mstats.brealloc += size;
    mpf->mnp->mstats.balloc += size - mpf->asize;
    pthread_mutex_unlock(memdeb_mutex);
    mpf->asize = size;
    return (mpf->real_data);
}

char *
rtpp_memdeb_strdup(const char *ptr, const char *fname, int linen, const char *funcn)
{
    struct memdeb_node *mnp;
    struct memdeb_pfx *mpf;
    size_t size;

    size = strlen(ptr) + 1;
    mpf = malloc(size + offsetof(struct memdeb_pfx, real_data));
    mnp = rtpp_memdeb_nget(fname, linen, funcn, 1);
    if (mpf == NULL) {
        mnp->mstats.afails++;
        pthread_mutex_unlock(memdeb_mutex);
        return (NULL);
    }
    mnp->mstats.nalloc++;
    mnp->mstats.balloc += size;
    pthread_mutex_unlock(memdeb_mutex);
    mpf->mnp = mnp;
    mpf->asize = size;
    memcpy(mpf->real_data, ptr, size);
    return (mpf->real_data);
}
#if 0

int
rtpp_memdeb_asprintf(char **pp, const char *fmt, const char *fname,
  int linen, const char *funcn, ...)
{
    va_list ap;
    int rval;

    va_start(ap, funcn);
    rval = rtpp_memdeb_vasprintf(pp, fmt, fname, linen, funcn, ap);
    va_end(ap);
    return (rval);
}

int
rtpp_memdeb_vasprintf(char **pp, const char *fmt, const char *fname,
  int linen, const char *funcn, va_list ap)
{
    int rval;
    void *tp;

    rval = vasprintf(pp, fmt, ap);
    if (rval <= 0) {
        return (rval);
    }
    tp = rtpp_memdeb_malloc(rval + 1, fname, linen, funcn);
    if (tp == NULL) {
        free(*pp);
        *pp = NULL;
        return (-1);
    }
    memcpy(tp, *pp, rval + 1);
    free(*pp);
    *pp = tp;
    return (rval);
}
#endif

static int
is_approved(const char *funcn)
{
    int i;

    for (i = 0; approved_unallocated[i].funcn != NULL; i++) {
        if (strcmp(approved_unallocated[i].funcn, funcn) != 0)
            continue;
        return (approved_unallocated[i].max_nunalloc);
    }
    return (0);
}

int
rtpp_memdeb_dumpstats(struct cfg *cf)
{
    struct memdeb_node *mnp;
    int errors_found, max_nunalloc;
    int64_t nunalloc;
    rtpp_log_t glog;

    errors_found = 0;
    if (cf != NULL) {
        glog = cf->stable->glog;
    } else {
        memset(&glog, '\0', sizeof(glog));
    }
    pthread_mutex_lock(memdeb_mutex);
    for (mnp = nodes; mnp != NULL; mnp = mnp->next) {
        nunalloc = mnp->mstats.nalloc - mnp->mstats.nfree;
        if (mnp->mstats.afails == 0) {
            if (mnp->mstats.nalloc == 0)
                continue;
            if (mnp->mstats.nalloc == mnp->mstats.nfree)
                continue;
            if (nunalloc <= mnp->mstats.nunalloc_baseln)
                continue;
        }
        if (nunalloc > 0) {
            max_nunalloc = is_approved(mnp->funcn);
            if (max_nunalloc > 0 && nunalloc <= max_nunalloc)
                continue;
        }
        if (errors_found == 0) {
            RTPP_MEMDEB_REPORT(glog,
              "MEMDEB suspicious allocations:");
        }
        errors_found++;
        RTPP_MEMDEB_REPORT(glog,
          "  %s+%d, %s(): nalloc = %ld, balloc = %ld, nfree = %ld, bfree = %ld,"
          " afails = %ld, nunalloc_baseln = %ld", mnp->fname, mnp->linen, 
          mnp->funcn, mnp->mstats.nalloc,
          mnp->mstats.balloc, mnp->mstats.nfree, mnp->mstats.bfree,
          mnp->mstats.afails, mnp->mstats.nunalloc_baseln);
    }
    pthread_mutex_unlock(memdeb_mutex);
    if (errors_found == 0) {
        RTPP_MEMDEB_REPORT(glog,
          "MEMDEB: all clear");
    } else {
        RTPP_MEMDEB_REPORT(glog,
          "MEMDEB: errors found: %d", errors_found);
    }
    return (errors_found);
}

void
rtpp_memdeb_setbaseln(void)
{

    struct memdeb_node *mnp;

    pthread_mutex_lock(memdeb_mutex);
    for (mnp = nodes; mnp != NULL; mnp = mnp->next) {
        if (mnp->magic != MEMDEB_SIGNATURE) {
            /* Nodelist is corrupt */
            abort();
        }
        if (mnp->mstats.nalloc == 0)
            continue;
        mnp->mstats.nunalloc_baseln = mnp->mstats.nalloc - mnp->mstats.nfree;
        mnp->mstats.bunalloc_baseln = mnp->mstats.balloc - mnp->mstats.bfree;
    }
    pthread_mutex_unlock(memdeb_mutex);
}

int
rtpp_memdeb_get_stats(const char *fname, const char *funcn,
  struct memdeb_stats *mstatp)
{
    struct memdeb_node *mnp;
    int nmatches;

    nmatches = 0;
    pthread_mutex_lock(memdeb_mutex);
    for (mnp = nodes; mnp != NULL; mnp = mnp->next) {
        if (mnp->magic != MEMDEB_SIGNATURE) {
            /* Nodelist is corrupt */
            abort();
        }
        if (funcn != NULL && strcmp(funcn, mnp->funcn) != 0) {
            continue;
        }
        if (fname != NULL && strcmp(fname, mnp->fname) != 0) {
            continue;
        }
        RTPP_MD_STATS_ADD(mstatp, &mnp->mstats);
        nmatches += 1;
    }
    pthread_mutex_unlock(memdeb_mutex);
    return (nmatches);
}
