/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Cryptographic API.
 *
 * MD4 Message Digest Algorithm (RFC1320).
 *
 * Implementation derived from Andrew Tridgell and Steve French's
 * CIFS MD4 implementation, and the cryptoapi implementation
 * originally based on the public domain implementation written
 * by Colin Plumb in 1993.
 *
 * Copyright (c) Andrew Tridgell 1997-1998.
 * Modified by Steve French (sfrench@us.ibm.com) 2002
 * Modified by Namjae Jeon (namjae.jeon@samsung.com) 2015
 * Copyright (c) Cryptoapi developers.
 * Copyright (c) 2002 David S. Miller (davem@redhat.com)
 * Copyright (c) 2002 James Morris <jmorris@intercode.com.au>
 */

#ifndef __MD4_HASH_H__
#define __MD4_HASH_H__

#define MD4_BLOCK_WORDS 16
#define MD4_HASH_WORDS  4

struct md4_ctx {
	unsigned int	hash[MD4_HASH_WORDS];
	unsigned int	block[MD4_BLOCK_WORDS];
	unsigned long	long byte_count;
};

void md4_init(struct md4_ctx *mctx);
void md4_update(struct md4_ctx *mctx, const unsigned char *data,
		unsigned int len);
void md4_final(struct md4_ctx *mctx, unsigned char *out);

#endif /* __MD4_HASH_H__ */
