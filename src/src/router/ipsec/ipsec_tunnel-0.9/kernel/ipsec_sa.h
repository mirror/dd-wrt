/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDE__ipsec_sa_h__200203291723
#define INCLUDE__ipsec_sa_h__200203291723

#include <net/ip.h>

#include <linux/list.h>

#define IPSEC_SPI_ANY                0

#define IPSEC_MAX_DIGEST_KEYLEN      20

struct ipsec_sa {
	struct list_head list;
	atomic_t users;

	u32 dst;
	u32 src;
	u32 spi;
	u32 flags;
	u32 tx_seq;

	struct cipher_context *cipher;
	struct digest_context *digest;
	u8 digest_key[IPSEC_MAX_DIGEST_KEYLEN];
	int digest_keylen; /* in bytes */
	int digest_hmaclen; /* in bytes */
};

int ipsec_sa_init(void);

void ipsec_sa_destroy(void);

int ipsec_sa_add(u32 dst,
				 u32 src,
				 u32 spi,
				 u32 flags,
				 const char *cipher,
				 const void *cipher_key,
				 int cipher_keylen,
				 const char *digest,
				 const void *digest_key,
				 int digest_keylen,
				 int digest_hmaclen);

struct ipsec_sa *ipsec_sa_get(u32 dst, u32 src, u32 spi);

struct ipsec_sa *ipsec_sa_get_num(int n);

void ipsec_sa_put(struct ipsec_sa *sa);

int ipsec_sa_del(u32 dst, u32 src, u32 spi);

#endif
