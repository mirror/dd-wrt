/*
 * aessiv.h
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: aessiv.h 453301 2014-02-04 19:49:09Z $
 */

#ifndef _AESSIV_H_
#define _AESSIV_H_

/* aessiv is an authenticated encryption mechanism described in
 * also http://tools.ietf.org/search/rfc5297
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif

#include <bcmcrypto/siv.h>
#include <bcmcrypto/aes.h>

struct aessiv_ctx {
	int	key_len;
	int	nrounds;
	siv_block_t iv_subk1;
	siv_block_t iv_subk2;
	uint32    iv_rkey[(AES_MAXROUNDS + 1) << 2];
	uint32    ctr_rkey[(AES_MAXROUNDS + 1) << 2];
	siv_ctx_t siv_ctx;
};

typedef struct aessiv_ctx aessiv_ctx_t;

/* API returns bcm error status unless specified */

/* Initialize aessiv context */
int aessiv_init(aessiv_ctx_t *aessiv_ctx, siv_op_type_t op_type,
	const size_t key_len, const uint8 *iv_key, const uint8 *ctr_key);

/* Update aessiv state with header data */
int aessiv_update(aessiv_ctx_t *ctx, const uint8 *hdr, const size_t hdr_len);

/* Finalize aessiv context. */
int aessiv_final(aessiv_ctx_t *ctx, uint8 *iv, uint8 *data, const size_t data_len);

#endif /* _AESSIV_H_ */
