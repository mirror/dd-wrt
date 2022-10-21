/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2015-2019 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#ifndef _XT_CHACHA8_H
#define _XT_CHACHA8_H

#include <asm/unaligned.h>
#include <linux/kernel.h>
#include <linux/types.h>

enum chacha_lengths {
	CHACHA20_NONCE_SIZE = 16,
	CHACHA20_KEY_SIZE = 32,
	CHACHA20_KEY_WORDS = CHACHA20_KEY_SIZE / sizeof(u32),
	CHACHA20_BLOCK_SIZE = 64,
	CHACHA20_BLOCK_WORDS = CHACHA20_BLOCK_SIZE / sizeof(u32),
	CHACHA8_INPUT_SIZE = 8,
	CHACHA8_OUTPUT_SIZE = 32,
	CHACHA8_OUTPUT_WORDS = CHACHA8_OUTPUT_SIZE / sizeof(u32),
};


/* hash a 8 bytes nonce into a 64 bytes hash */
void chacha8_hash(const u64 nonce, const u8 key[CHACHA20_KEY_SIZE], u8 *out);
#endif /* _XT_CHACHA8_H */
