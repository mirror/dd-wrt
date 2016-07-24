/*
 * Copyright (c) 2004-2006 Maxim Sobolev <sobomax@FreeBSD.org>
 * Copyright (c) 2006-2014 Sippy Software, Inc., http://www.sippysoft.com
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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "rtpp_types.h"
#include "rtpp_hash_table.h"

struct rtpp_hash_table_entry {
    struct rtpp_hash_table_entry *prev;
    struct rtpp_hash_table_entry *next;
    void *sptr;
    char *key;
    uint8_t hash;
};

struct rtpp_hash_table_priv
{
    uint8_t rand_table[256];
    struct rtpp_hash_table_entry *hash_table[256];
    pthread_mutex_t hash_table_lock;
};

struct rtpp_hash_table_full
{
    struct rtpp_hash_table_obj pub;
    struct rtpp_hash_table_priv pvt;
};

static struct rtpp_hash_table_entry * hash_table_append(struct rtpp_hash_table_obj *self, const char *key, void *sptr);
static void hash_table_remove(struct rtpp_hash_table_obj *self, const char *key, struct rtpp_hash_table_entry * sp);
static void hash_table_remove_nc(struct rtpp_hash_table_obj *self, struct rtpp_hash_table_entry * sp);
static struct rtpp_hash_table_entry * hash_table_findfirst(struct rtpp_hash_table_obj *self, const char *key, void **sptrp);
static struct rtpp_hash_table_entry * hash_table_findnext(struct rtpp_hash_table_obj *self, struct rtpp_hash_table_entry *psp, void **sptrp);
static void hash_table_dtor(struct rtpp_hash_table_obj *self);

struct rtpp_hash_table_obj *
rtpp_hash_table_ctor(void)
{
    struct rtpp_hash_table_full *rp;
    struct rtpp_hash_table_obj *pub;
    struct rtpp_hash_table_priv *pvt;
    int i;
    uint8_t rval;

    rp = malloc(sizeof(struct rtpp_hash_table_full));
    if (rp == NULL) {
        return (NULL);
    }
    memset(rp, '\0', sizeof(struct rtpp_hash_table_full));
    pvt = &(rp->pvt);
    pub = &(rp->pub);
    pub->append = &hash_table_append;
    pub->remove = &hash_table_remove;
    pub->remove_nc = &hash_table_remove_nc;
    pub->findfirst = &hash_table_findfirst;
    pub->findnext = &hash_table_findnext;
    pub->dtor = &hash_table_dtor;
    pthread_mutex_init(&pvt->hash_table_lock, NULL);
    for (i = 0; i < 256; i++) {
        if (i == 0)
            continue;
	do {
	    rval = random() & 0xff;
	} while (pvt->rand_table[rval] != 0);
	pvt->rand_table[rval] = i;
    }
    pub->pvt = pvt;
    return (pub);
}

static void
hash_table_dtor(struct rtpp_hash_table_obj *self)
{
    struct rtpp_hash_table_entry *sp, *sp_next;
    struct rtpp_hash_table_priv *pvt;
    int i;

    pvt = self->pvt;
    for (i = 0; i < 256; i++) {
        sp = pvt->hash_table[i];
        if (sp == NULL)
            continue;
        do {
            sp_next = sp->next;
            free(sp);
            sp = sp_next;
        } while (sp != NULL);
    }
    pthread_mutex_destroy(&pvt->hash_table_lock);

    free(self);
}

static uint8_t
hash_string(struct rtpp_hash_table_priv *pvt, const char *bp, const char *ep)
{
    uint8_t res;

    for (res = pvt->rand_table[0]; bp[0] != '\0' && bp != ep; bp++) {
	res = pvt->rand_table[res ^ bp[0]];
    }
    return res;
}

static struct rtpp_hash_table_entry *
hash_table_append(struct rtpp_hash_table_obj *self, const char *key, void *sptr)
{
    int malen, klen;
    struct rtpp_hash_table_entry *sp, *tsp;
    struct rtpp_hash_table_priv *pvt;

    klen = strlen(key);
    malen = sizeof(struct rtpp_hash_table_entry) + klen + 1;
    sp = malloc(malen);
    if (sp == NULL) {
        return (NULL);
    }
    memset(sp, '\0', malen);
    sp->sptr = sptr;
    sp->key = ((char *)sp) + sizeof(struct rtpp_hash_table_entry);
    memcpy(sp->key, key, klen);
    pvt = self->pvt;
    sp->hash = hash_string(pvt, key, NULL);

    pthread_mutex_lock(&pvt->hash_table_lock);
    tsp = pvt->hash_table[sp->hash];
    if (tsp == NULL) {
       	pvt->hash_table[sp->hash] = sp;
        goto out;
    }
    while (tsp->next != NULL) {
	tsp = tsp->next;
    }
    tsp->next = sp;
    sp->prev = tsp;
out:
    pthread_mutex_unlock(&pvt->hash_table_lock);
    return (sp);
}

static void
hash_table_remove(struct rtpp_hash_table_obj *self, const char *key,
  struct rtpp_hash_table_entry * sp)
{
    uint8_t hash;
    struct rtpp_hash_table_priv *pvt;

    pvt = self->pvt;
    pthread_mutex_lock(&pvt->hash_table_lock);
    if (sp->prev != NULL) {
	sp->prev->next = sp->next;
	if (sp->next != NULL) {
	    sp->next->prev = sp->prev;
	}
        goto out;
    }
    hash = hash_string(pvt, key, NULL);
    /* Make sure we are removing the right session */
    assert(pvt->hash_table[hash] == sp);
    pvt->hash_table[hash] = sp->next;
    if (sp->next != NULL) {
	sp->next->prev = NULL;
    }
