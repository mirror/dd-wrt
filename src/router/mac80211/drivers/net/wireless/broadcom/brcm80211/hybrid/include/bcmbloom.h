/*
 * Bloom filter support
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: bcmbloom.h 525587 2015-01-10 05:24:58Z $
 */

#ifndef _bcmbloom_h_
#define _bcmbloom_h_

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif

struct bcm_bloom_filter;
typedef struct bcm_bloom_filter bcm_bloom_filter_t;

typedef void* (*bcm_bloom_alloc_t)(void *ctx, uint size);
typedef void (*bcm_bloom_free_t)(void *ctx, void *buf, uint size);
typedef uint (*bcm_bloom_hash_t)(void* ctx, uint idx, const uint8 *tag, uint len);

/* create/allocate a bloom filter. filter size can be 0 for validate only filters */
int bcm_bloom_create(bcm_bloom_alloc_t alloc_cb,
	bcm_bloom_free_t free_cb, void *callback_ctx, uint max_hash,
	uint filter_size /* bytes */, bcm_bloom_filter_t **bloom);

/* destroy bloom filter */
int bcm_bloom_destroy(bcm_bloom_filter_t **bloom, bcm_bloom_free_t free_cb);

/* add a hash function to filter, return an index */
int bcm_bloom_add_hash(bcm_bloom_filter_t *filter, bcm_bloom_hash_t hash, uint *idx);

/* remove the hash function at index from filter */
int bcm_bloom_remove_hash(bcm_bloom_filter_t *filter, uint idx);

/* check if given tag is member of the filter. If buf is NULL and/or buf_len is 0
 * then use the internal state. BCME_OK if member, BCME_NOTFOUND if not,
 * or other error (e.g. BADARG)
 */
bool bcm_bloom_is_member(bcm_bloom_filter_t *filter,
	const uint8 *tag, uint tag_len, const uint8 *buf, uint buf_len);

/* add a member to the filter. invalid for validate_only filters */
int bcm_bloom_add_member(bcm_bloom_filter_t *filter, const uint8 *tag, uint tag_len);

/* no support for remove member */

/* get the filter data from state. BCME_BUFTOOSHORT w/ required length in buf_len
 * if supplied size is insufficient
 */
int bcm_bloom_get_filter_data(bcm_bloom_filter_t *filter,
	uint buf_size, uint8 *buf, uint *buf_len);

#endif /* _bcmbloom_h_ */
