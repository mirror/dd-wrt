/*
 * Copyright (c) 2017 Eric Leblond <eric@regit.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef LIB_NFTABLES_H
#define LIB_NFTABLES_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nft_ctx;

enum nft_debug_level {
	NFT_DEBUG_SCANNER		= 0x1,
	NFT_DEBUG_PARSER		= 0x2,
	NFT_DEBUG_EVALUATION		= 0x4,
	NFT_DEBUG_NETLINK		= 0x8,
	NFT_DEBUG_MNL			= 0x10,
	NFT_DEBUG_PROTO_CTX		= 0x20,
	NFT_DEBUG_SEGTREE		= 0x40,
};

/**
 * Possible flags to pass to nft_ctx_new()
 */
#define NFT_CTX_DEFAULT		0

struct nft_ctx *nft_ctx_new(uint32_t flags);
void nft_ctx_free(struct nft_ctx *ctx);

bool nft_ctx_get_dry_run(struct nft_ctx *ctx);
void nft_ctx_set_dry_run(struct nft_ctx *ctx, bool dry);

enum nft_optimize_flags {
	NFT_OPTIMIZE_ENABLED		= 0x1,
};

uint32_t nft_ctx_get_optimize(struct nft_ctx *ctx);
void nft_ctx_set_optimize(struct nft_ctx *ctx, uint32_t flags);

enum {
	NFT_CTX_INPUT_NO_DNS		= (1 << 0),
	NFT_CTX_INPUT_JSON		= (1 << 1),
};

unsigned int nft_ctx_input_get_flags(struct nft_ctx *ctx);
unsigned int nft_ctx_input_set_flags(struct nft_ctx *ctx, unsigned int flags);

enum {
	NFT_CTX_OUTPUT_REVERSEDNS	= (1 << 0),
	NFT_CTX_OUTPUT_SERVICE		= (1 << 1),
	NFT_CTX_OUTPUT_STATELESS	= (1 << 2),
	NFT_CTX_OUTPUT_HANDLE		= (1 << 3),
	NFT_CTX_OUTPUT_JSON		= (1 << 4),
	NFT_CTX_OUTPUT_ECHO		= (1 << 5),
	NFT_CTX_OUTPUT_GUID		= (1 << 6),
	NFT_CTX_OUTPUT_NUMERIC_PROTO	= (1 << 7),
	NFT_CTX_OUTPUT_NUMERIC_PRIO     = (1 << 8),
	NFT_CTX_OUTPUT_NUMERIC_SYMBOL	= (1 << 9),
	NFT_CTX_OUTPUT_NUMERIC_TIME	= (1 << 10),
	NFT_CTX_OUTPUT_NUMERIC_ALL	= (NFT_CTX_OUTPUT_NUMERIC_PROTO |
					   NFT_CTX_OUTPUT_NUMERIC_PRIO |
					   NFT_CTX_OUTPUT_NUMERIC_SYMBOL |
					   NFT_CTX_OUTPUT_NUMERIC_TIME),
	NFT_CTX_OUTPUT_TERSE		= (1 << 11),
};

unsigned int nft_ctx_output_get_flags(struct nft_ctx *ctx);
void nft_ctx_output_set_flags(struct nft_ctx *ctx, unsigned int flags);

unsigned int nft_ctx_output_get_debug(struct nft_ctx *ctx);
void nft_ctx_output_set_debug(struct nft_ctx *ctx, unsigned int mask);

FILE *nft_ctx_set_output(struct nft_ctx *ctx, FILE *fp);
int nft_ctx_buffer_output(struct nft_ctx *ctx);
int nft_ctx_unbuffer_output(struct nft_ctx *ctx);
const char *nft_ctx_get_output_buffer(struct nft_ctx *ctx);

FILE *nft_ctx_set_error(struct nft_ctx *ctx, FILE *fp);
int nft_ctx_buffer_error(struct nft_ctx *ctx);
int nft_ctx_unbuffer_error(struct nft_ctx *ctx);
const char *nft_ctx_get_error_buffer(struct nft_ctx *ctx);

int nft_ctx_add_include_path(struct nft_ctx *ctx, const char *path);
void nft_ctx_clear_include_paths(struct nft_ctx *ctx);

int nft_ctx_add_var(struct nft_ctx *ctx, const char *var);
void nft_ctx_clear_vars(struct nft_ctx *ctx);

int nft_run_cmd_from_buffer(struct nft_ctx *nft, const char *buf);
int nft_run_cmd_from_filename(struct nft_ctx *nft, const char *filename);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIB_NFTABLES_H */