out:
    pthread_mutex_unlock(&pvt->hash_table_lock);
    free(sp);
}

static void
hash_table_remove_nc(struct rtpp_hash_table_obj *self, struct rtpp_hash_table_entry * sp)
{
    struct rtpp_hash_table_priv *pvt;

    pvt = self->pvt;
    pthread_mutex_lock(&pvt->hash_table_lock);
    if (sp->prev != NULL) {
        sp->prev->next = sp->next;
        if (sp->next != NULL) {
            sp->next->prev = sp->prev;
        }
        goto out;
    }
    /* Make sure we are removing the right entry */
    assert(pvt->hash_table[sp->hash] == sp);
    pvt->hash_table[sp->hash] = sp->next;
    if (sp->next != NULL) {
        sp->next->prev = NULL;
    }
out:
    pthread_mutex_unlock(&pvt->hash_table_lock);
    free(sp);
}

static struct rtpp_hash_table_entry *
hash_table_findfirst(struct rtpp_hash_table_obj *self, const char *key, void **sptrp)
{
    uint8_t hash;
    struct rtpp_hash_table_entry *sp;
    struct rtpp_hash_table_priv *pvt;

    pvt = self->pvt;
    hash = hash_string(pvt, key, NULL);
    pthread_mutex_lock(&pvt->hash_table_lock);
    for (sp = pvt->hash_table[hash]; sp != NULL; sp = sp->next) {
	if (strcmp(sp->key, key) == 0) {
            *sptrp = sp->sptr;
	    break;
	}
    }
    pthread_mutex_unlock(&pvt->hash_table_lock);
    return (sp);
}

static struct rtpp_hash_table_entry *
hash_table_findnext(struct rtpp_hash_table_obj *self, struct rtpp_hash_table_entry *psp, void **sptrp)
{
    struct rtpp_hash_table_entry *sp;
    struct rtpp_hash_table_priv *pvt;

    pvt = self->pvt;
    pthread_mutex_lock(&pvt->hash_table_lock);
    for (sp = psp->next; sp != NULL; sp = sp->next) {
	if (strcmp(sp->key, psp->key) == 0) {
            *sptrp = sp->sptr;
	    break;
	}
    }
    pthread_mutex_unlock(&pvt->hash_table_lock);
    return (sp);
}
