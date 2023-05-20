/* SPDX-License-Identifier: GPL-2.0 OR MIT */
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 *
 * modified from Jason A. Donenfeld's wireguard, simd is not included
 */
#include "chacha8.h"


enum chacha20_constants { /* expand 32-byte k */
	CHACHA20_CONSTANT_EXPA = 0x61707865U,
	CHACHA20_CONSTANT_ND_3 = 0x3320646eU,
	CHACHA20_CONSTANT_2_BY = 0x79622d32U,
	CHACHA20_CONSTANT_TE_K = 0x6b206574U
};

struct chacha20_ctx {
	union {
		u32 state[16];
		struct {
			u32 constant[4];
			u32 key[8];
			u32 counter[4];
		};
	};
};

static inline void chacha20_init(struct chacha20_ctx *ctx,
                                 const u8 key[CHACHA20_KEY_SIZE],
                                 const u64 nonce)
{
	ctx->constant[0] = CHACHA20_CONSTANT_EXPA;
	ctx->constant[1] = CHACHA20_CONSTANT_ND_3;
	ctx->constant[2] = CHACHA20_CONSTANT_2_BY;
	ctx->constant[3] = CHACHA20_CONSTANT_TE_K;
	ctx->key[0] = get_unaligned_le32(key + 0);
	ctx->key[1] = get_unaligned_le32(key + 4);
	ctx->key[2] = get_unaligned_le32(key + 8);
	ctx->key[3] = get_unaligned_le32(key + 12);
	ctx->key[4] = get_unaligned_le32(key + 16);
	ctx->key[5] = get_unaligned_le32(key + 20);
	ctx->key[6] = get_unaligned_le32(key + 24);
	ctx->key[7] = get_unaligned_le32(key + 28);
	ctx->counter[0] = 0;
	ctx->counter[1] = 0;
	ctx->counter[2] = nonce & U32_MAX;
	ctx->counter[3] = nonce >> 32;
}

#define QUARTER_ROUND(x, a, b, c, d) ( \
	x[a] += x[b], \
	x[d] = rol32((x[d] ^ x[a]), 16), \
	x[c] += x[d], \
	x[b] = rol32((x[b] ^ x[c]), 12), \
	x[a] += x[b], \
	x[d] = rol32((x[d] ^ x[a]), 8), \
	x[c] += x[d], \
	x[b] = rol32((x[b] ^ x[c]), 7) \
)

#define C(i, j) (i * 4 + j)

#define DOUBLE_ROUND(x) ( \
	/* Column Round */ \
	QUARTER_ROUND(x, C(0, 0), C(1, 0), C(2, 0), C(3, 0)), \
	QUARTER_ROUND(x, C(0, 1), C(1, 1), C(2, 1), C(3, 1)), \
	QUARTER_ROUND(x, C(0, 2), C(1, 2), C(2, 2), C(3, 2)), \
	QUARTER_ROUND(x, C(0, 3), C(1, 3), C(2, 3), C(3, 3)), \
	/* Diagonal Round */ \
	QUARTER_ROUND(x, C(0, 0), C(1, 1), C(2, 2), C(3, 3)), \
	QUARTER_ROUND(x, C(0, 1), C(1, 2), C(2, 3), C(3, 0)), \
	QUARTER_ROUND(x, C(0, 2), C(1, 3), C(2, 0), C(3, 1)), \
	QUARTER_ROUND(x, C(0, 3), C(1, 0), C(2, 1), C(3, 2)) \
)

#define EIGHT_ROUNDS(x) ( \
	DOUBLE_ROUND(x), \
	DOUBLE_ROUND(x) \
)

/* hash a 8 bytes nonce into a 64 bytes hash */
void chacha8_hash(const u64 nonce, const u8 key[CHACHA20_KEY_SIZE], u8 *out)
{
	struct chacha20_ctx ctx;
	u32 x[CHACHA20_BLOCK_WORDS];
        __le32 *stream = (__le32 *) out;
        int i;

#ifdef __BIG_ENDIAN
	chacha20_init(&ctx, key, cpu_to_le64(nonce));
#else
 	chacha20_init(&ctx, key, nonce);
#endif

	for (i = 0; i < sizeof(x)/sizeof(x[0]); ++i)
		x[i] = ctx.state[i];

	EIGHT_ROUNDS(x);

	for (i = 0; i < CHACHA8_OUTPUT_WORDS; ++i)
		stream[i] = cpu_to_le32(x[i] + ctx.state[i]);
}
