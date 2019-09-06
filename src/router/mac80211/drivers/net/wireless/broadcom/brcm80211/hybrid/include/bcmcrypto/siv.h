/*
 * siv.h
 *
 * Copyright (C) 2015, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: siv.h 453301 2014-02-04 19:49:09Z $
 */

#ifndef _SIV_H_
#define _SIV_H_

/* SIV is an authenticated encryption mechanism described in
 * http://www.cs.ucdavis.edu/~rogaway/papers/siv.pdf
 * also http://tools.ietf.org/search/rfc5297
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif


/* SIV works with block ciphers (e.g. AES) with block length of 128 bits, but
 * is independent of the cipher and can support different key lengths.
 */

#define SIV_BLOCK_SZ 16
#define SIV_BLOCK_NBITS (SIV_BLOCK_SZ << 3)

/* SIV block - byte 0 is MSB byte, bit 0 is LSB bit */
typedef uint8 siv_block_t[SIV_BLOCK_SZ];

typedef int (*siv_cmac_fn_t)(void *cmac_ctx, const uint8* data, size_t data_len,
	uint8* mic, const size_t mic_len);
typedef int (*siv_ctr_fn_t)(void *ctr_ctx, const uint8 *iv, uint8* data, size_t data_len);

enum siv_op_type {
	SIV_ENCRYPT = 1,
	SIV_DECRYPT
};

typedef enum siv_op_type siv_op_type_t;

struct siv_ctx {
	siv_op_type_t op_type; 	/* selected operation */
	siv_cmac_fn_t cmac_cb;
	void *cmac_cb_ctx;
	siv_ctr_fn_t ctr_cb;
	void *ctr_cb_ctx;

	int num_hdr;
	siv_block_t iv;
};

typedef struct siv_ctx siv_ctx_t;

/* API returns bcm error status unless specified */

/* Initialize siv context */
int siv_init(siv_ctx_t *siv_ctx, siv_op_type_t op_type,
	siv_cmac_fn_t cmac_fn, void *cmac_ctx,
	siv_ctr_fn_t ctr_fn, void *ctr_ctx);

/* Update siv state with header data */
int siv_update(siv_ctx_t *ctx, const uint8 *hdr, const size_t hdr_len);

/* Finalize siv context. data length is for  plain/cipher text; for encryption, iv is
 * returned, for decryption iv is the iv received
 */
int siv_final(siv_ctx_t *ctx, uint8 *iv, uint8 *data, const size_t data_len);

#endif /* _SIV_H_ */
